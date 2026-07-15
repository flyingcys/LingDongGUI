#include "tinyui.h"
#include "widgets/arc.h"
#include "widgets/label.h"
#include "widgets/window.h"

#include <assert.h>

int main(void)
{
    struct tinyui_window *win;
    struct tinyui_label *label;
    struct tinyui_arc *arc;
    struct tinyui_image_source source = {0};

    assert(tinyui_init() == TINYUI_OK);
    win = (struct tinyui_window *)(void *)tinyui_screen_create();
    assert(win != 0);
    label = (struct tinyui_label *)(void *)tinyui_label_create((tinyui_obj_t *)win);
    assert(label != 0);
    arc = (struct tinyui_arc *)(void *)tinyui_arc_create((tinyui_obj_t *)win);
    assert(arc != 0);
    assert(tinyui_image_source_from_builtin(TINYUI_BUILTIN_IMAGE_ARC_QUARTER, &source) == TINYUI_OK);
    assert(tinyui_arc_set_quarter_source((tinyui_obj_t *)arc, &source) == 0);

    tinyui_deinit();
    return 0;
}
