#include "picoui/picoui.h"
#include "internal.h"

#include <assert.h>
#include <string.h>

int picoui_native_canvas_render_buffer(const struct picoui_canvas *canvas,
                                       unsigned int *buffer,
                                       int width,
                                       int height,
                                       struct picoui_rect *dirty_rect);
int picoui_canvas_draw_circle(struct picoui_canvas *canvas,
                              int center_x,
                              int center_y,
                              int radius,
                              unsigned int rgb,
                              int opacity_max,
                              int opacity_min);

static void test_canvas_primitives_mark_dirty_area_and_pixels(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_window *window;
    struct picoui_canvas *canvas;
    unsigned int buffer[64 * 64] = {0};
    struct picoui_rect dirty_rect = {-1, -1, 0, 0};
    int i;
    int changed_pixels = 0;

    assert(app != 0);
    window = picoui_window_create(app, "canvas_renderer_root");
    assert(window != 0);
    canvas = picoui_canvas_create(window, "canvas_renderer");
    assert(canvas != 0);
    assert(picoui_widget_set_size((struct picoui_widget *)canvas, 64, 64) == 0);

    assert(picoui_canvas_fill_rect(canvas, 4, 6, 12, 10, 0x112233U, 255) == 0);
    assert(picoui_canvas_draw_line(canvas, 2, 2, 30, 18, 2, 0x445566U, 255, 64) == 0);
    assert(picoui_canvas_draw_line(canvas, 20, 20, 33, 20, 2, 0x778899U, 255, 255) == 0);
    assert(picoui_canvas_draw_line(canvas, 33, 20, 33, 31, 2, 0x778899U, 255, 255) == 0);
    assert(picoui_canvas_draw_line(canvas, 33, 31, 20, 31, 2, 0x778899U, 255, 255) == 0);
    assert(picoui_canvas_draw_line(canvas, 20, 31, 20, 20, 2, 0x778899U, 255, 255) == 0);
    assert(picoui_canvas_draw_circle(canvas, 40, 40, 6, 0xAABBCCU, 220, 80) == 0);

    assert(picoui_native_canvas_render_buffer(canvas, buffer, 64, 64, &dirty_rect) == 0);
    assert(dirty_rect.x >= 0);
    assert(dirty_rect.y >= 0);
    assert(dirty_rect.width > 0);
    assert(dirty_rect.height > 0);

    for (i = 0; i < (int)(sizeof(buffer) / sizeof(buffer[0])); ++i) {
        if (buffer[i] != 0U) {
            changed_pixels++;
        }
    }

    assert(changed_pixels > 0);
    picoui_app_destroy(app);
}

static void test_canvas_renderer_rejects_invalid_inputs(void)
{
    unsigned int buffer[16];
    struct picoui_rect dirty_rect = {3, 3, 3, 3};
    unsigned int snapshot[16];

    memset(buffer, 0x5A, sizeof(buffer));
    memcpy(snapshot, buffer, sizeof(buffer));
    assert(picoui_native_canvas_render_buffer(0, buffer, 4, 4, &dirty_rect) == -1);
    assert(memcmp(buffer, snapshot, sizeof(buffer)) == 0);
}

int main(void)
{
    test_canvas_primitives_mark_dirty_area_and_pixels();
    test_canvas_renderer_rejects_invalid_inputs();
    return 0;
}
