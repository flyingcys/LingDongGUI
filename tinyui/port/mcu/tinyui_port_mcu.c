/*
 * tinyui_port_mcu.c
 *
 * TinyUI MCU (no-SDL) platform port.
 *
 * 帧循环归 core:tinyui/src/core/runtime_bridge.c 直接驱动平台无关的
 * tinyui_backend_neutral_step / _shutdown(tinyui_ldgui_neutral_runtime.c),
 * MCU 端**无需**再定义任何 host 帧步进符号 —— 只需在应用层注册显示 flush
 * 回调与 tick 源(可选 os 锁/延时、触摸 push),然后 for(;;) tinyui_timer_handler()。
 *
 * 本文件唯一职责是给 MCU 构建提供 arm_2d 参考时钟的默认实现(见下);它
 * 必须能为**任意**目标编译:不含 SDL、不含硬件访问。用户的板级胶水(LCD
 * flush、tick 源)在应用层 —— 见 tinyui_port_mcu_example.c 模板。
 */

#include <stdint.h>

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
