#ifndef TINYUI_CANVAS_H
#define TINYUI_CANVAS_H

#include "image.h"
#include "theme.h"

struct picoui_window;
struct picoui_canvas;

struct picoui_canvas *picoui_canvas_create(struct picoui_window *parent, const char *id);

int picoui_canvas_clear(struct picoui_canvas *canvas);

int picoui_canvas_fill_rect(struct picoui_canvas *canvas,
                            int x,
                            int y,
                            int width,
                            int height,
                            unsigned int rgb,
                            int opacity);

int picoui_canvas_draw_line(struct picoui_canvas *canvas,
                            int x0,
                            int y0,
                            int x1,
                            int y1,
                            int line_size,
                            unsigned int rgb,
                            int opacity_max,
                            int opacity_min);

int picoui_canvas_draw_image(struct picoui_canvas *canvas,
                             int x,
                             int y,
                             int width,
                             int height,
                             struct picoui_image_source *source,
                             unsigned int mask_color,
                             int opacity);

int picoui_canvas_draw_image_scaled(struct picoui_canvas *canvas,
                                    int x,
                                    int y,
                                    int width,
                                    int height,
                                    struct picoui_image_source *source,
                                    float scale,
                                    int opacity);

int picoui_canvas_draw_text(struct picoui_canvas *canvas,
                            int x,
                            int y,
                            int width,
                            int height,
                            const char *text,
                            enum picoui_align align,
                            unsigned int text_color,
                            int opacity);

int picoui_canvas_get_command_count(const struct picoui_canvas *canvas, int *count);

#endif
