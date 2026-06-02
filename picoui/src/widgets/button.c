#include "internal.h"
#include "picoui/button.h"
#include "../backend/ldgui/backend.h"
#include "../../../src/gui/ldButton.h"
#include "../../../src/misc/xBtnAction.h"

#include <stdlib.h>

int picoui_backend_button_set_font(struct picoui_button *button, const struct picoui_font *font);
int picoui_backend_button_set_release_image(struct picoui_button *button,
                                            struct picoui_image_source *source);
int picoui_backend_button_set_press_image(struct picoui_button *button,
                                          struct picoui_image_source *source);
int picoui_backend_button_set_transparent(struct picoui_button *button, int transparent);
int picoui_backend_button_get_transparent(struct picoui_button *button, int *transparent);
int picoui_backend_button_set_checkable(struct picoui_button *button, int checkable);
int picoui_backend_button_get_checkable(struct picoui_button *button, int *checkable);
int picoui_backend_button_set_key_value(struct picoui_button *button, unsigned int key_value);
int picoui_backend_button_get_key_value(struct picoui_button *button, unsigned int *key_value);
int picoui_backend_button_set_pressed(struct picoui_button *button, int pressed);
int picoui_backend_button_get_pressed(struct picoui_button *button, int *pressed);

static ldButton_t *picoui_button_get_ld(const struct picoui_button *button)
{
    const struct picoui_backend_widget *backend;

    if (button == 0 || button->widget.backend_widget == 0) {
        return 0;
    }

    backend = (const struct picoui_backend_widget *)button->widget.backend_widget;
    return (ldButton_t *)backend->ld_widget;
}

static int picoui_button_props_are_valid(const struct picoui_button_props *props)
{
    return props != 0
        && props->id != 0
        && (props->release_image == 0 || props->release_image->img_tile != 0)
        && (props->press_image == 0 || props->press_image->img_tile != 0)
        && props->width >= 0
        && props->height >= 0
        && props->radius >= 0
        && props->padding >= 0;
}

static struct picoui_button *picoui_button_alloc(struct picoui_window *parent, const char *id)
{
    struct picoui_button *button;

    if (parent == 0 || id == 0) {
        return 0;
    }

    button = calloc(1, sizeof(*button));
    if (button == 0) {
        return 0;
    }

    button->widget.backend_widget = picoui_backend_create_button(parent->widget.backend_widget, id);
    if (button->widget.backend_widget == 0) {
        free(button);
        return 0;
    }

    button->id = id;
    button->widget.visible = 1;
    button->widget.enabled = 1;
    if (picoui_backend_widget_bind_host(button->widget.backend_widget, &button->widget) != 0) {
        free(button);
        return 0;
    }
    return button;
}

struct picoui_button *picoui_button_create(struct picoui_window *parent, const char *id)
{
    return picoui_button_alloc(parent, id);
}

struct picoui_button *picoui_button_init(struct picoui_window *parent, const char *id)
{
    return picoui_button_create(parent, id);
}

struct picoui_button *picoui_button_create_with_props(struct picoui_window *parent,
                                                      const struct picoui_button_props *props)
{
    struct picoui_button *button;

    if (!picoui_button_props_are_valid(props)) {
        return 0;
    }

    button = picoui_button_alloc(parent, props->id);
    if (button == 0) {
        return 0;
    }

    button->on_clicked = props->on_clicked;
    button->user_data = props->user_data;
    if (picoui_widget_set_user_data(&button->widget, props->user_data) != 0) {
        free(button);
        return 0;
    }
    if (props->text != 0 && picoui_button_set_text(button, props->text) != 0) {
        free(button);
        return 0;
    }
    if (props->font != 0 && picoui_button_set_font(button, props->font) != 0) {
        free(button);
        return 0;
    }
    if ((props->width > 0 || props->height > 0)
        && picoui_widget_set_size(&button->widget, props->width, props->height) != 0) {
        free(button);
        return 0;
    }
    if (props->style_class != 0
        && picoui_widget_set_style_class(&button->widget, props->style_class) != 0) {
        free(button);
        return 0;
    }
    if (picoui_widget_set_bg_color(&button->widget, props->bg_color) != 0
        || picoui_widget_set_text_color(&button->widget, props->text_color) != 0
        || picoui_widget_set_border_color(&button->widget, props->border_color) != 0
        || picoui_widget_set_radius(&button->widget, props->radius) != 0
        || picoui_widget_set_padding(&button->widget, props->padding) != 0) {
        free(button);
        return 0;
    }
    if (picoui_button_set_release_image(button, props->release_image) != 0
        || picoui_button_set_press_image(button, props->press_image) != 0
        || picoui_button_set_transparent(button, props->transparent) != 0
        || picoui_button_set_checkable(button, props->checkable) != 0
        || picoui_button_set_key_value(button, props->key_value) != 0
        || picoui_button_set_pressed(button, props->pressed) != 0) {
        free(button);
        return 0;
    }

    return button;
}

static int picoui_button_set_event(struct picoui_button *button,
                                   picoui_event_cb cb,
                                   void *user_data,
                                   int kind)
{
    if (button == 0) {
        return -1;
    }

    if (kind == 0) {
        button->on_pressed = cb;
        button->on_pressed_user_data = user_data;
    } else if (kind == 1) {
        button->on_released = cb;
        button->on_released_user_data = user_data;
    } else {
        return -1;
    }
    return 0;
}

int picoui_button_set_text(struct picoui_button *button, const char *text)
{
    if (button == 0 || text == 0) {
        return -1;
    }

    if (picoui_widget_set_text(&button->widget, text) != 0) {
        return -1;
    }
    return picoui_backend_set_text(button->widget.backend_widget, text);
}

int picoui_button_get_text(struct picoui_button *button, const char **text)
{
    ldButton_t *ld_button;

    if (button == 0 || text == 0) {
        return -1;
    }

    ld_button = picoui_button_get_ld(button);
    if (ld_button == 0) {
        return -1;
    }

    *text = (const char *)ldButtonGetText(ld_button);
    return 0;
}

int picoui_button_set_font(struct picoui_button *button, const struct picoui_font *font)
{
    if (button == 0) {
        return -1;
    }

    if (picoui_backend_button_set_font(button, font) != 0) {
        return -1;
    }

    button->widget.font = font;
    return 0;
}

int picoui_button_get_font(struct picoui_button *button, const struct picoui_font **font)
{
    if (button == 0 || font == 0) {
        return -1;
    }

    *font = button->widget.font;
    return 0;
}

int picoui_button_set_color(struct picoui_button *button,
                            unsigned int release_color,
                            unsigned int press_color)
{
    ldButton_t *ld_button;

    if (button == 0 || release_color > 0xFFFFFFU || press_color > 0xFFFFFFU) {
        return -1;
    }

    ld_button = picoui_button_get_ld(button);
    if (ld_button == 0) {
        return -1;
    }

    ldButtonSetColor(ld_button, (ldColor)release_color, (ldColor)press_color);
    button->widget.bg_color = release_color;
    button->widget.border_color = press_color;
    return 0;
}

int picoui_button_get_release_color(struct picoui_button *button, unsigned int *rgb)
{
    ldButton_t *ld_button;

    if (button == 0 || rgb == 0) {
        return -1;
    }

    ld_button = picoui_button_get_ld(button);
    if (ld_button == 0) {
        return -1;
    }

    *rgb = button->widget.bg_color;
    return 0;
}

int picoui_button_get_press_color(struct picoui_button *button, unsigned int *rgb)
{
    ldButton_t *ld_button;

    if (button == 0 || rgb == 0) {
        return -1;
    }

    ld_button = picoui_button_get_ld(button);
    if (ld_button == 0) {
        return -1;
    }

    *rgb = button->widget.border_color;
    return 0;
}

int picoui_button_set_release_image(struct picoui_button *button,
                                    struct picoui_image_source *source)
{
    if (button == 0 || (source != 0 && source->img_tile == 0)) {
        return -1;
    }

    return picoui_backend_button_set_release_image(button, source);
}

int picoui_button_set_press_image(struct picoui_button *button,
                                  struct picoui_image_source *source)
{
    if (button == 0 || (source != 0 && source->img_tile == 0)) {
        return -1;
    }

    return picoui_backend_button_set_press_image(button, source);
}

int picoui_button_set_image(struct picoui_button *button,
                            struct picoui_image_source *release_source,
                            struct picoui_image_source *press_source)
{
    if (picoui_button_set_release_image(button, release_source) != 0) {
        return -1;
    }
    return picoui_button_set_press_image(button, press_source);
}

int picoui_button_set_transparent(struct picoui_button *button, int transparent)
{
    if (button == 0) {
        return -1;
    }

    return picoui_backend_button_set_transparent(button, transparent != 0);
}

int picoui_button_get_transparent(struct picoui_button *button, int *transparent)
{
    if (button == 0 || transparent == 0) {
        return -1;
    }

    return picoui_backend_button_get_transparent(button, transparent);
}

int picoui_button_set_checkable(struct picoui_button *button, int checkable)
{
    if (button == 0) {
        return -1;
    }

    return picoui_backend_button_set_checkable(button, checkable != 0);
}

int picoui_button_get_checkable(struct picoui_button *button, int *checkable)
{
    if (button == 0 || checkable == 0) {
        return -1;
    }

    return picoui_backend_button_get_checkable(button, checkable);
}

int picoui_button_set_key_value(struct picoui_button *button, unsigned int key_value)
{
    if (button == 0) {
        return -1;
    }

    return picoui_backend_button_set_key_value(button, key_value);
}

int picoui_button_get_key_value(struct picoui_button *button, unsigned int *key_value)
{
    if (button == 0 || key_value == 0) {
        return -1;
    }

    return picoui_backend_button_get_key_value(button, key_value);
}

int picoui_button_set_pressed(struct picoui_button *button, int pressed)
{
    if (button == 0) {
        return -1;
    }

    return picoui_backend_button_set_pressed(button, pressed != 0);
}

int picoui_button_set_press(struct picoui_button *button, int pressed)
{
    return picoui_button_set_pressed(button, pressed);
}

int picoui_button_get_pressed(struct picoui_button *button, int *pressed)
{
    if (button == 0 || pressed == 0) {
        return -1;
    }

    return picoui_backend_button_get_pressed(button, pressed);
}

int picoui_button_get_press(struct picoui_button *button, int *pressed)
{
    return picoui_button_get_pressed(button, pressed);
}

int picoui_button_get_pressed_by_name_id(const struct picoui_widget *root,
                                         int name_id,
                                         int *pressed)
{
    struct picoui_widget *widget;

    if (root == 0 || pressed == 0) {
        return -1;
    }

    widget = picoui_widget_find_by_name_id(root, name_id);
    if (widget == 0 || picoui_widget_get_type(widget) != PICOUI_WIDGET_TYPE_BUTTON) {
        return -1;
    }

    return picoui_button_get_pressed((struct picoui_button *)widget, pressed);
}

int picoui_button_get_action_state_by_name_id(const struct picoui_widget *root,
                                              int name_id,
                                              enum picoui_button_action_state action)
{
    struct picoui_widget *widget;

    if (root == 0 || name_id < 0 || name_id > 65535) {
        return -1;
    }

    widget = picoui_widget_find_by_name_id(root, name_id);
    if (widget == 0 || picoui_widget_get_type(widget) != PICOUI_WIDGET_TYPE_BUTTON) {
        return -1;
    }

    return (int)xBtnGetState((uint16_t)name_id, (uint8_t)action);
}

int picoui_button_set_text_color(struct picoui_button *button, unsigned int text_color)
{
    ldButton_t *ld_button;

    if (button == 0 || text_color > 0xFFFFFFU) {
        return -1;
    }

    ld_button = picoui_button_get_ld(button);
    if (ld_button == 0) {
        return -1;
    }

    ldButtonSetTextColor(ld_button,
                         (ldColor)__RGB((text_color >> 16) & 0xFFU,
                                        (text_color >> 8) & 0xFFU,
                                        text_color & 0xFFU));
    button->widget.text_color = text_color;
    return 0;
}

int picoui_button_get_text_color(struct picoui_button *button, unsigned int *rgb)
{
    ldButton_t *ld_button;

    if (button == 0 || rgb == 0) {
        return -1;
    }

    ld_button = picoui_button_get_ld(button);
    if (ld_button == 0) {
        return -1;
    }

    *rgb = button->widget.text_color;
    return 0;
}

int picoui_button_set_on_clicked(struct picoui_button *button,
                                 picoui_event_cb cb,
                                 void *user_data)
{
    if (button == 0) {
        return -1;
    }

    button->on_clicked = cb;
    button->user_data = user_data;
    return 0;
}

int picoui_button_set_on_pressed(struct picoui_button *button,
                                 picoui_event_cb cb,
                                 void *user_data)
{
    return picoui_button_set_event(button, cb, user_data, 0);
}

int picoui_button_set_on_released(struct picoui_button *button,
                                  picoui_event_cb cb,
                                  void *user_data)
{
    return picoui_button_set_event(button, cb, user_data, 1);
}
