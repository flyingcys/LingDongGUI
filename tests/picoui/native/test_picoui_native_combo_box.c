#include "picoui/picoui.h"
#include "internal.h"
#include "../../../src/gui/ldComboBox.h"

#include <assert.h>

int picoui_native_event_bind_root(struct picoui_screen *screen, struct picoui_window *root_window);
int picoui_backend_combo_box_set_open(void *backend_widget, int is_open);
int picoui_backend_combo_box_get_item_center(void *backend_widget, int item_index, int *x, int *y);
int picoui_native_combo_box_get_rendered_selected_index(const struct picoui_combo_box *combo_box, int *index);
int picoui_native_combo_box_get_rendered_selected_text(const struct picoui_combo_box *combo_box,
                                                       const char **text);
int picoui_native_combo_box_get_rendered_open(const struct picoui_combo_box *combo_box, int *is_open);

static int g_callback_count = 0;
static int g_selected_index = -1;
static void *g_callback_user_data = 0;
static struct picoui_combo_box *g_selected_combo_box = 0;

static void on_combo_box_selected(struct picoui_combo_box *combo_box, int index, void *user_data)
{
    g_callback_count++;
    g_selected_combo_box = combo_box;
    g_selected_index = index;
    g_callback_user_data = user_data;
}

int main(void)
{
    struct picoui_display *display;
    struct picoui_screen *screen;
    struct picoui_window *window;
    struct picoui_combo_box *combo_box;
    struct picoui_backend_widget *backend;
    ldComboBox_t *ld_combo_box;
    struct picoui_app *app;
    struct picoui_point origin;
    const char *rendered_text = 0;
    int rendered_index = -1;
    int is_open = -1;
    int callback_cookie = 41;
    int press_x;
    int header_y;
    int item2_y;
    int rc;

    g_callback_count = 0;
    g_selected_index = -1;
    g_callback_user_data = 0;
    g_selected_combo_box = 0;

    assert(picoui_init() == 0);

    display = picoui_display_create(320, 480);
    assert(display != 0);
    assert(picoui_display_set_default(display) == 0);

    screen = picoui_screen_active();
    assert(screen != 0);

    window = picoui_window_create_root(screen, "root");
    assert(window != 0);

    combo_box = picoui_combo_box_create(window, "settings_combo");
    assert(combo_box != 0);
    assert(picoui_widget_set_pos((struct picoui_widget *)combo_box, 24, 40) == 0);
    assert(picoui_widget_set_size((struct picoui_widget *)combo_box, 200, 32) == 0);

    assert(picoui_combo_box_add_item(combo_box, "wifi", "Wi-Fi") == 0);
    assert(picoui_combo_box_add_item(combo_box, "bluetooth", "Bluetooth") == 0);
    assert(picoui_combo_box_add_item(combo_box, "display", "Display") == 0);
    picoui_combo_box_set_on_selected(combo_box, on_combo_box_selected, &callback_cookie);
    assert(picoui_combo_box_set_selected_index(combo_box, 0) == 0);
    assert(picoui_combo_box_get_selected_index(combo_box) == 0);
    assert(picoui_combo_box_get_text(combo_box, 0) != 0);

    backend = (struct picoui_backend_widget *)combo_box->widget.backend_widget;
    assert(backend != 0);
    assert(backend->kind == PICOUI_BACKEND_WIDGET_COMBO_BOX);
    ld_combo_box = (ldComboBox_t *)backend->ld_widget;
    assert(ld_combo_box != 0);
    assert(picoui_backend_combo_box_set_open(backend, 1) == 0);
    assert(ld_combo_box->isExpand == true);
    assert(picoui_backend_combo_box_set_open(backend, 0) == 0);
    assert(ld_combo_box->isExpand == false);
    app = backend->owner;
    assert(app != 0);
    assert(picoui_native_event_bind_root(screen, window) == 0);
    assert(picoui_native_combo_box_get_rendered_selected_index(combo_box, &rendered_index) == -1);
    assert(picoui_native_combo_box_get_rendered_selected_text(combo_box, &rendered_text) == -1);
    assert(picoui_native_combo_box_get_rendered_open(combo_box, &is_open) == -1);
    assert(backend->value == 0);
    rc = picoui_timer_handler();
    assert(rc == 0 || rc == 1);
    origin = picoui_widget_get_absolute_pos((const struct picoui_widget *)combo_box,
                                            (struct picoui_point){0, 0});
    press_x = origin.x + (picoui_widget_get_width((const struct picoui_widget *)combo_box) / 2);
    header_y = origin.y + (picoui_widget_get_height((const struct picoui_widget *)combo_box) / 2);
    assert(picoui_backend_combo_box_get_item_center(backend, 2, &press_x, &item2_y) == 0);

    assert(picoui_native_combo_box_get_rendered_selected_index(combo_box, &rendered_index) == 0);
    assert(rendered_index == 0);
    assert(picoui_native_combo_box_get_rendered_selected_text(combo_box, &rendered_text) == 0);
    assert(rendered_text == combo_box->items[0].text);
    assert(picoui_native_combo_box_get_rendered_open(combo_box, &is_open) == 0);
    assert(is_open == 0);

    assert(picoui_input_push_pointer(app, press_x, header_y, 1) == 0);
    rc = picoui_timer_handler();
    assert(rc == 0 || rc == 1);
    assert(g_callback_count == 0);
    assert(backend->dispatch_count == 0);
    assert(ld_combo_box->isExpand == true);
    assert(picoui_combo_box_is_open(combo_box, &is_open) == 0);
    assert(is_open == 1);
    assert(picoui_input_push_pointer(app, press_x, header_y, 0) == 0);
    rc = picoui_timer_handler();
    assert(rc == 0 || rc == 1);
    assert(backend->dispatch_count == 0);
    assert(ld_combo_box->isExpand == true);
    assert(picoui_combo_box_is_open(combo_box, &is_open) == 0);
    assert(is_open == 1);
    assert(picoui_native_combo_box_get_rendered_open(combo_box, &is_open) == 0);
    assert(is_open == 1);

    assert(picoui_input_push_pointer(app, press_x, item2_y, 1) == 0);
    rc = picoui_timer_handler();
    assert(rc == 0 || rc == 1);
    assert(picoui_combo_box_is_open(combo_box, &is_open) == 0);
    assert(is_open == 1);
    assert(g_callback_count == 0);
    assert(picoui_input_push_pointer(app, press_x, item2_y, 0) == 0);
    rc = picoui_timer_handler();
    assert(rc == 0 || rc == 1);
    assert(backend->last_signal == PICOUI_BACKEND_SIGNAL_VALUE_CHANGED);
    assert(backend->last_native_value == 2);
    assert(backend->value == 2);
    assert(g_callback_count == 1);
    assert(g_selected_index == 2);
    assert((int)ldComboBoxGetSelectItem(ld_combo_box) == 2);
    assert(picoui_combo_box_get_selected_index(combo_box) == 2);
    assert(picoui_combo_box_get_text(combo_box, 2) != 0);
    assert((int)ldComboBoxGetSelectItem(ld_combo_box) == 2);
    assert(ld_combo_box->isExpand == false);
    assert(g_selected_combo_box == combo_box);
    assert(g_callback_user_data == &callback_cookie);
    rc = picoui_timer_handler();
    assert(rc == 0 || rc == 1);
    assert(picoui_native_combo_box_get_rendered_selected_index(combo_box, &rendered_index) == 0);
    assert(rendered_index == 2);
    assert(picoui_native_combo_box_get_rendered_selected_text(combo_box, &rendered_text) == 0);
    assert(rendered_text == combo_box->items[2].text);
    assert(picoui_native_combo_box_get_rendered_open(combo_box, &is_open) == 0);
    assert(is_open == 0);

    picoui_deinit();
    return 0;
}
