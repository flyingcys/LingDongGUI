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

#include "ldSwitchInternal.h"

#include <stddef.h>

static int16_t ldSwitchClampSize(int16_t value)
{
    return (value > 0) ? value : 0;
}

ldSwitchAxisMetrics_t ldSwitchResolveAxisMetrics(int16_t width,
                                                 int16_t height,
                                                 uint16_t knobPadding,
                                                 bool isHorizontal)
{
    ldSwitchAxisMetrics_t metrics = {0};
    int16_t major = isHorizontal ? width : height;
    int16_t minor = isHorizontal ? height : width;
    int16_t padding = (int16_t)knobPadding;

    metrics.trackStart = padding;
    metrics.knobSize = ldSwitchClampSize(minor);
    metrics.trackLength = major - metrics.knobSize;
    if (metrics.trackLength < 0)
    {
        metrics.trackLength = 0;
    }

    return metrics;
}

bool ldSwitchResolveIsHorizontal(int16_t width,
                                 int16_t height,
                                 ldSwitchDirection_t direction)
{
    if (direction == LD_SWITCH_DIRECTION_HORIZONTAL)
    {
        return true;
    }
    if (direction == LD_SWITCH_DIRECTION_VERTICAL)
    {
        return false;
    }

    return width >= height;
}

uint16_t ldSwitchResolveKnobOffset(const ldSwitchAxisMetrics_t *ptMetrics,
                                   uint16_t animProgress)
{
    if (ptMetrics == NULL)
    {
        return 0;
    }

    if (animProgress > 1000)
    {
        animProgress = 1000;
    }

    return (uint16_t)((ptMetrics->trackLength * animProgress) / 1000);
}

ldSwitchGeometry_t ldSwitchResolveGeometry(int16_t width,
                                           int16_t height,
                                           uint16_t knobPadding,
                                           ldSwitchDirection_t direction,
                                           uint16_t animProgress)
{
    ldSwitchGeometry_t geometry = {0};
    ldSwitchAxisMetrics_t metrics;
    uint16_t knobOffset;
    int16_t indicatorLength;
    int16_t trackWidth;
    int16_t trackHeight;
    int16_t padding;
    bool isHorizontal;

    if (animProgress > 1000)
    {
        animProgress = 1000;
    }

    width = ldSwitchClampSize(width);
    height = ldSwitchClampSize(height);
    isHorizontal = ldSwitchResolveIsHorizontal(width, height, direction);
    metrics = ldSwitchResolveAxisMetrics(width, height, knobPadding, isHorizontal);
    knobOffset = ldSwitchResolveKnobOffset(&metrics, animProgress);
    padding = (int16_t)knobPadding;
    trackWidth = ldSwitchClampSize(width - (int16_t)(knobPadding * 2U));
    trackHeight = ldSwitchClampSize(height - (int16_t)(knobPadding * 2U));
    indicatorLength = (int16_t)(((uint32_t)(isHorizontal ? trackWidth : trackHeight) * animProgress) / 1000U);

    geometry.isHorizontal = isHorizontal;
    geometry.track = (ldSwitchRect_t){
        .iX = padding,
        .iY = padding,
        .iWidth = trackWidth,
        .iHeight = trackHeight,
    };

    if (isHorizontal)
    {
        geometry.knob = (ldSwitchRect_t){
            .iX = (int16_t)knobOffset,
            .iY = 0,
            .iWidth = metrics.knobSize,
            .iHeight = metrics.knobSize,
        };
        geometry.indicator = (ldSwitchRect_t){
            .iX = padding,
            .iY = padding,
            .iWidth = indicatorLength,
            .iHeight = trackHeight,
        };
        if (geometry.indicator.iWidth > trackWidth)
        {
            geometry.indicator.iWidth = trackWidth;
        }
    }
    else
    {
        geometry.knob = (ldSwitchRect_t){
            .iX = 0,
            .iY = (int16_t)(metrics.trackLength - (int16_t)knobOffset),
            .iWidth = metrics.knobSize,
            .iHeight = metrics.knobSize,
        };
        geometry.indicator = (ldSwitchRect_t){
            .iX = padding,
            .iY = (indicatorLength > 0)
                ? (int16_t)(height - padding - indicatorLength)
                : height,
            .iWidth = trackWidth,
            .iHeight = indicatorLength,
        };
        if (geometry.indicator.iHeight > trackHeight)
        {
            geometry.indicator.iHeight = trackHeight;
        }
    }

    return geometry;
}

bool ldSwitchLayerUsesImage(const void *ptImgTile,
                            const void *ptMaskTile)
{
    return (ptImgTile != NULL) && (ptMaskTile != NULL);
}

bool ldSwitchAdvanceAnimation(ldSwitchAnimState_t *ptAnim,
                              uint16_t deltaMs,
                              uint16_t *pOutProgress)
{
    uint32_t span;
    uint32_t value;

    if ((ptAnim == NULL) || (pOutProgress == NULL))
    {
        return false;
    }

    if ((!ptAnim->running) || (ptAnim->durationMs == 0))
    {
        ptAnim->current = ptAnim->target;
        *pOutProgress = ptAnim->current;
        ptAnim->running = false;
        return false;
    }

    ptAnim->elapsedMs = (uint16_t)(ptAnim->elapsedMs + deltaMs);
    if (ptAnim->elapsedMs >= ptAnim->durationMs)
    {
        ptAnim->elapsedMs = ptAnim->durationMs;
    }

    span = (ptAnim->target >= ptAnim->start)
        ? (uint32_t)(ptAnim->target - ptAnim->start)
        : (uint32_t)(ptAnim->start - ptAnim->target);

    value = (span * ptAnim->elapsedMs) / ptAnim->durationMs;
    ptAnim->current = (ptAnim->target >= ptAnim->start)
        ? (uint16_t)(ptAnim->start + value)
        : (uint16_t)(ptAnim->start - value);

    if (ptAnim->elapsedMs >= ptAnim->durationMs)
    {
        ptAnim->current = ptAnim->target;
        ptAnim->running = false;
    }

    *pOutProgress = ptAnim->current;
    return ptAnim->running;
}
