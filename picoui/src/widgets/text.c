#include "internal.h"
#include "picoui/text.h"

#include <stdlib.h>

static int picoui_text_props_are_valid(const struct picoui_text_props *props)
{
    return props != 0
        && props->id != 0
        && props->width >= 0
        && props->height >= 0
        && props->radius >= 0
        && props->padding >= 0;
}

struct picoui_text *picoui_text_create(struct picoui_window *parent, const char *id)
{
    struct picoui_text *text;

    if (parent == 0 || id == 0) {
        return 0;
    }

    text = calloc(1, sizeof(*text));
    if (text == 0) {
        return 0;
    }

    text->widget.backend_widget = picoui_backend_create_text(parent->widget.backend_widget, id);
    if (text->widget.backend_widget == 0) {
        free(text);
        return 0;
    }

    text->id = id;
    text->widget.visible = 1;
    text->widget.enabled = 1;
    return text;
}

struct picoui_text *picoui_text_create_with_props(struct picoui_window *parent,
                                                  const struct picoui_text_props *props)
{
    struct picoui_text *text;

    if (!picoui_text_props_are_valid(props)) {
        return 0;
    }

    text = picoui_text_create(parent, props->id);
    if (text == 0) {
        return 0;
    }

    if (props->text != 0 && picoui_text_set_text(text, props->text) != 0) {
        free(text);
        return 0;
    }
    if (props->font != 0 && picoui_text_set_font(text, props->font) != 0) {
        free(text);
        return 0;
    }
    if (props->style_class != 0
        && picoui_widget_set_style_class(&text->widget, props->style_class) != 0) {
        free(text);
        return 0;
    }
    if (picoui_widget_set_user_data(&text->widget, props->user_data) != 0
        || picoui_widget_set_bg_color(&text->widget, props->bg_color) != 0
        || picoui_widget_set_text_color(&text->widget, props->text_color) != 0
        || picoui_widget_set_border_color(&text->widget, props->border_color) != 0
        || picoui_widget_set_radius(&text->widget, props->radius) != 0
        || picoui_widget_set_padding(&text->widget, props->padding) != 0) {
        free(text);
        return 0;
    }
    if ((props->width > 0 || props->height > 0)
        && picoui_widget_set_size(&text->widget, props->width, props->height) != 0) {
        free(text);
        return 0;
    }

    return text;
}

int picoui_text_set_text(struct picoui_text *text, const char *value)
{
    if (text == 0 || value == 0) {
        return -1;
    }

    if (picoui_widget_set_text(&text->widget, value) != 0) {
        return -1;
    }
    return picoui_backend_set_text(text->widget.backend_widget, value);
}

int picoui_text_set_static_text(struct picoui_text *text, const char *value)
{
    if (text == 0 || value == 0) {
        return -1;
    }

    if (picoui_backend_text_set_static_text(text->widget.backend_widget, value) != 0) {
        return -1;
    }
    text->widget.text = value;
    return 0;
}

int picoui_text_set_font(struct picoui_text *text, const struct picoui_font *font)
{
    if (text == 0) {
        return -1;
    }

    if (picoui_backend_widget_set_font(text->widget.backend_widget, font) != 0) {
        return -1;
    }

    text->widget.font = font;
    return 0;
}

int picoui_text_set_transparent(struct picoui_text *text, int transparent)
{
    if (text == 0) {
        return -1;
    }

    return picoui_backend_text_set_transparent(text->widget.backend_widget, transparent);
}

int picoui_text_set_text_color(struct picoui_text *text, unsigned int rgb)
{
    if (text == 0) {
        return -1;
    }

    if (picoui_backend_text_set_text_color(text->widget.backend_widget, rgb) != 0) {
        return -1;
    }
    text->widget.text_color = rgb;
    return 0;
}

int picoui_text_set_bg_color(struct picoui_text *text, unsigned int rgb)
{
    if (text == 0) {
        return -1;
    }

    if (picoui_backend_text_set_bg_color(text->widget.backend_widget, rgb) != 0) {
        return -1;
    }
    text->widget.bg_color = rgb;
    return 0;
}

int picoui_text_set_background_source(struct picoui_text *text,
                                      struct picoui_image_source *source)
{
    if (text == 0 || (source != 0 && source->img_tile == 0)) {
        return -1;
    }

    return picoui_backend_text_set_background_source(text->widget.backend_widget, source);
}

int picoui_text_set_consumed_font(struct picoui_text *text, const struct picoui_font *font)
{
    return picoui_text_set_font(text, font);
}

int picoui_text_scroll_seek(struct picoui_text *text, int offset)
{
    if (text == 0) {
        return -1;
    }

    return picoui_backend_text_scroll_seek(text->widget.backend_widget, offset);
}

int picoui_text_scroll_move(struct picoui_text *text, int move_value)
{
    if (text == 0) {
        return -1;
    }

    return picoui_backend_text_scroll_move(text->widget.backend_widget, move_value);
}
