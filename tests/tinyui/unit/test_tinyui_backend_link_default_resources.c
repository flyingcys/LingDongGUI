#include "tinyui.h"
#include "widgets/arc.h"
#include "widgets/label.h"
#include "widgets/window.h"

#include <assert.h>

int main(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *win;
    struct tinyui_label *label;
    struct tinyui_arc *arc;
    struct tinyui_image_source source = {0};

    assert(app != 0);
    win = tinyui_window_create(app, "root");
    assert(win != 0);
    label = tinyui_label_create(win, "label_default_font");
    assert(label != 0);
    arc = tinyui_arc_create((struct tinyui_widget *)win, "arc_default_resource");
    assert(arc != 0);
    assert(tinyui_image_source_from_builtin(TINYUI_BUILTIN_IMAGE_ARC_QUARTER, &source) == 0);
    assert(tinyui_arc_set_quarter_source(arc, &source) == 0);

    tinyui_app_destroy(app);
    return 0;
}
