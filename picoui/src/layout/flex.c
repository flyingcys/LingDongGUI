#include "internal.h"
#include "picoui/layout.h"

static int picoui_window_is_valid(struct picoui_window *window)
{
    return window != 0;
}

int picoui_flex_set_flow(struct picoui_window *window, enum picoui_flex_flow flow)
{
    if (!picoui_window_is_valid(window)) {
        return -1;
    }

    window->flex_flow = flow;
    return picoui_backend_window_set_flex_flow(window, flow);
}

int picoui_flex_set_align(struct picoui_window *window,
                          enum picoui_align main_align,
                          enum picoui_align cross_align,
                          enum picoui_align track_align)
{
    if (!picoui_window_is_valid(window)) {
        return -1;
    }

    window->flex_main_align = main_align;
    window->flex_cross_align = cross_align;
    window->flex_track_align = track_align;
    return picoui_backend_window_set_flex_align(window, main_align, cross_align, track_align);
}

int picoui_flex_set_gap(struct picoui_window *window, int item_gap, int track_gap)
{
    if (!picoui_window_is_valid(window) || item_gap < 0 || track_gap < 0) {
        return -1;
    }

    window->flex_item_gap = item_gap;
    window->flex_track_gap = track_gap;
    return picoui_backend_window_set_flex_gap(window, item_gap, track_gap);
}
