#ifndef PICOUI_BACKEND_LDGUI_H
#define PICOUI_BACKEND_LDGUI_H

#include "picoui/widget.h"

struct picoui_widget;
struct picoui_app;
struct picoui_theme;
struct picoui_window;
struct picoui_image_source;

struct picoui_backend_widget {
    void *parent;
    const char *id;
    const char *text;
};

int picoui_backend_apply_theme(struct picoui_app *app, struct picoui_theme *theme);
void *picoui_backend_create_window(struct picoui_app *app, const char *id);
void *picoui_backend_create_label(void *parent, const char *id);
void *picoui_backend_create_button(void *parent, const char *id);
void *picoui_backend_create_checkbox(void *parent, const char *id);
void *picoui_backend_create_switch(void *parent, const char *id);
void *picoui_backend_create_slider(void *parent, const char *id);
void *picoui_backend_create_text(void *parent, const char *id);
void *picoui_backend_create_image(void *parent, const char *id);
int picoui_backend_set_text(void *backend_widget, const char *text);
int picoui_backend_set_image_source(void *backend_widget, struct picoui_image_source *source);
int picoui_backend_window_set_flex_flow(struct picoui_window *window, enum picoui_flex_flow flow);
int picoui_backend_window_set_flex_align(struct picoui_window *window,
                                         enum picoui_align main_align,
                                         enum picoui_align cross_align,
                                         enum picoui_align track_align);
int picoui_backend_window_set_flex_gap(struct picoui_window *window, int item_gap, int track_gap);
int picoui_backend_window_set_grid_columns(struct picoui_window *window, const int *tracks, int count);
int picoui_backend_window_set_grid_rows(struct picoui_window *window, const int *tracks, int count);
int picoui_backend_window_set_grid_gap(struct picoui_window *window, int row_gap, int col_gap);
int picoui_backend_window_set_grid_align(struct picoui_window *window,
                                         enum picoui_align col_align,
                                         enum picoui_align row_align);
int picoui_backend_widget_set_flex_grow(struct picoui_widget *widget, int grow);
int picoui_backend_widget_set_flex_new_track(struct picoui_widget *widget, int new_track);
int picoui_backend_widget_set_ignore_layout(struct picoui_widget *widget, int ignore_layout);
int picoui_backend_widget_set_grid_cell(struct picoui_widget *widget,
                                        int col,
                                        int row,
                                        int col_span,
                                        int row_span,
                                        enum picoui_align x_align,
                                        enum picoui_align y_align);
void picoui_backend_emit_value_changed(picoui_value_changed_cb cb,
                                       struct picoui_widget *widget,
                                       int value,
                                       void *user_data);

#endif
