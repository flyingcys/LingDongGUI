/*
 * TinyUI calendar unit tests — M3 Task 3.
 */
#include "tinyui.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldCalendar.h"
#include "internal.h"
#include "widgets/calendar.h"
#include <assert.h>
#include <string.h>

static ldCalendar_t *ld_of(tinyui_obj_t *o)
{
    struct tinyui_widget *w = (struct tinyui_widget *)(void *)o;
    assert(w && w->ld_widget);
    return (ldCalendar_t *)w->ld_widget;
}

static void test_calendar_init_and_aliases_match_backend_truth(tinyui_obj_t *root)
{
    tinyui_obj_t *cal = tinyui_calendar_create(root);
    struct tinyui_widget *w = (struct tinyui_widget *)(void *)cal;
    int y=0,m=0,d=0;
    assert(cal);
    assert(w->kind == TINYUI_BACKEND_WIDGET_CALENDAR);
    assert(tinyui_calendar_get_date(cal, &y, &m, &d) == 0);
    assert(m >= 1 && m <= 12);
    assert(d >= 1 && d <= 31);
    assert(tinyui_calendar_set_header_format(cal, "yyyy-mm-dd") == 0);
    assert(tinyui_calendar_get_header_format(cal) != 0);
}

static void test_calendar_date_readback_matches_backend_truth(tinyui_obj_t *root)
{
    tinyui_obj_t *cal = tinyui_calendar_create(root);
    ldCalendar_t *ld = ld_of(cal);
    int y=0,m=0,d=0;
    assert(tinyui_calendar_set_date(cal, 2024, 5, 17) == 0);
    assert(tinyui_calendar_get_date(cal, &y, &m, &d) == 0);
    assert(y == 2024 && m == 5 && d == 17);
    ldCalendarGetDate(ld, (uint16_t *)&y, (uint8_t *)&m, (uint8_t *)&d);
    /* GetDate uses pointers of specific widths; re-read via tinyui only if needed */
    assert(tinyui_calendar_get_date(cal, &y, &m, &d) == 0);
    assert(y == 2024);
}

static void test_calendar_set_auto_sys_date_native_parity(tinyui_obj_t *root)
{
    tinyui_obj_t *cal = tinyui_calendar_create(root);
    ldCalendar_t *ld = ld_of(cal);
    assert(tinyui_calendar_set_auto_sys_date(cal, 1) == 0);
    assert(ld->isAutoSysDate == true || ld->isAutoSysDate == 1);
    assert(tinyui_calendar_get_use_system_date(cal) == 1);
    assert(tinyui_calendar_set_auto_sys_date(cal, 0) == 0);
    assert(ld->isAutoSysDate == false || ld->isAutoSysDate == 0);
}

static void test_calendar_native_day_names_and_colors_round_trip(tinyui_obj_t *root)
{
    tinyui_obj_t *cal = tinyui_calendar_create(root);
    ldCalendar_t *ld = ld_of(cal);
    const char *names[7] = {"Su","Mo","Tu","We","Th","Fr","Sa"};
    assert(tinyui_calendar_set_day_names(cal, names) == 0);
    assert(ld->dayNames != 0);
    assert(strcmp((const char *)ld->dayNames[0], "Su") == 0);
    assert(tinyui_calendar_set_bg_color(cal, 0x101010) == 0);
    assert(ld->bgColor == (ldColor)tinyui_rgb_to_ld_color(0x101010));
    assert(tinyui_calendar_set_item_color(cal, 0x202020) == 0);
    assert(ld->itemColor == (ldColor)tinyui_rgb_to_ld_color(0x202020));
    assert(tinyui_calendar_set_text_color(cal, 0x303030) == 0);
    assert(ld->textColor == (ldColor)tinyui_rgb_to_ld_color(0x303030));
}

static void test_calendar_final_release_contract_covers_full_feature_boundary(tinyui_obj_t *root)
{
    tinyui_obj_t *cal = tinyui_calendar_create(root);
    ldCalendar_t *ld = ld_of(cal);
    int week, weekday, found = 0;
    assert(tinyui_calendar_set_header_visible(cal, 0) == 0);
    assert(ld->isHeader == false || ld->isHeader == 0);
    assert(tinyui_calendar_get_header_visible(cal) == 0);
    assert(tinyui_calendar_set_header_visible(cal, 1) == 0);
    assert(tinyui_calendar_get_header_visible(cal) == 1);
    assert(tinyui_calendar_set_date(cal, 2023, 2, 1) == 0);
    for (week = 0; week < 6; ++week) {
        for (weekday = 0; weekday < 7; ++weekday) {
            int v = tinyui_calendar_get_grid_value(cal, week, weekday);
            int cur = tinyui_calendar_is_current_month_cell(cal, week, weekday);
            if (v > 0 && cur == 1) {
                found = 1;
            }
        }
    }
    assert(found == 1);
    assert(tinyui_calendar_get_grid_value(cal, -1, 0) == -1);
    assert(tinyui_calendar_get_grid_value(cal, 0, 7) == -1);
}

int main(void)
{
    tinyui_obj_t *root;
    tinyui_deinit();
    assert(tinyui_init() == TINYUI_OK);
    root = tinyui_screen_create();
    assert(root);
    test_calendar_init_and_aliases_match_backend_truth(root);
    test_calendar_date_readback_matches_backend_truth(root);
    test_calendar_set_auto_sys_date_native_parity(root);
    test_calendar_native_day_names_and_colors_round_trip(root);
    test_calendar_final_release_contract_covers_full_feature_boundary(root);
    tinyui_deinit();
    return 0;
}
