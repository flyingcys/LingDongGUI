/*
 * Copyright (c) 2023-2025 Ou Jianbo (59935554@qq.com). All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define __ARM_2D_HELPER_CONTROL_INHERIT__
#include "ldCanvas.h"

#include <assert.h>
#include <string.h>

static const ldBaseWidgetFunc_t ldCanvasFunc = {
    .depose = (ldDeposeFunc_t)ldCanvas_depose,
    .load = (ldLoadFunc_t)ldCanvas_on_load,
    .frameStart = (ldFrameStartFunc_t)ldCanvas_on_frame_start,
    .frameComplete = (ldFrameCompleteFunc_t)ldCanvas_on_frame_complete,
    .show = (ldShowFunc_t)ldCanvas_show,
};

ldCanvas_t *ldCanvas_init(ld_scene_t *ptScene,
                          ldCanvas_t *ptWidget,
                          uint16_t nameId,
                          uint16_t parentNameId,
                          int16_t x,
                          int16_t y,
                          int16_t width,
                          int16_t height)
{
    ldBase_t *ptParent;

    assert(ptScene != NULL);
    if (ptScene == NULL) {
        return NULL;
    }

    if (ptWidget == NULL) {
        ptWidget = ldCalloc(1, sizeof(*ptWidget));
        if (ptWidget == NULL) {
            return NULL;
        }
    } else {
        memset(ptWidget, 0, sizeof(*ptWidget));
    }

    ptParent = ldBaseGetWidget(ptScene->ptNodeRoot, parentNameId);
    ldBaseNodeAdd((arm_2d_control_node_t *)ptParent, (arm_2d_control_node_t *)ptWidget);

    ptWidget->use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX = x;
    ptWidget->use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY = y;
    ptWidget->use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iWidth = width;
    ptWidget->use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iHeight = height;
    ptWidget->use_as__ldBase_t.tTempRegion =
        ptWidget->use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion;
    ptWidget->use_as__ldBase_t.nameId = nameId;
    ptWidget->use_as__ldBase_t.widgetType = widgetTypeCanvas;
    ptWidget->use_as__ldBase_t.ptGuiFunc = &ldCanvasFunc;
    ptWidget->use_as__ldBase_t.isDirtyRegionUpdate = true;
    ptWidget->use_as__ldBase_t.isDirtyRegionAutoReset = true;
    ptWidget->use_as__ldBase_t.opacity = 255;
    return ptWidget;
}

void ldCanvas_depose(ld_scene_t *ptScene, ldCanvas_t *ptWidget)
{
    uint8_t i;

    (void)ptScene;
    if (ptWidget == NULL || ptWidget->use_as__ldBase_t.widgetType != widgetTypeCanvas) {
        return;
    }

    for (i = 0; i < ptWidget->commandCount; ++i) {
        if (ptWidget->commands[i].kind == ldCanvasCommandDrawText) {
            ldFree(ptWidget->commands[i].pStr);
            ptWidget->commands[i].pStr = NULL;
        }
    }

    ldBaseNodeRemove((arm_2d_control_node_t *)ptWidget);
    ldFree(ptWidget);
}

void ldCanvas_on_load(ld_scene_t *ptScene, ldCanvas_t *ptWidget)
{
    (void)ptScene;
    (void)ptWidget;
}

void ldCanvas_on_frame_start(ld_scene_t *ptScene, ldCanvas_t *ptWidget)
{
    (void)ptScene;
    (void)ptWidget;
}

void ldCanvas_on_frame_complete(ld_scene_t *ptScene, ldCanvas_t *ptWidget)
{
    (void)ptScene;
    (void)ptWidget;
}

void ldCanvas_show(ld_scene_t *ptScene, ldCanvas_t *ptWidget, const arm_2d_tile_t *ptTile, bool bIsNewFrame)
{
    arm_2d_region_t globalRegion;
    uint8_t i;

    (void)ptScene;
    if (ptWidget == NULL) {
        return;
    }

    arm_2d_helper_control_get_absolute_region((arm_2d_control_node_t *)ptWidget, &globalRegion, true);
    if (!arm_2d_helper_pfb_is_region_active(ptTile, &globalRegion, true)) {
        return;
    }

    arm_2d_container(ptTile, tTarget, &globalRegion)
    {
        if (ldBaseIsHidden((ldBase_t *)ptWidget)) {
            break;
        }

        for (i = 0; i < ptWidget->commandCount; ++i) {
            ldCanvasCommand_t *cmd = &ptWidget->commands[i];
            switch (cmd->kind) {
            case ldCanvasCommandFillRect:
                ldBaseColor(&tTarget, &cmd->region, cmd->color0, cmd->opacity0);
                break;
            case ldCanvasCommandDrawLine:
                ldBaseDrawLine(&tTarget,
                               cmd->region.tLocation.iX,
                               cmd->region.tLocation.iY,
                               cmd->x1,
                               cmd->y1,
                               cmd->lineSize,
                               cmd->color0,
                               cmd->opacity0,
                               cmd->opacity1);
                break;
            case ldCanvasCommandDrawImage:
                ldBaseImage(&tTarget,
                            &cmd->region,
                            cmd->ptImgTile,
                            cmd->ptMaskTile,
                            cmd->color0,
                            cmd->opacity0);
                break;
            case ldCanvasCommandDrawImageScale:
                ldBaseImageScale(&tTarget,
                                 &cmd->region,
                                 cmd->ptImgTile,
                                 cmd->ptMaskTile,
                                 cmd->scale,
                                 NULL,
                                 cmd->opacity0,
                                 bIsNewFrame);
                break;
            case ldCanvasCommandDrawText:
                ldBaseLabel(&tTarget,
                            &cmd->region,
                            cmd->pStr,
                            cmd->ptFont,
                            cmd->align,
                            cmd->color0,
                            cmd->opacity0);
                break;
            default:
                break;
            }
        }
    }
}

void ldCanvasClear(ldCanvas_t *ptWidget)
{
    uint8_t i;

    if (ptWidget == NULL) {
        return;
    }

    for (i = 0; i < ptWidget->commandCount; ++i) {
        if (ptWidget->commands[i].kind == ldCanvasCommandDrawText) {
            ldFree(ptWidget->commands[i].pStr);
            ptWidget->commands[i].pStr = NULL;
        }
    }
    memset(ptWidget->commands, 0, sizeof(ptWidget->commands));
    ptWidget->commandCount = 0;
}

int ldCanvasPushCommand(ldCanvas_t *ptWidget, const ldCanvasCommand_t *command)
{
    ldCanvasCommand_t *slot;
    size_t len;

    if (ptWidget == NULL || command == NULL || ptWidget->commandCount >= LD_CANVAS_MAX_COMMANDS) {
        return -1;
    }

    slot = &ptWidget->commands[ptWidget->commandCount];
    *slot = *command;
    if (command->kind == ldCanvasCommandDrawText && command->pStr != NULL) {
        len = strlen((const char *)command->pStr);
        slot->pStr = ldCalloc((uint32_t)len + 1U, sizeof(uint8_t));
        if (slot->pStr == NULL) {
            memset(slot, 0, sizeof(*slot));
            return -1;
        }
        memcpy(slot->pStr, command->pStr, len + 1U);
    }

    ptWidget->commandCount++;
    return 0;
}
