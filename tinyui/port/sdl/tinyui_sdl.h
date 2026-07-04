#ifndef TINYUI_PORT_SDL_H
#define TINYUI_PORT_SDL_H

/*
 * TinyUI SDL 平台驱动 —— 对齐 LVGL 的 lv_sdl_window_create / lv_sdl_mouse_create /
 * lv_sdl_quit。应用只需:tinyui_init() → tinyui_sdl_window_create(w,h) →
 * tinyui_sdl_mouse_create() → 建 UI → for(;;) tinyui_timer_handler();
 *
 * 帧循环归 core(tinyui_timer_handler);本驱动只注册能力:
 *   window_create → display flush + present + tick
 *   mouse_create  → indev read_cb(泵 SDL 事件)
 */

/* 建 SDL 窗口/渲染器/纹理与像素缓冲,并向当前 runtime app 注册
 * display flush + present 回调与(未设时的)默认 tick/delay。
 * 分辨率来自 width/height;像素格式/PFB 行高沿用已设的显示配置。
 * 返回 0 成功,-1 失败。对齐 LVGL lv_sdl_window_create。 */
int tinyui_sdl_window_create(int width, int height);

/* 向当前 runtime app 注册 indev read_cb:每帧泵 SDL 事件并 push 指针;
 * SDL_QUIT 时请求退出。返回 0 成功,-1 失败。对齐 LVGL lv_sdl_mouse_create。 */
int tinyui_sdl_mouse_create(void);

/* 释放 SDL 窗口/渲染器/纹理与缓冲并 SDL_Quit。对齐 LVGL lv_sdl_quit。 */
void tinyui_sdl_quit(void);

#endif /* TINYUI_PORT_SDL_H */
