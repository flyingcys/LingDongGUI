/*
 * TinyUI background unit tests — M3 Task 5 L3/L4 harness.
 *
 * background 直接映射到真实 ldWindow（nameId=0 root / widgetTypeBackground）。
 */

#include "tinyui.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldWindow.h"
#include "internal.h"
#include "resource/image_source.h"
#include "widgets/background.h"

#include <assert.h>
#include <string.h>

static unsigned int test_rgb_to_ld_color(unsigned int rgb)
{
    return (unsigned int)__RGB((rgb >> 16) & 0xFFU, (rgb >> 8) & 0xFFU, rgb & 0xFFU);
}

static void test_background_create_and_props(void)
{
    tinyui_obj_t *bg = tinyui_background_create();
    struct tinyui_widget *backend;
    ldWindow_t *ld_win;
    arm_2d_tile_t tile = {0};
    tinyui_image_source_t source;
    unsigned int color = 0;
    unsigned int expected;
    unsigned int actual;
    int ox = -1, oy = -1;

    assert(bg != 0);
    backend = (struct tinyui_widget *)(void *)bg;
    assert(backend->ld_widget != 0);
    assert(backend->kind == TINYUI_BACKEND_WIDGET_BACKGROUND);
    ld_win = (ldWindow_t *)backend->ld_widget;
    assert(((ldBase_t *)ld_win)->widgetType == widgetTypeBackground);

    assert(tinyui_background_set_color(bg, 0x102030U) == 0);
    expected = test_rgb_to_ld_color(0x102030U);
    actual = (unsigned int)ldWindowGetColor(ld_win);
    assert(actual == expected);
    assert(tinyui_background_get_color(bg, &color) == 0);
    {
        unsigned int color2 = 0;
        assert(tinyui_background_get_color(bg, &color2) == 0);
        assert(color == color2);
    }

    memset(&source, 0, sizeof(source));
    source.kind = TINYUI_IMAGE_SOURCE_RGB565_MEMORY;
    tile.tRegion.tSize.iWidth = 8;
    tile.tRegion.tSize.iHeight = 8;
    memcpy(source._image_private, &tile, sizeof(tile));
    assert(tinyui_background_set_source(bg, &source) == 0);
    assert(ld_win->ptImgTile == tinyui_image_source_get_image_tile(&source));

    assert(tinyui_background_set_offset(bg, 3, 4) == 0);
    assert(tinyui_background_get_offset(bg, &ox, &oy) == 0);
    assert(ox == 3 && oy == 4);
}

static void test_background_rejects_invalid(void)
{
    assert(tinyui_background_set_color(0, 0) == -1);
    assert(tinyui_background_set_source(0, 0) == -1);
    assert(tinyui_background_create() != 0); /* runtime already init in main */
}

int main(void)
{
    tinyui_deinit();
    assert(tinyui_init() == TINYUI_OK);

    test_background_create_and_props();
    test_background_rejects_invalid();

    tinyui_deinit();
    return 0;
}
