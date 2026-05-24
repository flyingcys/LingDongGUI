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

ldSwitchAxisMetrics_t ldSwitchResolveAxisMetrics(int16_t width,
                                                 int16_t height,
                                                 uint16_t knobPadding,
                                                 bool isHorizontal)
{
    ldSwitchAxisMetrics_t metrics = {0};
    int16_t major = isHorizontal ? width : height;
    int16_t minor = isHorizontal ? height : width;
    int16_t padding = (int16_t)knobPadding;
    int16_t knob = minor - (int16_t)(knobPadding * 2);

    if (knob < 0)
    {
        knob = 0;
    }

    metrics.trackStart = padding;
    metrics.knobSize = knob;
    metrics.trackLength = major - knob - (int16_t)(knobPadding * 2);
    if (metrics.trackLength < 0)
    {
        metrics.trackLength = 0;
    }

    return metrics;
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
