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

#include "ldWindowLayoutInternal.h"

uint16_t ldWindowCollectDirectChildren(ldBase_t *ptWindow, ldBase_t **ppChildren, uint16_t maxCount, bool skipHidden)
{
    uint16_t count = 0;
    ldBase_t *ptChild;

    if ((ptWindow == NULL) || (ppChildren == NULL) || (maxCount == 0))
    {
        return 0;
    }

    ptChild = ldBaseGetChildList(ptWindow);
    while ((ptChild != NULL) && (count < maxCount))
    {
        if ((skipHidden == false) || (ptChild->isHidden == false))
        {
            ppChildren[count] = ptChild;
            count++;
        }

        ptChild = ldBaseGetNextSibling(ptChild);
    }

    return count;
}

void ldBaseMarkParentLayoutDirty(ldBase_t *ptWidget)
{
    ldBase_t *ptCurrent;

    if (ptWidget == NULL)
    {
        return;
    }

    ptCurrent = ldBaseGetParent(ptWidget);
    while (ptCurrent != NULL)
    {
        if ((ptCurrent->widgetType == widgetTypeWindow) || (ptCurrent->widgetType == widgetTypeBackground))
        {
            ldWindow_t *ptWindow = (ldWindow_t *)ptCurrent;
            if (ptWindow->layoutTpye != layoutNone)
            {
                ptWindow->isLayoutUpdate = true;
            }
        }

        ptCurrent = ldBaseGetParent(ptCurrent);
    }
}

int16_t ldFlexResolveMainStart(ldFlexMainAlign_t align, int16_t innerMainSize, int16_t contentMainSize, uint16_t visibleCount, int16_t gap, int16_t *pResolvedGap)
{
    int16_t remain = innerMainSize - contentMainSize;

    if (pResolvedGap == NULL)
    {
        return 0;
    }

    if (remain <= 0)
    {
        *pResolvedGap = gap;
        return 0;
    }

    switch (align)
    {
    case ldFlexMainAlignCenter:
        *pResolvedGap = gap;
        return remain / 2;
    case ldFlexMainAlignEnd:
        *pResolvedGap = gap;
        return remain;
    case ldFlexMainAlignSpaceBetween:
        if (visibleCount >= 2)
        {
            *pResolvedGap = gap + remain / (visibleCount - 1);
            return 0;
        }
        *pResolvedGap = gap;
        return 0;
    default:
        *pResolvedGap = gap;
        return 0;
    }
}
