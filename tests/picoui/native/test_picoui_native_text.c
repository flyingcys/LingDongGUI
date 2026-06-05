#include "picoui/picoui.h"
#include "internal.h"

#include <assert.h>
#include <string.h>

int picoui_native_event_bind_root(struct picoui_screen *screen, struct picoui_window *root_window);
int picoui_native_text_get_rendered_text(const struct picoui_text *text, const char **value);
int picoui_native_text_get_rendered_wrap_width(const struct picoui_text *text, int *wrap_width);
void picoui_native_text_test_fail_next_set_text(void);

int main(void)
{
    struct picoui_display *display;
    struct picoui_screen *screen;
    struct picoui_window *window;
    struct picoui_text *text;
    const char *rendered_text = 0;
    int wrap_width = -1;
    int rc;

    assert(picoui_init() == 0);

    display = picoui_display_create(320, 480);
    assert(display != 0);
    assert(picoui_display_set_default(display) == 0);

    screen = picoui_screen_active();
    assert(screen != 0);

    window = picoui_window_create_root(screen, "root");
    assert(window != 0);

    text = picoui_text_create(window, "copy");
    assert(text != 0);
    assert(picoui_widget_set_pos((struct picoui_widget *)text, 12, 24) == 0);
    assert(picoui_widget_set_size((struct picoui_widget *)text, 96, 40) == 0);

    assert(picoui_text_set_text(text, "PicoUI UTF-8: 中文") == 0);
    assert(picoui_text_get_text(text) != 0);
    assert(strcmp(picoui_text_get_text(text), "PicoUI UTF-8: 中文") == 0);
    assert(picoui_widget_get_width((const struct picoui_widget *)text) == 96);

    assert(picoui_native_event_bind_root(screen, window) == 0);
    assert(picoui_native_text_get_rendered_text(text, &rendered_text) == -1);
    assert(picoui_native_text_get_rendered_wrap_width(text, &wrap_width) == -1);
    rc = picoui_timer_handler();
    assert(rc == 0 || rc == 1);
    assert(picoui_native_text_get_rendered_text(text, &rendered_text) == 0);
    assert(rendered_text != 0);
    assert(strcmp(rendered_text, "PicoUI UTF-8: 中文") == 0);
    assert(picoui_native_text_get_rendered_wrap_width(text, &wrap_width) == 0);
    assert(wrap_width == 96);

    picoui_native_text_test_fail_next_set_text();
    assert(picoui_text_set_text(text, "should not commit") == -1);
    assert(strcmp(picoui_text_get_text(text), "PicoUI UTF-8: 中文") == 0);

    assert(picoui_text_set_static_text(text, "static UTF-8: 文本") == 0);
    assert(picoui_text_get_text(text) != 0);
    assert(strcmp(picoui_text_get_text(text), "static UTF-8: 文本") == 0);
    assert(picoui_widget_set_size((struct picoui_widget *)text, 128, 40) == 0);
    rc = picoui_timer_handler();
    assert(rc == 0 || rc == 1);
    assert(picoui_native_text_get_rendered_text(text, &rendered_text) == 0);
    assert(rendered_text != 0);
    assert(strcmp(rendered_text, "static UTF-8: 文本") == 0);
    assert(picoui_native_text_get_rendered_wrap_width(text, &wrap_width) == 0);
    assert(wrap_width == 128);

    picoui_native_text_test_fail_next_set_text();
    assert(picoui_text_set_static_text(text, "static should not commit") == -1);
    assert(strcmp(picoui_text_get_text(text), "static UTF-8: 文本") == 0);

    picoui_deinit();
    return 0;
}
