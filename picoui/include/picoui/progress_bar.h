#ifndef PICOUI_PROGRESS_BAR_H
#define PICOUI_PROGRESS_BAR_H

#include "picoui/widget.h"

struct picoui_widget;
struct picoui_progress_bar;

struct picoui_progress_bar_props {
    const char *id;
    const char *style_class;
    void *user_data;
    int percent;
    int horizontal;
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

#endif
