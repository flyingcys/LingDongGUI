#include "picoui/picoui.h"

static void make_ui(struct picoui_window *win)
{
    picoui_flex_set_flow(win, PICOUI_FLEX_FLOW_ROW_WRAP);
    picoui_flex_set_align(win,
                          PICOUI_ALIGN_START,
                          PICOUI_ALIGN_CENTER,
                          PICOUI_ALIGN_SPACE_AROUND);
    picoui_flex_set_gap(win, 8, 12);
}
