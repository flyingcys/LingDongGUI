#ifndef PICOUI_INTERNAL_V1_1_H
#define PICOUI_INTERNAL_V1_1_H

#include "picoui/font.h"
#include "picoui/layout.h"
#include "picoui/theme.h"
#include "picoui/widget.h"

struct picoui_widget {
    int visible;
    int enabled;
    int dirty;
    const char *text;
};

struct picoui_window {
    struct picoui_widget widget;
    const char *id;
    enum picoui_flex_flow flex_flow;
    enum picoui_align flex_main_align;
    enum picoui_align flex_cross_align;
    enum picoui_align flex_track_align;
    enum picoui_align grid_col_align;
    enum picoui_align grid_row_align;
};

struct picoui_label {
    struct picoui_widget widget;
    const char *id;
};

struct picoui_button {
    struct picoui_widget widget;
    const char *id;
    int pressed;
    picoui_event_cb on_clicked;
    void *clicked_user_data;
};

#endif
