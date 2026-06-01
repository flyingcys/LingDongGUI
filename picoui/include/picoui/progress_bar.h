#ifndef PICOUI_PROGRESS_BAR_H
#define PICOUI_PROGRESS_BAR_H

#include "picoui/widget.h"

struct picoui_widget;
struct picoui_progress_bar;
struct picoui_image_source;

struct picoui_progress_bar_props {
    const char *id;
    const char *style_class;
    void *user_data;
    int percent;
    int horizontal;
    int inverted;
};

struct picoui_window;

struct picoui_progress_bar *picoui_progress_bar_create(struct picoui_window *parent, const char *id);
struct picoui_progress_bar *picoui_progress_bar_create_with_props(
    struct picoui_window *parent,
    const struct picoui_progress_bar_props *props);
int picoui_progress_bar_set_percent(struct picoui_progress_bar *bar, int percent);
int picoui_progress_bar_get_percent(const struct picoui_progress_bar *bar);
int picoui_progress_bar_set_horizontal(struct picoui_progress_bar *bar, int horizontal);
int picoui_progress_bar_get_horizontal(const struct picoui_progress_bar *bar);
int picoui_progress_bar_set_bg_source(struct picoui_progress_bar *bar, struct picoui_image_source *source);
int picoui_progress_bar_set_fg_source(struct picoui_progress_bar *bar, struct picoui_image_source *source);
int picoui_progress_bar_set_frame_source(struct picoui_progress_bar *bar, struct picoui_image_source *source);
int picoui_progress_bar_set_color(struct picoui_progress_bar *bar, unsigned int bg_color, unsigned int fg_color);
int picoui_progress_bar_set_frame_color(struct picoui_progress_bar *bar,
                                        unsigned int frame_color,
                                        int frame_color_size);
int picoui_progress_bar_set_inverted(struct picoui_progress_bar *bar, int inverted);
int picoui_progress_bar_get_inverted(const struct picoui_progress_bar *bar);

#endif
