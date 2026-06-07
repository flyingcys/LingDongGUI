#include "picoui/widget.h"
#include "../core/internal.h"

#include <stdlib.h>

static void picoui_native_canvas_clear_dirty_rect(struct picoui_rect *dirty_rect)
{
    if (dirty_rect == 0) {
        return;
    }

    dirty_rect->x = 0;
    dirty_rect->y = 0;
    dirty_rect->width = 0;
    dirty_rect->height = 0;
}

static unsigned int picoui_native_canvas_pack_color(unsigned int rgb, int opacity)
{
    return ((unsigned int)(opacity & 0xFF) << 24) | (rgb & 0x00FFFFFFU);
}

static void picoui_native_canvas_mark_pixel(unsigned int *buffer,
                                            int width,
                                            int height,
                                            int x,
                                            int y,
                                            unsigned int color,
                                            struct picoui_rect *dirty_rect,
                                            int *has_dirty)
{
    if (buffer == 0 || x < 0 || y < 0 || x >= width || y >= height) {
        return;
    }

    buffer[y * width + x] = color;

    if (*has_dirty == 0) {
        dirty_rect->x = x;
        dirty_rect->y = y;
        dirty_rect->width = 1;
        dirty_rect->height = 1;
        *has_dirty = 1;
        return;
    }

    if (x < dirty_rect->x) {
        dirty_rect->width += dirty_rect->x - x;
        dirty_rect->x = x;
    } else if (x >= dirty_rect->x + dirty_rect->width) {
        dirty_rect->width = x - dirty_rect->x + 1;
    }

    if (y < dirty_rect->y) {
        dirty_rect->height += dirty_rect->y - y;
        dirty_rect->y = y;
    } else if (y >= dirty_rect->y + dirty_rect->height) {
        dirty_rect->height = y - dirty_rect->y + 1;
    }
}

static void picoui_native_canvas_fill_rect_command(const struct picoui_canvas_command *command,
                                                   unsigned int *buffer,
                                                   int width,
                                                   int height,
                                                   struct picoui_rect *dirty_rect,
                                                   int *has_dirty)
{
    unsigned int color;
    int x0;
    int y0;
    int x1;
    int y1;
    int x;
    int y;

    if (command->width <= 0 || command->height <= 0) {
        return;
    }

    color = picoui_native_canvas_pack_color(command->rgb0, command->opacity0);
    x0 = command->x < 0 ? 0 : command->x;
    y0 = command->y < 0 ? 0 : command->y;
    x1 = command->x + command->width;
    y1 = command->y + command->height;
    if (x1 > width) {
        x1 = width;
    }
    if (y1 > height) {
        y1 = height;
    }

    for (y = y0; y < y1; ++y) {
        for (x = x0; x < x1; ++x) {
            picoui_native_canvas_mark_pixel(buffer, width, height, x, y, color, dirty_rect, has_dirty);
        }
    }
}

static void picoui_native_canvas_draw_line_command(const struct picoui_canvas_command *command,
                                                   unsigned int *buffer,
                                                   int width,
                                                   int height,
                                                   struct picoui_rect *dirty_rect,
                                                   int *has_dirty)
{
    unsigned int color;
    int dx;
    int dy;
    int sx;
    int sy;
    int err;
    int e2;
    int current_x;
    int current_y;
    int offset_x;
    int offset_y;
    int radius;

    if (command->line_size <= 0) {
        return;
    }

    color = picoui_native_canvas_pack_color(command->rgb0, command->opacity0);
    current_x = command->x;
    current_y = command->y;
    dx = abs(command->x1 - command->x);
    dy = abs(command->y1 - command->y);
    sx = command->x < command->x1 ? 1 : -1;
    sy = command->y < command->y1 ? 1 : -1;
    err = dx - dy;
    radius = command->line_size / 2;

    while (1) {
        for (offset_y = -radius; offset_y <= radius; ++offset_y) {
            for (offset_x = -radius; offset_x <= radius; ++offset_x) {
                picoui_native_canvas_mark_pixel(buffer,
                                                width,
                                                height,
                                                current_x + offset_x,
                                                current_y + offset_y,
                                                color,
                                                dirty_rect,
                                                has_dirty);
            }
        }

        if (current_x == command->x1 && current_y == command->y1) {
            break;
        }

        e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            current_x += sx;
        }
        if (e2 < dx) {
            err += dx;
            current_y += sy;
        }
    }
}

static void picoui_native_canvas_draw_circle_command(const struct picoui_canvas_command *command,
                                                     unsigned int *buffer,
                                                     int width,
                                                     int height,
                                                     struct picoui_rect *dirty_rect,
                                                     int *has_dirty)
{
    unsigned int color;
    int x;
    int y;
    int radius_sq;
    int min_radius_sq;

    if (command->line_size <= 0) {
        return;
    }

    color = picoui_native_canvas_pack_color(command->rgb0, command->opacity0);
    radius_sq = command->line_size * command->line_size;
    min_radius_sq = (command->line_size - 1) * (command->line_size - 1);

    for (y = command->y - command->line_size; y <= command->y + command->line_size; ++y) {
        for (x = command->x - command->line_size; x <= command->x + command->line_size; ++x) {
            int dx = x - command->x;
            int dy = y - command->y;
            int distance_sq = dx * dx + dy * dy;

            if (distance_sq <= radius_sq && distance_sq >= min_radius_sq) {
                picoui_native_canvas_mark_pixel(buffer, width, height, x, y, color, dirty_rect, has_dirty);
            }
        }
    }
}

int picoui_native_canvas_render_buffer(const struct picoui_canvas *canvas,
                                       unsigned int *buffer,
                                       int width,
                                       int height,
                                       struct picoui_rect *dirty_rect)
{
    int index;
    int has_dirty = 0;

    picoui_native_canvas_clear_dirty_rect(dirty_rect);

    if (canvas == 0 || buffer == 0 || width <= 0 || height <= 0 || dirty_rect == 0
        || canvas->command_count < 0 || canvas->command_count > PICOUI_CANVAS_MAX_COMMANDS) {
        return -1;
    }

    for (index = 0; index < canvas->command_count; ++index) {
        const struct picoui_canvas_command *command = &canvas->commands[index];
        int is_circle = command->kind == PICOUI_CANVAS_COMMAND_DRAW_LINE
            && command->x == command->x1
            && command->y == command->y1
            && command->line_size > 0;

        if (command->kind == PICOUI_CANVAS_COMMAND_FILL_RECT) {
            picoui_native_canvas_fill_rect_command(command, buffer, width, height, dirty_rect, &has_dirty);
        } else if (is_circle != 0) {
            picoui_native_canvas_draw_circle_command(command, buffer, width, height, dirty_rect, &has_dirty);
        } else if (command->kind == PICOUI_CANVAS_COMMAND_DRAW_LINE) {
            picoui_native_canvas_draw_line_command(command, buffer, width, height, dirty_rect, &has_dirty);
        }
    }

    return 0;
}
