#ifndef TINYUI_LDGUI_PORT_CONFIG_H
#define TINYUI_LDGUI_PORT_CONFIG_H

/* ARM-2D 功能特性开关 — backend 内部，用户不直接接触 */
#define __ARM_2D_HAS_ASYNC__                        0
#define __ARM_2D_HAS_ANTI_ALIAS_TRANSFORM__         1

/* 颜色深度：16-bit RGB565 */
#define __GLCD_CFG_COLOUR_DEPTH__                   16

/* PFB 使用堆分配，数量设为 0 让 disp_adapter 自己分配 */
#define __DISP0_CFG_PFB_HEAP_SIZE__                 0

/* 关闭不需要的旋转 */
#define __DISP0_CFG_ROTATE_SCREEN__                 0

/* 屏幕宽高占位宏，实际值由运行期 tinyui_display_set_config() 传入      */
/* 这里只是让依赖这些宏的 ARM-2D 头文件能通过编译，实际不在 PFB 分配中使用 */
#ifndef LD_CFG_SCREEN_WIDTH
#   define LD_CFG_SCREEN_WIDTH                      480
#endif
#ifndef LD_CFG_SCREEN_HEIGHT
#   define LD_CFG_SCREEN_HEIGHT                     320
#endif
#ifndef LD_CFG_PFB_LINES
#   define LD_CFG_PFB_LINES                         40
#endif

#endif /* TINYUI_LDGUI_PORT_CONFIG_H */
