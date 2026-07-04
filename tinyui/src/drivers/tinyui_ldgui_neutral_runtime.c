/*
 * tinyui_ldgui_neutral_runtime.c
 *
 * Backend-neutral (no-SDL) frame loop for TinyUI ports.
 *
 * core 的 runtime_bridge 直接驱动这里:一次性 setup + 每帧 step。SDL 特有的
 * 窗口/事件泵/present/观测通过 hal.c 的 read_cb/present_cb 钩子注册接入,本文件
 * 不含任何 SDL/observe/capture/auto-quit(平台无关,MCU 复用同一循环)。
 *
 * It provides two STRONG helpers that any platform port (mcu / none) can
 * forward the core frame-driver contract to:
 *     int  tinyui_backend_neutral_step(struct tinyui_app *app);
 *     void tinyui_backend_neutral_shutdown(struct tinyui_app *app);
 *
 * The application (MCU contract) is responsible for registering the display
 * flush callback and tick source before the first step; this file never
 * touches SDL and never installs a flush callback.
 *
 * Compiled into the tinyui_backend_ldgui_porting library.  Must not depend
 * on SDL.
 */

#include "tinyui_ldgui_port.h"

#include "internal.h"       /* struct tinyui_app, tinyui_app_pump_timers,
                             * tinyui_tick_get, tinyui_os_delay (via umbrella) */
#include "runtime_bridge.h" /* tinyui_runtime_bridge_init_app */

#include "ldConfig.h"       /* USE_LOG_LEVEL / LOG_LEVEL_INFO */
#include "ldBase.h"         /* ld_scene_t, ldPageFuncGroup_t */
#include "ldGui.h"          /* ldGuiSceneInit */

#include <stddef.h>

/* Provided by tinyui_ldgui_disp_adapter.c (same library). */
int  tinyui_backend_init(struct tinyui_app *app);
void tinyui_backend_step(struct tinyui_app *app);

/* ─── Static no-op page group ──────────────────────────────────────────────
 * Mirrors g_tinyui_runtime_host_page from tinyui/port/sdl/step.c so that
 * ldGuiSceneInit()/frame processing has a valid ldGuiFuncGroup with no-op
 * init/quit hooks.
 * ──────────────────────────────────────────────────────────────────────── */
static void tinyui_backend_neutral_page_init(ld_scene_t *scene)
{
    (void)scene;
}

static void tinyui_backend_neutral_page_quit(ld_scene_t *scene)
{
    (void)scene;
}

static const ldPageFuncGroup_t g_tinyui_backend_neutral_page = {
    .init = tinyui_backend_neutral_page_init,
    .loop = NULL,
    .quit = tinyui_backend_neutral_page_quit,
    .draw = NULL,
    .frameStart = NULL,
    .frameComplete = NULL,
#if (USE_LOG_LEVEL >= LOG_LEVEL_INFO)
    .pageName = "tinyui_neutral",
#endif
    .pointer = NULL,
};

/* ─── tinyui_backend_neutral_step ──────────────────────────────────────────
 * One-time setup (guarded) followed by a single frame step.  Non-SDL mirror
 * of step.c's prepare_runtime + step body:
 *   setup : ensure ld_scene, install neutral page group, tinyui_backend_init
 *           (allocates PFB when a flush callback is registered), ldGuiSceneInit
 *   step  : pump app timers, tinyui_backend_step (renders + flushes tiles via
 *           app->display_port.flush_callback), then os delay ~16ms (no-op if
 *           no delay callback registered).
 * Returns 0 on success, -1 on a bad app.
 * ──────────────────────────────────────────────────────────────────────── */
/* One-shot setup flag. Single-app by design: the ldgui disp adapter itself is
 * single-instance (static PFB helper / s_pfb_mem in tinyui_ldgui_disp_adapter.c),
 * so the whole backend supports one app at a time — matching the MCU model.
 * Reset by tinyui_backend_neutral_shutdown() so a fresh app can be brought up
 * after a teardown. */
static int s_neutral_inited;

int tinyui_backend_neutral_step(struct tinyui_app *app)
{
    if (app == NULL) {
        return -1;
    }

    if (!s_neutral_inited) {
        /* tinyui_backend_init() also calls this, but keep the SDL parity:
         * ensure the ld scene exists before touching it. */
        if (tinyui_runtime_bridge_init_app(app) != 0) {
            return -1;
        }
        if (app->ld_scene == NULL) {
            return -1;
        }

        app->ld_scene->ldGuiFuncGroup = &g_tinyui_backend_neutral_page;

        if (tinyui_backend_init(app) != 0) {
            return -1;
        }

        if (app->ld_scene->ptMsgQueue == NULL) {
            ldGuiSceneInit(app->ld_scene);
        }

        s_neutral_inited = 1;
    }

    /* LVGL 式帧:read_cb(采集/退出) → pump_timers → backend_step(→flush_cb)
     * → present_cb(上屏) → os delay。read_cb/present_cb 均为可选;全 NULL
     * 时行为与改造前逐字一致(MCU 直刷路径不变)。 */
    if (app->input_port.read_callback != NULL) {
        int rc = app->input_port.read_callback(app, app->input_port.read_user_data);
        if (rc != 0) {
            return rc;   /* >0 退出,<0 错误,直接冒泡给 timer_handler */
        }
    }

    /* NOTE: the application must have registered a tick source
     * (tinyui_tick_set_source) — otherwise tinyui_tick_get() returns 0 every
     * frame and timers/animations never advance (first frame still paints). */
    tinyui_app_pump_timers(app, tinyui_tick_get(app));
    tinyui_backend_step(app);

    if (app->display_port.present_callback != NULL) {
        app->display_port.present_callback(app->display_port.present_user_data);
    }

    tinyui_os_delay(app, 16);

    return 0;
}

/* ─── tinyui_backend_neutral_shutdown ──────────────────────────────────────
 * Neutral cleanup.  The SDL shutdown frees SDL window/renderer/texture and
 * runtime_state; the neutral loop allocates none of that, so there is nothing
 * to release here.
 * ──────────────────────────────────────────────────────────────────────── */
void tinyui_backend_neutral_shutdown(struct tinyui_app *app)
{
    (void)app;
    /* Allow a subsequent neutral_step() to re-run one-shot setup after the
     * core has torn down the scene (tinyui_runtime_bridge_shutdown_app frees
     * app->ld_scene). Without this, a re-init would be skipped and rendering
     * would silently stop. */
    s_neutral_inited = 0;
}
