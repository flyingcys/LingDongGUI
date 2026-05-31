#include "internal.h"
#include "picoui/progress_bar.h"

#include <stdlib.h>

int picoui_backend_progress_bar_set_percent(struct picoui_progress_bar *bar, int percent);
int picoui_backend_progress_bar_get_percent(struct picoui_progress_bar *bar, int *percent);
int picoui_backend_progress_bar_set_horizontal(struct picoui_progress_bar *bar, int horizontal);
int picoui_backend_progress_bar_get_horizontal(struct picoui_progress_bar *bar, int *horizontal);

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
        || picoui_progress_bar_set_horizontal(bar, 0) != 0) {
        free(bar);
        return 0;
    }
    return bar;
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
        || picoui_progress_bar_set_horizontal(bar, props->horizontal) != 0) {
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
