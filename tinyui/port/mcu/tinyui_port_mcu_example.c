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
 * Contract recap:
 *   - The core calls tinyui_runtime_host_step_app()/_shutdown_app(); the MCU
 *     port forwards those to tinyui_backend_neutral_step()/_shutdown().
 *   - Your application MUST register, before the first step:
 *       * a display config          (tinyui_display_set_config)
 *       * a display flush callback   (tinyui_display_set_flush_callback)
 *       * a tick source              (tinyui_tick_set_source)
 *   - Then just pump frames: tinyui_runtime_host_step_app(app) in your loop.
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

    /* Frame pump: the core drives the neutral runtime loop through the port. */
    for (;;) {
        if (tinyui_runtime_host_step_app(app) < 0) {
            break;
        }
    }

    tinyui_runtime_host_shutdown_app(app);
    tinyui_app_destroy(app);
    return 0;
}

#endif /* TINYUI_PORT_MCU_EXAMPLE */
