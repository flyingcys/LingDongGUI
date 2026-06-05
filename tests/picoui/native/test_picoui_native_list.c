#include "picoui/picoui.h"
#include "internal.h"

#include <assert.h>

int picoui_native_event_bind_root(struct picoui_screen *screen, struct picoui_window *root_window);
int picoui_native_list_get_rendered_selected_index(const struct picoui_list *list, int *index);
int picoui_native_list_get_rendered_selected_text(const struct picoui_list *list, const char **text);

static int g_selected_index = -1;
static int g_callback_count = 0;
static void *g_selected_user_data = 0;
static struct picoui_list *g_selected_list = 0;

static void on_list_selected(struct picoui_list *list, int index, void *user_data)
{
    g_callback_count++;
    g_selected_list = list;
    g_selected_index = index;
    g_selected_user_data = user_data;
}

int main(void)
{
    struct picoui_display *display;
    struct picoui_screen *screen;
    struct picoui_window *window;
    struct picoui_list *list;
    struct picoui_app *app;
    struct picoui_backend_widget *backend;
    struct picoui_point origin;
    const char *rendered_text = 0;
    int rendered_index = -1;
    int callback_cookie = 37;
    int rc;

    g_selected_index = -1;
    g_callback_count = 0;
    g_selected_user_data = 0;
    g_selected_list = 0;

    assert(picoui_init() == 0);

    display = picoui_display_create(320, 480);
    assert(display != 0);
    assert(picoui_display_set_default(display) == 0);

    screen = picoui_screen_active();
    assert(screen != 0);

    window = picoui_window_create_root(screen, "root");
    assert(window != 0);

    list = picoui_list_create((struct picoui_widget *)window, "city_list");
    assert(list != 0);
    assert(picoui_widget_set_pos((struct picoui_widget *)list, 20, 40) == 0);
    assert(picoui_widget_set_size((struct picoui_widget *)list, 120, 90) == 0);

    assert(picoui_list_add_item(list, "item_0", "one") == 0);
    assert(picoui_list_add_item(list, "item_1", "two") == 0);
    assert(picoui_list_add_item(list, "item_2", "three") == 0);

    picoui_list_set_on_selected(list, on_list_selected, &callback_cookie);
    assert(picoui_list_set_selected_index(list, 1) == 0);
    assert(picoui_list_get_selected_index(list) == 1);
    backend = (struct picoui_backend_widget *)list->widget.backend_widget;
    assert(backend != 0);
    assert(backend->value == 1);

    assert(picoui_native_event_bind_root(screen, window) == 0);
    assert(picoui_native_list_get_rendered_selected_index(list, &rendered_index) == -1);
    assert(picoui_native_list_get_rendered_selected_text(list, &rendered_text) == -1);

    rc = picoui_timer_handler();
    assert(rc == 0 || rc == 1);

    assert(picoui_native_list_get_rendered_selected_index(list, &rendered_index) == 0);
    assert(rendered_index == 1);
    assert(picoui_native_list_get_rendered_selected_text(list, &rendered_text) == 0);
    assert(rendered_text == list->items[1].text);

    app = backend->owner;
    assert(app != 0);
    origin = picoui_widget_get_absolute_pos((const struct picoui_widget *)list,
                                            (struct picoui_point){0, 0});
    assert(origin.x >= 0);
    assert(origin.y >= 0);

    assert(picoui_input_push_pointer(app, origin.x + 12, origin.y + 75, 1) == 0);
    rc = picoui_timer_handler();
    assert(rc == 0 || rc == 1);
    assert(g_callback_count == 0);
    assert(g_selected_index == -1);

    assert(picoui_input_push_pointer(app, origin.x + 12, origin.y + 75, 0) == 0);
    rc = picoui_timer_handler();
    assert(rc == 0 || rc == 1);

    assert(g_callback_count == 1);
    assert(picoui_list_get_selected_index(list) == 2);
    assert(backend->value == 2);
    assert(backend->last_signal == PICOUI_BACKEND_SIGNAL_VALUE_CHANGED);
    assert(backend->last_native_value == 2);
    assert(g_selected_list == list);
    assert(g_selected_index == 2);
    assert(g_selected_user_data == &callback_cookie);

    rc = picoui_timer_handler();
    assert(rc == 0 || rc == 1);
    assert(g_callback_count == 1);
    assert(picoui_native_list_get_rendered_selected_index(list, &rendered_index) == 0);
    assert(rendered_index == 2);
    assert(picoui_native_list_get_rendered_selected_text(list, &rendered_text) == 0);
    assert(rendered_text == list->items[2].text);

    picoui_deinit();
    return 0;
}
