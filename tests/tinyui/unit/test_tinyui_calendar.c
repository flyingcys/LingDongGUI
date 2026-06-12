#include "app.h"
#include "calendar.h"
#include "window.h"
#include "../../../src/gui/ldCalendar.h"
#include "../../../src/gui/ldBase.h"
#include "backend.h"
#include "internal.h"

#include <assert.h>
#include <dlfcn.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

static const char *test_self_binary_path = 0;
static const char *test_source_file_path = __FILE__;

static FILE *open_repo_file_from_test_source(const char *relative_path)
{
    char base_path[PATH_MAX];
    char *tests_dir;
    size_t base_len;

    assert(test_source_file_path != 0);
    assert(relative_path != 0);
    assert(strlen(test_source_file_path) < sizeof(base_path));
    snprintf(base_path, sizeof(base_path), "%s", test_source_file_path);
    tests_dir = strstr(base_path, "tests/tinyui/unit/");
    assert(tests_dir != 0);
    *tests_dir = '\0';
    base_len = strlen(base_path);
    assert(base_len + strlen(relative_path) + 1 < sizeof(base_path));
    snprintf(base_path + base_len, sizeof(base_path) - base_len, "%s", relative_path);
    return fopen(base_path, "rb");
}

static void assert_repo_file_lacks(const char *relative_path, const char *needle)
{
    FILE *file;
    char content[32768];
    size_t bytes_read;

    file = open_repo_file_from_test_source(relative_path);
    assert(file != 0);
    bytes_read = fread(content, 1, sizeof(content) - 1, file);
    assert(ferror(file) == 0);
    content[bytes_read] = '\0';
    assert(fclose(file) == 0);
    assert(strstr(content, needle) == 0);
}

static void assert_self_binary_lacks_symbol(const char *symbol)
{
    char command[PATH_MAX + 32];
    FILE *pipe;
    char line[512];

    assert(test_self_binary_path != 0);
    assert(symbol != 0);
    snprintf(command, sizeof(command), "nm %s 2>/dev/null", test_self_binary_path);
    pipe = popen(command, "r");
    assert(pipe != 0);
    while (fgets(line, sizeof(line), pipe) != 0) {
        size_t line_len = strlen(line);
        size_t symbol_len = strlen(symbol);

        while (line_len > 0 && (line[line_len - 1] == '\n' || line[line_len - 1] == '\r')) {
            line[--line_len] = '\0';
        }
        if (line_len >= symbol_len &&
            strcmp(line + line_len - symbol_len, symbol) == 0) {
            assert(!"unexpected symbol still present in test binary");
        }
    }
    assert(pclose(pipe) == 0);
}

static void test_calendar_internal_widget_local_seam_uses_tinyui_names(void)
{
    static const char *old_source_symbols[] = {
        "g_" "picoui_calendar_day_name_0",
        "g_" "picoui_calendar_day_name_1",
        "g_" "picoui_calendar_day_name_2",
        "g_" "picoui_calendar_day_name_3",
        "g_" "picoui_calendar_day_name_4",
        "g_" "picoui_calendar_day_name_5",
        "g_" "picoui_calendar_day_name_6",
        "g_" "picoui_calendar_day_names",
        "picoui_" "backend_calendar_rgb_to_ld",
        "picoui_" "backend_calendar_get_ld",
        "picoui_" "backend_calendar_sync_host_cache",
        "picoui_" "calendar_create_backend_local",
        "picoui_" "backend_calendar_set_day_names",
        "picoui_" "backend_calendar_set_date",
        "picoui_" "backend_calendar_get_date",
        "picoui_" "backend_calendar_set_header_visible",
        "picoui_" "backend_calendar_get_header_visible",
        "picoui_" "backend_calendar_set_header_format",
        "picoui_" "backend_calendar_set_bg_color",
        "picoui_" "backend_calendar_set_item_color",
        "picoui_" "backend_calendar_set_text_color",
        "picoui_" "backend_calendar_set_use_system_date",
        "picoui_" "backend_calendar_get_use_system_date",
        "picoui_" "backend_calendar_get_header_format",
        "picoui_" "backend_calendar_get_grid_value",
        "picoui_" "backend_calendar_is_current_month_cell",
        "picoui_" "calendar_props_are_valid",
        "picoui_" "calendar_sync_grid",
        "picoui_" "calendar_dispose_partial",
    };
    size_t i;

    assert_repo_file_lacks("tinyui/src/widgets/calendar.c", "g_" "picoui_calendar_day_names");
    assert_repo_file_lacks("tinyui/src/widgets/calendar.c", "picoui_" "calendar_create_backend_local");
    for (i = 0; i < sizeof(old_source_symbols) / sizeof(old_source_symbols[0]); ++i) {
        assert_repo_file_lacks("tinyui/src/widgets/calendar.c", old_source_symbols[i]);
        assert_self_binary_lacks_symbol(old_source_symbols[i]);
    }
}

static void test_calendar_date_readback_matches_backend_truth(void)
{
    struct picoui_app *app;
    struct picoui_window *win;
    struct picoui_calendar *calendar;
    struct picoui_backend_widget *backend;
    ldCalendar_t *ld_calendar;
    int year = 0;
    int month = 0;
    int day = 0;
    uint16_t backend_year = 0;
    uint8_t backend_month = 0;
    uint8_t backend_day = 0;

    app = picoui_app_create();
    assert(app != 0);
    win = picoui_window_create(app, "root");
    assert(win != 0);

    calendar = picoui_calendar_create(win, "calendar_readback");
    assert(calendar != 0);
    assert(picoui_calendar_set_date(calendar, 2026, 6, 15) == 0);
    assert(picoui_calendar_get_date(calendar, &year, &month, &day) == 0);
    assert(year == 2026);
    assert(month == 6);
    assert(day == 15);

    backend = (struct picoui_backend_widget *)calendar->widget.backend_widget;
    assert(backend != 0);
    ld_calendar = (ldCalendar_t *)backend->ld_widget;
    assert(ld_calendar != 0);
    ldCalendarGetDate(ld_calendar, &backend_year, &backend_month, &backend_day);
    assert(backend_year == 2026);
    assert(backend_month == 6);
    assert(backend_day == 15);

    picoui_app_destroy(app);
}

static void test_calendar_header_and_grid_visible_output_match_date_contract(void)
{
    struct picoui_app *app;
    struct picoui_window *win;
    struct picoui_calendar *calendar;
    struct picoui_backend_widget *backend;
    ldCalendar_t *ld_calendar;

    app = picoui_app_create();
    assert(app != 0);
    win = picoui_window_create(app, "root");
    assert(win != 0);

    calendar = picoui_calendar_create_with_props(
        win,
        &(struct picoui_calendar_props){
            .id = "calendar_contract",
            .year = 2026,
            .month = 6,
            .day = 15,
            .width = 280,
            .height = 180,
            .show_header = 1,
            .header_format = "yyyy/mm/dd",
        });
    assert(calendar != 0);

    assert(picoui_calendar_get_header_visible(calendar) == 1);
    assert(picoui_calendar_get_header_format(calendar) != 0);
    assert(strcmp(picoui_calendar_get_header_format(calendar), "yyyy/mm/dd") == 0);
    assert(picoui_calendar_get_grid_value(calendar, 2, 1) == 15);
    assert(picoui_calendar_is_current_month_cell(calendar, 2, 1) == 1);
    assert(picoui_calendar_get_grid_value(calendar, 0, 0) == 31);
    assert(picoui_calendar_is_current_month_cell(calendar, 0, 0) == 0);

    backend = (struct picoui_backend_widget *)calendar->widget.backend_widget;
    assert(backend != 0);
    ld_calendar = (ldCalendar_t *)backend->ld_widget;
    assert(ld_calendar != 0);
    assert(ld_calendar->isHeader == true);
    assert(strcmp((const char *)ld_calendar->headerNameFormat, "yyyy/mm/dd") == 0);
    assert(ld_calendar->calBuf[2 * 7 + 1] == (uint8_t)(15 | 0x80));
    assert(ld_calendar->calBuf[0] == 31);

    picoui_app_destroy(app);
}

static void test_calendar_final_release_contract_covers_full_feature_boundary(void)
{
    struct picoui_app *app;
    struct picoui_window *win;
    struct picoui_calendar *calendar;
    struct picoui_backend_widget *backend;
    ldCalendar_t *ld_calendar;

    app = picoui_app_create();
    assert(app != 0);
    win = picoui_window_create(app, "calendar_release_root");
    assert(win != 0);
    calendar = picoui_calendar_create_with_props(
        win,
        &(struct picoui_calendar_props){
            .id = "calendar_release_ready",
            .year = 2024,
            .month = 2,
            .day = 29,
            .width = 300,
            .height = 200,
            .show_header = 0,
            .header_format = "yyyy.mm.dd",
            .style_class = "calendar-card",
        });
    assert(calendar != 0);

    backend = (struct picoui_backend_widget *)calendar->widget.backend_widget;
    assert(backend != 0);
    assert(backend->kind == PICOUI_BACKEND_WIDGET_CALENDAR);
    assert(backend->style_class == (const char *)"calendar-card");
    ld_calendar = (ldCalendar_t *)backend->ld_widget;
    assert(ld_calendar != 0);

    assert(picoui_calendar_get_header_visible(calendar) == 0);
    assert(strcmp(picoui_calendar_get_header_format(calendar), "yyyy.mm.dd") == 0);
    assert(picoui_calendar_get_grid_value(calendar, 4, 4) == 29);
    assert(picoui_calendar_is_current_month_cell(calendar, 4, 4) == 1);
    assert(picoui_calendar_get_grid_value(calendar, 0, 0) >= 0);
    assert(picoui_calendar_is_current_month_cell(calendar, 0, 0) == 0);
    assert(picoui_calendar_set_header_visible(calendar, 1) == 0);
    assert(picoui_calendar_get_header_visible(calendar) == 1);
    assert(ld_calendar->isHeader == true);
    assert(picoui_calendar_set_date(calendar, 2024, 3, 1) == 0);
    assert(picoui_calendar_get_grid_value(calendar, 0, 5) == 1);
    assert(picoui_calendar_is_current_month_cell(calendar, 0, 5) == 1);

    picoui_app_destroy(app);
}

static void test_calendar_native_day_names_and_colors_round_trip(void)
{
    struct picoui_app *app;
    struct picoui_window *win;
    struct picoui_calendar *calendar;
    struct picoui_backend_widget *backend;
    ldCalendar_t *ld_calendar;
    static const char *day_names[7] = {
        "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat",
    };

    app = picoui_app_create();
    assert(app != 0);
    win = picoui_window_create(app, "calendar_native_root");
    assert(win != 0);
    calendar = picoui_calendar_create(win, "calendar_native_round_trip");
    assert(calendar != 0);

    assert(picoui_calendar_set_day_names(calendar, day_names) == 0);
    assert(picoui_calendar_set_bg_color(calendar, 0x112233U) == 0);
    assert(picoui_calendar_set_item_color(calendar, 0x445566U) == 0);
    assert(picoui_calendar_set_text_color(calendar, 0x778899U) == 0);

    backend = (struct picoui_backend_widget *)calendar->widget.backend_widget;
    assert(backend != 0);
    ld_calendar = (ldCalendar_t *)backend->ld_widget;
    assert(ld_calendar != 0);

    assert(strcmp((const char *)ld_calendar->dayNames[0], "Sun") == 0);
    assert(strcmp((const char *)ld_calendar->dayNames[6], "Sat") == 0);
    assert(ld_calendar->bgColor == (ldColor)0x112233U);
    assert(ld_calendar->itemColor == (ldColor)0x445566U);
    assert(ld_calendar->textColor == (ldColor)0x778899U);

    assert(picoui_calendar_set_day_names(calendar, 0) == -1);
    assert(picoui_calendar_set_bg_color(0, 0xAABBCCU) == -1);
    assert(picoui_calendar_set_item_color(calendar, 0x1000000U) == -1);
    assert(picoui_calendar_set_text_color(calendar, 0x1000000U) == -1);

    assert(strcmp((const char *)ld_calendar->dayNames[0], "Sun") == 0);
    assert(ld_calendar->bgColor == (ldColor)0x112233U);
    assert(ld_calendar->itemColor == (ldColor)0x445566U);
    assert(ld_calendar->textColor == (ldColor)0x778899U);

    picoui_app_destroy(app);
}

static void test_calendar_system_date_provider_round_trip(void)
{
    struct picoui_app *app;
    struct picoui_window *win;
    struct picoui_calendar *calendar;
    struct picoui_backend_widget *backend;
    ldCalendar_t *ld_calendar;
    int year = 0;
    int month = 0;
    int day = 0;

    app = picoui_app_create();
    assert(app != 0);
    win = picoui_window_create(app, "calendar_system_root");
    assert(win != 0);
    calendar = picoui_calendar_create(win, "calendar_system_date");
    assert(calendar != 0);

    backend = (struct picoui_backend_widget *)calendar->widget.backend_widget;
    assert(backend != 0);
    ld_calendar = (ldCalendar_t *)backend->ld_widget;
    assert(ld_calendar != 0);

    assert(picoui_calendar_set_date(calendar, 2024, 2, 29) == 0);
    assert(picoui_calendar_set_use_system_date(calendar, 1) == 0);
    assert(picoui_calendar_get_use_system_date(calendar) == 1);
    ldCalendar_on_frame_start(backend->owner->backend_app ? ((struct picoui_backend_app_state *)backend->owner->backend_app)->ld_scene : NULL,
                              ld_calendar);
    assert(picoui_calendar_get_date(calendar, &year, &month, &day) == 0);
    assert(year >= 1970);
    assert(month >= 1 && month <= 12);
    assert(day >= 1 && day <= 31);
    assert(picoui_calendar_get_grid_value(calendar, 0, ldBaseGetWeek((uint16_t)year, (uint8_t)month, 1)) == 1);

    assert(picoui_calendar_set_use_system_date(calendar, 0) == 0);
    assert(picoui_calendar_get_use_system_date(calendar) == 0);
    assert(picoui_calendar_set_date(calendar, 2024, 2, 29) == 0);
    assert(picoui_calendar_get_date(calendar, &year, &month, &day) == 0);
    assert(year == 2024 && month == 2 && day == 29);

    assert(picoui_calendar_set_use_system_date(0, 1) == -1);
    assert(picoui_calendar_get_use_system_date(0) == -1);

    picoui_app_destroy(app);
}

static void test_calendar_init_and_aliases_match_backend_truth(void)
{
    struct picoui_app *app;
    struct picoui_window *win;
    struct picoui_calendar *calendar;
    struct picoui_backend_widget *backend;
    ldBase_t *ld_base;
    int year = 0;
    int month = 0;
    int day = 0;

    app = picoui_app_create();
    assert(app != 0);
    win = picoui_window_create(app, "calendar_alias_root");
    assert(win != 0);
    calendar = picoui_calendar_create(win, "calendar_alias");
    assert(calendar != 0);
    assert(picoui_calendar_set_date(calendar, 2027, 1, 2) == 0);
    assert(picoui_calendar_get_date(calendar, &year, &month, &day) == 0);
    assert(year == 2027);
    assert(month == 1);
    assert(day == 2);
    assert(picoui_calendar_set_header_format(calendar, "yy/mm/dd") == 0);
    assert(strcmp(picoui_calendar_get_header_format(calendar), "yy/mm/dd") == 0);

    backend = (struct picoui_backend_widget *)calendar->widget.backend_widget;
    assert(backend != 0);
    ld_base = (ldBase_t *)backend->ld_widget;
    assert(ld_base != 0);
    picoui_app_destroy(app);
}

static void test_calendar_grid_value_round_trip(struct picoui_window *win)
{
    struct picoui_calendar *cal = picoui_calendar_create(win, "cal_grid");
    int grid_val;

    assert(cal != 0);
    assert(picoui_calendar_set_date(cal, 2026, 6, 3) == 0);
    grid_val = picoui_calendar_get_grid_value(cal, 0, 0);
    assert(grid_val >= 0);

    int is_current = picoui_calendar_is_current_month_cell(cal, 0, 0);
    assert(is_current == 0 || is_current == 1);
}

static void test_calendar_grid_value_boundary_args(struct picoui_window *win)
{
    struct picoui_calendar *cal = picoui_calendar_create(win, "cal_grid_boundary");
    assert(cal != 0);
    assert(picoui_calendar_set_date(cal, 2026, 1, 15) == 0);
    for (int week = 0; week < 6; week++) {
        for (int wday = 0; wday < 7; wday++) {
            int v = picoui_calendar_get_grid_value(cal, week, wday);
            assert(v >= 0 && v <= 31);
            int cur = picoui_calendar_is_current_month_cell(cal, week, wday);
            assert(cur == 0 || cur == 1);
        }
    }
}

static void test_calendar_grid_out_of_bounds(struct picoui_window *win)
{
    struct picoui_calendar *cal = picoui_calendar_create(win, "cal_oob");
    assert(cal != 0);
    assert(picoui_calendar_set_date(cal, 2026, 1, 1) == 0);
    // out-of-bounds access should not crash
    picoui_calendar_get_grid_value(cal, 10, 10);
    picoui_calendar_is_current_month_cell(cal, 10, 10);
}

static void test_calendar_public_create_uses_widget_local_backend(struct picoui_window *win)
{
    struct picoui_calendar *calendar;
    struct picoui_backend_widget *backend;

    assert(win != 0);
    calendar = picoui_calendar_create(win, "calendar_widget_local");
    assert(calendar != 0);

    backend = (struct picoui_backend_widget *)calendar->widget.backend_widget;
    assert(backend != 0);
    assert(backend->kind == PICOUI_BACKEND_WIDGET_CALENDAR);
    assert(backend->host_widget == &calendar->widget);
    assert(backend->parent == win->widget.backend_widget);
    assert(backend->ld_widget != 0);
}

int main(void)
{
    Dl_info self_info;

    assert(dladdr((void *)&main, &self_info) != 0);
    test_self_binary_path = self_info.dli_fname;
    test_calendar_internal_widget_local_seam_uses_tinyui_names();
    test_calendar_date_readback_matches_backend_truth();
    test_calendar_header_and_grid_visible_output_match_date_contract();
    test_calendar_final_release_contract_covers_full_feature_boundary();
    test_calendar_native_day_names_and_colors_round_trip();
    test_calendar_system_date_provider_round_trip();
    test_calendar_init_and_aliases_match_backend_truth();

    struct picoui_app *app = picoui_app_create();
    struct picoui_window *win = picoui_window_create(app, "root");
    test_calendar_public_create_uses_widget_local_backend(win);
    test_calendar_grid_value_round_trip(win);
    test_calendar_grid_value_boundary_args(win);
    test_calendar_grid_out_of_bounds(win);
    picoui_app_destroy(app);
    return 0;
}
