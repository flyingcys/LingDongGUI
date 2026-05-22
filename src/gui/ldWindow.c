/*
 * Copyright (c) 2023-2025 Ou Jianbo (59935554@qq.com). All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define __LD_WINDOW_IMPLEMENT__
#define __ARM_2D_HELPER_CONTROL_INHERIT__
#include "./__common.h"
#include "arm_2d.h"
#include "arm_2d_helper.h"
#include <assert.h>
#include <string.h>

#include "ldWindow.h"
#include "ldWindowLayoutInternal.h"

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-warning-option"
#pragma clang diagnostic ignored "-Wreserved-identifier"
#pragma clang diagnostic ignored "-Wdeclaration-after-statement"
#pragma clang diagnostic ignored "-Wsign-conversion"
#pragma clang diagnostic ignored "-Wpadded"
#pragma clang diagnostic ignored "-Wcast-qual"
#pragma clang diagnostic ignored "-Wcast-align"
#pragma clang diagnostic ignored "-Wmissing-field-initializers"
#pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
#pragma clang diagnostic ignored "-Wmissing-braces"
#pragma clang diagnostic ignored "-Wunused-const-variable"
#pragma clang diagnostic ignored "-Wmissing-declarations"
#pragma clang diagnostic ignored "-Wmissing-variable-declarations"
#endif

static void ldWindowApplyLayoutRegion(ldBase_t *ptItem, arm_2d_region_t tRegion)
{
    arm_2d_region_t tOldRegion;
    int16_t minX;
    int16_t minY;
    int16_t maxX;
    int16_t maxY;

    if (ptItem == NULL)
    {
        return;
    }

    tOldRegion = ptItem->use_as__arm_2d_control_node_t.tRegion;
    minX = MIN(tOldRegion.tLocation.iX, tRegion.tLocation.iX);
    minY = MIN(tOldRegion.tLocation.iY, tRegion.tLocation.iY);
    maxX = MAX(tOldRegion.tLocation.iX + tOldRegion.tSize.iWidth,
               tRegion.tLocation.iX + tRegion.tSize.iWidth);
    maxY = MAX(tOldRegion.tLocation.iY + tOldRegion.tSize.iHeight,
               tRegion.tLocation.iY + tRegion.tSize.iHeight);

    ptItem->isDirtyRegionUpdate = true;
    ptItem->tTempRegion.tLocation.iX = minX;
    ptItem->tTempRegion.tLocation.iY = minY;
    ptItem->tTempRegion.tSize.iWidth = maxX - minX;
    ptItem->tTempRegion.tSize.iHeight = maxY - minY;
    ptItem->use_as__arm_2d_control_node_t.tRegion = tRegion;
}

static void ldWindowMarkLayoutDirty(ldWindow_t *ptWidget)
{
    if (ptWidget == NULL)
    {
        return;
    }

    ptWidget->use_as__ldBase_t.isDirtyRegionUpdate = true;
    ptWidget->isLayoutUpdate = true;
}

static int16_t ldWindowResolveFlexCrossStart(ldFlexCrossAlign_t align, int16_t innerCrossSize, int16_t itemCrossSize)
{
    int16_t remain = innerCrossSize - itemCrossSize;

    switch (align)
    {
    case ldFlexCrossAlignCenter:
        return remain / 2;
    case ldFlexCrossAlignEnd:
        return remain;
    default:
        return 0;
    }
}

static void ldWindowApplyLegacyLayout(ldWindow_t *ptWidget)
{
    arm_2d_region_t globalRegion;
    uint16_t childCount = ldBaseGetChildCount((ldBase_t *)ptWidget);

    if (childCount == 0)
    {
        return;
    }

    arm_2d_helper_control_get_absolute_region((arm_2d_control_node_t *)ptWidget, &globalRegion, true);

    arm_2d_layout(globalRegion)
    {
        ldBase_t *children[childCount];
        arm_2d_size_t itemSize;
        arm_2d_size_t windowSize = ptWidget->use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize;
        uint16_t count = 0;

        childCount = ldWindowCollectDirectChildren((ldBase_t *)ptWidget, children, childCount, false);
        while (count < childCount)
        {
            ldBase_t *ptItem = children[count];
            itemSize = ptItem->use_as__arm_2d_control_node_t.tRegion.tSize;
            if (ptWidget->layoutTpye == layoutHorizontal)
            {
                int16_t slotW = windowSize.iWidth / childCount;

                int16_t left = (slotW - itemSize.iWidth) / 2;
                int16_t top = (windowSize.iHeight - itemSize.iHeight) / 2;
                int16_t right = left;
                int16_t bottom = top;
                LOG_DEBUG("==%d,%d,%d,%d", top, bottom, left, right);
                if (ptWidget->pLayoutPaddingGroup != NULL)
                {
                    top = ptWidget->pLayoutPaddingGroup[count].top;
                    bottom = ptWidget->pLayoutPaddingGroup[count].bottom;
                    left = ptWidget->pLayoutPaddingGroup[count].left;
                    right = ptWidget->pLayoutPaddingGroup[count].right;
                    LOG_DEBUG("%d,%d,%d,%d", top, bottom, left, right);
                }
                __item_line_horizontal(itemSize.iWidth, itemSize.iHeight, left, right, top, bottom) {
                    __item_region.tLocation.iX -= globalRegion.tLocation.iX;
                    __item_region.tLocation.iY -= globalRegion.tLocation.iY;
                    ldWindowApplyLayoutRegion((ldBase_t *)ptItem, __item_region);
                }
            }
            else
            {
                int16_t slotH = windowSize.iHeight / childCount;
                int16_t top = (slotH - itemSize.iHeight) / 2;
                int16_t left = (windowSize.iWidth - itemSize.iWidth) / 2;
                int16_t right = left;
                int16_t bottom = top;
                if (ptWidget->pLayoutPaddingGroup != NULL)
                {
                    top = ptWidget->pLayoutPaddingGroup[count].top;
                    bottom = ptWidget->pLayoutPaddingGroup[count].bottom;
                    left = ptWidget->pLayoutPaddingGroup[count].left;
                    right = ptWidget->pLayoutPaddingGroup[count].right;
                }
                __item_line_vertical(itemSize.iWidth, itemSize.iHeight, left, right, top, bottom) {
                    __item_region.tLocation.iX -= globalRegion.tLocation.iX;
                    __item_region.tLocation.iY -= globalRegion.tLocation.iY;
                    ldWindowApplyLayoutRegion(ptItem, __item_region);
                }
            }
            count++;
        }
    }
}

static void ldWindowApplyFlexLayout(ldWindow_t *ptWidget)
{
    uint16_t childCount = ldBaseGetChildCount((ldBase_t *)ptWidget);
    arm_2d_size_t windowSize;
    int16_t innerWidth;
    int16_t innerHeight;
    bool isRow;
    int16_t innerMainSize;
    int16_t innerCrossSize;
    int16_t resolvedGap;
    int16_t currentMain;
    int32_t contentMainSize = 0;
    uint16_t index;

    if (childCount == 0)
    {
        return;
    }

    ldBase_t *children[childCount];
    childCount = ldWindowCollectDirectChildren((ldBase_t *)ptWidget, children, childCount, true);
    if (childCount == 0)
    {
        return;
    }

    windowSize = ptWidget->use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize;
    innerWidth = MAX(0, windowSize.iWidth - ptWidget->flexPadding.left - ptWidget->flexPadding.right);
    innerHeight = MAX(0, windowSize.iHeight - ptWidget->flexPadding.top - ptWidget->flexPadding.bottom);
    isRow = (ptWidget->flexFlow == ldFlexFlowRow);
    innerMainSize = isRow ? innerWidth : innerHeight;
    innerCrossSize = isRow ? innerHeight : innerWidth;

    /* Only visible direct children participate in flex sizing. */
    for (index = 0; index < childCount; ++index)
    {
        arm_2d_size_t itemSize = children[index]->use_as__arm_2d_control_node_t.tRegion.tSize;
        contentMainSize += isRow ? itemSize.iWidth : itemSize.iHeight;
    }
    if (childCount > 1)
    {
        contentMainSize += (int32_t)ptWidget->flexGap * (childCount - 1);
    }

    resolvedGap = ptWidget->flexGap;
    currentMain = ldFlexResolveMainStart(ptWidget->flexMainAlign,
                                         innerMainSize,
                                         (int16_t)contentMainSize,
                                         childCount,
                                         ptWidget->flexGap,
                                         &resolvedGap);
    currentMain += isRow ? ptWidget->flexPadding.left : ptWidget->flexPadding.top;

    for (index = 0; index < childCount; ++index)
    {
        ldBase_t *ptItem = children[index];
        arm_2d_region_t tRegion = ptItem->use_as__arm_2d_control_node_t.tRegion;
        int16_t crossBase = isRow ? ptWidget->flexPadding.top : ptWidget->flexPadding.left;
        int16_t itemMainSize = isRow ? tRegion.tSize.iWidth : tRegion.tSize.iHeight;
        int16_t itemCrossSize = isRow ? tRegion.tSize.iHeight : tRegion.tSize.iWidth;
        int16_t crossStart = crossBase + ldWindowResolveFlexCrossStart(ptWidget->flexCrossAlign,
                                                                       innerCrossSize,
                                                                       itemCrossSize);

        if (isRow)
        {
            tRegion.tLocation.iX = currentMain;
            tRegion.tLocation.iY = crossStart;
        }
        else
        {
            tRegion.tLocation.iX = crossStart;
            tRegion.tLocation.iY = currentMain;
        }

        ldWindowApplyLayoutRegion(ptItem, tRegion);
        currentMain += itemMainSize + resolvedGap;
    }
}

const ldBaseWidgetFunc_t ldWindowFunc = {
    .depose = (ldDeposeFunc_t)ldWindow_depose,
    .load = (ldLoadFunc_t)ldWindow_on_load,
    .frameStart = (ldFrameStartFunc_t)ldWindow_on_frame_start,
    .frameComplete = (ldFrameCompleteFunc_t)ldWindow_on_frame_complete,
    .show = (ldShowFunc_t)ldWindow_show,
};

ldWindow_t* ldWindow_init(ld_scene_t *ptScene,ldWindow_t *ptWidget, uint16_t nameId, uint16_t parentNameId, int16_t x, int16_t y, int16_t width, int16_t height)
{
    assert(NULL != ptScene);
    ldBase_t *ptParent;

    if (NULL == ptWidget)
    {
        ptWidget = ldCalloc(1, sizeof(ldWindow_t));
        if (NULL == ptWidget)
        {
            LOG_ERROR("[init failed][window] id:%d", nameId);
            return NULL;
        }
    }
    else
    {
        memset(ptWidget, 0, sizeof(ldWindow_t));
    }

    ptWidget->use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX = x;
    ptWidget->use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY = y;
    ptWidget->use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iWidth = width;
    ptWidget->use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iHeight = height;
    ptWidget->use_as__ldBase_t.nameId = nameId;
    ptWidget->use_as__ldBase_t.ptGuiFunc = &ldWindowFunc;
    ptWidget->use_as__ldBase_t.isDirtyRegionUpdate = true;
    ptWidget->use_as__ldBase_t.isDirtyRegionAutoReset = true;
    ptWidget->use_as__ldBase_t.opacity=255;

    if(nameId==0)
    {
        ptWidget->use_as__ldBase_t.tTempRegion=ptWidget->use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion;
        ptScene->ptNodeRoot=(arm_2d_control_node_t*)ptWidget;
        ptWidget->use_as__ldBase_t.widgetType=widgetTypeBackground;
        ptWidget->bgColor=__RGB(240,240,240);
        LOG_INFO("[init][background] id:%d, size:%d", nameId,(int)sizeof (*ptWidget));
    }
    else
    {
        ptParent=ldBaseGetWidget(ptScene->ptNodeRoot,parentNameId);
        ldBaseNodeAdd((arm_2d_control_node_t*)ptParent,(arm_2d_control_node_t*)ptWidget);
        ptWidget->isTransparent=true;
        ptWidget->use_as__ldBase_t.widgetType=widgetTypeWindow;
        LOG_INFO("[init][window] id:%d, size:%d", nameId,(int)sizeof (*ptWidget));
    }

    LOG_INFO("[init][window] id:%d, size:%d", nameId,(int)sizeof (*ptWidget));
    return ptWidget;
}

void ldWindow_depose(ld_scene_t *ptScene, ldWindow_t *ptWidget)
{
    assert(NULL != ptWidget);
    if (ptWidget == NULL)
    {
        return;
    }
    if((ptWidget->use_as__ldBase_t.widgetType!=widgetTypeWindow)&&
       (ptWidget->use_as__ldBase_t.widgetType!=widgetTypeBackground))
    {
        return;
    }

#if (USE_LOG_LEVEL>=LOG_LEVEL_INFO)
    switch (ptWidget->use_as__ldBase_t.widgetType)
    {
    case widgetTypeWindow:
    {
        LOG_INFO("[depose][window] id:%d", ptWidget->use_as__ldBase_t.nameId);
        break;
    }
    case widgetTypeBackground:
    {
        LOG_INFO("[depose][background] id:%d", ptWidget->use_as__ldBase_t.nameId);
        break;
    }
    default:
        break;
    }
#endif

    if(ptWidget->use_as__ldBase_t.use_as__arm_2d_control_node_t.ptChildList!=NULL)
    {
        arm_ctrl_enum(ptWidget->use_as__ldBase_t.use_as__arm_2d_control_node_t.ptChildList, ptItem, PREORDER_TRAVERSAL)
        {
            ((ldBase_t *)ptItem)->ptGuiFunc->depose(ptScene,ptItem);
        }
    }

    ldMsgDelConnect(ptWidget);
    ldBaseNodeRemove((arm_2d_control_node_t*)ptWidget);
#if USE_VIRTUAL_RESOURCE == 1
    ldFree(ptWidget->ptImgTile);
    ldFree(ptWidget->ptMaskTile);
#endif
    ldFree(ptWidget);
}

void ldWindow_on_load(ld_scene_t *ptScene, ldWindow_t *ptWidget)
{
    assert(NULL != ptWidget);
    if(ptWidget == NULL)
    {
        return;
    }
}

void ldWindow_on_frame_start(ld_scene_t *ptScene, ldWindow_t *ptWidget)
{
    (void)ptScene;

    assert(NULL != ptWidget);
    if(ptWidget == NULL)
    {
        return;
    }

    if((ptWidget->isLayoutUpdate)&&(ptWidget->layoutTpye!=layoutNone))
    {
        ptWidget->use_as__ldBase_t.isDirtyRegionUpdate = true;
        ptWidget->isLayoutUpdate=false;

        switch (ptWidget->layoutTpye)
        {
        case layoutHorizontal:
        case layoutVertical:
            ldWindowApplyLegacyLayout(ptWidget);
            break;
        case layoutFlex:
            ldWindowApplyFlexLayout(ptWidget);
            break;
        default:
            break;
        }
    }
}

void ldWindow_on_frame_complete(ld_scene_t *ptScene, ldWindow_t *ptWidget)
{
    assert(NULL != ptWidget);
    if(ptWidget == NULL)
    {
        return;
    }
}

void ldWindow_show(ld_scene_t *ptScene, ldWindow_t *ptWidget, const arm_2d_tile_t *ptTile, bool bIsNewFrame)
{
    assert(NULL != ptWidget);
    if(ptWidget == NULL)
    {
        return;
    }

    arm_2d_region_t globalRegion;
    arm_2d_helper_control_get_absolute_region((arm_2d_control_node_t*)ptWidget,&globalRegion,true);

    if(arm_2d_helper_pfb_is_region_active(ptTile,&globalRegion,true))
    {
        arm_2d_container(ptTile, tTarget, &globalRegion)
        {
            if(ldBaseIsHidden((ldBase_t*)ptWidget)||(ptWidget->isTransparent))
            {
                break;
            }
            if((ptWidget->ptImgTile==NULL)&&(ptWidget->ptMaskTile==NULL))
            {
                if(ptWidget->use_as__ldBase_t.isCorner)
                {
                    draw_round_corner_box(&tTarget,
                                          NULL,
                                          ptWidget->bgColor,
                                          ptWidget->use_as__ldBase_t.opacity,
                                          bIsNewFrame);
                }
                else
                {
                    ldBaseColor(&tTarget,
                                NULL,
                                ptWidget->bgColor,
                                ptWidget->use_as__ldBase_t.opacity);
                }

            }
            else
            {
                if(ptWidget->use_as__ldBase_t.isCorner)
                {
                    draw_round_corner_image(ptWidget->ptImgTile,
                                            &tTarget,
                                            NULL,
                                            bIsNewFrame,
                                            ptWidget->use_as__ldBase_t.opacity);
                }
                else
                {
                    ldBaseImage(&tTarget,
                                NULL,
                                ptWidget->ptImgTile,
                                ptWidget->ptMaskTile,
                                ptWidget->bgColor,
                                ptWidget->use_as__ldBase_t.opacity);
                }
            }
            LD_BASE_WIDGET_SELECT;

            arm_2d_op_wait_async(NULL);
        }
    }
}

void ldWindowSetColor(ldWindow_t *ptWidget,ldColor bgColor)
{
    assert(NULL!= ptWidget);
    if(ptWidget == NULL)
    {
        return;
    }
    ptWidget->use_as__ldBase_t.isDirtyRegionUpdate = true;
    ptWidget->isTransparent=false;
    ptWidget->bgColor=bgColor;
}

ldColor ldWindowGetColor(ldWindow_t *ptWidget)
{
    assert(NULL!= ptWidget);
    if(ptWidget == NULL)
    {
        return 0;
    }
    return ptWidget->bgColor;
}

void ldWindowSetImage(ldWindow_t *ptWidget, arm_2d_tile_t* ptImgTile, arm_2d_tile_t* ptMaskTile)
{
    assert(NULL != ptWidget);
    if(ptWidget == NULL)
    {
        return;
    }
    ptWidget->use_as__ldBase_t.isDirtyRegionUpdate = true;
    ptWidget->isTransparent=false;
    ptWidget->ptImgTile=ptImgTile;
    ptWidget->ptMaskTile=ptMaskTile;
}

void ldWindowSetLayout(ldWindow_t *ptWidget, ldLayoutType_t type)
{
    assert(NULL != ptWidget);
    if(ptWidget == NULL)
    {
        return;
    }
    ptWidget->layoutTpye=type;
    ldWindowMarkLayoutDirty(ptWidget);
}

void ldWindowSetFlexFlow(ldWindow_t *ptWidget, ldFlexFlow_t flow)
{
    assert(NULL != ptWidget);
    if (ptWidget == NULL)
    {
        return;
    }
    ptWidget->flexFlow = flow;
    ptWidget->layoutTpye = layoutFlex;
    ldWindowMarkLayoutDirty(ptWidget);
}

void ldWindowSetFlexAlign(ldWindow_t *ptWidget,
                          ldFlexMainAlign_t mainAlign,
                          ldFlexCrossAlign_t crossAlign)
{
    assert(NULL != ptWidget);
    if (ptWidget == NULL)
    {
        return;
    }
    ptWidget->flexMainAlign = mainAlign;
    ptWidget->flexCrossAlign = crossAlign;
    ldWindowMarkLayoutDirty(ptWidget);
}

void ldWindowSetPadding(ldWindow_t *ptWidget, ldPadding_t padding)
{
    assert(NULL != ptWidget);
    if (ptWidget == NULL)
    {
        return;
    }
    ptWidget->flexPadding = padding;
    ldWindowMarkLayoutDirty(ptWidget);
}

void ldWindowSetGap(ldWindow_t *ptWidget, int16_t gap)
{
    assert(NULL != ptWidget);
    if (ptWidget == NULL)
    {
        return;
    }
    ptWidget->flexGap = gap;
    ldWindowMarkLayoutDirty(ptWidget);
}

void ldWindowSetPaddingGroup(ldWindow_t *ptWidget, ldPadding_t *pPaddingGroup)
{
    assert(NULL != ptWidget);
    if(ptWidget == NULL)
    {
        return;
    }
    ptWidget->pLayoutPaddingGroup=pPaddingGroup;
    ldWindowMarkLayoutDirty(ptWidget);
}

#if defined(__clang__)
#pragma clang diagnostic pop
#endif
