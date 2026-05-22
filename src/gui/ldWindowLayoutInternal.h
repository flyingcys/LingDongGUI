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

#ifndef __LD_WINDOW_LAYOUT_INTERNAL_H__
#define __LD_WINDOW_LAYOUT_INTERNAL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "ldWindow.h"

uint16_t ldWindowCollectDirectChildren(ldBase_t *ptWindow, ldBase_t **ppChildren, uint16_t maxCount, bool skipHidden);
void ldBaseMarkParentLayoutDirty(ldBase_t *ptWidget);
int16_t ldFlexResolveMainStart(ldFlexMainAlign_t align, int16_t innerMainSize, int16_t contentMainSize, uint16_t visibleCount, int16_t gap, int16_t *pResolvedGap);

#ifdef __cplusplus
}
#endif

#endif
