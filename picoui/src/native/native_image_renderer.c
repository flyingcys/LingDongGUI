#include "picoui/image.h"
#include "picoui/widget.h"
#include "../../../examples/common/Arm-2D/Library/Include/arm_2d_types.h"

#include <stdint.h>

static void picoui_native_image_clear_dirty_rect(struct picoui_rect *dirty_rect)
{
    if (dirty_rect == 0) {
        return;
    }

    dirty_rect->x = 0;
    dirty_rect->y = 0;
    dirty_rect->width = 0;
    dirty_rect->height = 0;
}

int picoui_native_image_render_buffer(const struct picoui_image_source *source,
                                      unsigned int mask_color,
                                      unsigned int *buffer,
                                      int width,
                                      int height,
                                      struct picoui_rect *dirty_rect)
{
    const arm_2d_tile_t *img_tile;
    const arm_2d_tile_t *mask_tile;
    const unsigned int *img_pixels;
    const unsigned int *mask_pixels;
    int tile_width;
    int tile_height;
    int render_width;
    int render_height;
    int x;
    int y;
    int i;

    picoui_native_image_clear_dirty_rect(dirty_rect);

    if (source == 0 || source->img_tile == 0 || buffer == 0 || width <= 0 || height <= 0
        || dirty_rect == 0) {
        return -1;
    }

    img_tile = (const arm_2d_tile_t *)source->img_tile;
    mask_tile = (const arm_2d_tile_t *)source->mask_tile;
    if (img_tile->pwBuffer == 0) {
        return -1;
    }

    tile_width = img_tile->tRegion.tSize.iWidth;
    tile_height = img_tile->tRegion.tSize.iHeight;
    if (tile_width <= 0 || tile_height <= 0) {
        return -1;
    }

    render_width = tile_width < width ? tile_width : width;
    render_height = tile_height < height ? tile_height : height;
    img_pixels = (const unsigned int *)img_tile->pwBuffer;
    mask_pixels = mask_tile != 0 ? (const unsigned int *)mask_tile->pwBuffer : 0;

    for (y = 0; y < render_height; ++y) {
        for (x = 0; x < render_width; ++x) {
            i = y * tile_width + x;
            if (mask_pixels != 0 && mask_pixels[i] != 0U) {
                buffer[y * width + x] = 0xFF000000U | (mask_color & 0x00FFFFFFU);
            } else {
                buffer[y * width + x] = img_pixels[i];
            }
        }
    }

    dirty_rect->x = 0;
    dirty_rect->y = 0;
    dirty_rect->width = render_width;
    dirty_rect->height = render_height;
    return 0;
}
