#include "core/obj.h"
#include "core/result.h"
#include "core/runtime.h"
#include "layout/layout.h"
#include "resource/font.h"
#include "resource/image_source.h"

#include <stddef.h>
#include <stdint.h>

_Static_assert(TINYUI_GRID_MAX_TRACKS == 16,
               "grid track capacity must remain 16");
_Static_assert(TINYUI_GRID_UNIT_PX != TINYUI_GRID_UNIT_FR,
               "grid units must be typed");
_Static_assert(TINYUI_GRID_UNIT_CONTENT != TINYUI_GRID_UNIT_PX,
               "content must not use a sentinel value");
_Static_assert(sizeof(((tinyui_image_source_t *)0)->_image_private) >=
                   6 * sizeof(uintptr_t),
               "image private storage ABI changed");
_Static_assert(sizeof(((tinyui_image_source_t *)0)->_mask_private) >=
                   6 * sizeof(uintptr_t),
               "mask private storage ABI changed");

#if UINTPTR_MAX == UINT32_MAX
_Static_assert(sizeof(tinyui_image_source_t) <= 80,
               "image descriptor exceeds the 32-bit ABI budget");
_Static_assert(sizeof(tinyui_font_t) <= 16,
               "font descriptor exceeds the 32-bit ABI budget");
#endif

static void verify_public_function_signatures(void)
{
    tinyui_result_t (*set_columns)(tinyui_obj_t *,
                                   const tinyui_grid_track_t *,
                                   uint8_t) = tinyui_grid_set_columns;
    tinyui_result_t (*set_rows)(tinyui_obj_t *,
                                const tinyui_grid_track_t *,
                                uint8_t) = tinyui_grid_set_rows;
    tinyui_result_t (*from_rgb565)(const uint16_t *,
                                   uint16_t,
                                   uint16_t,
                                   uint32_t,
                                   const uint8_t *,
                                   uint32_t,
                                   tinyui_image_source_t *) = tinyui_image_source_from_rgb565;
    tinyui_result_t (*from_builtin)(tinyui_builtin_image_t,
                                    tinyui_image_source_t *) = tinyui_image_source_from_builtin;
    tinyui_result_t (*from_vres)(uint32_t,
                                 tinyui_image_source_t *) = tinyui_image_source_from_vres;
    void (*image_deinit)(tinyui_image_source_t *) = tinyui_image_source_deinit;
    tinyui_result_t (*font_from_builtin)(tinyui_builtin_font_t,
                                         tinyui_font_t *) = tinyui_font_from_builtin;
    tinyui_result_t (*font_from_vres)(uint32_t, tinyui_font_t *) = tinyui_font_from_vres;
    void (*font_deinit)(tinyui_font_t *) = tinyui_font_deinit;

    (void)set_columns;
    (void)set_rows;
    (void)from_rgb565;
    (void)from_builtin;
    (void)from_vres;
    (void)image_deinit;
    (void)font_from_builtin;
    (void)font_from_vres;
    (void)font_deinit;
}

int main(void)
{
    const tinyui_grid_track_t valid[] = {
        { TINYUI_GRID_UNIT_PX, 1 },
        { TINYUI_GRID_UNIT_FR, 255 },
        { TINYUI_GRID_UNIT_CONTENT, 0 },
    };
    tinyui_image_source_t source = { 0 };
    tinyui_font_t font = { 0 };
    tinyui_obj_t *screen;

    verify_public_function_signatures();
    if (tinyui_grid_set_columns(NULL, valid, 3) != TINYUI_ERROR_INVALID_OBJECT) {
        return 1;
    }
    if (tinyui_init() != TINYUI_OK) {
        return 2;
    }
    screen = tinyui_screen_create();
    if (screen == NULL) {
        tinyui_deinit();
        return 3;
    }
    if (tinyui_grid_set_columns(screen, valid, TINYUI_GRID_MAX_TRACKS + 1) !=
        TINYUI_ERROR_OUT_OF_RANGE) {
        tinyui_deinit();
        return 4;
    }
    if (tinyui_grid_set_columns(screen,
                                &(const tinyui_grid_track_t){ TINYUI_GRID_UNIT_PX, 0 },
                                1) != TINYUI_ERROR_OUT_OF_RANGE) {
        tinyui_deinit();
        return 5;
    }
    tinyui_deinit();
    if (tinyui_image_source_from_rgb565(NULL, 1, 1, 2, NULL, 0, &source) !=
        TINYUI_ERROR_INVALID_ARG) {
        return 6;
    }
    if (tinyui_font_from_vres(0, &font) != TINYUI_ERROR_INVALID_ARG) {
        return 7;
    }
    return 0;
}
