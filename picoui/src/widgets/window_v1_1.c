#include "runtime_state.h"
#include "internal_v1_1.h"

#include "picoui/screen.h"
#include "picoui/window.h"

#include <stdlib.h>

struct picoui_window *picoui_window_create_root(struct picoui_screen *screen, const char *id)
{
    struct picoui_window *window;

    if (screen == 0 || id == 0) {
        return 0;
    }

    window = calloc(1, sizeof(*window));
    if (window == 0) {
        return 0;
    }

    window->id = id;
    window->widget.visible = 1;
    window->widget.enabled = 1;
    window->flex_flow = PICOUI_FLEX_FLOW_ROW;
    window->flex_main_align = PICOUI_ALIGN_START;
    window->flex_cross_align = PICOUI_ALIGN_START;
    window->flex_track_align = PICOUI_ALIGN_START;
    window->grid_col_align = PICOUI_ALIGN_START;
    window->grid_row_align = PICOUI_ALIGN_START;

    if (picoui_v1_1_widget_bind_root(screen, &window->widget) != 0
        || picoui_screen_set_root_window(screen, window) != 0) {
        picoui_v1_1_widget_unbind_root(&window->widget);
        free(window);
        return 0;
    }

    return window;
}
