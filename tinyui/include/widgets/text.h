#ifndef TINYUI_TEXT_H
#define TINYUI_TEXT_H

struct tinyui_window;
struct tinyui_image_source;
struct tinyui_text;

struct tinyui_text_props {
    const char *id;
    const char *text;
    const struct tinyui_font *font;
    const char *style_class;
    void *user_data;
    int width;
    int height;
    unsigned int bg_color;
    unsigned int text_color;
    unsigned int border_color;
    int radius;
    int padding;
};

struct tinyui_text *tinyui_text_create(struct tinyui_window *parent, const char *id);

struct tinyui_text *tinyui_text_create_with_props(struct tinyui_window *parent,
                                                  const struct tinyui_text_props *props);

int tinyui_text_set_text(struct tinyui_text *text, const char *value);

int tinyui_text_set_static_text(struct tinyui_text *text, const char *value);

int tinyui_text_set_font(struct tinyui_text *text, const struct tinyui_font *font);

int tinyui_text_set_transparent(struct tinyui_text *text, int transparent);

int tinyui_text_set_text_color(struct tinyui_text *text, unsigned int rgb);

int tinyui_text_set_bg_color(struct tinyui_text *text, unsigned int rgb);

int tinyui_text_set_background_source(struct tinyui_text *text,
                                      struct tinyui_image_source *source);

int tinyui_text_set_consumed_font(struct tinyui_text *text, const struct tinyui_font *font);

int tinyui_text_set_scroll_enabled(struct tinyui_text *text, int enabled);

int tinyui_text_scroll_seek(struct tinyui_text *text, int offset);

int tinyui_text_scroll_move(struct tinyui_text *text, int move_value);

#endif
