#include "picoui/picoui.h"
#include "../../../examples/common/Arm-2D/Library/Include/arm_2d_types.h"
#include "internal.h"

#include <assert.h>
#include <string.h>

int picoui_native_event_bind_root(struct picoui_screen *screen, struct picoui_window *root_window);
int picoui_native_image_get_rendered_source(const struct picoui_image *image,
                                            struct picoui_image_source **source);
int picoui_native_image_get_rendered_dirty_rect(const struct picoui_image *image,
                                                struct picoui_rect *dirty_rect);
int picoui_native_image_get_rendered_changed_pixels(const struct picoui_image *image, int *count);
void picoui_native_image_test_fail_next_set_source(void);

int main(void)
{
    struct picoui_display *display;
    struct picoui_screen *screen;
    struct picoui_window *window;
    struct picoui_image *image;
    struct picoui_image_source *rendered_source = (struct picoui_image_source *)1;
    unsigned int image_tile[96 * 40];
    unsigned int next_image_tile[96 * 40];
    arm_2d_tile_t image_tile_desc = {
        .tInfo = {
            .bIsRoot = true,
            .bHasEnforcedColour = true,
            .tColourInfo = {
                .chScheme = ARM_2D_COLOUR_32BIT,
            },
        },
        .tRegion = {
            .tSize = {
                .iWidth = 96,
                .iHeight = 40,
            },
        },
    };
    arm_2d_tile_t next_image_tile_desc = {
        .tInfo = {
            .bIsRoot = true,
            .bHasEnforcedColour = true,
            .tColourInfo = {
                .chScheme = ARM_2D_COLOUR_32BIT,
            },
        },
        .tRegion = {
            .tSize = {
                .iWidth = 96,
                .iHeight = 40,
            },
        },
    };
    struct picoui_image_source source = {0};
    struct picoui_image_source next_source = {0};
    struct picoui_backend_widget *backend;
    struct picoui_rect dirty_rect = {-1, -1, -1, -1};
    int changed_pixels = -1;
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
    memset(image_tile, 0x5A, sizeof(image_tile));
    memset(next_image_tile, 0xA5, sizeof(next_image_tile));
    image_tile_desc.pwBuffer = image_tile;
    next_image_tile_desc.pwBuffer = next_image_tile;
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
    assert(picoui_native_image_get_rendered_dirty_rect(image, &dirty_rect) == 0);
    assert(dirty_rect.width == 0);
    assert(dirty_rect.height == 0);
    assert(picoui_native_image_get_rendered_changed_pixels(image, &changed_pixels) == 0);
    assert(changed_pixels == 0);

    source.img_tile = &image_tile_desc;
    assert(picoui_image_set_source(image, &source) == 0);
    assert(image->source == &source);
    assert(backend->image_source == &source);

    rc = picoui_timer_handler();
    assert(rc == 0 || rc == 1);
    assert(picoui_native_image_get_rendered_source(image, &rendered_source) == 0);
    assert(rendered_source == &source);
    assert(picoui_native_image_get_rendered_dirty_rect(image, &dirty_rect) == 0);
    assert(dirty_rect.width == 96);
    assert(dirty_rect.height == 40);
    assert(picoui_native_image_get_rendered_changed_pixels(image, &changed_pixels) == 0);
    assert(changed_pixels > 0);

    next_source.img_tile = &next_image_tile_desc;
    picoui_native_image_test_fail_next_set_source();
    assert(picoui_image_set_source(image, &next_source) == -1);
    assert(image->source == &source);
    assert(backend->image_source == &source);

    rc = picoui_timer_handler();
    assert(rc == 0 || rc == 1);
    assert(picoui_native_image_get_rendered_source(image, &rendered_source) == 0);
    assert(rendered_source == &source);
    assert(picoui_native_image_get_rendered_dirty_rect(image, &dirty_rect) == 0);
    assert(dirty_rect.width == 96);
    assert(dirty_rect.height == 40);
    assert(picoui_native_image_get_rendered_changed_pixels(image, &changed_pixels) == 0);
    assert(changed_pixels > 0);

    picoui_deinit();
    return 0;
}
