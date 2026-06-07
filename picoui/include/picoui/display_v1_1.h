#ifndef PICOUI_DISPLAY_V1_1_H
#define PICOUI_DISPLAY_V1_1_H

struct picoui_display;

enum picoui_color_format {
    PICOUI_COLOR_FORMAT_RGB565 = 0,
    PICOUI_COLOR_FORMAT_ARGB8888,
};

struct picoui_area {
    int x;
    int y;
    int width;
    int height;
};

typedef void (*picoui_display_flush_cb_t)(const struct picoui_area *area,
                                          const void *pixels,
                                          void *user_data);

struct picoui_display *picoui_display_create(int width, int height);
int picoui_display_set_default(struct picoui_display *display);
struct picoui_display *picoui_display_get_default(void);
int picoui_display_get_size(const struct picoui_display *display, int *width, int *height);
int picoui_display_set_flush_cb(struct picoui_display *display,
                                picoui_display_flush_cb_t callback,
                                void *user_data);

#endif
