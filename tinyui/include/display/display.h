#ifndef TINYUI_DISPLAY_H
#define TINYUI_DISPLAY_H

struct tinyui_app;

enum tinyui_color_format {
    TINYUI_COLOR_FORMAT_RGB565 = 0,
    TINYUI_COLOR_FORMAT_ARGB8888,
};

struct tinyui_area {
    int x;
    int y;
    int width;
    int height;
};

struct tinyui_display_config {
    int width;
    int height;
    enum tinyui_color_format color_format;
    int buffer_height;
    void *user_data;
};

typedef void (*tinyui_display_flush_cb_t)(const struct tinyui_area *area,
                                          const void *pixels,
                                          void *user_data);

int tinyui_display_set_config(struct tinyui_app *app,
                              const struct tinyui_display_config *config);
int tinyui_display_get_config(const struct tinyui_app *app,
                              struct tinyui_display_config *out_config);
int tinyui_display_set_flush_callback(struct tinyui_app *app,
                                      tinyui_display_flush_cb_t callback,
                                      void *user_data);

typedef void (*tinyui_display_present_cb_t)(void *user_data);

/* 帧渲染完成后由 core 循环调用一次;窗口后端在此把缓冲推上屏幕
 * (SDL: texture upload + RenderPresent)。直刷 LCD 的 MCU 通常留 NULL。 */
int tinyui_display_set_present_callback(struct tinyui_app *app,
                                        tinyui_display_present_cb_t callback,
                                        void *user_data);

#endif
