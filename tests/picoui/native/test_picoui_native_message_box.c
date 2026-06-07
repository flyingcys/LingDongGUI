#include "picoui/picoui.h"
#include "internal.h"

#include <assert.h>
#include <string.h>

int picoui_native_event_bind_root(struct picoui_screen *screen, struct picoui_window *root_window);
int picoui_native_message_box_get_rendered_open(const struct picoui_message_box *box, int *is_open);
int picoui_native_message_box_get_rendered_button_count(const struct picoui_message_box *box,
                                                        int *button_count);
int picoui_native_message_box_get_button_center(const struct picoui_message_box *box,
                                                int index,
                                                int *x,
                                                int *y);

static int g_confirm_count = 0;
static int g_confirm_index = -1;
static const char *g_confirm_id = 0;
static void *g_confirm_user_data = 0;
static int g_underlay_click_count = 0;

static void on_underlay_clicked(struct picoui_widget *widget, void *user_data)
{
    (void)widget;
    (void)user_data;
    g_underlay_click_count++;
}

static void on_message_box_confirm(struct picoui_message_box *box, int index, void *user_data)
{
    g_confirm_count++;
    g_confirm_index = index;
    g_confirm_id = box != 0 ? box->id : 0;
    g_confirm_user_data = user_data;
}

int main(void)
{
    static const char *buttons[] = {
        "Cancel",
        "OK",
    };
    struct picoui_display *display;
    struct picoui_screen *screen;
    struct picoui_window *window;
    struct picoui_button *underlay;
    struct picoui_message_box *box;
    struct picoui_backend_widget *backend;
    struct picoui_app *app;
    struct picoui_point underlay_origin;
    int callback_cookie = 91;
    int is_open = -1;
    int button_count = -1;
    int ok_x = -1;
    int ok_y = -1;
    int rc;

    g_confirm_count = 0;
    g_confirm_index = -1;
    g_confirm_id = 0;
    g_confirm_user_data = 0;
    g_underlay_click_count = 0;

    assert(picoui_init() == 0);

    display = picoui_display_create(320, 480);
    assert(display != 0);
    assert(picoui_display_set_default(display) == 0);

    screen = picoui_screen_active();
    assert(screen != 0);

    window = picoui_window_create_root(screen, "root");
    assert(window != 0);

    underlay = picoui_button_create(window, "underlay");
    assert(underlay != 0);
    assert(picoui_button_set_text(underlay, "Underlay") == 0);
    assert(picoui_widget_set_pos((struct picoui_widget *)underlay, 50, 70) == 0);
    assert(picoui_widget_set_size((struct picoui_widget *)underlay, 220, 140) == 0);
    assert(picoui_button_set_on_clicked(underlay, on_underlay_clicked, 0) == 0);

    box = picoui_message_box_create((struct picoui_widget *)window, "modal_box");
    assert(box != 0);
    assert(picoui_widget_set_pos((struct picoui_widget *)box, 60, 80) == 0);
    assert(picoui_widget_set_size((struct picoui_widget *)box, 200, 120) == 0);
    assert(picoui_message_box_set_title(box, "Confirm") == 0);
    assert(picoui_message_box_set_message(box, "Apply changes?") == 0);
    assert(picoui_message_box_set_buttons(box, buttons, 2) == 0);
    picoui_message_box_set_on_confirm_indexed(box, on_message_box_confirm, &callback_cookie);

    backend = (struct picoui_backend_widget *)box->widget.backend_widget;
    assert(backend != 0);
    assert(backend->kind == PICOUI_BACKEND_WIDGET_MESSAGE_BOX);
    app = backend->owner;
    assert(app != 0);
    underlay_origin = picoui_widget_get_absolute_pos((const struct picoui_widget *)underlay,
                                                     (struct picoui_point){0, 0});
    assert(underlay_origin.x >= 0);
    assert(underlay_origin.y >= 0);

    assert(picoui_native_event_bind_root(screen, window) == 0);
    assert(picoui_native_message_box_get_rendered_open(box, &is_open) == -1);
    assert(picoui_native_message_box_get_rendered_button_count(box, &button_count) == -1);

    rc = picoui_timer_handler();
    assert(rc == 0 || rc == 1);

    assert(picoui_native_message_box_get_rendered_open(box, &is_open) == 0);
    assert(is_open == 1);
    assert(picoui_native_message_box_get_rendered_button_count(box, &button_count) == 0);
    assert(button_count == 2);
    assert(picoui_native_message_box_get_button_center(box, 1, &ok_x, &ok_y) == 0);
    assert(ok_x >= 0);
    assert(ok_y >= 0);

    assert(picoui_input_push_pointer(app, underlay_origin.x + 12, underlay_origin.y + 12, 1) == 0);
    rc = picoui_timer_handler();
    assert(rc == 0 || rc == 1);
    assert(picoui_input_push_pointer(app, underlay_origin.x + 12, underlay_origin.y + 12, 0) == 0);
    rc = picoui_timer_handler();
    assert(rc == 0 || rc == 1);
    assert(g_underlay_click_count == 0);
    assert(picoui_native_message_box_get_rendered_open(box, &is_open) == 0);
    assert(is_open == 1);

    assert(picoui_input_push_pointer(app, ok_x, ok_y, 1) == 0);
    rc = picoui_timer_handler();
    assert(rc == 0 || rc == 1);
    assert(g_confirm_count == 0);

    assert(picoui_input_push_pointer(app, ok_x, ok_y, 0) == 0);
    rc = picoui_timer_handler();
    assert(rc == 0 || rc == 1);

    assert(g_confirm_count == 1);
    assert(g_confirm_index == 1);
    assert(g_confirm_id != 0);
    assert(strcmp(g_confirm_id, "modal_box") == 0);
    assert(g_confirm_user_data == &callback_cookie);
    assert(picoui_native_message_box_get_rendered_open(box, &is_open) == 0);
    assert(is_open == 0);

    assert(picoui_input_push_pointer(app, underlay_origin.x + 12, underlay_origin.y + 12, 1) == 0);
    rc = picoui_timer_handler();
    assert(rc == 0 || rc == 1);
    assert(picoui_input_push_pointer(app, underlay_origin.x + 12, underlay_origin.y + 12, 0) == 0);
    rc = picoui_timer_handler();
    assert(rc == 0 || rc == 1);
    assert(g_underlay_click_count == 1);

    picoui_deinit();
    return 0;
}
