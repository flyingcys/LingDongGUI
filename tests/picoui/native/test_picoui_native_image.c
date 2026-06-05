#include "picoui/picoui.h"
#include "internal.h"

#include <assert.h>

int picoui_native_event_bind_root(struct picoui_screen *screen, struct picoui_window *root_window);
int picoui_native_image_get_rendered_source(const struct picoui_image *image,
                                            struct picoui_image_source **source);
void picoui_native_image_test_fail_next_set_source(void);

int main(void)
{
    struct picoui_display *display;
    struct picoui_screen *screen;
    struct picoui_window *window;
    struct picoui_image *image;
    struct picoui_image_source *rendered_source = (struct picoui_image_source *)1;
    int image_tile = 0;
    int next_image_tile = 0;
    struct picoui_image_source source = {0};
    struct picoui_image_source next_source = {0};
    struct picoui_backend_widget *backend;
    int rc;

    assert(picoui_init() == 0);

    display = picoui_display_create(320, 480);
    assert(display != 0);
    assert(picoui_display_set_default(display) == 0);

    screen = picoui_screen_active();
    assert(screen != 0);

    window = picoui_window_create_root(screen, "root");
    assert(window != 0);

    image = picoui_image_create(window, "cover");
    assert(image != 0);
    backend = (struct picoui_backend_widget *)image->widget.backend_widget;
    assert(backend != 0);
    assert(image->source == 0);
    assert(picoui_widget_set_pos((struct picoui_widget *)image, 12, 24) == 0);
    assert(picoui_widget_set_size((struct picoui_widget *)image, 96, 40) == 0);

    assert(picoui_image_set_source(image, 0) == 0);
    assert(image->source == 0);

    assert(picoui_native_event_bind_root(screen, window) == 0);
    assert(picoui_native_image_get_rendered_source(image, &rendered_source) == -1);
    rc = picoui_timer_handler();
    assert(rc == 0 || rc == 1);
    assert(picoui_native_image_get_rendered_source(image, &rendered_source) == 0);
    assert(rendered_source == 0);

    source.img_tile = &image_tile;
    assert(picoui_image_set_source(image, &source) == 0);
    assert(image->source == &source);
    assert(backend->image_source == &source);

    rc = picoui_timer_handler();
    assert(rc == 0 || rc == 1);
    assert(picoui_native_image_get_rendered_source(image, &rendered_source) == 0);
    assert(rendered_source == &source);

    next_source.img_tile = &next_image_tile;
    picoui_native_image_test_fail_next_set_source();
    assert(picoui_image_set_source(image, &next_source) == -1);
    assert(image->source == &source);
    assert(backend->image_source == &source);

    rc = picoui_timer_handler();
    assert(rc == 0 || rc == 1);
    assert(picoui_native_image_get_rendered_source(image, &rendered_source) == 0);
    assert(rendered_source == &source);

    picoui_deinit();
    return 0;
}
