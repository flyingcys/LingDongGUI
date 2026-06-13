#ifndef TINYUI_CHECKBOX_H
#define TINYUI_CHECKBOX_H

struct tinyui_window;
struct tinyui_checkbox;
struct tinyui_image_source;

struct tinyui_checkbox_props {
    const char *id;
    const char *text;
    int checked;
    tinyui_value_changed_cb on_toggled;
    void *user_data;
    const char *style_class;
    int width;
    int height;
    unsigned int bg_color;
    unsigned int text_color;
    unsigned int border_color;
    int radius;
    int padding;
    unsigned int check_color;
    struct tinyui_image_source *unchecked_source;
    struct tinyui_image_source *checked_source;
    int radio_group;
    int string_left_space;
    int has_check_color;
    int has_unchecked_source;
    int has_checked_source;
    int has_radio_group;
    int has_string_left_space;
};

struct tinyui_checkbox *tinyui_checkbox_create(struct tinyui_window *parent, const char *id);

struct tinyui_checkbox *tinyui_checkbox_create_with_props(struct tinyui_window *parent,
                                                          const struct tinyui_checkbox_props *props);

int tinyui_checkbox_set_checked(struct tinyui_checkbox *checkbox, int checked);

int tinyui_checkbox_is_checked(struct tinyui_checkbox *checkbox);

int tinyui_checkbox_set_text(struct tinyui_checkbox *checkbox, const char *text);

int tinyui_checkbox_set_check_color(struct tinyui_checkbox *checkbox, unsigned int rgb);

int tinyui_checkbox_set_text_color(struct tinyui_checkbox *checkbox, unsigned int rgb);

int tinyui_checkbox_set_unchecked_source(struct tinyui_checkbox *checkbox,
                                         struct tinyui_image_source *source);

int tinyui_checkbox_set_checked_source(struct tinyui_checkbox *checkbox,
                                       struct tinyui_image_source *source);

int tinyui_checkbox_set_radio_group(struct tinyui_checkbox *checkbox, int radio_group);

int tinyui_checkbox_set_string_left_space(struct tinyui_checkbox *checkbox, int space);

int tinyui_checkbox_set_on_toggled(struct tinyui_checkbox *checkbox,
                                   tinyui_value_changed_cb cb,
                                   void *user_data);

#endif
