#include "internal.h"
#include "picoui/gauge.h"
#include "picoui/widget.h"

#include <stdlib.h>

int picoui_backend_gauge_set_angle(struct picoui_gauge *gauge, float angle);
int picoui_backend_gauge_get_angle(struct picoui_gauge *gauge, float *angle);
int picoui_backend_gauge_set_pointer_color(struct picoui_gauge *gauge, unsigned int pointer_color);
int picoui_backend_gauge_get_pointer_color(struct picoui_gauge *gauge, unsigned int *pointer_color);
int picoui_backend_gauge_set_auto_move(struct picoui_gauge *gauge, int auto_move);
int picoui_backend_gauge_get_auto_move(struct picoui_gauge *gauge, int *auto_move);

static int picoui_gauge_props_are_valid(const struct picoui_gauge_props *props)
{
    return props != 0 && props->id != 0;
}

struct picoui_gauge *picoui_gauge_create(struct picoui_widget *parent, const char *id)
{
    struct picoui_gauge *gauge;

    if (parent == 0 || id == 0 || parent->backend_widget == 0) {
        return 0;
    }

    gauge = calloc(1, sizeof(*gauge));
    if (gauge == 0) {
        return 0;
    }

    gauge->widget.backend_widget = picoui_backend_create_gauge(parent->backend_widget, id);
    if (gauge->widget.backend_widget == 0) {
        free(gauge);
        return 0;
    }

    gauge->id = id;
    gauge->widget.visible = 1;
    gauge->widget.enabled = 1;
    if (picoui_backend_widget_bind_host(gauge->widget.backend_widget, &gauge->widget) != 0
        || picoui_gauge_set_angle(gauge, 0.0f) != 0
        || picoui_gauge_set_pointer_color(gauge, 0x000000) != 0
        || picoui_gauge_set_auto_move(gauge, 0) != 0) {
        free(gauge);
        return 0;
    }
    return gauge;
}

struct picoui_gauge *picoui_gauge_create_with_props(struct picoui_widget *parent,
                                                    const struct picoui_gauge_props *props)
{
    struct picoui_gauge *gauge;

    if (!picoui_gauge_props_are_valid(props)) {
        return 0;
    }

    gauge = picoui_gauge_create(parent, props->id);
    if (gauge == 0) {
        return 0;
    }

    if ((props->style_class != 0
         && picoui_widget_set_style_class(&gauge->widget, props->style_class) != 0)
        || picoui_widget_set_user_data(&gauge->widget, props->user_data) != 0
        || picoui_gauge_set_angle(gauge, props->angle) != 0
        || picoui_gauge_set_pointer_color(gauge, props->pointer_color) != 0
        || picoui_gauge_set_auto_move(gauge, props->auto_move) != 0) {
        free(gauge);
        return 0;
    }

    return gauge;
}

int picoui_gauge_set_angle(struct picoui_gauge *gauge, float angle)
{
    if (gauge == 0) {
        return -1;
    }
    if (picoui_backend_gauge_set_angle(gauge, angle) != 0) {
        return -1;
    }
    gauge->angle = angle;
    return 0;
}

float picoui_gauge_get_angle(const struct picoui_gauge *gauge)
{
    float angle = 0.0f;

    if (gauge == 0) {
        return 0.0f;
    }
    if (picoui_backend_gauge_get_angle((struct picoui_gauge *)gauge, &angle) != 0) {
        return 0.0f;
    }
    return angle;
}

int picoui_gauge_set_pointer_color(struct picoui_gauge *gauge, unsigned int pointer_color)
{
    if (gauge == 0) {
        return -1;
    }
    if (picoui_backend_gauge_set_pointer_color(gauge, pointer_color) != 0) {
        return -1;
    }
    gauge->pointer_color = pointer_color;
    return 0;
}

unsigned int picoui_gauge_get_pointer_color(const struct picoui_gauge *gauge)
{
    unsigned int pointer_color = 0;

    if (gauge == 0) {
        return 0;
    }
    if (picoui_backend_gauge_get_pointer_color((struct picoui_gauge *)gauge, &pointer_color) != 0) {
        return 0;
    }
    return pointer_color;
}

int picoui_gauge_set_auto_move(struct picoui_gauge *gauge, int auto_move)
{
    if (gauge == 0) {
        return -1;
    }
    if (picoui_backend_gauge_set_auto_move(gauge, auto_move != 0) != 0) {
        return -1;
    }
    gauge->auto_move = auto_move != 0 ? 1 : 0;
    return 0;
}

int picoui_gauge_get_auto_move(const struct picoui_gauge *gauge)
{
    int auto_move = 0;

    if (gauge == 0) {
        return -1;
    }
    if (picoui_backend_gauge_get_auto_move((struct picoui_gauge *)gauge, &auto_move) != 0) {
        return -1;
    }
    return auto_move;
}
