#include "picoui/picoui.h"
#include "internal.h"
#include "../../../src/gui/ldScrollSelecter.h"

#include <assert.h>
#include <string.h>

int picoui_native_event_bind_root(struct picoui_screen *screen, struct picoui_window *root_window);
int picoui_native_scroll_selecter_get_rendered_selected_index(const struct picoui_scroll_selecter *scroll_selecter,
                                                              int *index);
int picoui_native_scroll_selecter_get_rendered_selected_text(const struct picoui_scroll_selecter *scroll_selecter,
                                                             const char **text);
int picoui_native_scroll_selecter_get_rendered_scroll_delta(const struct picoui_scroll_selecter *scroll_selecter,
                                                            int *delta_y);

static int scroll_selecter_item_step_from_host(const struct picoui_scroll_selecter *scroll_selecter)
{
    const struct picoui_backend_widget *backend;
    const ldScrollSelecter_t *ld_scroll_selecter;

    assert(scroll_selecter != 0);
    backend = (const struct picoui_backend_widget *)scroll_selecter->widget.backend_widget;
    assert(backend != 0);
    ld_scroll_selecter = (const ldScrollSelecter_t *)backend->ld_widget;
    assert(ld_scroll_selecter != 0);
    assert(ld_scroll_selecter->ptFont != 0);

    return ld_scroll_selecter->itemSpace + ld_scroll_selecter->ptFont->tCharSize.iHeight;
}

int main(void)
{
    struct picoui_display *display;
    struct picoui_screen *screen;
    struct picoui_window *window;
    struct picoui_scroll_selecter *scroll_selecter;
    struct picoui_backend_widget *backend;
    struct picoui_app *app;
    struct picoui_point origin;
    const char *rendered_text = 0;
    int rendered_index = -1;
    int rendered_delta = 0;
    int item_step = 0;
    int press_x = 0;
    int press_y = 0;
    int rc;

    assert(picoui_init() == 0);

    display = picoui_display_create(320, 480);
    assert(display != 0);
    assert(picoui_display_set_default(display) == 0);

    screen = picoui_screen_active();
    assert(screen != 0);

    window = picoui_window_create_root(screen, "root");
    assert(window != 0);

    scroll_selecter = picoui_scroll_selecter_create(window, "scroll_selecter");
    assert(scroll_selecter != 0);
    assert(picoui_widget_set_size((struct picoui_widget *)scroll_selecter, 180, 120) == 0);
    assert(picoui_scroll_selecter_add_item(scroll_selecter, "wifi", "Wi-Fi") == 0);
    assert(picoui_scroll_selecter_add_item(scroll_selecter, "bluetooth", "Bluetooth") == 0);
    assert(picoui_scroll_selecter_add_item(scroll_selecter, "display", "Display") == 0);
    assert(picoui_scroll_selecter_add_item(scroll_selecter, "sound", "Sound") == 0);
    assert(picoui_scroll_selecter_add_item(scroll_selecter, "privacy", "Privacy") == 0);

    assert(picoui_native_event_bind_root(screen, window) == 0);
    assert(picoui_native_scroll_selecter_get_rendered_selected_index(scroll_selecter, &rendered_index) == -1);
    assert(picoui_native_scroll_selecter_get_rendered_selected_text(scroll_selecter, &rendered_text) == -1);
    assert(picoui_native_scroll_selecter_get_rendered_scroll_delta(scroll_selecter, &rendered_delta) == -1);

    assert(picoui_scroll_selecter_set_selected_index(scroll_selecter, 1) == 0);
    assert(picoui_scroll_selecter_get_selected_index(scroll_selecter) == 1);

    rc = picoui_timer_handler();
    assert(rc == 0 || rc == 1);
    backend = (struct picoui_backend_widget *)scroll_selecter->widget.backend_widget;
    assert(backend != 0);
    app = backend->owner;
    assert(app != 0);
    origin = picoui_widget_get_absolute_pos((const struct picoui_widget *)scroll_selecter,
                                            (struct picoui_point){0, 0});
    assert(origin.x >= 0);
    assert(origin.y >= 0);
    item_step = scroll_selecter_item_step_from_host(scroll_selecter);
    assert(item_step > 2);
    press_x = origin.x + (picoui_widget_get_width((const struct picoui_widget *)scroll_selecter) / 2);
    press_y = origin.y + (picoui_widget_get_height((const struct picoui_widget *)scroll_selecter) / 2);

    assert(picoui_native_scroll_selecter_get_rendered_selected_index(scroll_selecter, &rendered_index) == 0);
    assert(rendered_index == 1);
    assert(picoui_native_scroll_selecter_get_rendered_selected_text(scroll_selecter, &rendered_text) == 0);
    assert(rendered_text != 0);
    assert(strcmp(rendered_text, "Bluetooth") == 0);
    assert(picoui_input_push_pointer(app, press_x, press_y, 1) == 0);
    rc = picoui_timer_handler();
    assert(rc == 0 || rc == 1);
    assert(picoui_input_push_pointer(app, press_x, press_y - ((item_step / 2) - 1), 1) == 0);
    rc = picoui_timer_handler();
    assert(rc == 0 || rc == 1);
    assert(picoui_scroll_selecter_get_selected_index(scroll_selecter) == 1);
    assert(picoui_input_push_pointer(app, press_x, press_y - ((item_step / 2) - 1), 0) == 0);
    rc = picoui_timer_handler();
    assert(rc == 0 || rc == 1);
    assert(picoui_scroll_selecter_get_selected_index(scroll_selecter) == 1);
    assert(strcmp(picoui_scroll_selecter_get_selected_text(scroll_selecter), "Bluetooth") == 0);

    assert(picoui_input_push_pointer(app, press_x, press_y, 1) == 0);
    rc = picoui_timer_handler();
    assert(rc == 0 || rc == 1);
    assert(picoui_input_push_pointer(app, press_x, press_y - ((item_step / 2) + 1), 1) == 0);
    rc = picoui_timer_handler();
    assert(rc == 0 || rc == 1);
    assert(picoui_scroll_selecter_get_selected_index(scroll_selecter) == 1);
    assert(picoui_input_push_pointer(app, press_x, press_y - ((item_step / 2) + 1), 0) == 0);
    rc = picoui_timer_handler();
    assert(rc == 0 || rc == 1);
    assert(picoui_scroll_selecter_get_selected_index(scroll_selecter) == 2);
    assert(strcmp(picoui_scroll_selecter_get_selected_text(scroll_selecter), "Display") == 0);

    assert(picoui_native_scroll_selecter_get_rendered_selected_index(scroll_selecter, &rendered_index) == 0);
    assert(rendered_index == 2);
    assert(picoui_native_scroll_selecter_get_rendered_selected_text(scroll_selecter, &rendered_text) == 0);
    assert(strcmp(rendered_text, "Display") == 0);
    assert(picoui_native_scroll_selecter_get_rendered_scroll_delta(scroll_selecter, &rendered_delta) == 0);
    assert(rendered_delta == -((item_step / 2) + 1));

    picoui_deinit();
    return 0;
}
