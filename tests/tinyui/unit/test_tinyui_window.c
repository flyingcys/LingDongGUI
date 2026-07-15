/*
 * TinyUI window unit tests — M3 Task 5 L3/L4 harness.
 *
 * Task5 owns color/background_source (layout/flex/grid is Task7).
 */

#include "tinyui.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldWindow.h"
#include "internal.h"
#include "resource/image_source.h"
#include "widgets/window.h"

#include <assert.h>
#include <string.h>

static unsigned int test_rgb_to_ld_color(unsigned int rgb)
{
    return (unsigned int)__RGB((rgb >> 16) & 0xFFU, (rgb >> 8) & 0xFFU, rgb & 0xFFU);
}

static void test_window_create_and_ld_mapping(void)
{
    tinyui_obj_t *win = tinyui_window_create(0);
    struct tinyui_widget *backend;
    ldWindow_t *ld_win;

    assert(win != 0);
    backend = (struct tinyui_widget *)(void *)win;
    assert(backend->ld_widget != 0);
    assert(backend->kind == TINYUI_BACKEND_WIDGET_WINDOW);
    ld_win = (ldWindow_t *)backend->ld_widget;
    assert(((ldBase_t *)ld_win)->widgetType == widgetTypeWindow);
}

static void test_window_color_and_background_source(void)
{
    tinyui_obj_t *root = tinyui_screen_create();
    tinyui_obj_t *win = tinyui_window_create(root);
    ldWindow_t *ld_win;
    unsigned int color = 0;
    arm_2d_tile_t tile = {0};
    tinyui_image_source_t source;
    int ox = 0, oy = 0;

    assert(root != 0);
    assert(win != 0);
    ld_win = (ldWindow_t *)((struct tinyui_widget *)(void *)win)->ld_widget;
    assert(ld_win != 0);

    assert(tinyui_window_set_color(win, 0x0A0B0CU) == 0);
    assert(ld_win->bgColor == (ldColor)test_rgb_to_ld_color(0x0A0B0CU));
    assert(tinyui_window_get_color(win, &color) == 0);
    /* get_color is RGB565-lossy; require non-zero and stable re-get */
    {
        unsigned int color2 = 0;
        assert(tinyui_window_get_color(win, &color2) == 0);
        assert(color == color2);
        assert(color != 0U || 0x0A0B0CU == 0U);
    }

    memset(&source, 0, sizeof(source));
    source.kind = TINYUI_IMAGE_SOURCE_RGB565_MEMORY;
    tile.tRegion.tSize.iWidth = 16;
    tile.tRegion.tSize.iHeight = 16;
    memcpy(source._image_private, &tile, sizeof(tile));
    assert(tinyui_window_set_background_source(win, &source) == 0);
    assert(ld_win->ptImgTile == tinyui_image_source_get_image_tile(&source));

    assert(tinyui_window_set_background_offset(win, 1, 2) == 0);
    assert(tinyui_window_get_background_offset(win, &ox, &oy) == 0);
    assert(ox == 1 && oy == 2);
}

static void test_window_props_color_and_source(void)
{
    tinyui_window_props_t props;
    arm_2d_tile_t tile = {0};
    tinyui_image_source_t source;
    tinyui_obj_t *win;
    unsigned int color = 0;
    ldWindow_t *ld_win;

    memset(&source, 0, sizeof(source));
    source.kind = TINYUI_IMAGE_SOURCE_RGB565_MEMORY;
    tile.tRegion.tSize.iWidth = 4;
    tile.tRegion.tSize.iHeight = 4;
    memcpy(source._image_private, &tile, sizeof(tile));

    memset(&props, 0, sizeof(props));
    props.fields = TINYUI_WINDOW_FIELD_BG_COLOR | TINYUI_WINDOW_FIELD_BACKGROUND_SOURCE;
    props.bg_color = 0x010203U;
    props.background_source = &source;

    win = tinyui_window_create_with_props(0, &props);
    assert(win != 0);
    ld_win = (ldWindow_t *)((struct tinyui_widget *)(void *)win)->ld_widget;
    assert(ld_win != 0);
    assert(tinyui_window_get_color(win, &color) == 0);
    assert(ld_win->bgColor == (ldColor)test_rgb_to_ld_color(0x010203U));
    assert(ld_win->ptImgTile == tinyui_image_source_get_image_tile(&source));
}

static void test_window_rejects_invalid(void)
{
    tinyui_obj_t *win = tinyui_window_create(0);
    assert(win != 0);
    assert(tinyui_window_set_color(0, 0) == -1);
    assert(tinyui_window_get_color(win, 0) == -1);
    assert(tinyui_window_set_background_source(win, 0) == 0); /* clear allowed */
}

int main(void)
{
    tinyui_deinit();
    assert(tinyui_init() == TINYUI_OK);

    test_window_create_and_ld_mapping();
    test_window_color_and_background_source();
    test_window_props_color_and_source();
    test_window_rejects_invalid();

    tinyui_deinit();
    return 0;
}
