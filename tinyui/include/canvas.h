#ifndef TINYUI_CANVAS_H
#define TINYUI_CANVAS_H

#include "image.h"
#include "theme.h"

struct tinyui_window;
struct tinyui_canvas;

struct tinyui_canvas *tinyui_canvas_create(struct tinyui_window *parent, const char *id);

int tinyui_canvas_clear(struct tinyui_canvas *canvas);

int tinyui_canvas_fill_rect(struct tinyui_canvas *canvas,
                            int x,
                            int y,
                            int width,
                            int height,
                            unsigned int rgb,
                            int opacity);

int tinyui_canvas_draw_line(struct tinyui_canvas *canvas,
                            int x0,
                            int y0,
                            int x1,
                            int y1,
                            int line_size,
                            unsigned int rgb,
                            int opacity_max,
                            int opacity_min);

int tinyui_canvas_draw_image(struct tinyui_canvas *canvas,
                             int x,
                             int y,
                             int width,
                             int height,
                             struct tinyui_image_source *source,
                             unsigned int mask_color,
                             int opacity);

int tinyui_canvas_draw_image_scaled(struct tinyui_canvas *canvas,
                                    int x,
                                    int y,
                                    int width,
                                    int height,
                                    struct tinyui_image_source *source,
                                    float scale,
                                    int opacity);

int tinyui_canvas_draw_text(struct tinyui_canvas *canvas,
                            int x,
                            int y,
                            int width,
                            int height,
                            const char *text,
                            enum tinyui_align align,
                            unsigned int text_color,
                            int opacity);

int tinyui_canvas_get_command_count(const struct tinyui_canvas *canvas, int *count);

#endif
