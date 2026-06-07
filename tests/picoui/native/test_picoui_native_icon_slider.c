#include "picoui/picoui.h"
#include "internal.h"

#include <assert.h>

int picoui_native_event_bind_root(struct picoui_screen *screen, struct picoui_window *root_window);
int picoui_native_icon_slider_select_index(struct picoui_icon_slider *icon_slider, int index);
int picoui_native_icon_slider_get_rendered_selected_index(const struct picoui_icon_slider *icon_slider, int *index);
int picoui_native_icon_slider_get_rendered_selected_text(const struct picoui_icon_slider *icon_slider,
                                                         const char **text);

static int g_selected_index = -1;
static int g_callback_count = 0;
static void *g_selected_user_data = 0;
static struct picoui_icon_slider *g_selected_icon_slider = 0;

static void on_icon_slider_selected(struct picoui_icon_slider *icon_slider, int index, void *user_data)
{
    g_callback_count++;
    g_selected_icon_slider = icon_slider;
    g_selected_index = index;
    g_selected_user_data = user_data;
}

int main(void)
{
    struct picoui_display *display;
    struct picoui_screen *screen;
    struct picoui_window *window;
    struct picoui_icon_slider *icon_slider;
    struct picoui_backend_widget *backend;
    const char *rendered_text = 0;
    int rendered_index = -1;
    int callback_cookie = 59;
    int rc;

    g_selected_index = -1;
    g_callback_count = 0;
    g_selected_user_data = 0;
    g_selected_icon_slider = 0;

    assert(picoui_init() == 0);

    display = picoui_display_create(320, 480);
    assert(display != 0);
    assert(picoui_display_set_default(display) == 0);

    screen = picoui_screen_active();
    assert(screen != 0);

    window = picoui_window_create_root(screen, "root");
    assert(window != 0);

    icon_slider = picoui_icon_slider_create((struct picoui_widget *)window, "icon_slider");
    assert(icon_slider != 0);
    assert(picoui_widget_set_pos((struct picoui_widget *)icon_slider, 20, 40) == 0);
    assert(picoui_widget_set_size((struct picoui_widget *)icon_slider, 220, 86) == 0);

    assert(picoui_icon_slider_add_item(icon_slider, "weather", "Weather") == 0);
    assert(picoui_icon_slider_add_item(icon_slider, "note", "Note") == 0);
    assert(picoui_icon_slider_add_item(icon_slider, "book", "Book") == 0);

    backend = (struct picoui_backend_widget *)icon_slider->widget.backend_widget;
    assert(backend != 0);
    assert(backend->kind == PICOUI_BACKEND_WIDGET_ICON_SLIDER);

    assert(picoui_native_event_bind_root(screen, window) == 0);
    assert(picoui_native_icon_slider_get_rendered_selected_index(icon_slider, &rendered_index) == -1);
    assert(picoui_native_icon_slider_get_rendered_selected_text(icon_slider, &rendered_text) == -1);
    rc = picoui_timer_handler();
    assert(rc == 0 || rc == 1);
    assert(picoui_native_icon_slider_get_rendered_selected_index(icon_slider, &rendered_index) == 0);
    assert(rendered_index == -1);
    assert(picoui_native_icon_slider_get_rendered_selected_text(icon_slider, &rendered_text) == 0);
    assert(rendered_text == 0);

    picoui_icon_slider_set_on_selected(icon_slider, on_icon_slider_selected, &callback_cookie);
    assert(picoui_icon_slider_set_selected_index(icon_slider, 1) == 0);
    assert(picoui_icon_slider_get_selected_index(icon_slider) == 1);
    assert(backend->value == 1);

    assert(picoui_native_icon_slider_select_index(icon_slider, 2) == 0);
    assert(g_callback_count == 1);
    assert(g_selected_index == 2);
    assert(g_selected_icon_slider == icon_slider);
    assert(g_selected_user_data == &callback_cookie);
    assert(picoui_icon_slider_get_selected_index(icon_slider) == 2);
    assert(backend->value == 2);
    assert(backend->last_signal == PICOUI_BACKEND_SIGNAL_VALUE_CHANGED);
    assert(backend->dispatch_count == 1);
    assert(backend->last_native_signal == PICOUI_NATIVE_SIGNAL_CLICKED_ITEM);
    assert(backend->last_native_value == 2);
    assert(backend->last_data_source == PICOUI_BACKEND_DATA_SOURCE_NATIVE_EVENT);

    assert(picoui_native_icon_slider_get_rendered_selected_index(icon_slider, &rendered_index) == 0);
    assert(rendered_index == -1);
    assert(picoui_native_icon_slider_get_rendered_selected_text(icon_slider, &rendered_text) == 0);
    assert(rendered_text == 0);
    rc = picoui_timer_handler();
    assert(rc == 0 || rc == 1);
    assert(picoui_native_icon_slider_get_rendered_selected_index(icon_slider, &rendered_index) == 0);
    assert(rendered_index == 2);
    assert(picoui_native_icon_slider_get_rendered_selected_text(icon_slider, &rendered_text) == 0);
    assert(rendered_text == icon_slider->items[2].text);

    picoui_deinit();
    return 0;
}
