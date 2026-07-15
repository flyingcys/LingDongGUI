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
#include "display/display.h"
#include "internal/runtime_internal_legacy_api.h"
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
    struct tinyui_app *app;
    struct tinyui_window *win;
    struct tinyui_label *label;
    struct tinyui_display_config cfg;

    if (tinyui_init() != TINYUI_OK) {
        printf("MCU_HOST_SMOKE_FAIL init\n");
        return 1;
    }
    app = tinyui_runtime_internal_app_current();
    if (app == NULL) {
        printf("MCU_HOST_SMOKE_FAIL app_current\n");
        tinyui_deinit();
        return 1;
    }

    /* MCU contract: application registers display config + flush + tick. */
    cfg.width = 480;
    cfg.height = 320;
    cfg.color_format = TINYUI_COLOR_FORMAT_RGB565;
    cfg.buffer_height = 40;
    cfg.user_data = NULL;
    if (tinyui_display_set_config(app, &cfg) != 0) {
        printf("MCU_HOST_SMOKE_FAIL set_config\n");
        tinyui_deinit();
        return 1;
    }
    if (tinyui_display_set_flush_callback(app, mem_flush, NULL) != 0) {
        printf("MCU_HOST_SMOKE_FAIL set_flush\n");
        tinyui_deinit();
        return 1;
    }
    if (tinyui_tick_set_source(app, fake_tick, NULL) != 0) {
        printf("MCU_HOST_SMOKE_FAIL set_tick\n");
        tinyui_deinit();
        return 1;
    }

    /* Trivial UI via the TinyUI public API (mirrors demo/hello_world). */
    win = (struct tinyui_window *)(void *)tinyui_screen_create();
    if (win == NULL) {
        printf("MCU_HOST_SMOKE_FAIL window_create\n");
        tinyui_deinit();
        return 1;
    }
    /* Explicit non-black background guarantees drawn (non-zero) pixels. */
    tinyui_window_set_color((tinyui_obj_t *)win, 0x2E3440);
    tinyui_flex_set_flow((tinyui_obj_t *)win, TINYUI_FLEX_FLOW_COLUMN);
    tinyui_flex_set_align((tinyui_obj_t *)win,
                          TINYUI_ALIGN_CENTER,
                          TINYUI_ALIGN_CENTER,
                          TINYUI_ALIGN_CENTER);

    label = (struct tinyui_label *)(void *)tinyui_label_create((tinyui_obj_t *)win);
    if (label == NULL) {
        printf("MCU_HOST_SMOKE_FAIL label_create\n");
        tinyui_deinit();
        return 1;
    }
    tinyui_label_set_text((tinyui_obj_t *)label, "Hello MCU");

    tinyui_runtime_internal_app_set_window(app, win);

    /* Drive frames through the backend-neutral (no-SDL) runtime loop. */
    for (int i = 0; i < 30; i++) {
        if (tinyui_backend_neutral_step(app) != 0) {
            printf("MCU_HOST_SMOKE_FAIL step i=%d\n", i);
            tinyui_deinit();
            return 1;
        }
    }

    tinyui_backend_neutral_shutdown(app);
    tinyui_deinit();

    if (g_flush_calls > 0 && g_nonzero_px > 0) {
        printf("MCU_HOST_SMOKE_OK flushes=%d nonzero=%ld\n",
               g_flush_calls, g_nonzero_px);
        return 0;
    }

    printf("MCU_HOST_SMOKE_FAIL flushes=%d nonzero=%ld\n",
           g_flush_calls, g_nonzero_px);
    return 1;
}
