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
 * 用户模型（canonical v2.3，对齐 LVGL 风格；帧循环归 core）：
 *   - MCU 端无需定义任何 host 帧步进符号。
 *   - 使用 tinyui_init() / tinyui_screen_create() / tinyui_screen_load() /
 *     tinyui_process()，不要使用已删除的 tinyui_app_* 迁移桥。
 *   - 首帧前注册能力（port 细节仍延期，见 docs/v2.3/deferred-port-work.md）：
 *       * 显示配置 / flush / tick 等 integration 头
 *       * 可选：os 锁/延时、触摸 tinyui_input_send_* / push
 *   - 然后 for(;;) { tinyui_process(&next_ms); } 由用户/port 负责 sleep。
 *
 * 注意：本文件不是 L6 或生产可用证明；M4 SDL consumer/demo 也只是测试宿主证据。
 */

#if defined(TINYUI_PORT_MCU_EXAMPLE)

#include "tinyui.h"

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

/*
 * One-shot port wiring skeleton.
 * 真实 display/tick integration 签名以 integration/* 与 deferred-port-work 为准；
 * 此处仅示意“在 init 之后、process 之前完成注册”。
 */
static int tinyui_port_mcu_board_setup(int width, int height)
{
    (void)width;
    (void)height;
    (void)mcu_lcd_flush;
    (void)mcu_tick_ms;
    /* board_init(); systick_init_1ms(); lcd_init();
     * tinyui_display_set_config(...);
     * tinyui_display_set_flush_callback(..., mcu_lcd_flush, NULL);
     * tinyui_tick_set_source(..., mcu_tick_ms, NULL);
     */
    return 0;
}

/* ── Bare-metal main loop skeleton ────────────────────────────────────────── */
int main(void)
{
    uint32_t next_ms = 0;

    if (tinyui_init() != TINYUI_OK) {
        return 1;
    }

    if (tinyui_port_mcu_board_setup(480, 320) != 0) {
        tinyui_deinit();
        return 1;
    }

    /* Build UI with the TinyUI public API only. */
    tinyui_obj_t *screen = tinyui_screen_create();
    if (screen == 0) {
        tinyui_deinit();
        return 1;
    }

    tinyui_obj_t *label = tinyui_label_create(screen);
    if (label != 0) {
        (void)tinyui_obj_set_text(label, "Hello MCU");
    }

    if (tinyui_screen_load(screen, TINYUI_SCREEN_TRANSITION_NONE, 0) != TINYUI_OK) {
        tinyui_deinit();
        return 1;
    }

    for (;;) {
        if (tinyui_process(&next_ms) != TINYUI_OK) {
            break;
        }
        /* board_sleep_ms(next_ms); — RTOS/idle 由集成方持有 */
    }

    tinyui_deinit();
    return 0;
}

#endif /* TINYUI_PORT_MCU_EXAMPLE */
