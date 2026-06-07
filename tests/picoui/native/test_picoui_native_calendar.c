#include "picoui/picoui.h"
#include "internal.h"

#include <assert.h>

int picoui_native_event_bind_root(struct picoui_screen *screen, struct picoui_window *root_window);
int picoui_native_calendar_get_rendered_date(const struct picoui_calendar *calendar,
                                             int *year,
                                             int *month,
                                             int *day);
int picoui_native_calendar_get_rendered_selected_date(const struct picoui_calendar *calendar,
                                                      int *year,
                                                      int *month,
                                                      int *day);

static int g_callback_count = 0;
static int g_selected_year = 0;
static int g_selected_month = 0;
static int g_selected_day = 0;
static void *g_callback_user_data = 0;
static struct picoui_calendar *g_selected_calendar = 0;

static void on_calendar_selected(struct picoui_calendar *calendar,
                                 int year,
                                 int month,
                                 int day,
                                 void *user_data)
{
    g_callback_count++;
    g_selected_calendar = calendar;
    g_selected_year = year;
    g_selected_month = month;
    g_selected_day = day;
    g_callback_user_data = user_data;
}

static void calendar_day_center_from_ld_show(const struct picoui_calendar *calendar,
                                             struct picoui_point origin,
                                             int target_day,
                                             int *x,
                                             int *y)
{
    const int space = 2;
    int width;
    int height;
    int row_height;
    int header_height;
    int cell_width;
    int week = -1;
    int weekday = -1;

    assert(calendar != 0);
    assert(x != 0);
    assert(y != 0);

    for (int row = 0; row < 6; ++row) {
        for (int col = 0; col < 7; ++col) {
            if (picoui_calendar_is_current_month_cell(calendar, row, col) == 1
                && picoui_calendar_get_grid_value(calendar, row, col) == target_day) {
                week = row;
                weekday = col;
                break;
            }
        }
        if (week >= 0) {
            break;
        }
    }

    assert(week >= 0);
    assert(weekday >= 0);

    width = picoui_widget_get_width((const struct picoui_widget *)calendar);
    height = picoui_widget_get_height((const struct picoui_widget *)calendar);
    if (picoui_calendar_get_header_visible(calendar)) {
        row_height = (height - (space * 9)) / 8;
        header_height = row_height + space;
    } else {
        row_height = (height - (space * 8)) / 7;
        header_height = 0;
    }
    cell_width = (width - (space * 8)) / 7;

    *x = origin.x + space + weekday * (cell_width + space) + (cell_width / 2);
    *y = origin.y + header_height + space + row_height
        + week * (row_height + space) + (row_height / 2);
}

int main(void)
{
    struct picoui_display *display;
    struct picoui_screen *screen;
    struct picoui_window *window;
    struct picoui_calendar *calendar;
    struct picoui_backend_widget *backend;
    struct picoui_app *app;
    struct picoui_point origin;
    int press_x = 0;
    int press_y = 0;
    int rendered_year = 0;
    int rendered_month = 0;
    int rendered_day = 0;
    int selected_year = 0;
    int selected_month = 0;
    int selected_day = 0;
    int callback_cookie = 73;
    int rc;

    g_callback_count = 0;
    g_selected_year = 0;
    g_selected_month = 0;
    g_selected_day = 0;
    g_callback_user_data = 0;
    g_selected_calendar = 0;

    assert(picoui_init() == 0);

    display = picoui_display_create(320, 480);
    assert(display != 0);
    assert(picoui_display_set_default(display) == 0);

    screen = picoui_screen_active();
    assert(screen != 0);

    window = picoui_window_create_root(screen, "root");
    assert(window != 0);

    calendar = picoui_calendar_create((struct picoui_window *)window, "work_calendar");
    assert(calendar != 0);
    assert(picoui_widget_set_pos((struct picoui_widget *)calendar, 20, 40) == 0);
    assert(picoui_widget_set_size((struct picoui_widget *)calendar, 280, 180) == 0);

    assert(picoui_calendar_set_date(calendar, 2026, 6, 15) == 0);
    assert(picoui_calendar_get_date(calendar, &rendered_year, &rendered_month, &rendered_day) == 0);
    assert(rendered_year == 2026);
    assert(rendered_month == 6);
    assert(rendered_day == 15);

    assert(picoui_calendar_set_selected_date(calendar, 2026, 7, 4) == 0);
    assert(picoui_calendar_get_date(calendar, &rendered_year, &rendered_month, &rendered_day) == 0);
    assert(rendered_year == 2026);
    assert(rendered_month == 7);
    assert(rendered_day == 4);
    assert(picoui_calendar_get_selected_date(calendar,
                                             &selected_year,
                                             &selected_month,
                                             &selected_day) == 0);
    assert(selected_year == 2026);
    assert(selected_month == 7);
    assert(selected_day == 4);

    assert(picoui_calendar_set_date(calendar, 2026, 6, 15) == 0);
    assert(picoui_calendar_get_selected_date(calendar,
                                             &selected_year,
                                             &selected_month,
                                             &selected_day) == 0);
    assert(selected_year == 2026);
    assert(selected_month == 6);
    assert(selected_day == 15);

    assert(picoui_calendar_set_selected_date(calendar, 2026, 6, 18) == 0);
    assert(picoui_calendar_get_date(calendar, &rendered_year, &rendered_month, &rendered_day) == 0);
    assert(rendered_year == 2026);
    assert(rendered_month == 6);
    assert(rendered_day == 18);
    assert(picoui_calendar_get_selected_date(calendar,
                                             &selected_year,
                                             &selected_month,
                                             &selected_day) == 0);
    assert(selected_year == 2026);
    assert(selected_month == 6);
    assert(selected_day == 18);
    assert(picoui_calendar_get_grid_value(calendar, 2, 4) == 18);
    assert(picoui_calendar_is_current_month_cell(calendar, 2, 4) == 1);

    picoui_calendar_set_on_selected(calendar, on_calendar_selected, &callback_cookie);

    backend = (struct picoui_backend_widget *)calendar->widget.backend_widget;
    assert(backend != 0);
    assert(backend->kind == PICOUI_BACKEND_WIDGET_CALENDAR);

    assert(picoui_native_event_bind_root(screen, window) == 0);
    assert(picoui_native_calendar_get_rendered_date(calendar,
                                                    &rendered_year,
                                                    &rendered_month,
                                                    &rendered_day) == -1);
    assert(picoui_native_calendar_get_rendered_selected_date(calendar,
                                                             &selected_year,
                                                             &selected_month,
                                                             &selected_day) == -1);

    rc = picoui_timer_handler();
    assert(rc == 0 || rc == 1);

    assert(picoui_native_calendar_get_rendered_date(calendar,
                                                    &rendered_year,
                                                    &rendered_month,
                                                    &rendered_day) == 0);
    assert(rendered_year == 2026);
    assert(rendered_month == 6);
    assert(rendered_day == 18);
    assert(picoui_native_calendar_get_rendered_selected_date(calendar,
                                                             &selected_year,
                                                             &selected_month,
                                                             &selected_day) == 0);
    assert(selected_year == 2026);
    assert(selected_month == 6);
    assert(selected_day == 18);

    app = backend->owner;
    assert(app != 0);
    origin = picoui_widget_get_absolute_pos((const struct picoui_widget *)calendar,
                                            (struct picoui_point){0, 0});
    assert(origin.x >= 0);
    assert(origin.y >= 0);

    calendar_day_center_from_ld_show(calendar, origin, 19, &press_x, &press_y);
    assert(press_x > origin.x);
    assert(press_y > origin.y);

    assert(picoui_input_push_pointer(app, press_x, press_y, 1) == 0);
    rc = picoui_timer_handler();
    assert(rc == 0 || rc == 1);
    assert(g_callback_count == 0);

    assert(picoui_input_push_pointer(app, press_x, press_y, 0) == 0);
    rc = picoui_timer_handler();
    assert(rc == 0 || rc == 1);

    assert(g_callback_count == 1);
    assert(g_selected_calendar == calendar);
    assert(g_callback_user_data == &callback_cookie);
    assert(g_selected_year == 2026);
    assert(g_selected_month == 6);
    assert(g_selected_day == 19);
    assert(picoui_calendar_get_selected_date(calendar,
                                             &selected_year,
                                             &selected_month,
                                             &selected_day) == 0);
    assert(selected_year == 2026);
    assert(selected_month == 6);
    assert(selected_day == 19);
    assert(backend->last_signal == PICOUI_BACKEND_SIGNAL_VALUE_CHANGED);

    picoui_deinit();
    return 0;
}
