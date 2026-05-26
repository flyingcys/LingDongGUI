#include "picoui/picoui.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/misc/ldMsg.h"
#include "internal.h"

#include <assert.h>

static int press_count = 0;
static int release_count = 0;
static int click_count = 0;
static int event_order[4];
static int event_order_count = 0;
static struct picoui_widget *last_press_widget = 0;
static struct picoui_widget *last_release_widget = 0;
static struct picoui_widget *last_click_widget = 0;
static int last_press_cookie = 0;
static int last_release_cookie = 0;
static int last_click_cookie = 0;

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

int main(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_window *win = picoui_window_create(app, "root");
    struct picoui_button *button = picoui_button_create(win, "submit");
    struct picoui_backend_widget *backend;
    struct picoui_backend_app_state *app_state;
    int click_cookie = 33;
    int press_cookie = 11;
    int release_cookie = 22;

    assert(app != 0);
    assert(win != 0);
    assert(button != 0);
    backend = button->widget.backend_widget;
    assert(backend != 0);
    app_state = (struct picoui_backend_app_state *)app->backend_app;
    assert(app_state != 0);
    assert(app_state->ld_scene != 0);
    assert(ldMsgInit(&app_state->ld_scene->ptMsgQueue, 8) == true);

    assert(picoui_button_set_on_pressed(0, on_pressed, &press_cookie) == -1);
    assert(picoui_button_set_on_released(0, on_released, &release_cookie) == -1);

    assert(picoui_button_set_on_pressed(button, on_pressed, &press_cookie) == 0);
    assert(picoui_button_set_on_released(button, on_released, &release_cookie) == 0);
    assert(picoui_button_set_on_clicked(button, on_clicked, &click_cookie) == 0);
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

    ldMsgDeinit(&app_state->ld_scene->ptMsgQueue);
    picoui_app_destroy(app);
    return 0;
}
