/*
 * tinyui_port_mcu.c
 *
 * TinyUI MCU (no-SDL) platform port.
 *
 * This is the port's contribution to the core frame-driver *link contract*:
 * tinyui/src/core/runtime_bridge.c unconditionally calls
 *   tinyui_runtime_host_step_app(app)  and
 *   tinyui_runtime_host_shutdown_app(app)
 * On an SDL build these are provided by tinyui/port/sdl/step.c; on an
 * MCU-style build they are provided here by forwarding to the backend-neutral
 * runtime loop (tinyui_ldgui_neutral_runtime.c).
 *
 * This file must compile for *any* target: it contains NO SDL and NO hardware
 * access.  The user's board glue (LCD flush, tick source) lives in the
 * application layer — see tinyui_port_mcu_example.c for a copy-paste template.
 */

#include "tinyui_ldgui_port.h"  /* tinyui_backend_neutral_step / _shutdown,
                                 * forward decl of struct tinyui_app */

#include <stdint.h>

/* ── Core frame-driver link contract ─────────────────────────────────────── */

int tinyui_runtime_host_step_app(struct tinyui_app *app)
{
    return tinyui_backend_neutral_step(app);
}

void tinyui_runtime_host_shutdown_app(struct tinyui_app *app)
{
    tinyui_backend_neutral_shutdown(app);
}

/* ── arm_2d helper reference clock ────────────────────────────────────────
 * arm_2d_helper.c REFERENCES this symbol (arm_2d_helper.c:137, wMSUnit =
 * freq/1000) but does not define it in this vendored ARM-2D; on the SDL path
 * it is supplied by the demo runtime stub. This port provides the sole default
 * for MCU builds. It is declared weak so a board/BSP can override it with a
 * STRONG definition returning the real timer frequency. 1 kHz == 1 ms tick. */
__attribute__((weak))
uint32_t arm_2d_helper_get_reference_clock_frequency(void)
{
    return 1000u;
}
