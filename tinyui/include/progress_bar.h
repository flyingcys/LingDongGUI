#ifndef TINYUI_PROGRESS_BAR_H
#define TINYUI_PROGRESS_BAR_H

struct tinyui_widget;
struct tinyui_progress_bar;
struct tinyui_image_source;
struct tinyui_window;

struct tinyui_progress_bar_props {
    const char *id;
    const char *style_class;
    void *user_data;
    int percent;
    int horizontal;
    int inverted;
};

struct tinyui_progress_bar *tinyui_progress_bar_create(struct tinyui_window *parent, const char *id);

struct tinyui_progress_bar *tinyui_progress_bar_create_with_props(
    struct tinyui_window *parent,
    const struct tinyui_progress_bar_props *props);

struct tinyui_progress_bar *tinyui_progress_bar_init(struct tinyui_window *parent, const char *id);

int tinyui_progress_bar_set_percent(struct tinyui_progress_bar *bar, int percent);

int tinyui_progress_bar_get_percent(const struct tinyui_progress_bar *bar);

int tinyui_progress_bar_set_horizontal(struct tinyui_progress_bar *bar, int horizontal);

int tinyui_progress_bar_get_horizontal(const struct tinyui_progress_bar *bar);

int tinyui_progress_bar_set_image(struct tinyui_progress_bar *bar,
                                  struct tinyui_image_source *bg_source,
                                  struct tinyui_image_source *fg_source);

int tinyui_progress_bar_set_bg_source(struct tinyui_progress_bar *bar, struct tinyui_image_source *source);

int tinyui_progress_bar_set_fg_source(struct tinyui_progress_bar *bar, struct tinyui_image_source *source);

int tinyui_progress_bar_set_frame_source(struct tinyui_progress_bar *bar, struct tinyui_image_source *source);

int tinyui_progress_bar_set_color(struct tinyui_progress_bar *bar, unsigned int bg_color, unsigned int fg_color);

int tinyui_progress_bar_set_frame_color(struct tinyui_progress_bar *bar,
                                        unsigned int frame_color,
                                        int frame_color_size);

int tinyui_progress_bar_set_inverted(struct tinyui_progress_bar *bar, int inverted);

int tinyui_progress_bar_get_inverted(const struct tinyui_progress_bar *bar);

#endif
