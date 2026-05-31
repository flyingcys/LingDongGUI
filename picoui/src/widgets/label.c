#include "internal.h"
#include "picoui/label.h"

#include <stdlib.h>

struct picoui_image_source;

int picoui_backend_label_set_text_color(struct picoui_label *label, unsigned int rgb);
int picoui_backend_label_get_text_color(struct picoui_label *label, unsigned int *rgb);
int picoui_backend_label_set_bg_color(struct picoui_label *label, unsigned int rgb);
int picoui_backend_label_get_bg_color(struct picoui_label *label, unsigned int *rgb);
int picoui_backend_label_set_transparent(struct picoui_label *label, int transparent);
int picoui_backend_label_get_transparent(struct picoui_label *label, int *transparent);
int picoui_backend_label_set_align(struct picoui_label *label, enum picoui_align align);
int picoui_backend_label_get_align(struct picoui_label *label, enum picoui_align *align);
int picoui_backend_label_set_background_source(struct picoui_label *label,
                                               struct picoui_image_source *source);
const char *picoui_backend_label_get_text(struct picoui_label *label);

static int picoui_label_props_are_valid(const struct picoui_label_props *props)
{
    return props != 0
        && props->id != 0
        && props->width >= 0
        && props->height >= 0
        && props->radius >= 0
        && props->padding >= 0;
}

struct picoui_label *picoui_label_create(struct picoui_window *parent, const char *id)
{
    struct picoui_label *label;

    if (parent == 0 || id == 0) {
        return 0;
    }

    label = calloc(1, sizeof(*label));
    if (label == 0) {
        return 0;
    }

    label->widget.backend_widget = picoui_backend_create_label(parent->widget.backend_widget, id);
    if (label->widget.backend_widget == 0) {
        free(label);
        return 0;
    }

    label->id = id;
    label->widget.visible = 1;
    label->widget.enabled = 1;
    return label;
}

struct picoui_label *picoui_label_create_with_props(struct picoui_window *parent,
                                                    const struct picoui_label_props *props)
{
    struct picoui_label *label;

    if (!picoui_label_props_are_valid(props)) {
        return 0;
    }

    label = picoui_label_create(parent, props->id);
    if (label == 0) {
        return 0;
    }

    if (props->text != 0 && picoui_label_set_text(label, props->text) != 0) {
        free(label);
        return 0;
    }
    if (props->font != 0 && picoui_label_set_font(label, props->font) != 0) {
        free(label);
        return 0;
    }
    if (props->style_class != 0
        && picoui_widget_set_style_class(&label->widget, props->style_class) != 0) {
        free(label);
        return 0;
    }
    if (picoui_widget_set_user_data(&label->widget, props->user_data) != 0
        || picoui_widget_set_border_color(&label->widget, props->border_color) != 0
        || picoui_widget_set_radius(&label->widget, props->radius) != 0
        || picoui_widget_set_padding(&label->widget, props->padding) != 0) {
        free(label);
        return 0;
    }
    if ((props->width > 0 || props->height > 0)
        && picoui_widget_set_size(&label->widget, props->width, props->height) != 0) {
        free(label);
        return 0;
    }
    if (picoui_label_set_bg_color(label, props->bg_color) != 0
        || picoui_label_set_text_color(label, props->text_color) != 0) {
        free(label);
        return 0;
    }

    return label;
}

int picoui_label_set_text(struct picoui_label *label, const char *text)
{
    if (label == 0 || text == 0) {
        return -1;
    }

    if (picoui_widget_set_text(&label->widget, text) != 0) {
        return -1;
    }
    return picoui_backend_set_text(label->widget.backend_widget, text);
}

const char *picoui_label_get_text(struct picoui_label *label)
{
    if (label == 0 || label->widget.backend_widget == 0) {
        return 0;
    }

    return picoui_backend_label_get_text(label);
}

int picoui_label_set_font(struct picoui_label *label, const struct picoui_font *font)
{
    if (label == 0) {
        return -1;
    }

    label->widget.font = font;
    return picoui_backend_widget_set_font(label->widget.backend_widget, font);
}

int picoui_label_set_text_color(struct picoui_label *label, unsigned int rgb)
{
    if (label == 0 || picoui_widget_set_text_color(&label->widget, rgb) != 0) {
        return -1;
    }

    return picoui_backend_label_set_text_color(label, rgb);
}

int picoui_label_get_text_color(struct picoui_label *label, unsigned int *rgb)
{
    if (label == 0 || rgb == 0) {
        return -1;
    }

    return picoui_backend_label_get_text_color(label, rgb);
}

int picoui_label_set_bg_color(struct picoui_label *label, unsigned int rgb)
{
    if (label == 0 || picoui_widget_set_bg_color(&label->widget, rgb) != 0) {
        return -1;
    }

    return picoui_backend_label_set_bg_color(label, rgb);
}

int picoui_label_get_bg_color(struct picoui_label *label, unsigned int *rgb)
{
    if (label == 0 || rgb == 0) {
        return -1;
    }

    return picoui_backend_label_get_bg_color(label, rgb);
}

int picoui_label_set_transparent(struct picoui_label *label, int transparent)
{
    if (label == 0) {
        return -1;
    }

    return picoui_backend_label_set_transparent(label, transparent != 0);
}

int picoui_label_get_transparent(struct picoui_label *label, int *transparent)
{
    if (label == 0 || transparent == 0) {
        return -1;
    }

    return picoui_backend_label_get_transparent(label, transparent);
}

int picoui_label_set_align(struct picoui_label *label, enum picoui_align align)
{
    if (label == 0) {
        return -1;
    }

    return picoui_backend_label_set_align(label, align);
}

int picoui_label_get_align(struct picoui_label *label, enum picoui_align *align)
{
    if (label == 0 || align == 0) {
        return -1;
    }

    return picoui_backend_label_get_align(label, align);
}

int picoui_label_set_background_source(struct picoui_label *label,
                                       struct picoui_image_source *source)
{
    if (label == 0 || (source != 0 && source->img_tile == 0)) {
        return -1;
    }

    return picoui_backend_label_set_background_source(label, source);
}
