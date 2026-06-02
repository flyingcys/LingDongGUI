#include "picoui/picoui.h"
#include "../../../src/gui/ldBase.h"
#include "internal.h"

#include <assert.h>

int main(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_window *win = picoui_window_create(app, "root");
    struct picoui_canvas *canvas = picoui_canvas_create(win, "canvas");
    arm_2d_tile_t image_tile = {0};
    arm_2d_tile_t mask_tile = {0};
    struct picoui_image_source image_source = {
        .img_tile = &image_tile,
        .mask_tile = &mask_tile,
    };
    const struct picoui_backend_widget *backend;
    const ldBase_t *ld_canvas;
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
                                   "canvas",
                                   PICOUI_ALIGN_CENTER,
                                   0xAABBCCU,
                                   255) == 0);

    assert(picoui_canvas_get_command_count(canvas, &command_count) == 0);
    assert(command_count == 5);

    backend = canvas->widget.backend_widget;
    assert(backend != 0);
    ld_canvas = (const ldBase_t *)backend->ld_widget;
    assert(ld_canvas != 0);
    assert(ld_canvas->widgetType == widgetTypeCanvas);
    assert(ld_canvas->use_as__arm_2d_control_node_t.tRegion.tSize.iWidth == 120);
    assert(ld_canvas->use_as__arm_2d_control_node_t.tRegion.tSize.iHeight == 80);

    assert(picoui_canvas_clear(canvas) == 0);
    assert(picoui_canvas_get_command_count(canvas, &command_count) == 0);
    assert(command_count == 0);

    assert(picoui_canvas_fill_rect(0, 0, 0, 1, 1, 0, 255) == -1);
    assert(picoui_canvas_draw_line(0, 0, 0, 1, 1, 1, 0, 255, 255) == -1);
    assert(picoui_canvas_draw_image(canvas, 0, 0, 10, 10, 0, 0, 255) == -1);
    assert(picoui_canvas_draw_image_scaled(canvas, 0, 0, 10, 10, 0, 1.0f, 255) == -1);
    assert(picoui_canvas_draw_text(canvas, 0, 0, 10, 10, 0, PICOUI_ALIGN_START, 0, 255) == -1);
    assert(picoui_canvas_get_command_count(canvas, 0) == -1);

    picoui_app_destroy(app);
    return 0;
}
