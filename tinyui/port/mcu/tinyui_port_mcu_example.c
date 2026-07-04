/*
 * tinyui_port_mcu_example.c
 *
 * COPY-PASTE TEMPLATE — documentation-as-code, NOT part of any build.
 *
 * This file shows the *application/board* glue an MCU integrator writes on top
 * of the TinyUI MCU port (tinyui_port_mcu.c).  The whole file is guarded by
 * TINYUI_PORT_MCU_EXAMPLE, which the build never defines, so it compiles to
 * nothing and can never break the build.  Copy the pieces you need into your
 * own project and wire them to your real LCD driver and SysTick.
 *
 * 用户模型(与 tests 的 mcu_host_smoke 一致,对齐 LVGL:帧循环归 core):
 *   - MCU 端无需定义任何 host 帧步进符号。
 *   - 建 app:tinyui_app_create()。
 *   - 首帧前注册能力:
 *       * 显示配置    (tinyui_display_set_config)
 *       * 显示 flush  (tinyui_display_set_flush_callback)
 *       * tick 源     (tinyui_tick_set_source)
 *       * 可选:os 锁/延时、触摸 tinyui_input_push_pointer
 *   - 然后 for(;;) 泵帧:tinyui_backend_neutral_step(app) —— 平台无关帧循环,
 *     未注册 read_cb/present_cb 时直接经 flush 回调把像素刷到屏。
 */

#if defined(TINYUI_PORT_MCU_EXAMPLE)

#include "tinyui.h"
#include "tinyui_ldgui_port.h"  /* tinyui_backend_neutral_step / _shutdown */

#include <stdint.h>

/* ── Placeholder board primitives (replace with your real LCD driver) ─────── */
extern void lcd_set_window(int x, int y, int width, int height);
extern void lcd_write_pixels(const void *rgb565_pixels, int pixel_count);

/* Millisecond counter incremented by your SysTick_Handler(). */
extern volatile uint32_t g_systick_ms;

/* ── Display flush: push one PFB tile to the panel ────────────────────────── */
static void mcu_lcd_flush(const struct tinyui_area *area,
                          const void *pixels,
                          void *user_data)
{
    (void)user_data;
    lcd_set_window(area->x, area->y, area->width, area->height);
    lcd_write_pixels(pixels, area->width * area->height);
}

/* ── Tick source: TinyUI reads "now" in milliseconds ──────────────────────── */
static unsigned int mcu_tick_ms(void *user_data)
{
    (void)user_data;
    return (unsigned int)g_systick_ms;
}

/* ── Optional: arm_2d reference clock override ─────────────────────────────────
 * tinyui_port_mcu.c already provides a WEAK 1 kHz (1 ms) default. If your board
 * timer runs at a different frequency, define this STRONG version to override it
 * (arm_2d derives its millisecond unit as freq/1000):
 *
 *   uint32_t arm_2d_helper_get_reference_clock_frequency(void) { return MY_HZ; }
 */

/* ── One-shot port wiring ─────────────────────────────────────────────────── */
int tinyui_port_mcu_init(struct tinyui_app *app, int width, int height)
{
    struct tinyui_display_config cfg;

    cfg.width = width;
    cfg.height = height;
    cfg.color_format = TINYUI_COLOR_FORMAT_RGB565;
    cfg.buffer_height = 40;   /* PFB tile height; tune for your RAM budget. */
    cfg.user_data = NULL;

    if (tinyui_display_set_config(app, &cfg) != 0) {
        return -1;
    }
    if (tinyui_display_set_flush_callback(app, mcu_lcd_flush, NULL) != 0) {
        return -1;
    }
    if (tinyui_tick_set_source(app, mcu_tick_ms, NULL) != 0) {
        return -1;
    }
    return 0;
}

/* ── Bare-metal main loop skeleton ────────────────────────────────────────── */
int main(void)
{
    /* board_init(); systick_init_1ms(); lcd_init(); */

    struct tinyui_app *app = tinyui_app_create();
    if (app == NULL) {
        return 1;
    }

    if (tinyui_port_mcu_init(app, 480, 320) != 0) {
        return 1;
    }

    /* Build your UI with the TinyUI public API, e.g.: */
    struct tinyui_window *win = tinyui_window_create(app, "root");
    struct tinyui_label *label = tinyui_label_create(win, "title");
    tinyui_label_set_text(label, "Hello MCU");
    tinyui_app_set_window(app, win);

    /* Frame pump: core 的平台无关帧循环直接经 flush 回调出像素。 */
    for (;;) {
        if (tinyui_backend_neutral_step(app) < 0) {
            break;
        }
    }

    tinyui_backend_neutral_shutdown(app);
    tinyui_app_destroy(app);
    return 0;
}

#endif /* TINYUI_PORT_MCU_EXAMPLE */
