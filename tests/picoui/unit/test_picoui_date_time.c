#include "internal.h"
#include "ldDateTime.h"
#include "picoui/app.h"
#include "picoui/date_time.h"
#include "picoui/widget.h"
#include "picoui/window.h"
#include "../../../picoui/src/backend/ldgui/backend.h"

#include <assert.h>
#include <string.h>

static void test_date_time_create_and_props(struct picoui_window *win)
{
    int user_cookie = 13;
    struct picoui_date_time_props props = {
        .id = "date_time_props",
        .style_class = "date-time",
        .user_data = &user_cookie,
        .format = "yyyy-mm-dd hh:nn:ss",
        .year = 2026,
        .month = 5,
        .day = 31,
        .hour = 12,
        .minute = 34,
        .second = 56,
    };
    struct picoui_date_time *dt =
        picoui_date_time_create((struct picoui_widget *)win, "date_time");
    struct picoui_date_time *with_props =
        picoui_date_time_create_with_props((struct picoui_widget *)win, &props);

    assert(dt != 0);
    assert(with_props != 0);
    assert(picoui_date_time_get_format(dt) != 0);
    assert(strcmp(picoui_date_time_get_format(dt), "yyyy-mm-dd hh:nn:ss") == 0);
    assert(picoui_date_time_get_format(with_props) != 0);
    assert(strcmp(picoui_date_time_get_format(with_props), props.format) == 0);
}

static void test_date_time_setters(struct picoui_window *win)
{
    struct picoui_date_time *dt =
        picoui_date_time_create((struct picoui_widget *)win, "date_time_setters");

    assert(dt != 0);
    assert(picoui_date_time_set_format(dt, "hh:nn:ss") == 0);
    assert(picoui_date_time_get_format(dt) != 0);
    assert(strcmp(picoui_date_time_get_format(dt), "hh:nn:ss") == 0);
    assert(picoui_date_time_set_date(dt, 2026, 5, 31) == 0);
    assert(picoui_date_time_set_time(dt, 12, 34, 56) == 0);
}

static void test_date_time_manual_values_survive_frame_start(struct picoui_window *win)
{
    struct picoui_date_time *dt =
        picoui_date_time_create((struct picoui_widget *)win, "date_time_manual");
    struct picoui_backend_widget *backend;
    struct picoui_backend_app_state *app_state;
    ldDateTime_t *ld_date_time;

    assert(dt != 0);
    assert(picoui_date_time_set_format(dt, "yyyy-mm-dd hh:nn:ss") == 0);
    assert(picoui_date_time_set_date(dt, 2026, 5, 31) == 0);
    assert(picoui_date_time_set_time(dt, 12, 34, 56) == 0);

    backend = (struct picoui_backend_widget *)dt->widget.backend_widget;
    assert(backend != 0);
    app_state = (struct picoui_backend_app_state *)backend->owner->backend_app;
    assert(app_state != 0);
    ld_date_time = (ldDateTime_t *)backend->ld_widget;
    assert(ld_date_time != 0);

    ldDateTime_on_frame_start(app_state->ld_scene, ld_date_time);

    assert(ld_date_time->isAutoSysTime == false);
    assert(strcmp((const char *)ld_date_time->formatStrTemp, "2026-05-31 12:34:56") == 0);
}

static void test_date_time_rejects_invalid_inputs(struct picoui_window *win)
{
    struct picoui_date_time *dt =
        picoui_date_time_create((struct picoui_widget *)win, "date_time_invalid");

    assert(dt != 0);
    assert(picoui_date_time_create(0, "date_time") == 0);
    assert(picoui_date_time_create((struct picoui_widget *)win, 0) == 0);
    assert(picoui_date_time_create_with_props(0,
                                              &(struct picoui_date_time_props){
                                                  .id = "bad_parent",
                                                  .format = "yyyy-mm-dd",
                                              }) == 0);
    assert(picoui_date_time_create_with_props((struct picoui_widget *)win, 0) == 0);
    assert(picoui_date_time_create_with_props((struct picoui_widget *)win,
                                              &(struct picoui_date_time_props){
                                                  .format = "yyyy-mm-dd",
                                              }) == 0);
    assert(picoui_date_time_set_format(0, "yyyy-mm-dd") == -1);
    assert(picoui_date_time_set_date(0, 2026, 5, 31) == -1);
    assert(picoui_date_time_set_time(0, 12, 34, 56) == -1);
    assert(picoui_date_time_set_format(dt, 0) == -1);
    assert(picoui_date_time_set_date(dt, 2026, 0, 31) == -1);
    assert(picoui_date_time_set_date(dt, 2026, 13, 31) == -1);
    assert(picoui_date_time_set_date(dt, 2026, 5, 0) == -1);
    assert(picoui_date_time_set_date(dt, 2026, 5, 32) == -1);
    assert(picoui_date_time_set_time(dt, -1, 34, 56) == -1);
    assert(picoui_date_time_set_time(dt, 24, 34, 56) == -1);
    assert(picoui_date_time_set_time(dt, 12, -1, 56) == -1);
    assert(picoui_date_time_set_time(dt, 12, 60, 56) == -1);
    assert(picoui_date_time_set_time(dt, 12, 34, -1) == -1);
    assert(picoui_date_time_set_time(dt, 12, 34, 60) == -1);
}

static void test_date_time_final_release_contract_covers_public_readback_and_modes(
    struct picoui_window *win)
{
    struct picoui_date_time *dt =
        picoui_date_time_create((struct picoui_widget *)win, "date_time_release_ready");
    struct picoui_backend_widget *backend;
    ldDateTime_t *ld_date_time;
    int year = 0;
    int month = 0;
    int day = 0;
    int hour = 0;
    int minute = 0;
    int second = 0;

    assert(dt != 0);
    assert(picoui_date_time_set_format(dt, "yyyy/mm/dd hh:nn") == 0);
    assert(picoui_date_time_set_date(dt, 2026, 6, 1) == 0);
    assert(picoui_date_time_set_time(dt, 8, 9, 10) == 0);

    backend = (struct picoui_backend_widget *)dt->widget.backend_widget;
    assert(backend != 0);
    assert(backend->kind == PICOUI_BACKEND_WIDGET_DATE_TIME);
    ld_date_time = (ldDateTime_t *)backend->ld_widget;
    assert(ld_date_time != 0);

    assert(strcmp(picoui_date_time_get_format(dt), "yyyy/mm/dd hh:nn") == 0);
    assert(picoui_date_time_get_date(dt, &year, &month, &day) == 0);
    assert(picoui_date_time_get_time(dt, &hour, &minute, &second) == 0);
    assert(year == 2026);
    assert(month == 6);
    assert(day == 1);
    assert(hour == 8);
    assert(minute == 9);
    assert(second == 10);
    assert(ld_date_time->isAutoSysTime == false);
    assert(ld_date_time->year == 2026);
    assert(ld_date_time->month == 6);
    assert(ld_date_time->day == 1);
    assert(ld_date_time->hour == 8);
    assert(ld_date_time->minute == 9);
    assert(ld_date_time->second == 10);
    assert(strcmp((const char *)ld_date_time->formatStr, "yyyy/mm/dd hh:nn") == 0);
    assert(picoui_date_time_get_date(0, &year, &month, &day) == -1);
    assert(picoui_date_time_get_time(0, &hour, &minute, &second) == -1);
    assert(picoui_date_time_get_date(dt, 0, &month, &day) == -1);
    assert(picoui_date_time_get_time(dt, &hour, 0, &second) == -1);
}

static void test_date_time_native_transparent_color_and_align_round_trip(struct picoui_window *win)
{
    struct picoui_date_time *dt =
        picoui_date_time_create((struct picoui_widget *)win, "date_time_native_style");
    struct picoui_backend_widget *backend;
    ldDateTime_t *ld_date_time;

    assert(dt != 0);
    backend = (struct picoui_backend_widget *)dt->widget.backend_widget;
    assert(backend != 0);
    ld_date_time = (ldDateTime_t *)backend->ld_widget;
    assert(ld_date_time != 0);

    assert(picoui_date_time_set_text_color(dt, 0x112233U) == 0);
    assert(picoui_date_time_set_bg_color(dt, 0x445566U) == 0);
    assert(picoui_date_time_set_align(dt, PICOUI_ALIGN_END) == 0);
    assert(picoui_date_time_set_transparent(dt, 1) == 0);
    assert(picoui_date_time_get_transparent(dt) == 1);

    assert(ld_date_time->textColor == __RGB(0x11, 0x22, 0x33));
    assert(ld_date_time->bgColor == __RGB(0x44, 0x55, 0x66));
    assert(ld_date_time->tAlign == ARM_2D_ALIGN_RIGHT);
    assert(ld_date_time->isTransparent == true);

    assert(picoui_date_time_set_transparent(dt, 0) == 0);
    assert(picoui_date_time_get_transparent(dt) == 0);
    assert(ld_date_time->isTransparent == false);

    assert(picoui_date_time_set_text_color(0, 0x111111U) == -1);
    assert(picoui_date_time_set_bg_color(0, 0x222222U) == -1);
    assert(picoui_date_time_set_align(0, PICOUI_ALIGN_CENTER) == -1);
    assert(picoui_date_time_set_transparent(0, 1) == -1);
    assert(picoui_date_time_get_transparent(0) == -1);
}

static void test_date_time_init_and_shared_base_aliases_round_trip(struct picoui_window *win)
{
    struct picoui_date_time *dt =
        picoui_date_time_init((struct picoui_widget *)win, "date_time_alias");
    struct picoui_backend_widget *backend;
    ldDateTime_t *ld_date_time;

    assert(dt != 0);
    backend = (struct picoui_backend_widget *)dt->widget.backend_widget;
    assert(backend != 0);
    ld_date_time = (ldDateTime_t *)backend->ld_widget;
    assert(ld_date_time != 0);

    assert(picoui_date_time_set_background_color(dt, 0x556677U) == 0);
    assert(ld_date_time->bgColor == __RGB(0x55, 0x66, 0x77));

    assert(picoui_widget_set_pos(&dt->widget, 10, 14) == 0);
    assert(((ldBase_t *)ld_date_time)->tRegion.tLocation.iX == 10);
    assert(((ldBase_t *)ld_date_time)->tRegion.tLocation.iY == 14);

    assert(picoui_widget_set_visible(&dt->widget, 0) == 0);
    assert(((ldBase_t *)ld_date_time)->bIsVisible == false);
    assert(picoui_widget_set_opacity(&dt->widget, 73) == 0);
    assert(((ldBase_t *)ld_date_time)->chOpacity == 73);
    assert(picoui_widget_set_selectable(&dt->widget, 0) == 0);
    assert(((ldBase_t *)ld_date_time)->isSelectable == false);
    assert(picoui_widget_set_selected(&dt->widget, 1) == 0);
    assert(((ldBase_t *)ld_date_time)->isSelect == true);
    assert(picoui_widget_set_corner(&dt->widget, 5) == 0);
    assert(((ldBase_t *)ld_date_time)->chCorner == 5);
}

int main(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_window *win;

    assert(app != 0);
    win = picoui_window_create(app, "root");
    assert(win != 0);

    test_date_time_create_and_props(win);
    test_date_time_setters(win);
    test_date_time_manual_values_survive_frame_start(win);
    test_date_time_rejects_invalid_inputs(win);
    test_date_time_final_release_contract_covers_public_readback_and_modes(win);
    test_date_time_native_transparent_color_and_align_round_trip(win);
    test_date_time_init_and_shared_base_aliases_round_trip(win);

    picoui_app_destroy(app);
    return 0;
}
