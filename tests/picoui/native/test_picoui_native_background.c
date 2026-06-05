#include "picoui/picoui.h"
#include "internal.h"

#include <assert.h>

int picoui_native_background_get_rendered_size(const struct picoui_background *background,
                                               int *width,
                                               int *height);
int picoui_native_background_get_rendered_color(const struct picoui_background *background,
                                                unsigned int *rgb);
int picoui_native_background_get_rendered_source(const struct picoui_background *background,
                                                 struct picoui_image_source **source);

int main(void)
{
    struct picoui_display *display;
    struct picoui_screen *screen;
    struct picoui_background *background;
    struct picoui_image_source source = {0};
    struct picoui_image_source *rendered_source = (struct picoui_image_source *)1;
    struct picoui_backend_widget *backend;
    int image_tile = 0;
    int width = 0;
    int height = 0;
    unsigned int rgb = 0;
    int rc;

    assert(picoui_init() == 0);

    display = picoui_display_create(320, 480);
    assert(display != 0);
    assert(picoui_display_set_default(display) == 0);

    screen = picoui_screen_create();
    assert(screen != 0);

    background = picoui_background_create_root(screen, "bg_root");
    assert(background != 0);
    backend = (struct picoui_backend_widget *)background->window.widget.backend_widget;
    assert(backend != 0);
    assert(backend->kind == PICOUI_BACKEND_WIDGET_BACKGROUND);
    assert(picoui_widget_get_parent((struct picoui_widget *)background) == 0);

    assert(picoui_background_set_color(background, 0x224466U) == 0);
    source.img_tile = &image_tile;
    assert(picoui_background_set_source(background, &source) == 0);

    assert(picoui_screen_load(screen) == 0);
    assert(picoui_screen_get_root_window(screen) == (struct picoui_window *)background);

    assert(picoui_native_background_get_rendered_size(background, &width, &height) == -1);
    assert(picoui_native_background_get_rendered_color(background, &rgb) == -1);
    assert(picoui_native_background_get_rendered_source(background, &rendered_source) == -1);

    rc = picoui_timer_handler();
    assert(rc == 0 || rc == 1);

    assert(picoui_native_background_get_rendered_size(background, &width, &height) == 0);
    assert(width == 320);
    assert(height == 480);
    assert(picoui_native_background_get_rendered_color(background, &rgb) == 0);
    assert(rgb == 0x204462U);
    assert(picoui_native_background_get_rendered_source(background, &rendered_source) == 0);
    assert(rendered_source == &source);

    picoui_deinit();
    return 0;
}
