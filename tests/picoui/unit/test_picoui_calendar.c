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

int main(void)
{
    test_calendar_date_readback_matches_backend_truth();
    test_calendar_header_and_grid_visible_output_match_date_contract();
    return 0;
}
