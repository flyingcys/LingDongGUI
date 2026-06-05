#include "picoui/picoui.h"
#include "internal.h"

#include <assert.h>
#include <string.h>

int picoui_native_background_get_rendered_size(const struct picoui_background *background,
                                               int *width,
                                               int *height);
int picoui_native_text_get_rendered_text(const struct picoui_text *text, const char **value);

static void test_runtime_init_deinit_is_idempotent(void)
{
    assert(picoui_init() == 0);
    assert(picoui_init() == 0);
    assert(picoui_timer_handler() == 0);
    picoui_deinit();
    assert(picoui_init() == 0);
    picoui_deinit();
    picoui_deinit();
}

static void test_runtime_screen_load_renders_screen_root_window(void)
{
    struct picoui_display *display;
    struct picoui_screen *screen;
    struct picoui_window *window;
    struct picoui_text *text;
    const char *rendered_text = 0;
    int rc;

    assert(picoui_init() == 0);

    display = picoui_display_create(320, 480);
    assert(display != 0);
    assert(picoui_display_set_default(display) == 0);

    screen = picoui_screen_active();
    assert(screen != 0);

    window = picoui_window_create_root(screen, "runtime_root");
    assert(window != 0);
    text = picoui_text_create(window, "title");
    assert(text != 0);
    assert(picoui_text_set_text(text, "runtime root load") == 0);
    assert(picoui_screen_get_root_window(screen) == window);

    assert(picoui_screen_load(screen) == 0);
    assert(picoui_native_text_get_rendered_text(text, &rendered_text) == -1);
    rc = picoui_timer_handler();
    assert(rc == 0 || rc == 1);
    assert(picoui_native_text_get_rendered_text(text, &rendered_text) == 0);
    assert(rendered_text != 0);
    assert(strcmp(rendered_text, "runtime root load") == 0);

    picoui_deinit();
}

int main(void)
{
    test_runtime_init_deinit_is_idempotent();
    test_runtime_screen_load_renders_screen_root_window();
    return 0;
}
