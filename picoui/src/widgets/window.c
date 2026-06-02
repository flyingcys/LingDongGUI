#include "internal.h"
#include "picoui/widget.h"
#include "picoui/window.h"

#include <stdlib.h>

struct picoui_image_source;

int picoui_backend_window_set_background_source(struct picoui_window *window,
                                                struct picoui_image_source *source);
int picoui_backend_window_set_bg_color(struct picoui_window *window, unsigned int rgb);
int picoui_backend_window_get_bg_color(struct picoui_window *window, unsigned int *rgb);
int picoui_backend_window_set_padding_group(struct picoui_window *window,
                                            int left,
                                            int top,
                                            int right,
                                            int bottom);
int picoui_backend_window_get_padding_left(struct picoui_window *window);
int picoui_backend_window_get_padding_top(struct picoui_window *window);
int picoui_backend_window_get_padding_right(struct picoui_window *window);
int picoui_backend_window_get_padding_bottom(struct picoui_window *window);
int picoui_backend_window_set_layout_type(struct picoui_window *window,
                                          enum picoui_window_layout_type type);
int picoui_backend_window_set_padding(struct picoui_window *window,
                                      int left,
                                      int top,
                                      int right,
                                      int bottom);
int picoui_backend_window_set_grid_padding(struct picoui_window *window,
                                           int left,
                                           int top,
                                           int right,
                                           int bottom);
int picoui_backend_window_set_gap(struct picoui_window *window, int gap);

static int picoui_window_is_valid(struct picoui_window *window)
{
    return window != 0 && window->widget.backend_widget != 0;
}

static int picoui_window_props_are_valid(const struct picoui_window_props *props)
{
    return props != 0
        && props->id != 0
        && props->radius >= 0
        && props->padding >= 0
        && props->padding_left >= 0
        && props->padding_top >= 0
        && props->padding_right >= 0
        && props->padding_bottom >= 0;
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
    if (picoui_backend_widget_bind_host(window->widget.backend_widget, &window->widget) != 0) {
        free(window);
        return 0;
    }
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
        || picoui_backend_window_set_bg_color(window, props->bg_color) != 0
        || picoui_widget_set_text_color(&window->widget, props->text_color) != 0
        || picoui_widget_set_border_color(&window->widget, props->border_color) != 0
        || picoui_widget_set_radius(&window->widget, props->radius) != 0
        || picoui_widget_set_padding(&window->widget, props->padding) != 0
        || picoui_window_set_background_source(window, props->background_source) != 0
        || (props->has_padding_group != 0
            && picoui_window_set_padding_group(window,
                                               props->padding_left,
                                               props->padding_top,
                                               props->padding_right,
                                               props->padding_bottom) != 0)) {
        free(window);
        return 0;
    }

    return window;
}

int picoui_window_set_background_source(struct picoui_window *window,
                                        struct picoui_image_source *source)
{
    if (!picoui_window_is_valid(window)) {
        return -1;
    }

    return picoui_backend_window_set_background_source(window, source);
}

int picoui_window_set_color(struct picoui_window *window, unsigned int rgb)
{
    if (!picoui_window_is_valid(window) || rgb > 0xFFFFFFU) {
        return -1;
    }

    window->widget.bg_color = rgb;
    return picoui_backend_window_set_bg_color(window, rgb);
}

int picoui_window_get_color(struct picoui_window *window, unsigned int *rgb)
{
    if (!picoui_window_is_valid(window) || rgb == 0) {
        return -1;
    }

    return picoui_backend_window_get_bg_color(window, rgb);
}

int picoui_window_set_padding_group(struct picoui_window *window,
                                    int left,
                                    int top,
                                    int right,
                                    int bottom)
{
    if (!picoui_window_is_valid(window)
        || left < 0
        || top < 0
        || right < 0
        || bottom < 0) {
        return -1;
    }

    return picoui_backend_window_set_padding_group(window, left, top, right, bottom);
}

int picoui_window_set_layout_type(struct picoui_window *window,
                                  enum picoui_window_layout_type type)
{
    if (!picoui_window_is_valid(window)
        || (type != PICOUI_WINDOW_LAYOUT_NONE
            && type != PICOUI_WINDOW_LAYOUT_FLEX
            && type != PICOUI_WINDOW_LAYOUT_GRID)) {
        return -1;
    }

    return picoui_backend_window_set_layout_type(window, type);
}

int picoui_window_set_padding(struct picoui_window *window,
                              int left,
                              int top,
                              int right,
                              int bottom)
{
    if (!picoui_window_is_valid(window)
        || left < 0
        || top < 0
        || right < 0
        || bottom < 0) {
        return -1;
    }

    return picoui_backend_window_set_padding(window, left, top, right, bottom);
}

int picoui_window_set_grid_padding(struct picoui_window *window,
                                   int left,
                                   int top,
                                   int right,
                                   int bottom)
{
    if (!picoui_window_is_valid(window)
        || left < 0
        || top < 0
        || right < 0
        || bottom < 0) {
        return -1;
    }

    return picoui_backend_window_set_grid_padding(window, left, top, right, bottom);
}

int picoui_window_set_gap(struct picoui_window *window, int gap)
{
    if (!picoui_window_is_valid(window) || gap < 0) {
        return -1;
    }

    if (picoui_backend_window_set_gap(window, gap) != 0) {
        return -1;
    }
    window->flex_item_gap = gap;
    window->flex_track_gap = gap;
    return 0;
}

int picoui_window_get_padding_left(struct picoui_window *window)
{
    if (!picoui_window_is_valid(window)) {
        return -1;
    }
    return picoui_backend_window_get_padding_left(window);
}

int picoui_window_get_padding_top(struct picoui_window *window)
{
    if (!picoui_window_is_valid(window)) {
        return -1;
    }
    return picoui_backend_window_get_padding_top(window);
}

int picoui_window_get_padding_right(struct picoui_window *window)
{
    if (!picoui_window_is_valid(window)) {
        return -1;
    }
    return picoui_backend_window_get_padding_right(window);
}

int picoui_window_get_padding_bottom(struct picoui_window *window)
{
    if (!picoui_window_is_valid(window)) {
        return -1;
    }
    return picoui_backend_window_get_padding_bottom(window);
}
