#ifndef TINYUI_DISPLAY_H
#define TINYUI_DISPLAY_H

struct picoui_app;

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

struct picoui_display_config {
    int width;
    int height;
    enum picoui_color_format color_format;
    int buffer_height;
    void *user_data;
};

typedef void (*picoui_display_flush_cb_t)(const struct picoui_area *area,
                                          const void *pixels,
                                          void *user_data);

int picoui_display_set_config(struct picoui_app *app,
                              const struct picoui_display_config *config);
int picoui_display_get_config(const struct picoui_app *app,
                              struct picoui_display_config *out_config);
int picoui_display_set_flush_callback(struct picoui_app *app,
                                      picoui_display_flush_cb_t callback,
                                      void *user_data);

#endif
