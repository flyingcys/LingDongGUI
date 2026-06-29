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
#include "arm_2d_helper_control.h"
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
static int                 s_pfb_step_limit;

enum {
    TINYUI_LDGUI_SCENE_DR_UPDATE = 0,
    TINYUI_LDGUI_SCENE_DR_DONE,
};

static void ldgui_port_update_widget_dirty_region(ld_scene_t *scene,
                                                  const arm_2d_tile_t *ptTile,
                                                  ldBase_t **current_widget,
                                                  uint8_t *item_count,
                                                  arm_2d_location_t *widget_location)
{
    if (scene == NULL || ptTile == NULL || current_widget == NULL ||
        item_count == NULL || widget_location == NULL) {
        return;
    }

    if (*current_widget == NULL) {
        do {
            if (arm_2d_helper_control_enum_get_next_node(&scene->tEnum)) {
                ldBase_t *widget = (ldBase_t *)scene->tEnum.ptCurrent;
                arm_2d_region_t region;

                if (!widget->isDirtyRegionUpdate) {
                    continue;
                }

                if (widget->isDirtyRegionAutoReset) {
                    widget->isDirtyRegionUpdate = false;
                }

                if (widget->itemCount) {
                    region = widget->use_as__arm_2d_control_node_t.tRegion;
                    *current_widget = widget;
                    *item_count = 0;
                    widget_location->iX = 0;
                    widget_location->iY = 0;
                    *widget_location = ldBaseGetAbsoluteLocation(widget, *widget_location);

                    region.tLocation.iX += widget_location->iX;
                    region.tLocation.iY += widget_location->iY;
                    arm_2d_dynamic_dirty_region_update(&scene->tDirtyRegionItem,
                                                       (arm_2d_tile_t *)ptTile,
                                                       &region,
                                                       TINYUI_LDGUI_SCENE_DR_UPDATE);
                    break;
                }

                widget_location->iX = 0;
                widget_location->iY = 0;
                *widget_location =
                    ldBaseGetAbsoluteLocation(ldBaseGetParent(widget), *widget_location);

                if (memcmp(&widget->tTempRegion,
                           &widget->use_as__arm_2d_control_node_t.tRegion,
                           sizeof(arm_2d_region_t)) == 0) {
                    region = widget->use_as__arm_2d_control_node_t.tRegion;
                } else {
                    region = widget->tTempRegion;
                    widget->tTempRegion = widget->use_as__arm_2d_control_node_t.tRegion;
                }

                region.tLocation.iX += widget_location->iX;
                region.tLocation.iY += widget_location->iY;
                arm_2d_dynamic_dirty_region_update(&scene->tDirtyRegionItem,
                                                   (arm_2d_tile_t *)ptTile,
                                                   &region,
                                                   TINYUI_LDGUI_SCENE_DR_UPDATE);
                break;
            }

            arm_2d_dynamic_dirty_region_change_user_region_index_only(
                &scene->tDirtyRegionItem,
                TINYUI_LDGUI_SCENE_DR_DONE);
            break;
        } while (true);
    }

    if (*current_widget != NULL) {
        ldBase_t *widget = *current_widget;
        while (*item_count < widget->itemCount) {
            ldBaseItemRegion_t *item = &widget->ptItemRegionList[*item_count];
            if (item->isDRUpdate) {
                if (item->isDRReset) {
                    item->isDRUpdate = false;
                }

                item->tTempItemRegion.tLocation.iX += widget_location->iX;
                item->tTempItemRegion.tLocation.iY += widget_location->iY;
                arm_2d_dynamic_dirty_region_update(&scene->tDirtyRegionItem,
                                                   (arm_2d_tile_t *)ptTile,
                                                   &item->tTempItemRegion,
                                                   TINYUI_LDGUI_SCENE_DR_UPDATE);

                item->tTempItemRegion = item->itemRegion;
                (*item_count)++;
                break;
            }

            item->tTempItemRegion = item->itemRegion;
            (*item_count)++;
        }

        if (*item_count >= widget->itemCount) {
            widget->tTempRegion = widget->use_as__arm_2d_control_node_t.tRegion;
            *current_widget = NULL;
            arm_2d_dynamic_dirty_region_change_user_region_index_only(
                &scene->tDirtyRegionItem,
                TINYUI_LDGUI_SCENE_DR_UPDATE);
        }
    }
}

/* ─── PFB draw handler ─── */
IMPL_PFB_ON_DRAW(ldgui_port_pfb_draw_handler)
{
    struct tinyui_app *app = (struct tinyui_app *)pTarget;
    struct tinyui_app *app_state = app;
    static ldBase_t *current_widget = NULL;
    static uint8_t item_count = 0;
    static arm_2d_location_t widget_location = {0};
    ld_scene_t *scene;

    if (app_state != NULL && app_state->ld_scene != NULL) {
        scene = app_state->ld_scene;
        if (bIsNewFrame || scene->ptNodeRoot == NULL) {
            current_widget = NULL;
            item_count = 0;
            widget_location.iX = 0;
            widget_location.iY = 0;
        }

        ldGuiDraw(scene, (arm_2d_tile_t *)ptTile, bIsNewFrame);

        if (scene->ptNodeRoot != NULL) {
            switch (arm_2d_dynamic_dirty_region_wait_next(&scene->tDirtyRegionItem)) {
            case TINYUI_LDGUI_SCENE_DR_UPDATE:
                ldgui_port_update_widget_dirty_region(scene,
                                                       ptTile,
                                                       &current_widget,
                                                       &item_count,
                                                       &widget_location);
                break;
            case TINYUI_LDGUI_SCENE_DR_DONE:
            default:
                break;
            }
        }
    }
    ARM_2D_OP_WAIT_ASYNC();
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
    s_pfb_step_limit = (cfg.height + pfb_height - 1) / pfb_height + 4;

    s_pfb_mem = calloc(1, sizeof(arm_2d_pfb_t) + buf_size);
    if (s_pfb_mem == NULL) return -1;

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
        free(s_pfb_mem);
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
    arm_fsm_rt_t result;
    int remaining_steps;

    if (app == NULL) return;

    ldgui_port_set_current_app(app);

    struct tinyui_app *app_state = app;
    if (app_state == NULL || app_state->ld_scene == NULL) return;

    ldGuiFrameStart(app_state->ld_scene);
    ldGuiTouchProcess(app_state->ld_scene);
    ldMsgProcess(app_state->ld_scene);
    arm_2d_dynamic_dirty_region_on_frame_start(&app_state->ld_scene->tDirtyRegionItem,
                                               TINYUI_LDGUI_SCENE_DR_UPDATE);
    arm_2d_helper_control_enum_init(&app_state->ld_scene->tEnum,
                                    &ARM_2D_CONTROL_ENUMERATION_POLICY_PREORDER_TRAVERSAL,
                                    app_state->ld_scene->ptNodeRoot);

    if (s_pfb_inited > 0) {
        remaining_steps = s_pfb_step_limit > 0 ? s_pfb_step_limit : 32;
        do {
            result = arm_2d_helper_pfb_task(
                &s_tPFBHelper,
                app_state->ld_scene->use_as__arm_2d_scene_t.ptDirtyRegion);
            remaining_steps--;
        } while (result == arm_fsm_rt_on_going && remaining_steps > 0);
    }

    if (app_state->ld_scene->ptNodeRoot != NULL) {
        arm_2d_helper_control_enum_depose(&app_state->ld_scene->tEnum);
    }
    ldGuiFrameComplete(app_state->ld_scene);
}
