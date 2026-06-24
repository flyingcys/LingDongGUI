#ifndef TINYUI_BACKGROUND_H
#define TINYUI_BACKGROUND_H

struct tinyui_app;
struct tinyui_background;
struct tinyui_image_source;

struct tinyui_background *tinyui_background_create(struct tinyui_app *app, const char *id);
int tinyui_background_set_source(struct tinyui_background *background,
                                 struct tinyui_image_source *source);
int tinyui_background_set_color(struct tinyui_background *background, unsigned int rgb);
int tinyui_background_get_color(struct tinyui_background *background, unsigned int *rgb);
int tinyui_background_set_offset(struct tinyui_background *background, int offset_x, int offset_y);
int tinyui_background_get_offset(struct tinyui_background *background,
                                 int *offset_x,
                                 int *offset_y);

#endif
