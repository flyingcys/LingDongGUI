/*
 * tinyui_ldgui_disp_adapter.c
 *
 * 运行期 PFB display adapter。
 * 提供 tinyui_backend_init 和 tinyui_backend_step 公共 API。
 *
 * MCU 路径：动态分配 PFB，通过 flush_callback 推送像素到硬件。
 * SDL/host 路径：flush_callback == NULL，跳过 PFB 初始化，依赖
 *               Disp0_DrawBitmap（在 tinyui_ldgui_port.c 中）转发。
 */

#include "tinyui_ldgui_port.h"
#include "tinyui_ldgui_port_config.h"
#include "internal.h"
#include "runtime_bridge.h"

#include "arm_2d.h"
#include "arm_2d_helper.h"
#include "arm_2d_helper_pfb.h"
#include "ldGui.h"
#include "ldMsg.h"
#include "ldMem.h"

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* ─── PFB helper 单例 ─── */
static arm_2d_helper_pfb_t s_tPFBHelper;
static void               *s_pfb_mem;
static int                 s_pfb_inited;

/* ─── PFB draw handler ─── */
IMPL_PFB_ON_DRAW(ldgui_port_pfb_draw_handler)
{
    struct tinyui_app *app = (struct tinyui_app *)pTarget;
    struct tinyui_backend_app_state *app_state = tinyui_runtime_bridge_backend_state(app);
    if (app_state != NULL && app_state->ld_scene != NULL) {
        ldGuiDraw(app_state->ld_scene, (arm_2d_tile_t *)ptTile, bIsNewFrame);
    }
    return arm_fsm_rt_cpl;
}

/* ─── PFB flush handler ─── */
IMPL_PFB_ON_LOW_LV_RENDERING(ldgui_port_pfb_flush_handler)
{
    const arm_2d_tile_t *ptTile = &(ptPFB->tTile);
    struct tinyui_app *app = (struct tinyui_app *)pTarget;

    if (app != NULL && app->display_port.flush_callback != NULL) {
        struct tinyui_area area;
        area.x      = (int)ptTile->tRegion.tLocation.iX;
        area.y      = (int)ptTile->tRegion.tLocation.iY;
        area.width  = (int)ptTile->tRegion.tSize.iWidth;
        area.height = (int)ptTile->tRegion.tSize.iHeight;
        app->display_port.flush_callback(&area,
                                         (const void *)ptTile->pchBuffer,
                                         app->display_port.flush_user_data);
    }
    arm_2d_helper_pfb_report_rendering_complete(&s_tPFBHelper);
}

/* ─── tinyui_backend_init ─── */
int tinyui_backend_init(struct tinyui_app *app)
{
    if (app == NULL) return -1;

    /* 重入保护：已初始化则直接返回（SDL 路径为 -1，MCU 路径为 1）*/
    if (s_pfb_inited != 0 || s_pfb_mem != NULL) return 0;

    ldgui_port_set_current_app(app);

    if (tinyui_runtime_bridge_init_app(app) != 0) return -1;

    arm_2d_init();
    arm_2d_helper_init();

    /* SDL/host 路径（无 flush callback）：不初始化 PFB */
    if (app->display_port.flush_callback == NULL) {
        s_pfb_inited = -1;  /* SDL path sentinel */
        return 0;
    }

    /* MCU 路径：动态分配 PFB */
    struct tinyui_display_config cfg;
    if (tinyui_display_get_config(app, &cfg) != 0) return -1;

    int pfb_height = cfg.buffer_height > 0 ? cfg.buffer_height : LD_CFG_PFB_LINES;
    size_t buf_size = (size_t)cfg.width * (size_t)pfb_height * 2u; /* RGB565 = 2 bytes/pixel */

    s_pfb_mem = ldMalloc(sizeof(arm_2d_pfb_t) + buf_size);
    if (s_pfb_mem == NULL) return -1;
    memset(s_pfb_mem, 0, sizeof(arm_2d_pfb_t) + buf_size);

    arm_2d_helper_pfb_cfg_t pfb_cfg;
    memset(&pfb_cfg, 0, sizeof(pfb_cfg));

    pfb_cfg.tDisplayArea.tSize.iWidth  = (int16_t)cfg.width;
    pfb_cfg.tDisplayArea.tSize.iHeight = (int16_t)cfg.height;

    pfb_cfg.FrameBuffer.ptPFBs             = (arm_2d_pfb_t *)s_pfb_mem;
    pfb_cfg.FrameBuffer.tFrameSize.iWidth  = (int16_t)cfg.width;
    pfb_cfg.FrameBuffer.tFrameSize.iHeight = (int16_t)pfb_height;
    pfb_cfg.FrameBuffer.wBufferSize        = (uint32_t)buf_size;
    pfb_cfg.FrameBuffer.u8PFBNum          = 1;
    pfb_cfg.FrameBuffer.u7ColourFormat     = ARM_2D_COLOUR_RGB565;

    pfb_cfg.Dependency.evtOnDrawing.fnHandler           = &ldgui_port_pfb_draw_handler;
    pfb_cfg.Dependency.evtOnDrawing.pTarget              = app;
    pfb_cfg.Dependency.evtOnLowLevelRendering.fnHandler  = &ldgui_port_pfb_flush_handler;
    pfb_cfg.Dependency.evtOnLowLevelRendering.pTarget    = app;

    if (arm_2d_helper_pfb_init(&s_tPFBHelper, &pfb_cfg) != ARM_2D_ERR_NONE) {
        ldFree(s_pfb_mem);
        s_pfb_mem = NULL;
        memset(&s_tPFBHelper, 0, sizeof(s_tPFBHelper));
        return -1;
    }

    s_pfb_inited = 1;
    return 0;
}

/* ─── tinyui_backend_step ─── */
void tinyui_backend_step(struct tinyui_app *app)
{
    if (app == NULL) return;

    ldgui_port_set_current_app(app);

    struct tinyui_backend_app_state *app_state = tinyui_runtime_bridge_backend_state(app);
    if (app_state == NULL || app_state->ld_scene == NULL) return;

    ldGuiFrameStart(app_state->ld_scene);
    ldGuiTouchProcess(app_state->ld_scene);
    ldMsgProcess(app_state->ld_scene);

    if (s_pfb_inited) {
        arm_2d_helper_pfb_task(&s_tPFBHelper, NULL);
    }

    ldGuiFrameComplete(app_state->ld_scene);
}
