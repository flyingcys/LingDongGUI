#include "internal.h"
#include "picoui/image.h"

#include <stdlib.h>

static int picoui_image_props_are_valid(const struct picoui_image_props *props)
{
    return props != 0
        && props->id != 0
        && (props->source == 0 || props->source->img_tile != 0)
        && props->width >= 0
        && props->height >= 0
        && props->radius >= 0
        && props->padding >= 0;
}

struct picoui_image *picoui_image_create(struct picoui_window *parent, const char *id)
{
    struct picoui_image *image;

    if (parent == 0 || id == 0) {
        return 0;
    }

    image = calloc(1, sizeof(*image));
    if (image == 0) {
        return 0;
    }

    image->widget.backend_widget = picoui_backend_create_image(parent->widget.backend_widget, id);
    if (image->widget.backend_widget == 0) {
        free(image);
        return 0;
    }

    image->id = id;
    image->widget.visible = 1;
    image->widget.enabled = 1;
    return image;
}

struct picoui_image *picoui_image_create_with_props(struct picoui_window *parent,
                                                    const struct picoui_image_props *props)
{
    struct picoui_image *image;

    if (!picoui_image_props_are_valid(props)) {
        return 0;
    }

    image = picoui_image_create(parent, props->id);
    if (image == 0) {
        return 0;
    }

    if (props->source != 0 && picoui_image_set_source(image, props->source) != 0) {
        free(image);
        return 0;
    }
    if (props->style_class != 0
        && picoui_widget_set_style_class(&image->widget, props->style_class) != 0) {
        free(image);
        return 0;
    }
    if (picoui_widget_set_user_data(&image->widget, props->user_data) != 0
        || picoui_widget_set_bg_color(&image->widget, props->bg_color) != 0
        || picoui_widget_set_text_color(&image->widget, props->text_color) != 0
        || picoui_widget_set_border_color(&image->widget, props->border_color) != 0
        || picoui_widget_set_radius(&image->widget, props->radius) != 0
        || picoui_widget_set_padding(&image->widget, props->padding) != 0) {
        free(image);
        return 0;
    }
    if ((props->width > 0 || props->height > 0)
        && picoui_widget_set_size(&image->widget, props->width, props->height) != 0) {
        free(image);
        return 0;
    }

    return image;
}

int picoui_image_set_source(struct picoui_image *image, struct picoui_image_source *source)
{
    if (image == 0 || (source != 0 && source->img_tile == 0)) {
        return -1;
    }

    if (picoui_backend_set_image_source(image->widget.backend_widget, source) != 0) {
        return -1;
    }

    image->source = source;
    return 0;
}

int picoui_image_set_mask_color(struct picoui_image *image, unsigned int rgb)
{
    if (image == 0) {
        return -1;
    }

    if (picoui_backend_image_set_mask_color(image->widget.backend_widget, rgb) != 0) {
        return -1;
    }

    image->widget.bg_color = rgb;
    return 0;
}
