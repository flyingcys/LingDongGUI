#include "tinyui.h"

#include <assert.h>
#include <stddef.h>

int main(void)
{
    struct picoui_app *app = picoui_app_create();
    assert(app != NULL);

    struct picoui_window *win = picoui_window_create(app, "root_window");
    assert(win != NULL);

    picoui_app_destroy(app);
    return 0;
}
