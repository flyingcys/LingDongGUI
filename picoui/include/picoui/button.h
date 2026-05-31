#ifndef PICOUI_BUTTON_H
#define PICOUI_BUTTON_H

#include "picoui/widget.h"

struct picoui_window;
struct picoui_button;
struct picoui_image_source;

struct picoui_button_props {
    const char *id;
    const char *text;
    const struct picoui_font *font;
    struct picoui_image_source *release_image;
    struct picoui_image_source *press_image;
    int transparent;
    int checkable;
    unsigned int key_value;
    int pressed;
    int width;
    int height;
    picoui_event_cb on_clicked;
    void *user_data;
    const char *style_class;
    unsigned int bg_color;
    unsigned int text_color;
    unsigned int border_color;
    int radius;
    int padding;
};

struct picoui_button *picoui_button_create(struct picoui_window *parent, const char *id);
struct picoui_button *picoui_button_create_with_props(struct picoui_window *parent,
                                                      const struct picoui_button_props *props);
int picoui_button_set_text(struct picoui_button *button, const char *text);
int picoui_button_set_font(struct picoui_button *button, const struct picoui_font *font);
int picoui_button_set_release_image(struct picoui_button *button,
                                    struct picoui_image_source *source);
int picoui_button_set_press_image(struct picoui_button *button,
                                  struct picoui_image_source *source);
int picoui_button_set_transparent(struct picoui_button *button, int transparent);
int picoui_button_get_transparent(struct picoui_button *button, int *transparent);
int picoui_button_set_checkable(struct picoui_button *button, int checkable);
int picoui_button_get_checkable(struct picoui_button *button, int *checkable);
int picoui_button_set_key_value(struct picoui_button *button, unsigned int key_value);
int picoui_button_get_key_value(struct picoui_button *button, unsigned int *key_value);
int picoui_button_set_pressed(struct picoui_button *button, int pressed);
int picoui_button_get_pressed(struct picoui_button *button, int *pressed);
int picoui_button_set_on_clicked(struct picoui_button *button,
                                 picoui_event_cb cb,
                                 void *user_data);
int picoui_button_set_on_pressed(struct picoui_button *button,
                                 picoui_event_cb cb,
                                 void *user_data);
int picoui_button_set_on_released(struct picoui_button *button,
                                  picoui_event_cb cb,
                                  void *user_data);

#endif
