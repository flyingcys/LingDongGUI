#include "internal.h"
#include "backend.h"
#include "picoui/gauge.h"
#include "picoui/widget.h"

#include <stdlib.h>

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

struct picoui_gauge *picoui_gauge_init(struct picoui_widget *parent, const char *id)
{
    return picoui_gauge_create(parent, id);
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
        || (props->bg_source != 0 && picoui_gauge_set_bg_source(gauge, props->bg_source) != 0)
        || (props->pointer_source != 0 && picoui_gauge_set_pointer_source(gauge, props->pointer_source) != 0)
        || picoui_gauge_set_centre_offset(gauge, props->centre_offset_x, props->centre_offset_y) != 0
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

int picoui_gauge_set_bg_source(struct picoui_gauge *gauge, struct picoui_image_source *source)
{
    if (gauge == 0 || source == 0 || source->img_tile == 0 || source->mask_tile == 0) {
        return -1;
    }
    if (picoui_backend_gauge_set_bg_source(gauge, source) != 0) {
        return -1;
    }
    gauge->bg_source = source;
    return 0;
}

int picoui_gauge_set_pointer_source(struct picoui_gauge *gauge, struct picoui_image_source *source)
{
    if (gauge == 0 || source == 0 || source->img_tile == 0 || source->mask_tile == 0) {
        return -1;
    }
    if (picoui_backend_gauge_set_pointer_source(gauge, source) != 0) {
        return -1;
    }
    gauge->pointer_source = source;
    return 0;
}

int picoui_gauge_set_centre_offset(struct picoui_gauge *gauge, int centre_offset_x, int centre_offset_y)
{
    if (gauge == 0) {
        return -1;
    }
    if (picoui_backend_gauge_set_centre_offset(gauge, centre_offset_x, centre_offset_y) != 0) {
        return -1;
    }
    gauge->centre_offset_x = centre_offset_x;
    gauge->centre_offset_y = centre_offset_y;
    return 0;
}

int picoui_gauge_set_trail(struct picoui_gauge *gauge,
                           struct picoui_image_source *bg_trail_source,
                           struct picoui_image_source *pointer_trail_source)
{
    if (gauge == 0
        || bg_trail_source == 0
        || pointer_trail_source == 0
        || bg_trail_source->mask_tile == 0
        || pointer_trail_source->mask_tile == 0) {
        return -1;
    }

    if (picoui_backend_gauge_set_trail(gauge, bg_trail_source, pointer_trail_source) != 0) {
        return -1;
    }

    return 0;
}

int picoui_gauge_set_progress_bar(struct picoui_gauge *gauge,
                                  struct picoui_image_source *bg_progress_source,
                                  struct picoui_image_source *pointer_progress_source)
{
    if (gauge == 0
        || bg_progress_source == 0
        || pointer_progress_source == 0
        || bg_progress_source->mask_tile == 0
        || pointer_progress_source->mask_tile == 0) {
        return -1;
    }

    if (picoui_backend_gauge_set_progress_bar(gauge, bg_progress_source, pointer_progress_source) != 0) {
        return -1;
    }

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
