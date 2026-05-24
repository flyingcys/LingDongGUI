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

/**
 * @file ldSwitch.c
 * @brief 拨动开关控件，支持颜色或图片两种绘制模式，并带有开关动画。
 * @signal SIGNAL_VALUE_CHANGED 开关状态变化时发送，value 为 0 或 1。
 */

#define __LD_SWITCH_IMPLEMENT__

#include "__common.h"
#include "arm_2d.h"
#include "arm_2d_helper.h"
#include <assert.h>
#include <string.h>

#include "ldSwitch.h"
#include "ldSwitchInternal.h"

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

#define LD_SWITCH_ANIM_DURATION_MS     150U
#define LD_SWITCH_DEFAULT_KNOB_PADDING 2U
#define LD_SWITCH_FRAME_STEP_MS        10U
#define LD_SWITCH_PRESSED_KNOB_COLOR   __RGB(255, 243, 202)

static bool slotSwitchProcess(ld_scene_t *ptScene, ldMsg_t msg);
static void ldSwitchApplyValue(ld_scene_t *ptScene, ldSwitch_t *ptWidget, bool isChecked);
static arm_2d_region_t ldSwitchRectToRegion(ldSwitchRect_t rect);

const ldBaseWidgetFunc_t ldSwitchFunc = {
    .depose = (ldDeposeFunc_t)ldSwitch_depose,
    .load = (ldLoadFunc_t)ldSwitch_on_load,
    .frameStart = (ldFrameStartFunc_t)ldSwitch_on_frame_start,
    .frameComplete = (ldFrameCompleteFunc_t)ldSwitch_on_frame_complete,
    .show = (ldShowFunc_t)ldSwitch_show,
};

static void ldSwitchApplyValue(ld_scene_t *ptScene, ldSwitch_t *ptWidget, bool isChecked)
{
    uint16_t target;
    bool valueChanged;

    assert(NULL != ptWidget);
    if (ptWidget == NULL)
    {
        return;
    }

    target = isChecked ? 1000U : 0U;
    valueChanged = (ptWidget->isChecked != isChecked);
    if (!valueChanged)
    {
        return;
    }

    ptWidget->isChecked = isChecked;
    ptWidget->animStartProgress = ptWidget->animProgress;
    ptWidget->animTargetProgress = target;
    ptWidget->animElapsedMs = 0;
    ptWidget->isAnimating = ptWidget->hasRenderedFrame && (ptWidget->animProgress != target);
    if (!ptWidget->isAnimating)
    {
        ptWidget->animProgress = target;
    }
    ptWidget->use_as__ldBase_t.isDirtyRegionUpdate = true;

    if ((ptScene != NULL) && (ptScene->ptMsgQueue != NULL))
    {
        ldMsgEmit(ptScene->ptMsgQueue, ptWidget, SIGNAL_VALUE_CHANGED, isChecked ? 1U : 0U);
    }
}

static arm_2d_region_t ldSwitchRectToRegion(ldSwitchRect_t rect)
{
    return (arm_2d_region_t){
        .tLocation = {
            .iX = rect.iX,
            .iY = rect.iY,
        },
        .tSize = {
            .iWidth = rect.iWidth,
            .iHeight = rect.iHeight,
        },
    };
}

static bool slotSwitchProcess(ld_scene_t *ptScene, ldMsg_t msg)
{
    ldSwitch_t *ptWidget = msg.ptSender;

    if (ptWidget == NULL)
    {
        return false;
    }

    if (ptWidget->isDisabled)
    {
        return false;
    }

    if (msg.signal == SIGNAL_PRESS)
    {
        ptWidget->isPressed = true;
        ptWidget->use_as__ldBase_t.isDirtyRegionUpdate = true;
    }
    else if (msg.signal == SIGNAL_RELEASE)
    {
        if (ptWidget->isPressed)
        {
            ptWidget->isPressed = false;
            ldSwitchApplyValue(ptScene, ptWidget, !ptWidget->isChecked);
        }
    }

    return false;
}

/**
 * @brief 初始化拨动开关控件，默认使用横向布局和颜色绘制模式。
 * @param ptScene 场景指针
 * @param ptWidget 外部传入的控件实例，传 NULL 时自动分配
 * @param nameId 新控件 id
 * @param parentNameId 父控件 id
 * @param x 相对坐标 x 轴
 * @param y 相对坐标 y 轴
 * @param width 控件宽度
 * @param height 控件高度
 * @return ldSwitch_t* 新控件指针
 */
ldSwitch_t *ldSwitch_init(ld_scene_t *ptScene,
                          ldSwitch_t *ptWidget,
                          uint16_t nameId,
                          uint16_t parentNameId,
                          int16_t x,
                          int16_t y,
                          int16_t width,
                          int16_t height)
{
    ldBase_t *ptParent;

    assert(NULL != ptScene);

    if (ptWidget == NULL)
    {
        ptWidget = ldCalloc(1, sizeof(ldSwitch_t));
        if (ptWidget == NULL)
        {
            LOG_ERROR("[init failed][switch] id:%d", nameId);
            return NULL;
        }
    }
    else
    {
        memset(ptWidget, 0, sizeof(ldSwitch_t));
    }

    ptParent = ldBaseGetWidget(ptScene->ptNodeRoot, parentNameId);
    ldBaseNodeAdd((arm_2d_control_node_t *)ptParent, (arm_2d_control_node_t *)ptWidget);

    ptWidget->use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX = x;
    ptWidget->use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY = y;
    ptWidget->use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iWidth = width;
    ptWidget->use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iHeight = height;
    ptWidget->use_as__ldBase_t.nameId = nameId;
    ptWidget->use_as__ldBase_t.widgetType = widgetTypeSwitch;
    ptWidget->use_as__ldBase_t.ptGuiFunc = &ldSwitchFunc;
    ptWidget->use_as__ldBase_t.isDirtyRegionUpdate = true;
    ptWidget->use_as__ldBase_t.isDirtyRegionAutoReset = true;
    ptWidget->use_as__ldBase_t.opacity = 255;
    ptWidget->use_as__ldBase_t.isCorner = true;
    ptWidget->use_as__ldBase_t.tTempRegion = ptWidget->use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion;

    ptWidget->offTrackColor = __RGB(190, 190, 190);
    ptWidget->onTrackColor = __RGB(72, 184, 120);
    ptWidget->knobColor = GLCD_COLOR_WHITE;
    ptWidget->borderColor = __RGB(120, 120, 120);
    ptWidget->knobPadding = LD_SWITCH_DEFAULT_KNOB_PADDING;
    ptWidget->direction = LD_SWITCH_DIRECTION_AUTO;
    ptWidget->isHorizontal = true;

    ldMsgConnect(ptWidget, SIGNAL_PRESS, slotSwitchProcess);
    ldMsgConnect(ptWidget, SIGNAL_RELEASE, slotSwitchProcess);

    LOG_INFO("[init][switch] id:%d, size:%d", nameId, (int)sizeof(*ptWidget));
    return ptWidget;
}

void ldSwitch_depose(ld_scene_t *ptScene, ldSwitch_t *ptWidget)
{
    (void)ptScene;

    assert(NULL != ptWidget);
    if (ptWidget == NULL)
    {
        return;
    }
    if (ptWidget->use_as__ldBase_t.widgetType != widgetTypeSwitch)
    {
        return;
    }

    LOG_INFO("[depose][switch] id:%d", ptWidget->use_as__ldBase_t.nameId);

    ldMsgDelConnect(ptWidget);
    ldBaseNodeRemove((arm_2d_control_node_t *)ptWidget);
    ldFree(ptWidget);
}

void ldSwitch_on_load(ld_scene_t *ptScene, ldSwitch_t *ptWidget)
{
    (void)ptScene;
    assert(NULL != ptWidget);
}

void ldSwitch_on_frame_start(ld_scene_t *ptScene, ldSwitch_t *ptWidget)
{
    ldSwitchAnimState_t anim;
    uint16_t progress;

    (void)ptScene;

    assert(NULL != ptWidget);
    if ((ptWidget == NULL) || (!ptWidget->isAnimating))
    {
        return;
    }

    anim = (ldSwitchAnimState_t){
        .start = ptWidget->animStartProgress,
        .target = ptWidget->animTargetProgress,
        .current = ptWidget->animProgress,
        .elapsedMs = ptWidget->animElapsedMs,
        .durationMs = LD_SWITCH_ANIM_DURATION_MS,
        .running = ptWidget->isAnimating,
    };
    progress = ptWidget->animProgress;

    ptWidget->isAnimating = ldSwitchAdvanceAnimation(&anim, LD_SWITCH_FRAME_STEP_MS, &progress);
    ptWidget->animElapsedMs = anim.elapsedMs;
    ptWidget->animProgress = progress;
    ptWidget->use_as__ldBase_t.isDirtyRegionUpdate = true;
}

void ldSwitch_on_frame_complete(ld_scene_t *ptScene, ldSwitch_t *ptWidget)
{
    (void)ptScene;
    assert(NULL != ptWidget);
    if (ptWidget == NULL)
    {
        return;
    }

    ptWidget->hasRenderedFrame = true;
}

void ldSwitch_show(ld_scene_t *ptScene, ldSwitch_t *ptWidget, const arm_2d_tile_t *ptTile, bool bIsNewFrame)
{
    arm_2d_region_t globalRegion;
    ldSwitchGeometry_t geometry;
    arm_2d_region_t tIndicatorRegion;
    arm_2d_region_t tKnobRegion;
    uint8_t opacity;
    ldColor knobColor;
    bool useOffImageStyle;
    bool useOnImageStyle;
    bool useKnobImageStyle;

    (void)ptScene;

    assert(NULL != ptWidget);
    if (ptWidget == NULL)
    {
        return;
    }

    arm_2d_helper_control_get_absolute_region((arm_2d_control_node_t *)ptWidget, &globalRegion, true);

    if (arm_2d_helper_pfb_is_region_active(ptTile, &globalRegion, true))
    {
        arm_2d_container(ptTile, tTarget, &globalRegion)
        {
            if (ldBaseIsHidden((ldBase_t *)ptWidget))
            {
                break;
            }

            opacity = ptWidget->isDisabled ? (uint8_t)(ptWidget->use_as__ldBase_t.opacity / 2U) : ptWidget->use_as__ldBase_t.opacity;
            knobColor = ptWidget->isPressed ? LD_SWITCH_PRESSED_KNOB_COLOR : ptWidget->knobColor;
            geometry = ldSwitchResolveGeometry(tTarget_canvas.tSize.iWidth,
                                               tTarget_canvas.tSize.iHeight,
                                               ptWidget->knobPadding,
                                               ptWidget->direction,
                                               ptWidget->animProgress);
            ptWidget->isHorizontal = geometry.isHorizontal;
            tIndicatorRegion = ldSwitchRectToRegion(geometry.indicator);
            tKnobRegion = ldSwitchRectToRegion(geometry.knob);
            useOffImageStyle = ptWidget->useImageStyle
                && ldSwitchLayerUsesImage(ptWidget->ptOffImgTile, ptWidget->ptOffMaskTile);
            useOnImageStyle = ptWidget->useImageStyle
                && ldSwitchLayerUsesImage(ptWidget->ptOnImgTile, ptWidget->ptOnMaskTile);
            useKnobImageStyle = ptWidget->useImageStyle
                && ldSwitchLayerUsesImage(ptWidget->ptKnobImgTile, ptWidget->ptKnobMaskTile);

            if (useOffImageStyle)
            {
                ldBaseImage(&tTarget, NULL, ptWidget->ptOffImgTile, ptWidget->ptOffMaskTile, ptWidget->offTrackColor, opacity);
            }
            else if (ptWidget->use_as__ldBase_t.isCorner)
            {
                draw_round_corner_box(&tTarget, NULL, ptWidget->offTrackColor, opacity, bIsNewFrame);
            }
            else
            {
                ldBaseColor(&tTarget, NULL, ptWidget->offTrackColor, opacity);
            }

            if (ptWidget->use_as__ldBase_t.isCorner)
            {
                draw_round_corner_border(&tTarget,
                                         NULL,
                                         ptWidget->borderColor,
                                         (arm_2d_border_opacity_t){opacity, opacity, opacity, opacity},
                                         (arm_2d_corner_opacity_t){opacity, opacity, opacity, opacity});
            }
            else
            {
                arm_2d_draw_box(&tTarget, NULL, 1, ptWidget->borderColor, opacity);
            }

            if (useOnImageStyle)
            {
                ldBaseImage(&tTarget, &tIndicatorRegion, ptWidget->ptOnImgTile, ptWidget->ptOnMaskTile, ptWidget->onTrackColor, opacity);
            }
            else if (ptWidget->use_as__ldBase_t.isCorner)
            {
                draw_round_corner_box(&tTarget, &tIndicatorRegion, ptWidget->onTrackColor, opacity, bIsNewFrame);
            }
            else
            {
                ldBaseColor(&tTarget, &tIndicatorRegion, ptWidget->onTrackColor, opacity);
            }

            if (useKnobImageStyle)
            {
                ldBaseImage(&tTarget,
                            &tKnobRegion,
                            ptWidget->ptKnobImgTile,
                            ptWidget->ptKnobMaskTile,
                            knobColor,
                            opacity);
            }
            else if (ptWidget->use_as__ldBase_t.isCorner)
            {
                draw_round_corner_box(&tTarget, &tKnobRegion, knobColor, opacity, bIsNewFrame);
            }
            else
            {
                ldBaseColor(&tTarget, &tKnobRegion, knobColor, opacity);
            }

            if (ptWidget->use_as__ldBase_t.isCorner)
            {
                draw_round_corner_border(&tTarget,
                                         &tKnobRegion,
                                         ptWidget->borderColor,
                                         (arm_2d_border_opacity_t){opacity, opacity, opacity, opacity},
                                         (arm_2d_corner_opacity_t){opacity, opacity, opacity, opacity});
            }
            else
            {
                arm_2d_draw_box(&tTarget, &tKnobRegion, 1, ptWidget->borderColor, opacity);
            }

            LD_BASE_WIDGET_SELECT;
            arm_2d_op_wait_async(NULL);
            ptWidget->hasRenderedFrame = true;
        }
    }
}

/**
 * @brief 设置颜色绘制模式，同时清除当前图片资源配置。
 * @param ptWidget 目标控件指针
 * @param offTrackColor 未选中时轨道颜色
 * @param onTrackColor 选中时轨道颜色
 * @param knobColor 滑块颜色
 * @param borderColor 轨道和滑块边框颜色
 */
void ldSwitchSetColor(ldSwitch_t *ptWidget, ldColor offTrackColor, ldColor onTrackColor, ldColor knobColor, ldColor borderColor)
{
    assert(NULL != ptWidget);
    if (ptWidget == NULL)
    {
        return;
    }

    ptWidget->use_as__ldBase_t.isDirtyRegionUpdate = true;
    ptWidget->offTrackColor = offTrackColor;
    ptWidget->onTrackColor = onTrackColor;
    ptWidget->knobColor = knobColor;
    ptWidget->borderColor = borderColor;
    ptWidget->useImageStyle = false;
    ptWidget->ptOffImgTile = NULL;
    ptWidget->ptOffMaskTile = NULL;
    ptWidget->ptOnImgTile = NULL;
    ptWidget->ptOnMaskTile = NULL;
    ptWidget->ptKnobImgTile = NULL;
    ptWidget->ptKnobMaskTile = NULL;
}

/**
 * @brief 设置图片绘制模式，分别指定关闭轨道、打开轨道和滑块的图片资源。
 * @param ptWidget 目标控件指针
 * @param ptOffImgTile 未选中轨道图片
 * @param ptOffMaskTile 未选中轨道蒙板图片
 * @param ptOnImgTile 选中轨道图片
 * @param ptOnMaskTile 选中轨道蒙板图片
 * @param ptKnobImgTile 滑块图片
 * @param ptKnobMaskTile 滑块蒙板图片
 */
void ldSwitchSetImage(ldSwitch_t *ptWidget,
                      arm_2d_tile_t *ptOffImgTile,
                      arm_2d_tile_t *ptOffMaskTile,
                      arm_2d_tile_t *ptOnImgTile,
                      arm_2d_tile_t *ptOnMaskTile,
                      arm_2d_tile_t *ptKnobImgTile,
                      arm_2d_tile_t *ptKnobMaskTile)
{
    assert(NULL != ptWidget);
    if (ptWidget == NULL)
    {
        return;
    }

    ptWidget->use_as__ldBase_t.isDirtyRegionUpdate = true;
    ptWidget->useImageStyle = true;
    ptWidget->ptOffImgTile = ptOffImgTile;
    ptWidget->ptOffMaskTile = ptOffMaskTile;
    ptWidget->ptOnImgTile = ptOnImgTile;
    ptWidget->ptOnMaskTile = ptOnMaskTile;
    ptWidget->ptKnobImgTile = ptKnobImgTile;
    ptWidget->ptKnobMaskTile = ptKnobMaskTile;
}

void _ldSwitchSetChecked(ld_scene_t *ptScene, ldSwitch_t *ptWidget, bool isChecked)
{
    ldSwitchApplyValue(ptScene, ptWidget, isChecked);
}

/**
 * @brief 设置开关方向，切换为横向或纵向布局。
 * @param ptWidget 目标控件指针
 * @param isHorizontal true 表示横向，false 表示纵向
 */
void ldSwitchSetHorizontal(ldSwitch_t *ptWidget, bool isHorizontal)
{
    ldSwitchSetDirection(ptWidget, isHorizontal ? LD_SWITCH_DIRECTION_HORIZONTAL : LD_SWITCH_DIRECTION_VERTICAL);
}

/**
 * @brief 设置开关方向；AUTO 根据控件宽高自动选择横向或纵向。
 * @param ptWidget 目标控件指针
 * @param direction AUTO/HORIZONTAL/VERTICAL 三态方向
 */
void ldSwitchSetDirection(ldSwitch_t *ptWidget, ldSwitchDirection_t direction)
{
    arm_2d_size_t size;

    assert(NULL != ptWidget);
    if (ptWidget == NULL)
    {
        return;
    }

    if ((direction != LD_SWITCH_DIRECTION_AUTO)
        && (direction != LD_SWITCH_DIRECTION_HORIZONTAL)
        && (direction != LD_SWITCH_DIRECTION_VERTICAL))
    {
        direction = LD_SWITCH_DIRECTION_AUTO;
    }

    size = ptWidget->use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize;
    ptWidget->direction = direction;
    ptWidget->isHorizontal = ldSwitchResolveIsHorizontal(size.iWidth, size.iHeight, direction);
    ptWidget->use_as__ldBase_t.isDirtyRegionUpdate = true;
}

/**
 * @brief 设置禁用状态；禁用后控件不再响应点击切换。
 * @param ptWidget 目标控件指针
 * @param isDisabled true 表示禁用，false 表示启用
 */
void ldSwitchSetDisabled(ldSwitch_t *ptWidget, bool isDisabled)
{
    assert(NULL != ptWidget);
    if (ptWidget == NULL)
    {
        return;
    }

    ptWidget->isDisabled = isDisabled;
    if (isDisabled)
    {
        ptWidget->isPressed = false;
    }
    ptWidget->use_as__ldBase_t.isDirtyRegionUpdate = true;
}

/**
 * @brief 获取当前是否为选中状态。
 * @param ptWidget 目标控件指针
 * @return bool true 表示打开，false 表示关闭
 */
bool ldSwitchIsChecked(ldSwitch_t *ptWidget)
{
    assert(NULL != ptWidget);
    if (ptWidget == NULL)
    {
        return false;
    }

    return ptWidget->isChecked;
}

/**
 * @brief 获取当前布局方向。
 * @param ptWidget 目标控件指针
 * @return bool true 表示横向，false 表示纵向
 */
bool ldSwitchIsHorizontal(ldSwitch_t *ptWidget)
{
    arm_2d_size_t size;

    assert(NULL != ptWidget);
    if (ptWidget == NULL)
    {
        return true;
    }

    size = ptWidget->use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize;
    ptWidget->isHorizontal = ldSwitchResolveIsHorizontal(size.iWidth, size.iHeight, ptWidget->direction);
    return ptWidget->isHorizontal;
}

/**
 * @brief 获取当前方向配置。
 * @param ptWidget 目标控件指针
 * @return ldSwitchDirection_t AUTO/HORIZONTAL/VERTICAL
 */
ldSwitchDirection_t ldSwitchGetDirection(ldSwitch_t *ptWidget)
{
    assert(NULL != ptWidget);
    if (ptWidget == NULL)
    {
        return LD_SWITCH_DIRECTION_AUTO;
    }

    return ptWidget->direction;
}

/**
 * @brief 获取当前是否为禁用状态。
 * @param ptWidget 目标控件指针
 * @return bool true 表示禁用，false 表示启用
 */
bool ldSwitchIsDisabled(ldSwitch_t *ptWidget)
{
    assert(NULL != ptWidget);
    if (ptWidget == NULL)
    {
        return false;
    }

    return ptWidget->isDisabled;
}

#if defined(__clang__)
#pragma clang diagnostic pop
#endif
