#include "internal.h"
#include "backend.h"
#include "picoui/animation.h"

#include <stdlib.h>

static int picoui_animation_props_are_valid(const struct picoui_animation_props *props)
{
    return props != 0
        && props->id != 0
        && props->width > 0
        && props->height > 0
        && props->period_ms > 0
        && props->source != 0
        && props->source->img_tile != 0;
}

struct picoui_animation *picoui_animation_create(struct picoui_widget *parent, const char *id)
{
    struct picoui_animation *animation;

    if (parent == 0 || id == 0 || parent->backend_widget == 0) {
        return 0;
    }

    animation = calloc(1, sizeof(*animation));
    if (animation == 0) {
        return 0;
    }

    animation->id = id;
    animation->widget.visible = 1;
    animation->widget.enabled = 1;
    return animation;
}

struct picoui_animation *picoui_animation_init(struct picoui_widget *parent, const char *id)
{
    return picoui_animation_create(parent, id);
}

struct picoui_animation *picoui_animation_create_with_props(
    struct picoui_widget *parent,
    const struct picoui_animation_props *props)
{
    struct picoui_animation *animation;

    if (!picoui_animation_props_are_valid(props) || parent == 0 || parent->backend_widget == 0) {
        return 0;
    }

    animation = calloc(1, sizeof(*animation));
    if (animation == 0) {
        return 0;
    }

    animation->widget.backend_widget = picoui_backend_create_animation(parent->backend_widget,
                                                                       props->id,
                                                                       props->width,
                                                                       props->height,
                                                                       props->source,
                                                                       props->period_ms);
    if (animation->widget.backend_widget == 0) {
        free(animation);
        return 0;
    }

    animation->id = props->id;
    animation->width = props->width;
    animation->height = props->height;
    animation->period_ms = props->period_ms;
    animation->source = props->source;
    animation->widget.width = props->width;
    animation->widget.height = props->height;
    animation->widget.visible = 1;
    animation->widget.enabled = 1;

    if (picoui_backend_widget_bind_host(animation->widget.backend_widget, &animation->widget) != 0) {
        free(animation);
        return 0;
    }
    if (picoui_widget_set_user_data(&animation->widget, props->user_data) != 0) {
        free(animation);
        return 0;
    }
    if (props->style_class != 0 &&
        picoui_widget_set_style_class(&animation->widget, props->style_class) != 0) {
        free(animation);
        return 0;
    }

    return animation;
}

int picoui_animation_set_source(struct picoui_animation *animation, struct picoui_image_source *source)
{
    if (animation == 0 || source == 0 || source->img_tile == 0) {
        return -1;
    }

    if (picoui_backend_animation_set_source(animation, source) != 0) {
        return -1;
    }

    animation->source = source;
    return 0;
}

int picoui_animation_set_period_ms(struct picoui_animation *animation, int period_ms)
{
    if (animation == 0 || period_ms <= 0) {
        return -1;
    }

    if (picoui_backend_animation_set_period_ms(animation, period_ms) != 0) {
        return -1;
    }

    animation->period_ms = period_ms;
    return 0;
}

int picoui_animation_show_frame(struct picoui_animation *animation, int frame_index)
{
    if (animation == 0 || frame_index < 0) {
        return -1;
    }

    return picoui_backend_animation_show_frame(animation, frame_index);
}
