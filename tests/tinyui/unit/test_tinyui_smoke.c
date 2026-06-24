#include "tinyui.h"
#include "core/app.h"

#include <assert.h>
#include <stddef.h>

int main(void)
{
    struct tinyui_app *app = tinyui_app_create();
    assert(app != NULL);

    struct tinyui_window *win = tinyui_window_create(app, "root_window");
    assert(win != NULL);

    tinyui_app_destroy(app);
    return 0;
}
