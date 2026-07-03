/*
 * mcu_host_smoke.c
 *
 * Host verification harness for the TinyUI MCU (no-SDL) port.
 *
 * Proves, on the build host and WITHOUT linking SDL, that:
 *   1. the no-SDL bundle links (tinyui_port_mcu + tinyui_core + longdonggui +
 *      longdonggui_arm2d + tinyui_backend_ldgui_porting, NO tinyui_port_sdl),
 *   2. the backend-neutral runtime loop drives frames, and
 *   3. real pixels are produced and pushed through a user-supplied memory
 *      flush callback (i.e. TinyUI actually renders on the MCU path).
 *
 * Success line: "MCU_HOST_SMOKE_OK flushes=<n> nonzero=<m>", exit 0.
 */

#include "tinyui.h"
#include "tinyui_ldgui_port.h"   /* tinyui_backend_neutral_step */

#include <stdint.h>
#include <stdio.h>

/* ── User memory flush callback: counts frames and non-black pixels ───────── */
static int  g_flush_calls;
static long g_nonzero_px;

static void mem_flush(const struct tinyui_area *a, const void *px, void *ud)
{
    (void)ud;
    g_flush_calls++;
    const uint16_t *p = (const uint16_t *)px;
    for (int i = 0; i < a->width * a->height; i++) {
        if (p[i]) {
            g_nonzero_px++;
        }
    }
}

/* ── Fake monotonic tick (16 ms/frame) ────────────────────────────────────── */
static unsigned int t;
static unsigned int fake_tick(void *u)
{
    (void)u;
    return (t += 16);
}

int main(void)
{
    struct tinyui_app *app = tinyui_app_create();
    if (app == NULL) {
        printf("MCU_HOST_SMOKE_FAIL app_create\n");
        return 1;
    }

    /* MCU contract: application registers display config + flush + tick. */
    struct tinyui_display_config cfg;
    cfg.width = 480;
    cfg.height = 320;
    cfg.color_format = TINYUI_COLOR_FORMAT_RGB565;
    cfg.buffer_height = 40;
    cfg.user_data = NULL;
    if (tinyui_display_set_config(app, &cfg) != 0) {
        printf("MCU_HOST_SMOKE_FAIL set_config\n");
        return 1;
    }
    if (tinyui_display_set_flush_callback(app, mem_flush, NULL) != 0) {
        printf("MCU_HOST_SMOKE_FAIL set_flush\n");
        return 1;
    }
    if (tinyui_tick_set_source(app, fake_tick, NULL) != 0) {
        printf("MCU_HOST_SMOKE_FAIL set_tick\n");
        return 1;
    }

    /* Trivial UI via the TinyUI public API (mirrors demo/hello_world). */
    struct tinyui_window *win = tinyui_window_create(app, "root");
    if (win == NULL) {
        printf("MCU_HOST_SMOKE_FAIL window_create\n");
        return 1;
    }
    /* Explicit non-black background guarantees drawn (non-zero) pixels. */
    tinyui_window_set_color(win, 0x2E3440);
    tinyui_flex_set_flow(win, TINYUI_FLEX_FLOW_COLUMN);
    tinyui_flex_set_align(win,
                          TINYUI_ALIGN_CENTER,
                          TINYUI_ALIGN_CENTER,
                          TINYUI_ALIGN_CENTER);

    struct tinyui_label *label = tinyui_label_create(win, "title");
    if (label == NULL) {
        printf("MCU_HOST_SMOKE_FAIL label_create\n");
        return 1;
    }
    tinyui_label_set_text(label, "Hello MCU");

    tinyui_app_set_window(app, win);

    /* Drive frames through the backend-neutral (no-SDL) runtime loop. */
    for (int i = 0; i < 30; i++) {
        if (tinyui_backend_neutral_step(app) != 0) {
            printf("MCU_HOST_SMOKE_FAIL step i=%d\n", i);
            return 1;
        }
    }

    tinyui_backend_neutral_shutdown(app);

    if (g_flush_calls > 0 && g_nonzero_px > 0) {
        printf("MCU_HOST_SMOKE_OK flushes=%d nonzero=%ld\n",
               g_flush_calls, g_nonzero_px);
        return 0;
    }

    printf("MCU_HOST_SMOKE_FAIL flushes=%d nonzero=%ld\n",
           g_flush_calls, g_nonzero_px);
    return 1;
}
