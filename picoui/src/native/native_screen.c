#include "picoui/screen.h"
#include "../core/internal.h"

#include <stdlib.h>

struct picoui_screen {
    int loaded;
    struct picoui_window *root_window;
};

static struct picoui_screen g_default_screen = { 1 };
static struct picoui_screen *g_active_screen = &g_default_screen;

int picoui_native_widget_bind_root(struct picoui_screen *screen, struct picoui_window *root_window);
int picoui_native_render_bind_root(struct picoui_screen *screen, struct picoui_window *root_window);

struct picoui_screen *picoui_native_screen_active_impl(void)
{
    return g_active_screen;
}

struct picoui_screen *picoui_native_screen_create_impl(void)
{
    return (struct picoui_screen *)calloc(1, sizeof(struct picoui_screen));
}

int picoui_native_screen_load_impl(struct picoui_screen *screen)
{
    if (screen == 0) {
        return -1;
    }

    if (screen->root_window != 0) {
        if (picoui_native_widget_bind_root(screen, screen->root_window) != 0) {
            return -1;
        }
        if (picoui_native_render_bind_root(screen, screen->root_window) != 0) {
            return -1;
        }
    }

    screen->loaded = 1;
    g_active_screen = screen;
    return 0;
}

int picoui_screen_set_root_window(struct picoui_screen *screen, struct picoui_window *root_window)
{
    if (screen == 0 || root_window == 0 || root_window->widget.backend_widget == 0) {
        return -1;
    }

    screen->root_window = root_window;
    return 0;
}

struct picoui_window *picoui_screen_get_root_window(const struct picoui_screen *screen)
{
    if (screen == 0) {
        return 0;
    }

    return screen->root_window;
}
