#include "picoui/app.h"
#include "picoui/calendar.h"
#include "picoui/window.h"
#include "../../../src/gui/ldCalendar.h"
#include "backend.h"
#include "internal.h"

#include <assert.h>
#include <string.h>

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

int main(void)
{
    test_calendar_date_readback_matches_backend_truth();
    test_calendar_header_and_grid_visible_output_match_date_contract();
    test_calendar_final_release_contract_covers_full_feature_boundary();
    test_calendar_native_day_names_and_colors_round_trip();
    test_calendar_init_and_aliases_match_backend_truth();
    return 0;
}
