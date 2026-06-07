#include "../backend/ldgui/backend.h"
#include "../core/internal.h"
#include "picoui/checkbox.h"
#include "picoui/combo_box.h"
#include "picoui/screen.h"
#include "picoui/slider.h"
#include "picoui/switch.h"
#include "picoui/calendar.h"

#include "../../../src/gui/ldBase.h"
#include "../../../src/misc/ldMsg.h"

int picoui_native_list_select_index(struct picoui_list *list, int index);
int picoui_native_combo_box_open(struct picoui_combo_box *combo_box);
int picoui_native_combo_box_select_index(struct picoui_combo_box *combo_box, int index);
int picoui_combo_box_apply_open_state(struct picoui_combo_box *combo_box, int is_open);
int picoui_native_radial_menu_select_point(struct picoui_radial_menu *radial_menu, int x, int y);
int picoui_native_message_box_handle_press(struct picoui_message_box *box, int x, int y);
int picoui_native_message_box_handle_release(struct picoui_message_box *box, int x, int y);
int picoui_native_scroll_selecter_apply_scroll_delta(struct picoui_scroll_selecter *scroll_selecter, int delta_y);
int picoui_native_table_select_point(struct picoui_table *table, int x, int y);
int picoui_native_render_bind_root(struct picoui_screen *screen, struct picoui_window *root_window);
int picoui_native_render_timer_handler(void);
void picoui_native_render_deinit(void);

struct picoui_native_event_binding {
    struct picoui_screen *screen;
    struct picoui_window *root_window;
    struct picoui_app *app;
    struct picoui_backend_widget *pressed_backend;
    int last_pointer_pressed;
    int press_x;
    int press_y;
};

static struct picoui_native_event_binding g_picoui_native_event_binding;

static void picoui_native_event_reset(void)
{
    g_picoui_native_event_binding.screen = 0;
    g_picoui_native_event_binding.root_window = 0;
    g_picoui_native_event_binding.app = 0;
    g_picoui_native_event_binding.pressed_backend = 0;
    g_picoui_native_event_binding.last_pointer_pressed = 0;
    g_picoui_native_event_binding.press_x = 0;
    g_picoui_native_event_binding.press_y = 0;
}

static int picoui_native_event_point_in_widget(const struct picoui_widget *widget, int x, int y)
{
    struct picoui_point origin;
    int width;
    int height;
    const struct picoui_backend_widget *backend;

    if (widget == 0 || !widget->visible || !widget->enabled) {
        return 0;
    }

    backend = (const struct picoui_backend_widget *)widget->backend_widget;
    width = picoui_widget_get_width(widget);
    height = picoui_widget_get_height(widget);
    if (backend != 0
        && backend->kind == PICOUI_BACKEND_WIDGET_COMBO_BOX
        && backend->open
        && backend->value >= -1) {
        const struct picoui_combo_box *combo_box = (const struct picoui_combo_box *)backend->host_widget;
        if (combo_box != 0 && combo_box->item_count > 0) {
            height *= (combo_box->item_count + 1);
        }
    }
    if (width <= 0 || height <= 0) {
        return 0;
    }

    origin = picoui_widget_get_absolute_pos(widget, (struct picoui_point){0, 0});
    if (origin.x < 0 || origin.y < 0) {
        return 0;
    }

    return x >= origin.x && x < (origin.x + width) && y >= origin.y && y < (origin.y + height);
}

static int picoui_native_event_is_input_backend(const struct picoui_backend_widget *backend)
{
    return backend != 0
        && (backend->kind == PICOUI_BACKEND_WIDGET_BUTTON
            || backend->kind == PICOUI_BACKEND_WIDGET_CALENDAR
            || backend->kind == PICOUI_BACKEND_WIDGET_CHECKBOX
            || backend->kind == PICOUI_BACKEND_WIDGET_COMBO_BOX
            || backend->kind == PICOUI_BACKEND_WIDGET_LIST
            || backend->kind == PICOUI_BACKEND_WIDGET_RADIAL_MENU
            || backend->kind == PICOUI_BACKEND_WIDGET_SCROLL_SELECTER
            || backend->kind == PICOUI_BACKEND_WIDGET_TABLE
            || backend->kind == PICOUI_BACKEND_WIDGET_MESSAGE_BOX
            || backend->kind == PICOUI_BACKEND_WIDGET_SLIDER
            || backend->kind == PICOUI_BACKEND_WIDGET_SWITCH);
}

static int picoui_native_event_dispatch_calendar_click(struct picoui_backend_widget *backend, int x, int y)
{
    struct picoui_calendar *calendar;
    int selected_year;
    int selected_month;
    int selected_day;
    int packed_value;

    if (backend == 0 || backend->kind != PICOUI_BACKEND_WIDGET_CALENDAR || backend->host_widget == 0) {
        return -1;
    }

    calendar = (struct picoui_calendar *)backend->host_widget;
    if (picoui_backend_calendar_point_to_date(backend,
                                              x,
                                              y,
                                              &selected_year,
                                              &selected_month,
                                              &selected_day) != 0) {
        return -1;
    }

    if (picoui_calendar_set_selected_date(calendar, selected_year, selected_month, selected_day) != 0) {
        return -1;
    }

    packed_value = (selected_year << 16) | (selected_month << 8) | selected_day;
    backend->last_signal = PICOUI_BACKEND_SIGNAL_VALUE_CHANGED;
    backend->dispatch_count++;
    backend->last_native_signal = SIGNAL_CLICKED_ITEM;
    backend->last_native_value = (uint64_t)packed_value;
    backend->last_data_source = PICOUI_BACKEND_DATA_SOURCE_NATIVE_EVENT;

    if (calendar->cb != 0) {
        calendar->cb(calendar,
                     selected_year,
                     selected_month,
                     selected_day,
                     calendar->user_data);
    }

    return 0;
}

static int picoui_native_event_list_y_to_index(const struct picoui_list *list, int y)
{
    struct picoui_point origin;
    int height;
    int local_y;
    int index;

    if (list == 0 || list->item_count <= 0) {
        return -1;
    }

    height = picoui_widget_get_height((const struct picoui_widget *)list);
    if (height <= 0) {
        return -1;
    }

    origin = picoui_widget_get_absolute_pos((const struct picoui_widget *)list,
                                            (struct picoui_point){0, 0});
    local_y = y - origin.y;
    if (local_y < 0 || local_y >= height) {
        return -1;
    }

    index = (local_y * list->item_count) / height;
    if (index < 0) {
        return -1;
    }
    if (index >= list->item_count) {
        index = list->item_count - 1;
    }
    return index;
}

static int picoui_native_event_slider_x_to_permille(const struct picoui_slider *slider, int x)
{
    struct picoui_point origin;
    int width;
    int local_x;

    if (slider == 0) {
        return 0;
    }

    width = picoui_widget_get_width((const struct picoui_widget *)slider);
    if (width <= 1) {
        return 0;
    }

    origin = picoui_widget_get_absolute_pos((const struct picoui_widget *)slider,
                                            (struct picoui_point){0, 0});
    local_x = x - origin.x;
    if (local_x <= 0) {
        return 0;
    }
    if (local_x >= width - 1) {
        return 1000;
    }

    return (local_x * 1000) / (width - 1);
}

static int picoui_native_event_dispatch_slider_drag(struct picoui_backend_widget *backend, int x)
{
    struct picoui_slider *slider;
    int permille;

    if (backend == 0 || backend->kind != PICOUI_BACKEND_WIDGET_SLIDER) {
        return -1;
    }

    slider = (struct picoui_slider *)backend->host_widget;
    if (slider == 0) {
        return -1;
    }

    permille = picoui_native_event_slider_x_to_permille(slider, x);
    return picoui_backend_widget_dispatch_native_signal(backend,
                                                        SIGNAL_VALUE_CHANGED,
                                                        (uint64_t)permille);
}

static int picoui_native_event_dispatch_list_click(struct picoui_backend_widget *backend, int y)
{
    struct picoui_list *list;
    int selected_index;
    int was_selected_index;
    int was_backend_value;

    if (backend == 0 || backend->kind != PICOUI_BACKEND_WIDGET_LIST || backend->host_widget == 0) {
        return -1;
    }

    list = (struct picoui_list *)backend->host_widget;
    selected_index = picoui_native_event_list_y_to_index(list, y);
    if (selected_index < 0) {
        return -1;
    }

    was_selected_index = list->selected_index;
    was_backend_value = backend->value;
    if (picoui_backend_widget_claim_focus(backend) != 0) {
        return -1;
    }
    if (picoui_native_list_select_index(list, selected_index) != 0) {
        return -1;
    }
    if (picoui_backend_list_set_selected_index(backend, selected_index) != 0) {
        return -1;
    }

    list->selected_index = selected_index;
    if (was_selected_index == selected_index && was_backend_value == selected_index) {
        return 0;
    }

    backend->last_signal = PICOUI_BACKEND_SIGNAL_VALUE_CHANGED;
    backend->dispatch_count++;
    backend->last_native_signal = SIGNAL_CLICKED_ITEM;
    backend->last_native_value = (uint64_t)selected_index;
    backend->last_data_source = PICOUI_BACKEND_DATA_SOURCE_NATIVE_EVENT;
    if (list->cb != 0) {
        list->cb(list, selected_index, list->user_data);
    }
    return 0;
}

static int picoui_native_event_dispatch_combo_box_click(struct picoui_backend_widget *backend, int x, int y)
{
    struct picoui_combo_box *combo_box;
    int slot;

    if (backend == 0 || backend->kind != PICOUI_BACKEND_WIDGET_COMBO_BOX || backend->host_widget == 0) {
        return -1;
    }

    combo_box = (struct picoui_combo_box *)backend->host_widget;
    slot = picoui_backend_combo_box_point_to_slot(backend, x, y);
    if (slot < 0) {
        return -1;
    }

    if (slot == 0) {
        return 0;
    }

    slot -= 1;
    if (slot < 0 || slot >= combo_box->item_count) {
        return -1;
    }

    return picoui_native_combo_box_select_index(combo_box, slot);
}

static int picoui_native_event_dispatch_radial_menu_click(struct picoui_backend_widget *backend, int x, int y)
{
    if (backend == 0 || backend->kind != PICOUI_BACKEND_WIDGET_RADIAL_MENU || backend->host_widget == 0) {
        return -1;
    }

    return picoui_native_radial_menu_select_point((struct picoui_radial_menu *)backend->host_widget, x, y);
}

static int picoui_native_event_dispatch_scroll_selecter_release(struct picoui_backend_widget *backend, int y)
{
    struct picoui_scroll_selecter *scroll_selecter;
    int delta_y;
    int previous_index;

    if (backend == 0 || backend->kind != PICOUI_BACKEND_WIDGET_SCROLL_SELECTER || backend->host_widget == 0) {
        return -1;
    }

    scroll_selecter = (struct picoui_scroll_selecter *)backend->host_widget;
    previous_index = scroll_selecter->selected_index;
    delta_y = y - g_picoui_native_event_binding.press_y;
    if (picoui_native_scroll_selecter_apply_scroll_delta(scroll_selecter, delta_y) != 0) {
        return -1;
    }

    if (scroll_selecter->selected_index == previous_index) {
        return 0;
    }
    if (picoui_backend_scroll_selecter_set_selected_index(backend,
                                                          scroll_selecter->selected_index) != 0) {
        return -1;
    }

    backend->last_signal = PICOUI_BACKEND_SIGNAL_VALUE_CHANGED;
    backend->dispatch_count++;
    backend->last_native_signal = SIGNAL_RELEASE;
    backend->last_data_source = PICOUI_BACKEND_DATA_SOURCE_NATIVE_EVENT;
    return 0;
}

static int picoui_native_event_dispatch_table_click(struct picoui_backend_widget *backend, int x, int y)
{
    struct picoui_table *table;
    int previous_row;
    int previous_column;

    if (backend == 0 || backend->kind != PICOUI_BACKEND_WIDGET_TABLE || backend->host_widget == 0) {
        return -1;
    }

    table = (struct picoui_table *)backend->host_widget;
    previous_row = table->current_row;
    previous_column = table->current_column;
    if (picoui_native_table_select_point(table, x, y) != 0) {
        return -1;
    }

    if (table->current_row == previous_row && table->current_column == previous_column) {
        return 0;
    }
    if (picoui_backend_table_set_current_cell(backend,
                                              table->current_row,
                                              table->current_column) != 0) {
        return -1;
    }

    backend->last_signal = PICOUI_BACKEND_SIGNAL_VALUE_CHANGED;
    backend->dispatch_count++;
    backend->last_native_signal = SIGNAL_CLICKED_ITEM;
    backend->last_native_value =
        (((uint64_t)(uint32_t)table->current_row & 0xFFFFu) << 16)
        | ((uint64_t)(uint32_t)table->current_column & 0xFFFFu);
    backend->last_data_source = PICOUI_BACKEND_DATA_SOURCE_NATIVE_EVENT;
    return 0;
}

static struct picoui_backend_widget *picoui_native_event_find_open_message_box(
    struct picoui_backend_widget *backend)
{
    struct picoui_backend_widget *child;

    if (backend == 0) {
        return 0;
    }

    child = backend->first_child;
    while (child != 0) {
        struct picoui_backend_widget *found = picoui_native_event_find_open_message_box(child);
        if (found != 0) {
            return found;
        }
        child = child->next_sibling;
    }

    if (backend->kind == PICOUI_BACKEND_WIDGET_MESSAGE_BOX
        && backend->host_widget != 0
        && ((const struct picoui_message_box *)backend->host_widget)->is_open != 0
        && ((const struct picoui_message_box *)backend->host_widget)->widget.visible != 0
        && ((const struct picoui_message_box *)backend->host_widget)->widget.enabled != 0) {
        return backend;
    }

    return 0;
}

static struct picoui_backend_widget *picoui_native_event_find_input_backend(struct picoui_backend_widget *backend,
                                                                            int x,
                                                                            int y)
{
    struct picoui_backend_widget *child;

    if (backend == 0) {
        return 0;
    }

    child = backend->first_child;
    while (child != 0) {
        struct picoui_backend_widget *found = picoui_native_event_find_input_backend(child, x, y);
        if (found != 0) {
            return found;
        }
        child = child->next_sibling;
    }

    if (picoui_native_event_is_input_backend(backend) &&
        picoui_native_event_point_in_widget(backend->host_widget, x, y)) {
        return backend;
    }

    return 0;
}

int picoui_native_event_bind_root(struct picoui_screen *screen, struct picoui_window *root_window)
{
    struct picoui_backend_widget *root_backend;

    if (screen == 0 || root_window == 0 || root_window->widget.backend_widget == 0) {
        return -1;
    }

    root_backend = (struct picoui_backend_widget *)root_window->widget.backend_widget;
    if (root_backend->owner == 0) {
        return -1;
    }

    g_picoui_native_event_binding.screen = screen;
    g_picoui_native_event_binding.root_window = root_window;
    g_picoui_native_event_binding.app = root_backend->owner;
    g_picoui_native_event_binding.pressed_backend = 0;
    g_picoui_native_event_binding.last_pointer_pressed = 0;
    g_picoui_native_event_binding.press_x = 0;
    g_picoui_native_event_binding.press_y = 0;
    return picoui_native_render_bind_root(screen, root_window);
}

int picoui_native_event_pump(void)
{
    struct picoui_backend_widget *root_backend;
    struct picoui_backend_widget *input_backend;
    int x;
    int y;
    int pointer_pressed;
    uint32_t signal;

    if (g_picoui_native_event_binding.screen == 0
        || g_picoui_native_event_binding.root_window == 0
        || g_picoui_native_event_binding.app == 0) {
        return 0;
    }

    if (picoui_screen_active() != g_picoui_native_event_binding.screen) {
        return -1;
    }

    if (picoui_input_get_pointer(g_picoui_native_event_binding.app, &x, &y, &pointer_pressed) != 0) {
        return -1;
    }

    root_backend = (struct picoui_backend_widget *)g_picoui_native_event_binding.root_window->widget.backend_widget;
    if (root_backend == 0) {
        return -1;
    }

    input_backend = picoui_native_event_find_open_message_box(root_backend);
    if (input_backend == 0) {
        input_backend = picoui_native_event_find_input_backend(root_backend,
                                                               x,
                                                               y);
    }
    if (pointer_pressed == g_picoui_native_event_binding.last_pointer_pressed) {
        if (pointer_pressed
            && g_picoui_native_event_binding.pressed_backend != 0
            && g_picoui_native_event_binding.pressed_backend->kind == PICOUI_BACKEND_WIDGET_SLIDER) {
            return picoui_native_event_dispatch_slider_drag(
                g_picoui_native_event_binding.pressed_backend,
                x);
        }
        return 0;
    }
    if (pointer_pressed) {
        g_picoui_native_event_binding.last_pointer_pressed = 1;
        g_picoui_native_event_binding.pressed_backend = input_backend;
        g_picoui_native_event_binding.press_x = x;
        g_picoui_native_event_binding.press_y = y;
        if (input_backend == 0) {
            return 0;
        }
        signal = SIGNAL_PRESS;
        if (input_backend->kind == PICOUI_BACKEND_WIDGET_CHECKBOX
            || input_backend->kind == PICOUI_BACKEND_WIDGET_SWITCH
            || input_backend->kind == PICOUI_BACKEND_WIDGET_COMBO_BOX
            || input_backend->kind == PICOUI_BACKEND_WIDGET_LIST
            || input_backend->kind == PICOUI_BACKEND_WIDGET_CALENDAR
            || input_backend->kind == PICOUI_BACKEND_WIDGET_RADIAL_MENU
            || input_backend->kind == PICOUI_BACKEND_WIDGET_SCROLL_SELECTER
            || input_backend->kind == PICOUI_BACKEND_WIDGET_TABLE) {
            if (input_backend->kind == PICOUI_BACKEND_WIDGET_COMBO_BOX) {
                struct picoui_combo_box *combo_box = (struct picoui_combo_box *)input_backend->host_widget;
                int slot;

                if (combo_box == 0) {
                    return -1;
                }
                slot = picoui_backend_combo_box_point_to_slot(input_backend, x, y);
                if (slot == 0 && !input_backend->open) {
                    return picoui_combo_box_apply_open_state(combo_box, 1);
                }
            }
            return 0;
        }
        if (input_backend->kind == PICOUI_BACKEND_WIDGET_MESSAGE_BOX) {
            return picoui_native_message_box_handle_press(
                (struct picoui_message_box *)input_backend->host_widget,
                x,
                y);
        }
        if (input_backend->kind == PICOUI_BACKEND_WIDGET_SLIDER) {
            return picoui_native_event_dispatch_slider_drag(input_backend, x);
        }
        return picoui_backend_widget_dispatch_native_signal(input_backend, signal, 0);
    }

    g_picoui_native_event_binding.last_pointer_pressed = 0;
    if (g_picoui_native_event_binding.pressed_backend == 0
        || input_backend != g_picoui_native_event_binding.pressed_backend) {
        g_picoui_native_event_binding.pressed_backend = 0;
        g_picoui_native_event_binding.press_x = 0;
        g_picoui_native_event_binding.press_y = 0;
        return 0;
    }

    g_picoui_native_event_binding.pressed_backend = 0;
    if (input_backend->kind == PICOUI_BACKEND_WIDGET_CHECKBOX) {
        struct picoui_checkbox *checkbox = (struct picoui_checkbox *)input_backend->host_widget;

        if (checkbox == 0) {
            return -1;
        }
        signal = SIGNAL_VALUE_CHANGED;
        return picoui_backend_widget_dispatch_native_signal(input_backend,
                                                            signal,
                                                            picoui_checkbox_is_checked(checkbox) == 0);
    }
    if (input_backend->kind == PICOUI_BACKEND_WIDGET_SWITCH) {
        struct picoui_switch *sw = (struct picoui_switch *)input_backend->host_widget;

        if (sw == 0) {
            return -1;
        }
        signal = SIGNAL_VALUE_CHANGED;
        return picoui_backend_widget_dispatch_native_signal(input_backend,
                                                            signal,
                                                            picoui_switch_is_checked(sw) == 0);
    }
    if (input_backend->kind == PICOUI_BACKEND_WIDGET_SLIDER) {
        return 0;
    }
    if (input_backend->kind == PICOUI_BACKEND_WIDGET_LIST) {
        return picoui_native_event_dispatch_list_click(input_backend, y);
    }
    if (input_backend->kind == PICOUI_BACKEND_WIDGET_RADIAL_MENU) {
        return picoui_native_event_dispatch_radial_menu_click(input_backend, x, y);
    }
    if (input_backend->kind == PICOUI_BACKEND_WIDGET_SCROLL_SELECTER) {
        return picoui_native_event_dispatch_scroll_selecter_release(input_backend, y);
    }
    if (input_backend->kind == PICOUI_BACKEND_WIDGET_TABLE) {
        return picoui_native_event_dispatch_table_click(input_backend, x, y);
    }
    if (input_backend->kind == PICOUI_BACKEND_WIDGET_COMBO_BOX) {
        return picoui_native_event_dispatch_combo_box_click(input_backend, x, y);
    }
    if (input_backend->kind == PICOUI_BACKEND_WIDGET_CALENDAR) {
        return picoui_native_event_dispatch_calendar_click(input_backend, x, y);
    }
    if (input_backend->kind == PICOUI_BACKEND_WIDGET_MESSAGE_BOX) {
        return picoui_native_message_box_handle_release(
            (struct picoui_message_box *)input_backend->host_widget,
            x,
            y);
    }

    signal = SIGNAL_RELEASE;
    return picoui_backend_widget_dispatch_native_signal(input_backend, signal, 0);
}

int picoui_native_event_timer_handler(void)
{
    return picoui_native_event_pump();
}

void picoui_native_event_deinit(void)
{
    picoui_native_event_reset();
}
int picoui_combo_box_apply_open_state(struct picoui_combo_box *combo_box, int is_open);
