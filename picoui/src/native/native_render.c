#include "../core/internal.h"
#include "picoui/screen.h"

struct picoui_native_screen_binding {
    struct picoui_screen *screen;
    struct picoui_window *root_window;
    int visited_widget_count;
    int label_count;
    int button_count;
};

static struct picoui_native_screen_binding g_picoui_native_render_binding;

int picoui_native_runtime_timer_handler(void);
void picoui_native_runtime_deinit(void);
int picoui_native_event_pump(void);
void picoui_native_event_deinit(void);
int picoui_native_checkbox_render(const struct picoui_backend_widget *backend);
int picoui_native_switch_render(const struct picoui_backend_widget *backend);
int picoui_native_slider_render(const struct picoui_backend_widget *backend);
int picoui_native_arc_render(const struct picoui_backend_widget *backend);
int picoui_native_gauge_render(const struct picoui_backend_widget *backend);
int picoui_native_calendar_render(const struct picoui_backend_widget *backend);
int picoui_native_clock_render(const struct picoui_backend_widget *backend);
int picoui_native_combo_box_render(const struct picoui_backend_widget *backend);
int picoui_native_icon_slider_render(const struct picoui_backend_widget *backend);
int picoui_native_graph_render(const struct picoui_backend_widget *backend);
int picoui_native_message_box_render(const struct picoui_backend_widget *backend);
int picoui_native_progress_bar_render(const struct picoui_backend_widget *backend);
int picoui_native_progress_wheel_render(const struct picoui_backend_widget *backend);
int picoui_native_qrcode_render(const struct picoui_backend_widget *backend);
int picoui_native_radial_menu_render(const struct picoui_backend_widget *backend);
int picoui_native_scroll_selecter_render(const struct picoui_backend_widget *backend);
int picoui_native_table_render(const struct picoui_backend_widget *backend);
int picoui_native_animation_render(const struct picoui_animation *animation);
int picoui_native_canvas_render(const struct picoui_canvas *canvas);
int picoui_native_text_render(const struct picoui_backend_widget *backend);
int picoui_native_line_edit_render(const struct picoui_backend_widget *backend);
int picoui_native_image_render(const struct picoui_backend_widget *backend);
int picoui_native_list_render(const struct picoui_backend_widget *backend);
int picoui_native_background_render(const struct picoui_backend_widget *backend);
int picoui_native_layout_apply_root(struct picoui_window *root_window);
int picoui_backend_native_render_capture(struct picoui_window *window);

static void picoui_native_render_reset(void)
{
    g_picoui_native_render_binding.screen = 0;
    g_picoui_native_render_binding.root_window = 0;
    g_picoui_native_render_binding.visited_widget_count = 0;
    g_picoui_native_render_binding.label_count = 0;
    g_picoui_native_render_binding.button_count = 0;
}

static int picoui_native_render_visit(const struct picoui_backend_widget *widget)
{
    while (widget != 0) {
        g_picoui_native_render_binding.visited_widget_count++;
        if (widget->kind == PICOUI_BACKEND_WIDGET_LABEL) {
            g_picoui_native_render_binding.label_count++;
        } else if (widget->kind == PICOUI_BACKEND_WIDGET_BUTTON) {
            g_picoui_native_render_binding.button_count++;
        } else if (widget->kind == PICOUI_BACKEND_WIDGET_BACKGROUND
                   && picoui_native_background_render(widget) != 0) {
            return -1;
        } else if (widget->kind == PICOUI_BACKEND_WIDGET_CHECKBOX
                   && picoui_native_checkbox_render(widget) != 0) {
            return -1;
        } else if (widget->kind == PICOUI_BACKEND_WIDGET_SWITCH
                   && picoui_native_switch_render(widget) != 0) {
            return -1;
        } else if (widget->kind == PICOUI_BACKEND_WIDGET_SLIDER
                   && picoui_native_slider_render(widget) != 0) {
            return -1;
        } else if (widget->kind == PICOUI_BACKEND_WIDGET_ARC
                   && picoui_native_arc_render(widget) != 0) {
            return -1;
        } else if (widget->kind == PICOUI_BACKEND_WIDGET_GAUGE
                   && picoui_native_gauge_render(widget) != 0) {
            return -1;
        } else if (widget->kind == PICOUI_BACKEND_WIDGET_CALENDAR
                   && picoui_native_calendar_render(widget) != 0) {
            return -1;
        } else if (widget->kind == PICOUI_BACKEND_WIDGET_CLOCK
                   && picoui_native_clock_render(widget) != 0) {
            return -1;
        } else if (widget->kind == PICOUI_BACKEND_WIDGET_COMBO_BOX
                   && picoui_native_combo_box_render(widget) != 0) {
            return -1;
        } else if (widget->kind == PICOUI_BACKEND_WIDGET_ICON_SLIDER
                   && picoui_native_icon_slider_render(widget) != 0) {
            return -1;
        } else if (widget->kind == PICOUI_BACKEND_WIDGET_GRAPH
                   && picoui_native_graph_render(widget) != 0) {
            return -1;
        } else if (widget->kind == PICOUI_BACKEND_WIDGET_MESSAGE_BOX
                   && picoui_native_message_box_render(widget) != 0) {
            return -1;
        } else if (widget->kind == PICOUI_BACKEND_WIDGET_PROGRESS_BAR
                   && picoui_native_progress_bar_render(widget) != 0) {
            return -1;
        } else if (widget->kind == PICOUI_BACKEND_WIDGET_PROGRESS_WHEEL
                   && picoui_native_progress_wheel_render(widget) != 0) {
            return -1;
        } else if (widget->kind == PICOUI_BACKEND_WIDGET_QRCODE
                   && picoui_native_qrcode_render(widget) != 0) {
            return -1;
        } else if (widget->kind == PICOUI_BACKEND_WIDGET_RADIAL_MENU
                   && picoui_native_radial_menu_render(widget) != 0) {
            return -1;
        } else if (widget->kind == PICOUI_BACKEND_WIDGET_SCROLL_SELECTER
                   && picoui_native_scroll_selecter_render(widget) != 0) {
            return -1;
        } else if (widget->kind == PICOUI_BACKEND_WIDGET_TABLE
                   && picoui_native_table_render(widget) != 0) {
            return -1;
        } else if (widget->kind == PICOUI_BACKEND_WIDGET_ANIMATION
                   && picoui_native_animation_render((const struct picoui_animation *)widget->host_widget) != 0) {
            return -1;
        } else if (widget->kind == PICOUI_BACKEND_WIDGET_CANVAS
                   && picoui_native_canvas_render((const struct picoui_canvas *)widget->host_widget) != 0) {
            return -1;
        } else if (widget->kind == PICOUI_BACKEND_WIDGET_TEXT) {
            if (widget->host_widget != 0
                && picoui_widget_get_type((const struct picoui_widget *)widget->host_widget)
                       == PICOUI_WIDGET_TYPE_LINE_EDIT) {
                if (picoui_native_line_edit_render(widget) != 0) {
                    return -1;
                }
            } else if (picoui_native_text_render(widget) != 0) {
                return -1;
            }
        } else if (widget->kind == PICOUI_BACKEND_WIDGET_IMAGE
                   && picoui_native_image_render(widget) != 0) {
            return -1;
        } else if (widget->kind == PICOUI_BACKEND_WIDGET_LIST
                   && picoui_native_list_render(widget) != 0) {
            return -1;
        }

        if (widget->first_child != 0 && picoui_native_render_visit(widget->first_child) != 0) {
            return -1;
        }

        widget = widget->next_sibling;
    }

    return 0;
}

int picoui_native_render_bind_root(struct picoui_screen *screen, struct picoui_window *root_window)
{
    if (screen == 0 || root_window == 0 || root_window->widget.backend_widget == 0) {
        return -1;
    }

    g_picoui_native_render_binding.screen = screen;
    g_picoui_native_render_binding.root_window = root_window;
    g_picoui_native_render_binding.visited_widget_count = 0;
    g_picoui_native_render_binding.label_count = 0;
    g_picoui_native_render_binding.button_count = 0;
    return 0;
}

int picoui_native_render_once(void *root_backend_widget)
{
    const struct picoui_backend_widget *root = (const struct picoui_backend_widget *)root_backend_widget;

    if (root == 0) {
        if (g_picoui_native_render_binding.screen == 0) {
            return 0;
        }
        if (picoui_screen_active() != g_picoui_native_render_binding.screen) {
            return -1;
        }

        if (g_picoui_native_render_binding.root_window == 0) {
            g_picoui_native_render_binding.root_window =
                picoui_screen_get_root_window(g_picoui_native_render_binding.screen);
            if (g_picoui_native_render_binding.root_window == 0) {
                return 0;
            }
        }

        root =
            (const struct picoui_backend_widget *)g_picoui_native_render_binding.root_window->widget.backend_widget;
        if (root == 0) {
            return -1;
        }
        if (picoui_native_layout_apply_root(g_picoui_native_render_binding.root_window) != 0) {
            return -1;
        }
    }

    if ((root->kind != PICOUI_BACKEND_WIDGET_WINDOW
         && root->kind != PICOUI_BACKEND_WIDGET_BACKGROUND)
        || root->parent != 0) {
        return -1;
    }

    g_picoui_native_render_binding.visited_widget_count = 0;
    g_picoui_native_render_binding.label_count = 0;
    g_picoui_native_render_binding.button_count = 0;
    return picoui_native_render_visit(root);
}

void picoui_native_core_deinit_impl(void)
{
    picoui_native_render_reset();
    picoui_native_event_deinit();
    picoui_native_runtime_deinit();
}

int picoui_native_core_timer_handler_impl(void)
{
    int rc;

    rc = picoui_native_runtime_timer_handler();

    if (rc < 0) {
        return rc;
    }

    if (picoui_native_render_once(0) != 0) {
        return -1;
    }

    if (picoui_native_event_pump() != 0) {
        return -1;
    }

    if (g_picoui_native_render_binding.root_window == 0
        && g_picoui_native_render_binding.screen != 0) {
        g_picoui_native_render_binding.root_window =
            picoui_screen_get_root_window(g_picoui_native_render_binding.screen);
    }
    if (g_picoui_native_render_binding.root_window != 0
        && picoui_backend_native_render_capture(g_picoui_native_render_binding.root_window) != 0) {
        return -1;
    }

    return rc;
}
