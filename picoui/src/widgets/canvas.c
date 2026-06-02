#include "internal.h"
#include "picoui/canvas.h"

#include <stdlib.h>

static int picoui_canvas_is_valid(const struct picoui_canvas *canvas)
{
    return canvas != 0 && canvas->widget.backend_widget != 0;
}

static int picoui_canvas_push(struct picoui_canvas *canvas,
                              const struct picoui_canvas_command *command)
{
    if (!picoui_canvas_is_valid(canvas)
        || command == 0
        || canvas->command_count >= PICOUI_CANVAS_MAX_COMMANDS) {
        return -1;
    }

    canvas->commands[canvas->command_count++] = *command;
    return picoui_backend_canvas_sync(canvas);
}

struct picoui_canvas *picoui_canvas_create(struct picoui_window *parent, const char *id)
{
    struct picoui_canvas *canvas;

    if (parent == 0 || id == 0) {
        return 0;
    }

    canvas = calloc(1, sizeof(*canvas));
    if (canvas == 0) {
        return 0;
    }

    canvas->widget.backend_widget = picoui_backend_create_canvas(parent->widget.backend_widget, id);
    if (canvas->widget.backend_widget == 0) {
        free(canvas);
        return 0;
    }

    canvas->id = id;
    canvas->widget.visible = 1;
    canvas->widget.enabled = 1;
    return canvas;
}

int picoui_canvas_clear(struct picoui_canvas *canvas)
{
    if (!picoui_canvas_is_valid(canvas)) {
        return -1;
    }

    canvas->command_count = 0;
    return picoui_backend_canvas_sync(canvas);
}

int picoui_canvas_fill_rect(struct picoui_canvas *canvas,
                            int x,
                            int y,
                            int width,
                            int height,
                            unsigned int rgb,
                            int opacity)
{
    struct picoui_canvas_command command;

    if (width < 0 || height < 0 || opacity < 0 || opacity > 255) {
        return -1;
    }

    command = (struct picoui_canvas_command){
        .kind = PICOUI_CANVAS_COMMAND_FILL_RECT,
        .x = x,
        .y = y,
        .width = width,
        .height = height,
        .rgb0 = rgb,
        .opacity0 = opacity,
    };
    return picoui_canvas_push(canvas, &command);
}

int picoui_canvas_draw_line(struct picoui_canvas *canvas,
                            int x0,
                            int y0,
                            int x1,
                            int y1,
                            int line_size,
                            unsigned int rgb,
                            int opacity_max,
                            int opacity_min)
{
    struct picoui_canvas_command command;

    if (line_size <= 0
        || opacity_max < 0
        || opacity_max > 255
        || opacity_min < 0
        || opacity_min > 255) {
        return -1;
    }

    command = (struct picoui_canvas_command){
        .kind = PICOUI_CANVAS_COMMAND_DRAW_LINE,
        .x = x0,
        .y = y0,
        .x1 = x1,
        .y1 = y1,
        .line_size = line_size,
        .rgb0 = rgb,
        .opacity0 = opacity_max,
        .opacity1 = opacity_min,
    };
    return picoui_canvas_push(canvas, &command);
}

int picoui_canvas_draw_image(struct picoui_canvas *canvas,
                             int x,
                             int y,
                             int width,
                             int height,
                             struct picoui_image_source *source,
                             unsigned int mask_color,
                             int opacity)
{
    struct picoui_canvas_command command;

    if (source == 0 || source->img_tile == 0 || width < 0 || height < 0 || opacity < 0 || opacity > 255) {
        return -1;
    }

    command = (struct picoui_canvas_command){
        .kind = PICOUI_CANVAS_COMMAND_DRAW_IMAGE,
        .x = x,
        .y = y,
        .width = width,
        .height = height,
        .rgb0 = mask_color,
        .opacity0 = opacity,
        .source = source,
    };
    return picoui_canvas_push(canvas, &command);
}

int picoui_canvas_draw_image_scaled(struct picoui_canvas *canvas,
                                    int x,
                                    int y,
                                    int width,
                                    int height,
                                    struct picoui_image_source *source,
                                    float scale,
                                    int opacity)
{
    struct picoui_canvas_command command;

    if (source == 0 || source->img_tile == 0 || width < 0 || height < 0 || scale <= 0.0f || opacity < 0 || opacity > 255) {
        return -1;
    }

    command = (struct picoui_canvas_command){
        .kind = PICOUI_CANVAS_COMMAND_DRAW_IMAGE_SCALED,
        .x = x,
        .y = y,
        .width = width,
        .height = height,
        .scale = scale,
        .opacity0 = opacity,
        .source = source,
    };
    return picoui_canvas_push(canvas, &command);
}

int picoui_canvas_draw_text(struct picoui_canvas *canvas,
                            int x,
                            int y,
                            int width,
                            int height,
                            const char *text,
                            enum picoui_align align,
                            unsigned int text_color,
                            int opacity)
{
    struct picoui_canvas_command command;

    if (text == 0
        || width < 0
        || height < 0
        || opacity < 0
        || opacity > 255
        || (align != PICOUI_ALIGN_START && align != PICOUI_ALIGN_CENTER && align != PICOUI_ALIGN_END)) {
        return -1;
    }

    command = (struct picoui_canvas_command){
        .kind = PICOUI_CANVAS_COMMAND_DRAW_TEXT,
        .x = x,
        .y = y,
        .width = width,
        .height = height,
        .rgb0 = text_color,
        .opacity0 = opacity,
        .align = align,
        .text = text,
    };
    return picoui_canvas_push(canvas, &command);
}

int picoui_canvas_get_command_count(const struct picoui_canvas *canvas, int *count)
{
    if (!picoui_canvas_is_valid(canvas) || count == 0) {
        return -1;
    }

    *count = canvas->command_count;
    return 0;
}
