#include "internal.h"
#include "picoui/widget.h"
#include "picoui/window.h"

#include <stdlib.h>

static int picoui_window_props_are_valid(const struct picoui_window_props *props)
{
    return props != 0
        && props->id != 0
        && props->radius >= 0
        && props->padding >= 0;
}

struct picoui_window *picoui_window_create(struct picoui_app *app, const char *id)
{
    struct picoui_window *window;
    void *backend_widget;

    if (app == 0 || id == 0) {
        return 0;
    }

    backend_widget = picoui_backend_create_window(app, id);
    if (backend_widget == 0) {
        return 0;
    }

    window = calloc(1, sizeof(*window));
    if (window == 0) {
        free(backend_widget);
        return 0;
    }

    window->id = id;
    window->widget.backend_widget = backend_widget;
    window->widget.visible = 1;
    window->widget.enabled = 1;
    window->flex_flow = PICOUI_FLEX_FLOW_ROW;
    window->flex_main_align = PICOUI_ALIGN_START;
    window->flex_cross_align = PICOUI_ALIGN_START;
    window->flex_track_align = PICOUI_ALIGN_START;
    window->grid_col_align = PICOUI_ALIGN_START;
    window->grid_row_align = PICOUI_ALIGN_START;
    return window;
}

struct picoui_window *picoui_window_create_with_props(struct picoui_app *app,
                                                      const struct picoui_window_props *props)
{
    struct picoui_window *window;

    if (!picoui_window_props_are_valid(props)) {
        return 0;
    }

    window = picoui_window_create(app, props->id);
    if (window == 0) {
        return 0;
    }

    if (props->style_class != 0
        && picoui_widget_set_style_class(&window->widget, props->style_class) != 0) {
        free(window);
        return 0;
    }
    if (picoui_widget_set_user_data(&window->widget, props->user_data) != 0
        || picoui_widget_set_bg_color(&window->widget, props->bg_color) != 0
        || picoui_widget_set_text_color(&window->widget, props->text_color) != 0
        || picoui_widget_set_border_color(&window->widget, props->border_color) != 0
        || picoui_widget_set_radius(&window->widget, props->radius) != 0
        || picoui_widget_set_padding(&window->widget, props->padding) != 0) {
        free(window);
        return 0;
    }

    return window;
}
