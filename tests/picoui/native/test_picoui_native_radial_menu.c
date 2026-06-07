#include "picoui/picoui.h"
#include "internal.h"
#include "../../../src/gui/ldRadialMenu.h"

#include <assert.h>
#include <string.h>

int picoui_native_event_bind_root(struct picoui_screen *screen, struct picoui_window *root_window);
int picoui_native_radial_menu_get_rendered_state(const struct picoui_radial_menu *radial_menu,
                                                 int *item_count,
                                                 int *selected_index);

static int g_selected_count = 0;
static int g_selected_index = -1;
static const char *g_selected_id = 0;
static void *g_selected_user_data = 0;

static void on_radial_menu_selected(struct picoui_radial_menu *radial_menu, int index, void *user_data)
{
    g_selected_count++;
    g_selected_index = index;
    g_selected_id = radial_menu != 0 ? radial_menu->id : 0;
    g_selected_user_data = user_data;
}

static void radial_menu_click_point_from_host_region(const struct picoui_radial_menu *radial_menu,
                                              int target_index,
                                              int *x,
                                              int *y)
{
    struct picoui_point origin;
    const struct picoui_backend_widget *backend;
    const ldRadialMenu_t *ld_radial_menu;
    arm_2d_region_t region;

    assert(radial_menu != 0);
    assert(x != 0);
    assert(y != 0);
    assert(radial_menu->item_count > 0);
    assert(target_index >= 0 && target_index < radial_menu->item_count);

    backend = (const struct picoui_backend_widget *)radial_menu->widget.backend_widget;
    assert(backend != 0);
    ld_radial_menu = (const ldRadialMenu_t *)backend->ld_widget;
    assert(ld_radial_menu != 0);

    origin = picoui_widget_get_absolute_pos((const struct picoui_widget *)radial_menu,
                                            (struct picoui_point){0, 0});
    region = ld_radial_menu->use_as__ldBase_t.ptItemRegionList[target_index].itemRegion;
    assert(region.tSize.iWidth > 0);
    assert(region.tSize.iHeight > 0);

    *x = origin.x + region.tLocation.iX + (region.tSize.iWidth / 2);
    *y = origin.y + region.tLocation.iY + (region.tSize.iHeight / 2);
}

int main(void)
{
    struct picoui_display *display;
    struct picoui_screen *screen;
    struct picoui_window *window;
    struct picoui_radial_menu *radial_menu;
    struct picoui_backend_widget *backend;
    struct picoui_app *app;
    int callback_cookie = 73;
    int rendered_item_count = -1;
    int rendered_selected_index = -1;
    int note_x = -1;
    int note_y = -1;
    int rc;

    g_selected_count = 0;
    g_selected_index = -1;
    g_selected_id = 0;
    g_selected_user_data = 0;

    assert(picoui_init() == 0);

    display = picoui_display_create(320, 480);
    assert(display != 0);
    assert(picoui_display_set_default(display) == 0);

    screen = picoui_screen_active();
    assert(screen != 0);

    window = picoui_window_create_root(screen, "root");
    assert(window != 0);

    radial_menu = picoui_radial_menu_create((struct picoui_widget *)window, "radial_menu");
    assert(radial_menu != 0);
    assert(picoui_widget_set_size((struct picoui_widget *)radial_menu, 194, 96) == 0);
    assert(picoui_radial_menu_add_item(radial_menu, "weather") == 0);
    assert(picoui_radial_menu_add_item(radial_menu, "note") == 0);
    assert(picoui_radial_menu_add_item(radial_menu, "book") == 0);
    assert(picoui_radial_menu_add_item(radial_menu, "chart") == 0);
    assert(picoui_radial_menu_get_selected_index(radial_menu) == 0);
    picoui_radial_menu_set_on_selected(radial_menu, on_radial_menu_selected, &callback_cookie);

    backend = (struct picoui_backend_widget *)radial_menu->widget.backend_widget;
    assert(backend != 0);
    app = backend->owner;
    assert(app != 0);

    assert(picoui_native_event_bind_root(screen, window) == 0);
    assert(picoui_native_radial_menu_get_rendered_state(radial_menu,
                                                        &rendered_item_count,
                                                        &rendered_selected_index) == -1);

    rc = picoui_timer_handler();
    assert(rc == 0 || rc == 1);

    assert(picoui_native_radial_menu_get_rendered_state(radial_menu,
                                                        &rendered_item_count,
                                                        &rendered_selected_index) == 0);
    assert(rendered_item_count == 4);
    assert(rendered_selected_index == 0);
    radial_menu_click_point_from_host_region(radial_menu, 1, &note_x, &note_y);
    assert(note_x >= 0);
    assert(note_y >= 0);

    assert(picoui_input_push_pointer(app, note_x, note_y, 1) == 0);
    rc = picoui_timer_handler();
    assert(rc == 0 || rc == 1);
    assert(g_selected_count == 0);

    assert(picoui_input_push_pointer(app, note_x, note_y, 0) == 0);
    rc = picoui_timer_handler();
    assert(rc == 0 || rc == 1);

    assert(g_selected_count == 1);
    assert(g_selected_index == 1);
    assert(g_selected_id != 0);
    assert(strcmp(g_selected_id, "radial_menu") == 0);
    assert(g_selected_user_data == &callback_cookie);
    assert(picoui_radial_menu_get_selected_index(radial_menu) == 1);
    assert(picoui_native_radial_menu_get_rendered_state(radial_menu,
                                                        &rendered_item_count,
                                                        &rendered_selected_index) == 0);
    assert(rendered_item_count == 4);
    assert(rendered_selected_index == 1);
    assert(backend->value == 1);
    assert(backend->last_signal == PICOUI_BACKEND_SIGNAL_VALUE_CHANGED);
    assert(backend->last_native_signal == PICOUI_NATIVE_SIGNAL_CLICKED_ITEM);
    assert(backend->last_native_value == 1u);

    picoui_deinit();
    return 0;
}
