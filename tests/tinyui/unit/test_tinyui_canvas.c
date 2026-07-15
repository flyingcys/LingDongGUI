/*
 * TinyUI canvas unit tests — M3 Task 5 L3/L4 harness.
 */

#include "tinyui.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldCanvas.h"
#include "internal.h"
#include "resource/image_source.h"
#include "widgets/canvas.h"

#include <assert.h>
#include <string.h>

static unsigned int test_rgb_to_ld_color(unsigned int rgb)
{
    return (unsigned int)__RGB((rgb >> 16) & 0xFFU, (rgb >> 8) & 0xFFU, rgb & 0xFFU);
}

static void bind_test_tiles(tinyui_image_source_t *source, arm_2d_tile_t *img_tile)
{
    memset(source, 0, sizeof(*source));
    source->kind = TINYUI_IMAGE_SOURCE_RGB565_MEMORY;
    memcpy(source->_image_private, img_tile, sizeof(*img_tile));
}

static void test_canvas_create_and_ld_mapping(tinyui_obj_t *root)
{
    tinyui_obj_t *canvas = tinyui_canvas_create(root);
    struct tinyui_widget *backend;
    ldBase_t *ld_base;

    assert(canvas != 0);
    backend = (struct tinyui_widget *)(void *)canvas;
    assert(backend->ld_widget != 0);
    assert(backend->kind == TINYUI_BACKEND_WIDGET_CANVAS);
    ld_base = (ldBase_t *)backend->ld_widget;
    assert(ld_base->widgetType == widgetTypeCanvas);
}

static void test_canvas_draw_commands_push_native(tinyui_obj_t *root)
{
    tinyui_obj_t *canvas = tinyui_canvas_create(root);
    ldCanvas_t *ld_canvas;
    arm_2d_tile_t img_tile = {0};
    tinyui_image_source_t source;
    int count = -1;
    char text[] = "hi";

    assert(canvas != 0);
    ld_canvas = (ldCanvas_t *)((struct tinyui_widget *)(void *)canvas)->ld_widget;
    assert(ld_canvas != 0);

    assert(tinyui_canvas_fill_rect(canvas, 1, 2, 10, 12, 0xFF0000U, 200) == 0);
    assert(ld_canvas->commandCount == 1);
    assert(ld_canvas->commands[0].kind == ldCanvasCommandFillRect);
    assert(ld_canvas->commands[0].region.tLocation.iX == 1);
    assert(ld_canvas->commands[0].region.tLocation.iY == 2);
    assert(ld_canvas->commands[0].region.tSize.iWidth == 10);
    assert(ld_canvas->commands[0].region.tSize.iHeight == 12);
    assert(ld_canvas->commands[0].color0 == (ldColor)test_rgb_to_ld_color(0xFF0000U));
    assert(ld_canvas->commands[0].opacity0 == 200);

    assert(tinyui_canvas_draw_line(canvas, 0, 0, 5, 6, 2, 0x00FF00U, 255, 10) == 0);
    assert(ld_canvas->commandCount == 2);
    assert(ld_canvas->commands[1].kind == ldCanvasCommandDrawLine);
    assert(ld_canvas->commands[1].x1 == 5);
    assert(ld_canvas->commands[1].y1 == 6);
    assert(ld_canvas->commands[1].lineSize == 2);
    assert(ld_canvas->commands[1].color0 == (ldColor)test_rgb_to_ld_color(0x00FF00U));
    assert(ld_canvas->commands[1].opacity0 == 255);
    assert(ld_canvas->commands[1].opacity1 == 10);

    img_tile.tRegion.tSize.iWidth = 8;
    img_tile.tRegion.tSize.iHeight = 8;
    bind_test_tiles(&source, &img_tile);
    assert(tinyui_canvas_draw_image(canvas, 3, 4, 8, 8, &source, 0xABCDEFU, 128) == 0);
    assert(ld_canvas->commandCount == 3);
    assert(ld_canvas->commands[2].kind == ldCanvasCommandDrawImage);
    assert(ld_canvas->commands[2].ptImgTile == tinyui_image_source_get_image_tile(&source));
    assert(ld_canvas->commands[2].color0 == (ldColor)test_rgb_to_ld_color(0xABCDEFU));
    assert(ld_canvas->commands[2].opacity0 == 128);

    assert(tinyui_canvas_draw_image_scaled(canvas, 0, 0, 8, 8, &source, 1.5f, 90) == 0);
    assert(ld_canvas->commandCount == 4);
    assert(ld_canvas->commands[3].kind == ldCanvasCommandDrawImageScale);
    assert(ld_canvas->commands[3].scale == 1.5f);
    assert(ld_canvas->commands[3].opacity0 == 90);

    assert(tinyui_canvas_draw_text(canvas, 2, 3, 40, 12, text, TINYUI_ALIGN_CENTER, 0x0000FFU, 255) == 0);
    assert(ld_canvas->commandCount == 5);
    assert(ld_canvas->commands[4].kind == ldCanvasCommandDrawText);
    assert(ld_canvas->commands[4].pStr != 0);
    assert(strcmp((const char *)ld_canvas->commands[4].pStr, text) == 0);
    assert(ld_canvas->commands[4].color0 == (ldColor)test_rgb_to_ld_color(0x0000FFU));

    assert(tinyui_canvas_get_command_count(canvas, &count) == 0);
    assert(count == 5);

    assert(tinyui_canvas_clear(canvas) == 0);
    assert(ld_canvas->commandCount == 0);
    assert(tinyui_canvas_get_command_count(canvas, &count) == 0);
    assert(count == 0);
}

static void test_canvas_rejects_invalid(tinyui_obj_t *root)
{
    tinyui_obj_t *canvas = tinyui_canvas_create(root);
    assert(canvas != 0);
    assert(tinyui_canvas_create(0) == 0);
    assert(tinyui_canvas_clear(0) == -1);
    assert(tinyui_canvas_fill_rect(canvas, 0, 0, -1, 1, 0, 0) == -1);
    assert(tinyui_canvas_draw_line(canvas, 0, 0, 1, 1, 0, 0, 0, 0) == -1);
    assert(tinyui_canvas_draw_image(canvas, 0, 0, 1, 1, 0, 0, 0) == -1);
    assert(tinyui_canvas_draw_text(canvas, 0, 0, 1, 1, 0, TINYUI_ALIGN_START, 0, 0) == -1);
}

int main(void)
{
    tinyui_obj_t *root;

    tinyui_deinit();
    assert(tinyui_init() == TINYUI_OK);
    root = tinyui_screen_create();
    assert(root != 0);

    test_canvas_create_and_ld_mapping(root);
    test_canvas_draw_commands_push_native(root);
    test_canvas_rejects_invalid(root);

    tinyui_deinit();
    return 0;
}
