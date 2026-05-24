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

#ifndef __LD_SWITCH_INTERNAL_H__
#define __LD_SWITCH_INTERNAL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    int16_t trackStart;
    int16_t trackLength;
    int16_t knobSize;
} ldSwitchAxisMetrics_t;

typedef struct {
    uint16_t start;
    uint16_t target;
    uint16_t current;
    uint16_t elapsedMs;
    uint16_t durationMs;
    bool running;
} ldSwitchAnimState_t;

ldSwitchAxisMetrics_t ldSwitchResolveAxisMetrics(int16_t width,
                                                 int16_t height,
                                                 uint16_t knobPadding,
                                                 bool isHorizontal);

uint16_t ldSwitchResolveKnobOffset(const ldSwitchAxisMetrics_t *ptMetrics,
                                   uint16_t animProgress);

bool ldSwitchAdvanceAnimation(ldSwitchAnimState_t *ptAnim,
                              uint16_t deltaMs,
                              uint16_t *pOutProgress);

#ifdef __cplusplus
}
#endif

#endif
