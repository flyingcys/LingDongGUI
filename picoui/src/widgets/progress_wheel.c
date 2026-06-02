#include "internal.h"
#include "picoui/progress_wheel.h"
#include "picoui/widget.h"

#include <stdlib.h>

int picoui_backend_progress_wheel_set_percent(struct picoui_progress_wheel *wheel, int percent);
int picoui_backend_progress_wheel_get_percent(struct picoui_progress_wheel *wheel, int *percent);
int picoui_backend_progress_wheel_set_wheel_color(void *backend_widget, unsigned int rgb);
int picoui_backend_progress_wheel_set_dot_color(void *backend_widget, unsigned int rgb);
int picoui_backend_progress_wheel_set_dot_enabled(void *backend_widget, int enabled);
int picoui_backend_progress_wheel_get_dot_enabled(void *backend_widget);

static int picoui_progress_wheel_props_are_valid(const struct picoui_progress_wheel_props *props)
{
    return props != 0 && props->id != 0 && props->percent >= 0 && props->percent <= 100;
}

struct picoui_progress_wheel *picoui_progress_wheel_create(struct picoui_widget *parent, const char *id)
{
    struct picoui_progress_wheel *wheel;

    if (parent == 0 || id == 0 || parent->backend_widget == 0) {
        return 0;
    }

    wheel = calloc(1, sizeof(*wheel));
    if (wheel == 0) {
        return 0;
    }

    wheel->widget.backend_widget = picoui_backend_create_progress_wheel(parent->backend_widget, id);
    if (wheel->widget.backend_widget == 0) {
        free(wheel);
        return 0;
    }

    wheel->id = id;
    wheel->widget.visible = 1;
    wheel->widget.enabled = 1;
    if (picoui_backend_widget_bind_host(wheel->widget.backend_widget, &wheel->widget) != 0) {
        free(wheel);
        return 0;
    }
    if (picoui_progress_wheel_set_percent(wheel, 0) != 0
        || picoui_progress_wheel_set_dot_enabled(wheel, 1) != 0) {
        free(wheel);
        return 0;
    }
    return wheel;
}

struct picoui_progress_wheel *picoui_progress_wheel_init(struct picoui_widget *parent, const char *id)
{
    return picoui_progress_wheel_create(parent, id);
}

struct picoui_progress_wheel *picoui_progress_wheel_create_with_props(
    struct picoui_widget *parent,
    const struct picoui_progress_wheel_props *props)
{
    struct picoui_progress_wheel *wheel;

    if (!picoui_progress_wheel_props_are_valid(props)) {
        return 0;
    }

    wheel = picoui_progress_wheel_create(parent, props->id);
    if (wheel == 0) {
        return 0;
    }

    if (props->style_class != 0
        && picoui_widget_set_style_class(&wheel->widget, props->style_class) != 0) {
        free(wheel);
        return 0;
    }
    if (picoui_widget_set_user_data(&wheel->widget, props->user_data) != 0
        || picoui_progress_wheel_set_percent(wheel, props->percent) != 0) {
        free(wheel);
        return 0;
    }

    return wheel;
}

int picoui_progress_wheel_set_percent(struct picoui_progress_wheel *wheel, int percent)
{
    if (wheel == 0 || percent < 0 || percent > 100) {
        return -1;
    }

    if (picoui_backend_progress_wheel_set_percent(wheel, percent) != 0) {
        return -1;
    }

    wheel->percent = percent;
    return 0;
}

int picoui_progress_wheel_set_progress(struct picoui_progress_wheel *wheel, int percent)
{
    return picoui_progress_wheel_set_percent(wheel, percent);
}

int picoui_progress_wheel_get_percent(const struct picoui_progress_wheel *wheel)
{
    int percent = 0;

    if (wheel == 0) {
        return -1;
    }

    if (picoui_backend_progress_wheel_get_percent((struct picoui_progress_wheel *)wheel, &percent) != 0) {
        return -1;
    }

    return percent;
}

int picoui_progress_wheel_set_wheel_color(struct picoui_progress_wheel *wheel, unsigned int rgb)
{
    if (wheel == 0 || rgb > 0xFFFFFFU) {
        return -1;
    }

    if (picoui_backend_progress_wheel_set_wheel_color(wheel->widget.backend_widget, rgb) != 0) {
        return -1;
    }

    wheel->wheel_color = rgb;
    return 0;
}

int picoui_progress_wheel_set_dot_color(struct picoui_progress_wheel *wheel, unsigned int rgb)
{
    if (wheel == 0 || rgb > 0xFFFFFFU) {
        return -1;
    }

    if (picoui_backend_progress_wheel_set_dot_color(wheel->widget.backend_widget, rgb) != 0) {
        return -1;
    }

    wheel->dot_color = rgb;
    return 0;
}

int picoui_progress_wheel_set_dot_enabled(struct picoui_progress_wheel *wheel, int enabled)
{
    if (wheel == 0) {
        return -1;
    }

    if (picoui_backend_progress_wheel_set_dot_enabled(wheel->widget.backend_widget, enabled != 0) != 0) {
        return -1;
    }

    wheel->dot_enabled = enabled != 0 ? 1 : 0;
    return 0;
}

int picoui_progress_wheel_get_dot_enabled(const struct picoui_progress_wheel *wheel)
{
    if (wheel == 0) {
        return -1;
    }

    return picoui_backend_progress_wheel_get_dot_enabled((void *)wheel->widget.backend_widget);
}
