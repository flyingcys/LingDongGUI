#ifndef PICOUI_IMAGE_H
#define PICOUI_IMAGE_H

struct picoui_window;
struct picoui_image;
struct picoui_image_source;

struct picoui_image *picoui_image_create(struct picoui_window *parent, const char *id);
int picoui_image_set_source(struct picoui_image *image, struct picoui_image_source *source);

#endif
