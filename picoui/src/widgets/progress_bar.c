#include "internal.h"
#include "picoui/progress_bar.h"

#include <stdlib.h>

int picoui_backend_progress_bar_set_percent(struct picoui_progress_bar *bar, int percent);
int picoui_backend_progress_bar_get_percent(struct picoui_progress_bar *bar, int *percent);
int picoui_backend_progress_bar_set_horizontal(struct picoui_progress_bar *bar, int horizontal);
int picoui_backend_progress_bar_get_horizontal(struct picoui_progress_bar *bar, int *horizontal);
int picoui_backend_progress_bar_set_bg_source(void *backend_widget, struct picoui_image_source *source);
int picoui_backend_progress_bar_set_fg_source(void *backend_widget, struct picoui_image_source *source);
int picoui_backend_progress_bar_set_frame_source(void *backend_widget, struct picoui_image_source *source);
int picoui_backend_progress_bar_set_color(void *backend_widget, unsigned int bg_color, unsigned int fg_color);
int picoui_backend_progress_bar_set_frame_color(void *backend_widget,
                                                unsigned int frame_color,
                                                int frame_color_size);
int picoui_backend_progress_bar_set_inverted(void *backend_widget, int inverted);
int picoui_backend_progress_bar_get_inverted(void *backend_widget);

static int picoui_progress_bar_props_are_valid(const struct picoui_progress_bar_props *props)
{
    return props != 0
        && props->id != 0
        && props->percent >= 0
        && props->percent <= 100;
}

struct picoui_progress_bar *picoui_progress_bar_create(struct picoui_window *parent, const char *id)
{
    struct picoui_progress_bar *bar;

    if (parent == 0 || id == 0) {
        return 0;
    }

    bar = calloc(1, sizeof(*bar));
    if (bar == 0) {
        return 0;
    }

    bar->widget.backend_widget = picoui_backend_create_progress_bar(parent->widget.backend_widget, id);
    if (bar->widget.backend_widget == 0) {
        free(bar);
        return 0;
    }

    bar->id = id;
    bar->widget.visible = 1;
    bar->widget.enabled = 1;
    if (picoui_backend_widget_bind_host(bar->widget.backend_widget, &bar->widget) != 0) {
        free(bar);
        return 0;
    }
    if (picoui_progress_bar_set_percent(bar, 0) != 0
        || picoui_progress_bar_set_horizontal(bar, 0) != 0
        || picoui_progress_bar_set_inverted(bar, 0) != 0) {
        free(bar);
        return 0;
    }
    return bar;
}

struct picoui_progress_bar *picoui_progress_bar_init(struct picoui_window *parent, const char *id)
{
    return picoui_progress_bar_create(parent, id);
}

struct picoui_progress_bar *picoui_progress_bar_create_with_props(
    struct picoui_window *parent,
    const struct picoui_progress_bar_props *props)
{
    struct picoui_progress_bar *bar;

    if (!picoui_progress_bar_props_are_valid(props)) {
        return 0;
    }

    bar = picoui_progress_bar_create(parent, props->id);
    if (bar == 0) {
        return 0;
    }

    if (picoui_widget_set_user_data(&bar->widget, props->user_data) != 0) {
        free(bar);
        return 0;
    }
    if (props->style_class != 0
        && picoui_widget_set_style_class(&bar->widget, props->style_class) != 0) {
        free(bar);
        return 0;
    }
    if (picoui_progress_bar_set_percent(bar, props->percent) != 0
        || picoui_progress_bar_set_horizontal(bar, props->horizontal) != 0
        || picoui_progress_bar_set_inverted(bar, props->inverted) != 0) {
        free(bar);
        return 0;
    }

    return bar;
}

int picoui_progress_bar_set_percent(struct picoui_progress_bar *bar, int percent)
{
    if (bar == 0 || percent < 0 || percent > 100) {
        return -1;
    }

    if (picoui_backend_progress_bar_set_percent(bar, percent) != 0) {
        return -1;
    }

    bar->percent = percent;
    return 0;
}

int picoui_progress_bar_get_percent(const struct picoui_progress_bar *bar)
{
    int percent = 0;

    if (bar == 0) {
        return -1;
    }

    if (picoui_backend_progress_bar_get_percent((struct picoui_progress_bar *)bar, &percent) != 0) {
        return -1;
    }

    return percent;
}

int picoui_progress_bar_set_horizontal(struct picoui_progress_bar *bar, int horizontal)
{
    if (bar == 0) {
        return -1;
    }

    if (picoui_backend_progress_bar_set_horizontal(bar, horizontal != 0) != 0) {
        return -1;
    }

    bar->horizontal = horizontal != 0 ? 1 : 0;
    return 0;
}

int picoui_progress_bar_get_horizontal(const struct picoui_progress_bar *bar)
{
    int horizontal = 0;

    if (bar == 0) {
        return -1;
    }

    if (picoui_backend_progress_bar_get_horizontal((struct picoui_progress_bar *)bar, &horizontal) != 0) {
        return -1;
    }

    return horizontal;
}

int picoui_progress_bar_set_image(struct picoui_progress_bar *bar,
                                  struct picoui_image_source *bg_source,
                                  struct picoui_image_source *fg_source)
{
    if (picoui_progress_bar_set_bg_source(bar, bg_source) != 0) {
        return -1;
    }
    return picoui_progress_bar_set_fg_source(bar, fg_source);
}

int picoui_progress_bar_set_bg_source(struct picoui_progress_bar *bar, struct picoui_image_source *source)
{
    if (bar == 0 || source == 0 || source->img_tile == 0) {
        return -1;
    }

    if (picoui_backend_progress_bar_set_bg_source(bar->widget.backend_widget, source) != 0) {
        return -1;
    }

    bar->bg_source = source;
    return 0;
}

int picoui_progress_bar_set_fg_source(struct picoui_progress_bar *bar, struct picoui_image_source *source)
{
    if (bar == 0 || source == 0 || source->img_tile == 0) {
        return -1;
    }

    if (picoui_backend_progress_bar_set_fg_source(bar->widget.backend_widget, source) != 0) {
        return -1;
    }

    bar->fg_source = source;
    return 0;
}

int picoui_progress_bar_set_frame_source(struct picoui_progress_bar *bar, struct picoui_image_source *source)
{
    if (bar == 0 || source == 0 || source->img_tile == 0) {
        return -1;
    }

    if (picoui_backend_progress_bar_set_frame_source(bar->widget.backend_widget, source) != 0) {
        return -1;
    }

    bar->frame_source = source;
    return 0;
}

int picoui_progress_bar_set_color(struct picoui_progress_bar *bar, unsigned int bg_color, unsigned int fg_color)
{
    if (bar == 0 || bg_color > 0xFFFFFFU || fg_color > 0xFFFFFFU) {
        return -1;
    }

    if (picoui_backend_progress_bar_set_color(bar->widget.backend_widget, bg_color, fg_color) != 0) {
        return -1;
    }

    bar->bg_color = bg_color;
    bar->fg_color = fg_color;
    return 0;
}

int picoui_progress_bar_set_frame_color(struct picoui_progress_bar *bar,
                                        unsigned int frame_color,
                                        int frame_color_size)
{
    if (bar == 0 || frame_color > 0xFFFFFFU || frame_color_size < 0 || frame_color_size > 255) {
        return -1;
    }

    if (picoui_backend_progress_bar_set_frame_color(bar->widget.backend_widget,
                                                    frame_color,
                                                    frame_color_size) != 0) {
        return -1;
    }

    bar->frame_color = frame_color;
    bar->frame_color_size = frame_color_size;
    return 0;
}

int picoui_progress_bar_set_inverted(struct picoui_progress_bar *bar, int inverted)
{
    if (bar == 0) {
        return -1;
    }

    if (picoui_backend_progress_bar_set_inverted(bar->widget.backend_widget, inverted != 0) != 0) {
        return -1;
    }

    bar->inverted = inverted != 0 ? 1 : 0;
    return 0;
}

int picoui_progress_bar_get_inverted(const struct picoui_progress_bar *bar)
{
    if (bar == 0) {
        return -1;
    }

    return picoui_backend_progress_bar_get_inverted((void *)bar->widget.backend_widget);
}
