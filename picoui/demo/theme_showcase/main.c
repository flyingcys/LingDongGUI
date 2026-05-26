#include "picoui/picoui.h"

static void make_ui(struct picoui_window *win)
{
    struct picoui_label *title = picoui_label_create(win, "title");
    struct picoui_text *body = picoui_text_create(win, "body");
    struct picoui_button *accent = picoui_button_create(win, "accent");

    picoui_label_set_text(title, "Theme");
    picoui_text_set_text(body, "Accent preview");
    picoui_button_set_text(accent, "Primary");
}
