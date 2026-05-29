#include "picoui/picoui.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/misc/ldMsg.h"
#include "internal.h"

#include <assert.h>

static int press_count = 0;
static int release_count = 0;
static int click_count = 0;
static int value_count = 0;
static int event_order[4];
static int event_order_count = 0;
static struct picoui_widget *last_press_widget = 0;
static struct picoui_widget *last_release_widget = 0;
static struct picoui_widget *last_click_widget = 0;
static struct picoui_widget *last_value_widget = 0;
static int last_press_cookie = 0;
static int last_release_cookie = 0;
static int last_click_cookie = 0;
static int last_value_cookie = 0;
static int last_value = 0;

static void on_pressed(struct picoui_widget *widget, void *user_data)
{
    press_count++;
    event_order[event_order_count++] = 1;
    last_press_widget = widget;
    last_press_cookie = user_data != 0 ? *(const int *)user_data : -1;
}

static void on_released(struct picoui_widget *widget, void *user_data)
{
    release_count++;
    event_order[event_order_count++] = 2;
    last_release_widget = widget;
    last_release_cookie = user_data != 0 ? *(const int *)user_data : -1;
}

static void on_clicked(struct picoui_widget *widget, void *user_data)
{
    click_count++;
    event_order[event_order_count++] = 3;
    last_click_widget = widget;
    last_click_cookie = user_data != 0 ? *(const int *)user_data : -1;
}

static void on_value_changed(struct picoui_widget *widget, int value, void *user_data)
{
    value_count++;
    last_value_widget = widget;
    last_value = value;
    last_value_cookie = user_data != 0 ? *(const int *)user_data : -1;
}

int main(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_window *win = picoui_window_create(app, "root");
    struct picoui_button *button = picoui_button_create(win, "submit");
    struct picoui_checkbox *checkbox = picoui_checkbox_create(win, "accept");
    struct picoui_switch *sw = picoui_switch_create(win, "power");
    struct picoui_slider *slider = picoui_slider_create(win, "level");
    struct picoui_backend_widget *backend;
    struct picoui_backend_widget *checkbox_backend;
    struct picoui_backend_widget *switch_backend;
    struct picoui_backend_widget *slider_backend;
    struct picoui_backend_app_state *app_state;
    int click_cookie = 33;
    int press_cookie = 11;
    int release_cookie = 22;
    int checkbox_cookie = 44;
    int switch_cookie = 55;
    int slider_cookie = 66;

    assert(app != 0);
    assert(win != 0);
    assert(button != 0);
    assert(checkbox != 0);
    assert(sw != 0);
    assert(slider != 0);
    backend = button->widget.backend_widget;
    checkbox_backend = checkbox->widget.backend_widget;
    switch_backend = sw->widget.backend_widget;
    slider_backend = slider->widget.backend_widget;
    assert(backend != 0);
    assert(checkbox_backend != 0);
    assert(switch_backend != 0);
    assert(slider_backend != 0);
    app_state = (struct picoui_backend_app_state *)app->backend_app;
    assert(app_state != 0);
    assert(app_state->ld_scene != 0);
    assert(ldMsgInit(&app_state->ld_scene->ptMsgQueue, 8) == true);

    assert(picoui_button_set_on_pressed(0, on_pressed, &press_cookie) == -1);
    assert(picoui_button_set_on_released(0, on_released, &release_cookie) == -1);

    assert(picoui_button_set_on_pressed(button, on_pressed, &press_cookie) == 0);
    assert(picoui_button_set_on_released(button, on_released, &release_cookie) == 0);
    assert(picoui_button_set_on_clicked(button, on_clicked, &click_cookie) == 0);
    assert(picoui_checkbox_set_on_toggled(checkbox, on_value_changed, &checkbox_cookie) == 0);
    assert(picoui_switch_set_on_toggled(sw, on_value_changed, &switch_cookie) == 0);
    assert(picoui_slider_set_on_value_changed(slider, on_value_changed, &slider_cookie) == 0);
    assert(button->on_pressed == on_pressed);
    assert(button->on_pressed_user_data == &press_cookie);
    assert(button->on_released == on_released);
    assert(button->on_released_user_data == &release_cookie);
    assert(button->on_clicked == on_clicked);
    assert(button->user_data == &click_cookie);

    assert(press_count == 0);
    assert(release_count == 0);
    assert(click_count == 0);
    assert(event_order_count == 0);
    assert(backend->last_signal == PICOUI_BACKEND_SIGNAL_NONE);
    assert(backend->dispatch_count == 0);

    assert(picoui_backend_widget_dispatch_event(button->widget.backend_widget,
                                                PICOUI_BACKEND_SIGNAL_PRESSED,
                                                button->on_pressed,
                                                &button->widget,
                                                button->on_pressed_user_data) == 0);
    assert(press_count == 1);
    assert(release_count == 0);
    assert(click_count == 0);
    assert(event_order_count == 1);
    assert(event_order[0] == 1);
    assert(last_press_widget == &button->widget);
    assert(last_press_cookie == press_cookie);
    assert(backend->last_signal == PICOUI_BACKEND_SIGNAL_PRESSED);
    assert(backend->dispatch_count == 1);

    assert(picoui_backend_widget_dispatch_event(button->widget.backend_widget,
                                                PICOUI_BACKEND_SIGNAL_RELEASED,
                                                button->on_released,
                                                &button->widget,
                                                button->on_released_user_data) == 0);
    assert(press_count == 1);
    assert(release_count == 1);
    assert(click_count == 0);
    assert(event_order_count == 2);
    assert(event_order[1] == 2);
    assert(last_release_widget == &button->widget);
    assert(last_release_cookie == release_cookie);
    assert(backend->last_signal == PICOUI_BACKEND_SIGNAL_RELEASED);
    assert(backend->dispatch_count == 2);

    assert(picoui_backend_widget_dispatch_event(0,
                                                PICOUI_BACKEND_SIGNAL_PRESSED,
                                                on_pressed,
                                                &button->widget,
                                                &press_cookie) == -1);
    assert(picoui_backend_widget_dispatch_event(button->widget.backend_widget,
                                                PICOUI_BACKEND_SIGNAL_VALUE_CHANGED,
                                                button->on_pressed,
                                                &button->widget,
                                                button->on_pressed_user_data) == -1);

    press_count = 0;
    release_count = 0;
    click_count = 0;
    event_order_count = 0;
    backend->last_signal = PICOUI_BACKEND_SIGNAL_NONE;
    backend->dispatch_count = 0;

    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue,
                     backend->ld_widget,
                     SIGNAL_PRESS,
                     0) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(press_count == 1);
    assert(release_count == 0);
    assert(click_count == 0);
    assert(event_order_count == 1);
    assert(event_order[0] == 1);
    assert(last_press_widget == &button->widget);
    assert(last_press_cookie == press_cookie);
    assert(backend->last_signal == PICOUI_BACKEND_SIGNAL_PRESSED);
    assert(backend->dispatch_count == 1);

    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue,
                     backend->ld_widget,
                     SIGNAL_RELEASE,
                     0) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(press_count == 1);
    assert(release_count == 1);
    assert(click_count == 1);
    assert(event_order_count == 3);
    assert(event_order[1] == 2);
    assert(event_order[2] == 3);
    assert(last_release_widget == &button->widget);
    assert(last_release_cookie == release_cookie);
    assert(last_click_widget == &button->widget);
    assert(last_click_cookie == click_cookie);
    assert(backend->last_signal == PICOUI_BACKEND_SIGNAL_RELEASED);
    assert(backend->dispatch_count == 2);

    assert(picoui_widget_set_enabled(&button->widget, 0) == 0);
    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue,
                     backend->ld_widget,
                     SIGNAL_PRESS,
                     0) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(press_count == 1);
    assert(release_count == 1);
    assert(click_count == 1);
    assert(event_order_count == 3);
    assert(backend->dispatch_count == 2);
    assert(picoui_widget_set_enabled(&button->widget, 1) == 0);

    assert(picoui_widget_set_visible(&button->widget, 0) == 0);
    assert(picoui_backend_widget_dispatch_event(button->widget.backend_widget,
                                                PICOUI_BACKEND_SIGNAL_PRESSED,
                                                button->on_pressed,
                                                &button->widget,
                                                button->on_pressed_user_data) == 0);
    assert(picoui_backend_widget_dispatch_event(button->widget.backend_widget,
                                                PICOUI_BACKEND_SIGNAL_RELEASED,
                                                button->on_released,
                                                &button->widget,
                                                button->on_released_user_data) == 0);
    assert(press_count == 1);
    assert(release_count == 1);
    assert(click_count == 1);
    assert(event_order_count == 3);
    assert(backend->dispatch_count == 2);

    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue,
                     backend->ld_widget,
                     SIGNAL_PRESS,
                     0) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue,
                     backend->ld_widget,
                     SIGNAL_RELEASE,
                     0) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(press_count == 1);
    assert(release_count == 1);
    assert(click_count == 1);
    assert(event_order_count == 3);
    assert(backend->dispatch_count == 2);
    assert(picoui_widget_set_visible(&button->widget, 1) == 0);

    assert(picoui_checkbox_set_checked(checkbox, 1) == 0);
    assert(value_count == 0);
    assert(picoui_switch_set_checked(sw, 1) == 0);
    assert(value_count == 0);
    assert(picoui_slider_set_value(slider, 40) == 0);
    assert(value_count == 0);

    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue,
                     checkbox_backend->ld_widget,
                     SIGNAL_VALUE_CHANGED,
                     0) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(value_count == 1);
    assert(last_value_widget == &checkbox->widget);
    assert(last_value == 0);
    assert(last_value_cookie == checkbox_cookie);

    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue,
                     switch_backend->ld_widget,
                     SIGNAL_VALUE_CHANGED,
                     0) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(value_count == 2);
    assert(last_value_widget == &sw->widget);
    assert(last_value == 0);
    assert(last_value_cookie == switch_cookie);

    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue,
                     slider_backend->ld_widget,
                     SIGNAL_VALUE_CHANGED,
                     750) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(value_count == 3);
    assert(last_value_widget == &slider->widget);
    assert(last_value == 75);
    assert(last_value_cookie == slider_cookie);

    assert(picoui_widget_set_visible(&checkbox->widget, 0) == 0);
    assert(picoui_backend_widget_dispatch_signal(checkbox->widget.backend_widget,
                                                 PICOUI_BACKEND_SIGNAL_VALUE_CHANGED,
                                                 1,
                                                 checkbox->cb,
                                                 &checkbox->widget,
                                                 checkbox->user_data) == 0);
    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue,
                     checkbox_backend->ld_widget,
                     SIGNAL_VALUE_CHANGED,
                     1) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(value_count == 3);
    assert(last_value_widget == &slider->widget);
    assert(last_value == 75);
    assert(last_value_cookie == slider_cookie);
    assert(checkbox_backend->dispatch_count == 1);
    assert(picoui_checkbox_is_checked(checkbox) == 0);
    assert(picoui_widget_set_visible(&checkbox->widget, 1) == 0);

    assert(picoui_widget_set_visible(&sw->widget, 0) == 0);
    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue,
                     switch_backend->ld_widget,
                     SIGNAL_VALUE_CHANGED,
                     1) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(value_count == 3);
    assert(last_value_widget == &slider->widget);
    assert(last_value == 75);
    assert(last_value_cookie == slider_cookie);
    assert(switch_backend->dispatch_count == 1);
    assert(picoui_switch_is_checked(sw) == 0);
    assert(picoui_widget_set_visible(&sw->widget, 1) == 0);

    assert(picoui_widget_set_visible(&slider->widget, 0) == 0);
    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue,
                     slider_backend->ld_widget,
                     SIGNAL_VALUE_CHANGED,
                     250) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(value_count == 3);
    assert(last_value_widget == &slider->widget);
    assert(last_value == 75);
    assert(last_value_cookie == slider_cookie);
    assert(slider_backend->dispatch_count == 1);
    assert(picoui_slider_get_value(slider) == 75);
    assert(picoui_widget_set_visible(&slider->widget, 1) == 0);

    ldMsgDeinit(&app_state->ld_scene->ptMsgQueue);
    picoui_app_destroy(app);
    return 0;
}
