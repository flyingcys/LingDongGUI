#include "picoui/picoui.h"
#include "internal.h"
#include "../../../src/gui/ldBase.h"

#include <assert.h>
#include <time.h>

int picoui_native_event_bind_root(struct picoui_screen *screen, struct picoui_window *root_window);
int picoui_native_animation_get_rendered_frame_index(const struct picoui_animation *animation, int *frame_index);
int picoui_native_canvas_get_rendered_command_count(const struct picoui_canvas *canvas, int *count);
int picoui_native_canvas_get_rendered_command_kind(const struct picoui_canvas *canvas, int index, int *kind);
int picoui_native_canvas_get_rendered_dirty_rect(const struct picoui_canvas *canvas, struct picoui_rect *dirty_rect);
int picoui_native_canvas_get_rendered_changed_pixels(const struct picoui_canvas *canvas, int *count);
int picoui_canvas_draw_circle(struct picoui_canvas *canvas,
                              int center_x,
                              int center_y,
                              int radius,
                              unsigned int rgb,
                              int opacity_max,
                              int opacity_min);

static arm_2d_tile_t s_animation_tile = {
    .tRegion = {
        .tSize = {
            .iWidth = 64,
            .iHeight = 16,
        },
    },
};

static void sleep_ms(long duration_ms)
{
    struct timespec ts;

    ts.tv_sec = duration_ms / 1000L;
    ts.tv_nsec = (duration_ms % 1000L) * 1000000L;
    (void)nanosleep(&ts, 0);
}

int main(void)
{
    struct picoui_display *display;
    struct picoui_screen *screen;
    struct picoui_window *window;
    struct picoui_animation *animation;
    struct picoui_canvas *canvas;
    struct picoui_image_source source = {
        .img_tile = &s_animation_tile,
        .mask_tile = 0,
    };
    struct picoui_animation_props props = {
        .id = "anim",
        .width = 16,
        .height = 16,
        .period_ms = 40,
        .source = &source,
    };
    int frame_index = -1;
    int command_count = -1;
    int command_kind = -1;
    int changed_pixels = -1;
    struct picoui_rect dirty_rect = {-1, -1, -1, -1};

    assert(picoui_init() == 0);

    display = picoui_display_create(320, 480);
    assert(display != 0);
    assert(picoui_display_set_default(display) == 0);

    screen = picoui_screen_active();
    assert(screen != 0);

    window = picoui_window_create_root(screen, "root");
    assert(window != 0);

    animation = picoui_animation_create_with_props((struct picoui_widget *)window, &props);
    assert(animation != 0);
    assert(picoui_widget_set_pos((struct picoui_widget *)animation, 16, 24) == 0);
    assert(picoui_widget_set_size((struct picoui_widget *)animation, 16, 16) == 0);

    canvas = picoui_canvas_create(window, "canvas");
    assert(canvas != 0);
    assert(picoui_widget_set_pos((struct picoui_widget *)canvas, 48, 24) == 0);
    assert(picoui_widget_set_size((struct picoui_widget *)canvas, 160, 120) == 0);

    assert(picoui_canvas_fill_rect(canvas, 4, 6, 24, 18, 0x112233U, 255) == 0);
    assert(picoui_canvas_draw_line(canvas, 8, 10, 72, 42, 2, 0x445566U, 255, 48) == 0);
    assert(picoui_canvas_draw_circle(canvas, 60, 40, 12, 0x778899U, 220, 80) == 0);
    assert(picoui_canvas_get_command_count(canvas, &command_count) == 0);
    assert(command_count == 3);

    assert(picoui_native_event_bind_root(screen, window) == 0);
    assert(picoui_native_animation_get_rendered_frame_index(animation, &frame_index) == -1);
    assert(picoui_animation_show_frame(animation, 0) == 0);
    assert(picoui_native_canvas_get_rendered_command_count(canvas, &command_count) == -1);

    sleep_ms(50);
    assert(picoui_timer_handler() == 0);

    assert(picoui_native_animation_get_rendered_frame_index(animation, &frame_index) == 0);
    assert(frame_index == 1);

    assert(picoui_native_canvas_get_rendered_command_count(canvas, &command_count) == 0);
    assert(command_count == 3);
    assert(picoui_native_canvas_get_rendered_command_kind(canvas, 0, &command_kind) == 0);
    assert(command_kind == PICOUI_CANVAS_COMMAND_FILL_RECT);
    assert(picoui_native_canvas_get_rendered_command_kind(canvas, 1, &command_kind) == 0);
    assert(command_kind == PICOUI_CANVAS_COMMAND_DRAW_LINE);
    assert(picoui_native_canvas_get_rendered_command_kind(canvas, 2, &command_kind) == 0);
    assert(command_kind == 5);
    assert(picoui_native_canvas_get_rendered_dirty_rect(canvas, &dirty_rect) == 0);
    assert(dirty_rect.width > 0);
    assert(dirty_rect.height > 0);
    assert(picoui_native_canvas_get_rendered_changed_pixels(canvas, &changed_pixels) == 0);
    assert(changed_pixels > 0);

    picoui_deinit();
    return 0;
}
