#include "picoui/picoui.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldCanvas.h"
#include "internal.h"

#include <assert.h>
#include <string.h>

static unsigned int encode_ld_color(unsigned int rgb)
{
    return (unsigned int)__RGB((rgb >> 16) & 0xFFU, (rgb >> 8) & 0xFFU, rgb & 0xFFU);
}

int main(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_window *win = picoui_window_create(app, "root");
    struct picoui_canvas *canvas = picoui_canvas_create(win, "canvas");
    char mutable_text[] = "canvas";
    arm_2d_tile_t image_tile = {0};
    arm_2d_tile_t mask_tile = {0};
    struct picoui_image_source image_source = {
        .img_tile = &image_tile,
        .mask_tile = &mask_tile,
    };
    const struct picoui_backend_widget *backend;
    const struct picoui_backend_widget *parent_backend;
    const ldCanvas_t *ld_canvas;
    int command_count = 0;

    assert(app != 0);
    assert(win != 0);
    assert(canvas != 0);
    assert(picoui_widget_set_size((struct picoui_widget *)canvas, 120, 80) == 0);

    assert(picoui_canvas_fill_rect(canvas, 1, 2, 30, 40, 0x112233U, 200) == 0);
    assert(picoui_canvas_draw_line(canvas, 0, 0, 20, 10, 3, 0x445566U, 255, 32) == 0);
    assert(picoui_canvas_draw_image(canvas, 5, 6, 24, 18, &image_source, 0x778899U, 180) == 0);
    assert(picoui_canvas_draw_image_scaled(canvas, 7, 8, 32, 20, &image_source, 0.5f, 210) == 0);
    assert(picoui_canvas_draw_text(canvas,
                                   9,
                                   10,
                                   50,
                                   16,
                                   mutable_text,
                                   PICOUI_ALIGN_CENTER,
                                   0xAABBCCU,
                                   255) == 0);

    assert(picoui_canvas_get_command_count(canvas, &command_count) == 0);
    assert(command_count == 5);

    backend = canvas->widget.backend_widget;
    assert(backend != 0);
    parent_backend = win->widget.backend_widget;
    assert(parent_backend != 0);
    assert(backend->parent == parent_backend);
    assert(backend->root == parent_backend->root);
    assert(backend->owner == parent_backend->owner);
    assert(backend->host_widget == &canvas->widget);
    ld_canvas = (const ldCanvas_t *)backend->ld_widget;
    assert(ld_canvas != 0);
    assert(ld_canvas->use_as__ldBase_t.widgetType == widgetTypeCanvas);
    assert(ld_canvas->use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iWidth == 120);
    assert(ld_canvas->use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iHeight == 80);
    assert(ld_canvas->commandCount == 5);
    assert(ld_canvas->commands[0].kind == ldCanvasCommandFillRect);
    assert(ld_canvas->commands[0].region.tLocation.iX == 1);
    assert(ld_canvas->commands[0].region.tLocation.iY == 2);
    assert(ld_canvas->commands[0].region.tSize.iWidth == 30);
    assert(ld_canvas->commands[0].region.tSize.iHeight == 40);
    assert(ld_canvas->commands[0].color0 == encode_ld_color(0x112233U));
    assert(ld_canvas->commands[0].opacity0 == 200);
    assert(ld_canvas->commands[1].kind == ldCanvasCommandDrawLine);
    assert(ld_canvas->commands[1].x1 == 20);
    assert(ld_canvas->commands[1].y1 == 10);
    assert(ld_canvas->commands[1].lineSize == 3);
    assert(ld_canvas->commands[1].color0 == encode_ld_color(0x445566U));
    assert(ld_canvas->commands[1].opacity0 == 255);
    assert(ld_canvas->commands[1].opacity1 == 32);
    assert(ld_canvas->commands[2].kind == ldCanvasCommandDrawImage);
    assert(ld_canvas->commands[2].ptImgTile == &image_tile);
    assert(ld_canvas->commands[2].ptMaskTile == &mask_tile);
    assert(ld_canvas->commands[2].color0 == encode_ld_color(0x778899U));
    assert(ld_canvas->commands[2].opacity0 == 180);
    assert(ld_canvas->commands[3].kind == ldCanvasCommandDrawImageScale);
    assert(ld_canvas->commands[3].ptImgTile == &image_tile);
    assert(ld_canvas->commands[3].ptMaskTile == &mask_tile);
    assert(ld_canvas->commands[3].scale == 0.5f);
    assert(ld_canvas->commands[3].opacity0 == 210);
    assert(ld_canvas->commands[4].kind == ldCanvasCommandDrawText);
    assert(ld_canvas->commands[4].align == ARM_2D_ALIGN_CENTRE);
    assert(ld_canvas->commands[4].color0 == encode_ld_color(0xAABBCCU));
    assert(ld_canvas->commands[4].opacity0 == 255);
    assert(strcmp((const char *)ld_canvas->commands[4].pStr, "canvas") == 0);

    mutable_text[0] = 'X';
    assert(picoui_canvas_fill_rect(canvas, 11, 12, 13, 14, 0x010203U, 99) == 0);
    assert(ld_canvas->commandCount == 6);
    assert(strcmp((const char *)ld_canvas->commands[4].pStr, "canvas") == 0);
    assert(ld_canvas->commands[5].kind == ldCanvasCommandFillRect);
    assert(ld_canvas->commands[5].region.tLocation.iX == 11);
    assert(ld_canvas->commands[5].region.tLocation.iY == 12);
    assert(ld_canvas->commands[5].region.tSize.iWidth == 13);
    assert(ld_canvas->commands[5].region.tSize.iHeight == 14);
    assert(ld_canvas->commands[5].color0 == encode_ld_color(0x010203U));
    assert(ld_canvas->commands[5].opacity0 == 99);

    assert(picoui_canvas_clear(canvas) == 0);
    assert(picoui_canvas_get_command_count(canvas, &command_count) == 0);
    assert(command_count == 0);
    assert(ld_canvas->commandCount == 0);
    assert(ld_canvas->commands[4].pStr == 0);

    assert(picoui_canvas_fill_rect(0, 0, 0, 1, 1, 0, 255) == -1);
    assert(picoui_canvas_draw_line(0, 0, 0, 1, 1, 1, 0, 255, 255) == -1);
    assert(picoui_canvas_draw_image(canvas, 0, 0, 10, 10, 0, 0, 255) == -1);
    assert(picoui_canvas_draw_image_scaled(canvas, 0, 0, 10, 10, 0, 1.0f, 255) == -1);
    assert(picoui_canvas_draw_text(canvas, 0, 0, 10, 10, 0, PICOUI_ALIGN_START, 0, 255) == -1);
    assert(picoui_canvas_get_command_count(canvas, 0) == -1);

    picoui_app_destroy(app);
    return 0;
}
