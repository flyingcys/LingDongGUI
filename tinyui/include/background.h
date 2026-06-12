#ifndef TINYUI_BACKGROUND_H
#define TINYUI_BACKGROUND_H

struct picoui_app;
struct picoui_background;
struct picoui_image_source;

struct picoui_background *picoui_background_create(struct picoui_app *app, const char *id);
int picoui_background_set_source(struct picoui_background *background,
                                 struct picoui_image_source *source);
int picoui_background_set_color(struct picoui_background *background, unsigned int rgb);
int picoui_background_get_color(struct picoui_background *background, unsigned int *rgb);
int picoui_background_set_offset(struct picoui_background *background, int offset_x, int offset_y);
int picoui_background_get_offset(struct picoui_background *background,
                                 int *offset_x,
                                 int *offset_y);

#endif
