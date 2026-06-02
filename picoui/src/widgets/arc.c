#include "internal.h"
#include "picoui/arc.h"
#include "picoui/widget.h"

#include <stdlib.h>

static int picoui_arc_props_are_valid(const struct picoui_arc_props *props)
{
    return props != 0
        && props->id != 0
        && props->bg_start_angle >= 0.0f
        && props->bg_end_angle >= props->bg_start_angle
        && props->fg_end_angle >= 0.0f
        && props->rotation_angle >= 0.0f;
}

struct picoui_arc *picoui_arc_create(struct picoui_widget *parent, const char *id)
{
    struct picoui_arc *arc;

    if (parent == 0 || id == 0 || parent->backend_widget == 0) {
        return 0;
    }

    arc = calloc(1, sizeof(*arc));
    if (arc == 0) {
        return 0;
    }

    arc->widget.backend_widget = picoui_backend_create_arc(parent->backend_widget, id);
    if (arc->widget.backend_widget == 0) {
        free(arc);
        return 0;
    }

    arc->id = id;
    arc->widget.visible = 1;
    arc->widget.enabled = 1;
    arc->bg_end_angle = 360.0f;
    if (picoui_backend_widget_bind_host(arc->widget.backend_widget, &arc->widget) != 0
        || picoui_arc_set_background_angle(arc, 0.0f, 360.0f) != 0
        || picoui_arc_set_foreground_angle(arc, 0.0f) != 0
        || picoui_arc_set_rotation_angle(arc, 0.0f) != 0
        || picoui_arc_set_color(arc, 0xFFFFFF, 0xADD8E6) != 0) {
        free(arc);
        return 0;
    }
    return arc;
}

struct picoui_arc *picoui_arc_init(struct picoui_widget *parent, const char *id)
{
    return picoui_arc_create(parent, id);
}

struct picoui_arc *picoui_arc_create_with_props(struct picoui_widget *parent,
                                                const struct picoui_arc_props *props)
{
    struct picoui_arc *arc;

    if (!picoui_arc_props_are_valid(props)) {
        return 0;
    }

    arc = picoui_arc_create(parent, props->id);
    if (arc == 0) {
        return 0;
    }

    if ((props->style_class != 0
         && picoui_widget_set_style_class(&arc->widget, props->style_class) != 0)
        || picoui_widget_set_user_data(&arc->widget, props->user_data) != 0
        || picoui_arc_set_background_angle(arc, props->bg_start_angle, props->bg_end_angle) != 0
        || picoui_arc_set_foreground_angle(arc, props->fg_end_angle) != 0
        || picoui_arc_set_rotation_angle(arc, props->rotation_angle) != 0
        || (props->quarter_source != 0
            && picoui_arc_set_quarter_source(arc, props->quarter_source) != 0)
        || picoui_arc_set_parent_color(arc, props->parent_color) != 0
        || picoui_arc_set_color(arc, props->bg_color, props->fg_color) != 0) {
        free(arc);
        return 0;
    }

    return arc;
}

int picoui_arc_set_background_angle(struct picoui_arc *arc, float bg_start_angle, float bg_end_angle)
{
    if (arc == 0 || bg_start_angle < 0.0f || bg_end_angle < bg_start_angle) {
        return -1;
    }
    if (picoui_backend_arc_set_background_angle(arc, bg_start_angle, bg_end_angle) != 0) {
        return -1;
    }
    arc->bg_start_angle = bg_start_angle;
    arc->bg_end_angle = bg_end_angle;
    return 0;
}

int picoui_arc_set_foreground_angle(struct picoui_arc *arc, float fg_end_angle)
{
    if (arc == 0 || fg_end_angle < 0.0f) {
        return -1;
    }
    if (picoui_backend_arc_set_foreground_angle(arc, fg_end_angle) != 0) {
        return -1;
    }
    arc->fg_end_angle = fg_end_angle;
    return 0;
}

int picoui_arc_set_rotation_angle(struct picoui_arc *arc, float rotation_angle)
{
    if (arc == 0 || rotation_angle < 0.0f) {
        return -1;
    }
    if (picoui_backend_arc_set_rotation_angle(arc, rotation_angle) != 0) {
        return -1;
    }
    arc->rotation_angle = rotation_angle;
    return 0;
}

int picoui_arc_set_color(struct picoui_arc *arc, unsigned int bg_color, unsigned int fg_color)
{
    if (arc == 0) {
        return -1;
    }
    if (picoui_backend_arc_set_color(arc, bg_color, fg_color) != 0) {
        return -1;
    }
    arc->bg_color = bg_color;
    arc->fg_color = fg_color;
    return 0;
}

int picoui_arc_set_quarter_source(struct picoui_arc *arc, struct picoui_image_source *source)
{
    if (arc == 0 || source == 0 || source->img_tile == 0 || source->mask_tile == 0) {
        return -1;
    }

    if (picoui_backend_arc_set_quarter_source(arc, source) != 0) {
        return -1;
    }

    arc->quarter_source = source;
    return 0;
}

int picoui_arc_set_parent_color(struct picoui_arc *arc, unsigned int parent_color)
{
    if (arc == 0 || parent_color > 0xFFFFFFU) {
        return -1;
    }

    if (picoui_backend_arc_set_parent_color(arc, parent_color) != 0) {
        return -1;
    }

    arc->parent_color = parent_color;
    return 0;
}

float picoui_arc_get_background_start_angle(const struct picoui_arc *arc)
{
    float bg_start_angle = 0.0f;
    float bg_angle = 0.0f;

    if (arc == 0) {
        return 0.0f;
    }
    if (picoui_backend_arc_get_background_angle((struct picoui_arc *)arc, &bg_start_angle, &bg_angle) != 0) {
        return 0.0f;
    }
    return bg_start_angle;
}

float picoui_arc_get_background_angle(const struct picoui_arc *arc)
{
    float bg_start_angle = 0.0f;
    float bg_end_angle = 0.0f;

    if (arc == 0) {
        return 0.0f;
    }
    if (picoui_backend_arc_get_background_angle((struct picoui_arc *)arc, &bg_start_angle, &bg_end_angle) != 0) {
        return 0.0f;
    }
    return bg_end_angle - bg_start_angle;
}

float picoui_arc_get_foreground_angle(const struct picoui_arc *arc)
{
    float fg_end_angle = 0.0f;

    if (arc == 0) {
        return 0.0f;
    }
    if (picoui_backend_arc_get_foreground_angle((struct picoui_arc *)arc, &fg_end_angle) != 0) {
        return 0.0f;
    }
    return fg_end_angle;
}

float picoui_arc_get_rotation_angle(const struct picoui_arc *arc)
{
    float rotation_angle = 0.0f;

    if (arc == 0) {
        return 0.0f;
    }
    if (picoui_backend_arc_get_rotation_angle((struct picoui_arc *)arc, &rotation_angle) != 0) {
        return 0.0f;
    }
    return rotation_angle;
}

unsigned int picoui_arc_get_background_color(const struct picoui_arc *arc)
{
    unsigned int bg_color = 0;
    unsigned int fg_color = 0;

    if (arc == 0) {
        return 0;
    }
    if (picoui_backend_arc_get_color((struct picoui_arc *)arc, &bg_color, &fg_color) != 0) {
        return 0;
    }
    return bg_color;
}

unsigned int picoui_arc_get_foreground_color(const struct picoui_arc *arc)
{
    unsigned int bg_color = 0;
    unsigned int fg_color = 0;

    if (arc == 0) {
        return 0;
    }
    if (picoui_backend_arc_get_color((struct picoui_arc *)arc, &bg_color, &fg_color) != 0) {
        return 0;
    }
    return fg_color;
}
