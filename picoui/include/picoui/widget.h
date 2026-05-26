#ifndef PICOUI_WIDGET_H
#define PICOUI_WIDGET_H

#include "picoui/layout.h"

struct picoui_widget;

typedef void (*picoui_value_changed_cb)(struct picoui_widget *widget,
                                        int value,
                                        void *user_data);
typedef void (*picoui_event_cb)(struct picoui_widget *widget, void *user_data);

int picoui_widget_set_pos(struct picoui_widget *widget, int x, int y);
int picoui_widget_set_size(struct picoui_widget *widget, int width, int height);
int picoui_widget_set_visible(struct picoui_widget *widget, int visible);
int picoui_widget_set_enabled(struct picoui_widget *widget, int enabled);
int picoui_widget_set_flex_grow(struct picoui_widget *widget, int grow);
int picoui_widget_set_flex_new_track(struct picoui_widget *widget, int new_track);
int picoui_widget_set_ignore_layout(struct picoui_widget *widget, int ignore_layout);
int picoui_widget_set_grid_cell(struct picoui_widget *widget,
                                int col,
                                int row,
                                int col_span,
                                int row_span,
                                enum picoui_align x_align,
                                enum picoui_align y_align);

#endif
