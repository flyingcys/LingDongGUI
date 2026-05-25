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
#include "arm_2d_helper_shape.h"
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

typedef struct
{
    ldBase_t *ptItem;
    arm_2d_region_t tRegion;
    int16_t baseMainSize;
    int16_t baseCrossSize;
    int16_t resolvedMainSize;
} ldFlexItemLayout_t;

typedef struct
{
    uint16_t startOrderIndex;
    uint16_t count;
    int16_t crossSize;
    int32_t baseContentMainSize;
    uint32_t totalGrow;
} ldFlexTrackLayout_t;

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
    uint16_t itemCount = 0;
    arm_2d_size_t windowSize;
    int16_t innerWidth;
    int16_t innerHeight;
    bool isColumn;
    bool isWrap;
    bool isReverse;
    bool isWrapReverse;
    int16_t innerMainSize;
    int16_t innerCrossSize;
    int16_t mainPadding;
    int16_t crossPadding;
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

    ldFlexItemLayout_t items[childCount];
    uint16_t orderedIndices[childCount];
    ldFlexTrackLayout_t tracks[childCount];

    for (index = 0; index < childCount; ++index)
    {
        ldBase_t *ptChild = children[index];

        if (ptChild->ignoreLayout)
        {
            continue;
        }

        items[itemCount].ptItem = ptChild;
        items[itemCount].tRegion = ptChild->use_as__arm_2d_control_node_t.tRegion;
        items[itemCount].resolvedMainSize = 0;
        if (ptChild->hasFlexBasisSize == false)
        {
            ptChild->flexBasisSize = ptChild->use_as__arm_2d_control_node_t.tRegion.tSize;
            ptChild->hasFlexBasisSize = true;
        }
        ptChild->flexBasisSize = ldFlexClampAbsoluteSize(ptChild, ptChild->flexBasisSize);
        itemCount++;
    }

    if (itemCount == 0)
    {
        return;
    }

    windowSize = ptWidget->use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize;
    innerWidth = MAX(0, windowSize.iWidth - ptWidget->flexPadding.left - ptWidget->flexPadding.right);
    innerHeight = MAX(0, windowSize.iHeight - ptWidget->flexPadding.top - ptWidget->flexPadding.bottom);
    isColumn = ldFlexFlowIsColumn(ptWidget->flexFlow);
    isWrap = ldFlexFlowIsWrap(ptWidget->flexFlow);
    isReverse = ldFlexFlowIsReverse(ptWidget->flexFlow);
    isWrapReverse = ldFlexFlowIsWrapReverse(ptWidget->flexFlow);
    innerMainSize = isColumn ? innerHeight : innerWidth;
    innerCrossSize = isColumn ? innerWidth : innerHeight;
    mainPadding = isColumn ? ptWidget->flexPadding.top : ptWidget->flexPadding.left;
    crossPadding = isColumn ? ptWidget->flexPadding.left : ptWidget->flexPadding.top;

    for (index = 0; index < itemCount; ++index)
    {
        arm_2d_size_t tClampedBasis = ldFlexClampAbsoluteSize(items[index].ptItem,
                                                              items[index].ptItem->flexBasisSize);
        orderedIndices[index] = isReverse ? (uint16_t)(itemCount - 1 - index) : index;
        items[index].baseMainSize = isColumn ? tClampedBasis.iHeight : tClampedBasis.iWidth;
        items[index].baseCrossSize = isColumn ? tClampedBasis.iWidth : tClampedBasis.iHeight;
        items[index].resolvedMainSize = items[index].baseMainSize;
    }

    uint16_t trackCount = 1;
    memset(tracks, 0, sizeof(tracks));
    tracks[0].startOrderIndex = 0;
    for (index = 0; index < itemCount; ++index)
    {
        uint16_t itemIndex = orderedIndices[index];
        ldFlexItemLayout_t *ptItem = &items[itemIndex];
        ldFlexTrackLayout_t *ptTrack = &tracks[trackCount - 1];
        int16_t gapBeforeItem = ptTrack->count > 0 ? ptWidget->flexItemGap : 0;
        bool forceNewTrack = isWrap && (ptTrack->count > 0) && ptItem->ptItem->flexInNewTrack;
        bool exceedsTrack = false;

        if (isWrap && (ptTrack->count > 0))
        {
            int32_t nextMainSize = ptTrack->baseContentMainSize + gapBeforeItem + ptItem->baseMainSize;
            exceedsTrack = nextMainSize > innerMainSize;
        }

        if (forceNewTrack || exceedsTrack)
        {
            trackCount++;
            ptTrack = &tracks[trackCount - 1];
            memset(ptTrack, 0, sizeof(*ptTrack));
            ptTrack->startOrderIndex = index;
            gapBeforeItem = 0;
        }

        if (ptTrack->count == 0)
        {
            ptTrack->startOrderIndex = index;
        }

        ptTrack->baseContentMainSize += gapBeforeItem + ptItem->baseMainSize;
        ptTrack->crossSize = MAX(ptTrack->crossSize, ptItem->baseCrossSize);
        ptTrack->totalGrow += ptItem->ptItem->flexGrow;
        ptTrack->count++;
    }

    if ((!isWrap) && (trackCount == 1))
    {
        tracks[0].crossSize = innerCrossSize;
    }

    int32_t totalTrackCrossSize = 0;
    for (index = 0; index < trackCount; ++index)
    {
        totalTrackCrossSize += tracks[index].crossSize;
    }
    if (trackCount > 1)
    {
        totalTrackCrossSize += (int32_t)ptWidget->flexTrackGap * (trackCount - 1);
    }

    int16_t resolvedTrackGap = ptWidget->flexTrackGap;
    int16_t trackStartOffset = ldFlexResolveMainStart((ldFlexMainAlign_t)ptWidget->flexTrackAlign,
                                                      innerCrossSize,
                                                      (int16_t)totalTrackCrossSize,
                                                      trackCount,
                                                      ptWidget->flexTrackGap,
                                                      &resolvedTrackGap);
    int16_t currentTrackCross = isWrapReverse
                                    ? (int16_t)(crossPadding + innerCrossSize - trackStartOffset)
                                    : (int16_t)(crossPadding + trackStartOffset);

    for (index = 0; index < trackCount; ++index)
    {
        ldFlexTrackLayout_t *ptTrack = &tracks[index];
        int32_t contentMainSize = ptTrack->baseContentMainSize;
        int32_t remainingMainSize = MAX(0, innerMainSize - (int16_t)ptTrack->baseContentMainSize);
        uint32_t remainingGrow = ptTrack->totalGrow;
        int16_t resolvedItemGap = ptWidget->flexItemGap;
        int16_t trackCrossBase;
        uint16_t orderIndex;

        if (isWrapReverse)
        {
            currentTrackCross -= ptTrack->crossSize;
            trackCrossBase = currentTrackCross;
        }
        else
        {
            trackCrossBase = currentTrackCross;
        }

        if ((remainingMainSize > 0) && (remainingGrow > 0))
        {
            for (orderIndex = ptTrack->startOrderIndex;
                 orderIndex < (uint16_t)(ptTrack->startOrderIndex + ptTrack->count);
                 ++orderIndex)
            {
                ldFlexItemLayout_t *ptItem = &items[orderedIndices[orderIndex]];

                if (ptItem->ptItem->flexGrow == 0)
                {
                    continue;
                }

                int16_t share = (remainingGrow == ptItem->ptItem->flexGrow)
                                    ? (int16_t)remainingMainSize
                                    : (int16_t)((remainingMainSize * ptItem->ptItem->flexGrow) / remainingGrow);
                int16_t clampedMainSize;
                int16_t appliedShare;

                if (isColumn)
                {
                    arm_2d_size_t tClamped = ldFlexClampAbsoluteSize(ptItem->ptItem,
                                                                     (arm_2d_size_t){
                                                                         ptItem->baseCrossSize,
                                                                         (int16_t)(ptItem->resolvedMainSize + share),
                                                                     });
                    clampedMainSize = tClamped.iHeight;
                }
                else
                {
                    arm_2d_size_t tClamped = ldFlexClampAbsoluteSize(ptItem->ptItem,
                                                                     (arm_2d_size_t){
                                                                         (int16_t)(ptItem->resolvedMainSize + share),
                                                                         ptItem->baseCrossSize,
                                                                     });
                    clampedMainSize = tClamped.iWidth;
                }

                appliedShare = MAX(0, clampedMainSize - ptItem->resolvedMainSize);

                ptItem->resolvedMainSize = clampedMainSize;
                contentMainSize += appliedShare;
                remainingMainSize -= appliedShare;
                remainingGrow -= ptItem->ptItem->flexGrow;
            }
        }

        int16_t currentMain = ldFlexResolveMainStart(ptWidget->flexMainAlign,
                                                     innerMainSize,
                                                     (int16_t)contentMainSize,
                                                     ptTrack->count,
                                                     ptWidget->flexItemGap,
                                                     &resolvedItemGap);

        if (isReverse)
        {
            currentMain = mainPadding + innerMainSize - currentMain;
        }
        else
        {
            currentMain += mainPadding;
        }

        for (orderIndex = ptTrack->startOrderIndex;
             orderIndex < (uint16_t)(ptTrack->startOrderIndex + ptTrack->count);
             ++orderIndex)
        {
            ldFlexItemLayout_t *ptItem = &items[orderedIndices[orderIndex]];
            arm_2d_region_t tRegion = ptItem->tRegion;
            int16_t trackCrossSpan = (trackCount > 1) ? ptTrack->crossSize : innerCrossSize;
            int16_t itemCrossStart = trackCrossBase + ldWindowResolveFlexCrossStart(ptWidget->flexCrossAlign,
                                                                                    trackCrossSpan,
                                                                                    ptItem->baseCrossSize);

            if (isReverse)
            {
                currentMain -= ptItem->resolvedMainSize;
            }

            if (isColumn)
            {
                tRegion.tLocation.iX = itemCrossStart;
                tRegion.tLocation.iY = currentMain;
                tRegion.tSize.iWidth = MAX(0, ptItem->baseCrossSize);
                tRegion.tSize.iHeight = MAX(0, ptItem->resolvedMainSize);
            }
            else
            {
                tRegion.tLocation.iX = currentMain;
                tRegion.tLocation.iY = itemCrossStart;
                tRegion.tSize.iWidth = MAX(0, ptItem->resolvedMainSize);
                tRegion.tSize.iHeight = MAX(0, ptItem->baseCrossSize);
            }

            ldWindowApplyLayoutRegion(ptItem->ptItem, tRegion);

            if (isReverse)
            {
                currentMain -= resolvedItemGap;
            }
            else
            {
                currentMain += ptItem->resolvedMainSize + resolvedItemGap;
            }
        }

        if (isWrapReverse)
        {
            currentTrackCross -= resolvedTrackGap;
        }
        else
        {
            currentTrackCross += ptTrack->crossSize + resolvedTrackGap;
        }
    }
}

typedef enum {
    ldGridTrackFixed = 0,
    ldGridTrackContent,
    ldGridTrackFr,
} ldGridTrackType_t;

typedef struct {
    int16_t colPos;
    int16_t rowPos;
    int16_t colSpan;
    int16_t rowSpan;
    ldGridAlign_t xAlign;
    ldGridAlign_t yAlign;
} ldWindowGridCell_t;

static uint16_t ldWindowGridGetTrackCount(const int16_t *ptDsc)
{
    uint16_t count = 0;

    if (ptDsc == NULL)
    {
        return 0;
    }

    while (ptDsc[count] != LD_GRID_TEMPLATE_LAST)
    {
        count++;
    }

    return count;
}

static bool ldWindowGridDescriptorIsFr(int16_t value)
{
    return (value < 0) && (value != LD_GRID_TEMPLATE_LAST) && (value != LD_GRID_CONTENT);
}

static int16_t ldWindowGridGetFrValue(int16_t value)
{
    if (ldWindowGridDescriptorIsFr(value) == false)
    {
        return 0;
    }

    return MAX(1, (int16_t)(-value - 1));
}

static bool ldWindowGridHasDescriptors(const ldWindow_t *ptWidget)
{
    return (ptWidget != NULL)
        && (ptWidget->gridColDsc != NULL)
        && (ptWidget->gridRowDsc != NULL)
        && (ldWindowGridGetTrackCount(ptWidget->gridColDsc) > 0)
        && (ldWindowGridGetTrackCount(ptWidget->gridRowDsc) > 0);
}

static void ldWindowGridClampCell(int16_t *pPos, int16_t *pSpan, uint16_t trackCount)
{
    if ((pPos == NULL) || (pSpan == NULL))
    {
        return;
    }

    if (trackCount == 0)
    {
        *pPos = 0;
        *pSpan = 1;
        return;
    }

    if (*pSpan <= 0)
    {
        *pSpan = 1;
    }

    if (*pPos < 0)
    {
        *pPos = 0;
    }

    if (*pPos >= (int16_t)trackCount)
    {
        *pPos = (int16_t)trackCount - 1;
        *pSpan = 1;
        return;
    }

    if ((*pPos + *pSpan) > (int16_t)trackCount)
    {
        *pSpan = (int16_t)trackCount - *pPos;
    }

    if (*pSpan <= 0)
    {
        *pSpan = 1;
    }
}

static bool ldWindowGridCellIsExplicit(const ldBase_t *ptItem)
{
    if (ptItem == NULL)
    {
        return false;
    }

    return ptItem->isGridCellSet
        || (ptItem->gridColSpan > 0)
        || (ptItem->gridRowSpan > 0);
}

static void ldWindowGridFindNextAutoCell(const bool *pOccupied,
                                         uint16_t slotCount,
                                         uint16_t colCount,
                                         int16_t *pColPos,
                                         int16_t *pRowPos)
{
    uint16_t slotIndex;

    if ((pOccupied == NULL) || (pColPos == NULL) || (pRowPos == NULL))
    {
        return;
    }

    for (slotIndex = 0; slotIndex < slotCount; ++slotIndex)
    {
        if (pOccupied[slotIndex] == false)
        {
            *pColPos = (colCount > 0) ? (int16_t)(slotIndex % colCount) : 0;
            *pRowPos = (colCount > 0) ? (int16_t)(slotIndex / colCount) : 0;
            return;
        }
    }

    *pColPos = 0;
    *pRowPos = 0;
}

static void ldWindowGridMarkOccupied(bool *pOccupied,
                                     uint16_t colCount,
                                     uint16_t rowCount,
                                     const ldWindowGridCell_t *ptCell)
{
    uint16_t rowIndex;
    uint16_t colIndex;

    if ((pOccupied == NULL) || (ptCell == NULL))
    {
        return;
    }

    for (rowIndex = 0; rowIndex < (uint16_t)ptCell->rowSpan; ++rowIndex)
    {
        uint16_t targetRow = (uint16_t)ptCell->rowPos + rowIndex;
        if (targetRow >= rowCount)
        {
            break;
        }

        for (colIndex = 0; colIndex < (uint16_t)ptCell->colSpan; ++colIndex)
        {
            uint16_t targetCol = (uint16_t)ptCell->colPos + colIndex;
            if (targetCol >= colCount)
            {
                break;
            }

            pOccupied[targetRow * colCount + targetCol] = true;
        }
    }
}

static void ldWindowGridResolveCells(ldBase_t **children,
                                     uint16_t childCount,
                                     uint16_t colCount,
                                     uint16_t rowCount,
                                     ldWindowGridCell_t *pResolvedCells)
{
    uint16_t childIndex;
    uint16_t slotCount = colCount * rowCount;
    bool occupied[slotCount > 0 ? slotCount : 1];

    memset(occupied, 0, sizeof(occupied));

    for (childIndex = 0; childIndex < childCount; ++childIndex)
    {
        ldBase_t *ptItem = children[childIndex];
        ldWindowGridCell_t *ptCell = &pResolvedCells[childIndex];

        memset(ptCell, 0, sizeof(*ptCell));

        if (ldWindowGridCellIsExplicit(ptItem))
        {
            ptCell->colPos = ptItem->gridColPos;
            ptCell->rowPos = ptItem->gridRowPos;
            ptCell->colSpan = ptItem->gridColSpan;
            ptCell->rowSpan = ptItem->gridRowSpan;
            ptCell->xAlign = ptItem->gridCellXAlign;
            ptCell->yAlign = ptItem->gridCellYAlign;
        }
        else
        {
            ptCell->colSpan = 1;
            ptCell->rowSpan = 1;
            ptCell->xAlign = ldGridAlignStart;
            ptCell->yAlign = ldGridAlignStart;
            ldWindowGridFindNextAutoCell(occupied, slotCount, colCount, &ptCell->colPos, &ptCell->rowPos);
        }

        ldWindowGridClampCell(&ptCell->colPos, &ptCell->colSpan, colCount);
        ldWindowGridClampCell(&ptCell->rowPos, &ptCell->rowSpan, rowCount);
        ldWindowGridMarkOccupied(occupied, colCount, rowCount, ptCell);
    }
}

static int16_t ldWindowGridGetTrackUsage(uint16_t trackCount, const int16_t *pTrackSizes, int16_t gap)
{
    int32_t total = 0;
    uint16_t index;

    for (index = 0; index < trackCount; ++index)
    {
        total += pTrackSizes[index];
    }

    if (trackCount > 1)
    {
        total += (int32_t)gap * (trackCount - 1);
    }

    return (int16_t)MAX(0, total);
}

static void ldWindowGridResolveCellAxis(ldGridAlign_t align,
                                        int16_t cellSize,
                                        int16_t itemSize,
                                        int16_t *pOffset,
                                        int16_t *pResolvedSize)
{
    int16_t resolvedSize = MAX(0, itemSize);
    int16_t remain;

    if ((pOffset == NULL) || (pResolvedSize == NULL))
    {
        return;
    }

    cellSize = MAX(0, cellSize);

    if (align == ldGridAlignStretch)
    {
        *pOffset = 0;
        *pResolvedSize = cellSize;
        return;
    }

    resolvedSize = MIN(resolvedSize, cellSize);
    remain = cellSize - resolvedSize;

    switch (align)
    {
    case ldGridAlignCenter:
        *pOffset = remain / 2;
        break;
    case ldGridAlignEnd:
        *pOffset = remain;
        break;
    default:
        *pOffset = 0;
        break;
    }

    *pResolvedSize = resolvedSize;
}

static void ldWindowGridResolveTrackKinds(const int16_t *ptDsc,
                                          uint16_t trackCount,
                                          int16_t *pTrackSizes,
                                          uint8_t *pTrackKinds,
                                          int16_t *pFrValues)
{
    uint16_t index;

    for (index = 0; index < trackCount; ++index)
    {
        int16_t dscValue = ptDsc[index];

        if (dscValue == LD_GRID_CONTENT)
        {
            pTrackSizes[index] = 0;
            pTrackKinds[index] = ldGridTrackContent;
            pFrValues[index] = 0;
        }
        else if (ldWindowGridDescriptorIsFr(dscValue))
        {
            pTrackSizes[index] = 0;
            pTrackKinds[index] = ldGridTrackFr;
            pFrValues[index] = ldWindowGridGetFrValue(dscValue);
        }
        else
        {
            pTrackSizes[index] = MAX(0, dscValue);
            pTrackKinds[index] = ldGridTrackFixed;
            pFrValues[index] = 0;
        }
    }
}

static void ldWindowGridResolveContentTracks(ldBase_t **children,
                                             const ldWindowGridCell_t *pResolvedCells,
                                             uint16_t childCount,
                                             bool isColumnAxis,
                                             uint16_t trackCount,
                                             int16_t gap,
                                             const uint8_t *pTrackKinds,
                                             int16_t *pTrackSizes)
{
    uint16_t childIndex;

    for (childIndex = 0; childIndex < childCount; ++childIndex)
    {
        arm_2d_size_t itemSize = children[childIndex]->use_as__arm_2d_control_node_t.tRegion.tSize;
        int16_t desiredSize;
        uint16_t start;
        uint16_t span;
        uint16_t trackIndex;
        uint16_t contentTrackCount = 0;
        int32_t usedSize = 0;
        int16_t extra;
        int16_t share;
        int16_t remainder;

        const ldWindowGridCell_t *ptCell = &pResolvedCells[childIndex];
        start = isColumnAxis ? (uint16_t)ptCell->colPos : (uint16_t)ptCell->rowPos;
        span = isColumnAxis ? (uint16_t)ptCell->colSpan : (uint16_t)ptCell->rowSpan;
        desiredSize = isColumnAxis ? itemSize.iWidth : itemSize.iHeight;

        if ((start >= trackCount) || (span == 0))
        {
            continue;
        }

        usedSize = (int32_t)gap * (span > 0 ? (span - 1) : 0);
        for (trackIndex = start; trackIndex < (start + span); ++trackIndex)
        {
            usedSize += pTrackSizes[trackIndex];
            if (pTrackKinds[trackIndex] == ldGridTrackContent)
            {
                contentTrackCount++;
            }
        }

        if ((contentTrackCount == 0) || (usedSize >= desiredSize))
        {
            continue;
        }

        extra = (int16_t)(desiredSize - usedSize);
        share = extra / (int16_t)contentTrackCount;
        remainder = extra % (int16_t)contentTrackCount;

        for (trackIndex = start; trackIndex < (start + span); ++trackIndex)
        {
            if (pTrackKinds[trackIndex] != ldGridTrackContent)
            {
                continue;
            }

            pTrackSizes[trackIndex] += share;
            if (remainder > 0)
            {
                pTrackSizes[trackIndex]++;
                remainder--;
            }
        }
    }
}

static void ldWindowGridResolveFrTracks(uint16_t trackCount,
                                        int16_t innerSize,
                                        int16_t gap,
                                        const uint8_t *pTrackKinds,
                                        const int16_t *pFrValues,
                                        int16_t *pTrackSizes)
{
    int16_t remain;
    int32_t frTotal = 0;
    int32_t allocated = 0;
    uint16_t index;

    remain = innerSize - ldWindowGridGetTrackUsage(trackCount, pTrackSizes, gap);
    if (remain <= 0)
    {
        return;
    }

    for (index = 0; index < trackCount; ++index)
    {
        if (pTrackKinds[index] == ldGridTrackFr)
        {
            frTotal += pFrValues[index];
        }
    }

    if (frTotal <= 0)
    {
        return;
    }

    for (index = 0; index < trackCount; ++index)
    {
        int16_t share;

        if (pTrackKinds[index] != ldGridTrackFr)
        {
            continue;
        }

        frTotal -= pFrValues[index];
        if (frTotal <= 0)
        {
            share = (int16_t)MAX(0, remain - allocated);
        }
        else
        {
            share = (int16_t)(((int32_t)remain * pFrValues[index]) / (frTotal + pFrValues[index]));
        }
        pTrackSizes[index] += share;
        allocated += share;
    }
}

static void ldWindowGridResolveAxisAlign(ldGridAlign_t align,
                                         uint16_t trackCount,
                                         int16_t innerSize,
                                         int16_t *pTrackSizes,
                                         int16_t baseGap,
                                         int16_t *pStartOffset,
                                         int16_t *pResolvedGap)
{
    int16_t usedSize;
    int16_t remain;
    uint16_t index;

    if ((pTrackSizes == NULL) || (pStartOffset == NULL) || (pResolvedGap == NULL))
    {
        return;
    }

    *pStartOffset = 0;
    *pResolvedGap = baseGap;
    usedSize = ldWindowGridGetTrackUsage(trackCount, pTrackSizes, baseGap);
    remain = innerSize - usedSize;

    if (remain <= 0)
    {
        return;
    }

    switch (align)
    {
    case ldGridAlignCenter:
        *pStartOffset = remain / 2;
        break;
    case ldGridAlignEnd:
        *pStartOffset = remain;
        break;
    case ldGridAlignSpaceBetween:
        if (trackCount > 1)
        {
            *pResolvedGap = baseGap + remain / (int16_t)(trackCount - 1);
        }
        else
        {
            *pStartOffset = remain / 2;
        }
        break;
    case ldGridAlignSpaceAround:
        if (trackCount > 0)
        {
            int16_t extraGap = remain / (int16_t)trackCount;
            *pResolvedGap = baseGap + extraGap;
            *pStartOffset = extraGap / 2;
        }
        break;
    case ldGridAlignSpaceEvenly:
        if (trackCount > 0)
        {
            int16_t extraGap = remain / (int16_t)(trackCount + 1);
            *pResolvedGap = baseGap + extraGap;
            *pStartOffset = extraGap;
        }
        break;
    case ldGridAlignStretch:
        if (trackCount > 0)
        {
            int16_t share = remain / (int16_t)trackCount;
            int16_t remainder = remain % (int16_t)trackCount;

            for (index = 0; index < trackCount; ++index)
            {
                pTrackSizes[index] += share;
                if (remainder > 0)
                {
                    pTrackSizes[index]++;
                    remainder--;
                }
            }
        }
        break;
    default:
        break;
    }
}

static void ldWindowGridResolveAxisPositions(uint16_t trackCount,
                                             const int16_t *pTrackSizes,
                                             int16_t gap,
                                             int16_t startOffset,
                                             int16_t *pTrackPos)
{
    uint16_t index;
    int16_t current = startOffset;

    for (index = 0; index < trackCount; ++index)
    {
        pTrackPos[index] = current;
        current += pTrackSizes[index] + gap;
    }
}

static int16_t ldWindowGridGetCellSize(const int16_t *pTrackPos,
                                       const int16_t *pTrackSizes,
                                       uint16_t start,
                                       uint16_t span)
{
    uint16_t end = start + span - 1;

    return (int16_t)(pTrackPos[end] + pTrackSizes[end] - pTrackPos[start]);
}

static uint16_t ldWindowCollectLayoutChildren(ldWindow_t *ptWidget,
                                              ldBase_t **children,
                                              uint16_t childCount)
{
    uint16_t visibleCount;
    uint16_t layoutCount = 0;
    uint16_t childIndex;

    if ((ptWidget == NULL) || (children == NULL) || (childCount == 0))
    {
        return 0;
    }

    visibleCount = ldWindowCollectDirectChildren((ldBase_t *)ptWidget, children, childCount, true);
    for (childIndex = 0; childIndex < visibleCount; ++childIndex)
    {
        if (children[childIndex]->ignoreLayout)
        {
            continue;
        }

        children[layoutCount++] = children[childIndex];
    }

    return layoutCount;
}

static void ldWindowApplyLegacyGridLayout(ldWindow_t *ptWidget)
{
    uint16_t childCount = ldBaseGetChildCount((ldBase_t *)ptWidget);
    arm_2d_size_t windowSize;
    int16_t innerWidth;
    uint16_t visibleCount;
    uint16_t columns;
    int16_t columnGap;
    int16_t rowGap;
    int16_t columnWidth;
    uint16_t startIndex;

    if (childCount == 0)
    {
        return;
    }

    ldBase_t *children[childCount];
    visibleCount = ldWindowCollectLayoutChildren(ptWidget, children, childCount);
    if (visibleCount == 0)
    {
        return;
    }

    windowSize = ptWidget->use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize;
    innerWidth = MAX(0, windowSize.iWidth - ptWidget->gridPadding.left - ptWidget->gridPadding.right);
    columns = ptWidget->gridColumns > 0 ? ptWidget->gridColumns : 1;
    columnGap = ptWidget->gridColumnGap;
    rowGap = ptWidget->gridRowGap;

    if (columns > visibleCount)
    {
        columns = visibleCount;
    }

    if (columns > 1)
    {
        int32_t totalGap = (int32_t)columnGap * (columns - 1);
        columnWidth = (int16_t)MAX(0, innerWidth - totalGap) / (int16_t)columns;
    }
    else
    {
        columnWidth = innerWidth;
    }

    startIndex = 0;
    int16_t currentY = ptWidget->gridPadding.top;
    while (startIndex < visibleCount)
    {
        uint16_t rowItemCount = MIN(columns, visibleCount - startIndex);
        int16_t rowHeight = 0;
        uint16_t columnIndex;

        for (columnIndex = 0; columnIndex < rowItemCount; ++columnIndex)
        {
            arm_2d_size_t itemSize = children[startIndex + columnIndex]->use_as__arm_2d_control_node_t.tRegion.tSize;
            rowHeight = MAX(rowHeight, itemSize.iHeight);
        }

        for (columnIndex = 0; columnIndex < rowItemCount; ++columnIndex)
        {
            ldBase_t *ptItem = children[startIndex + columnIndex];
            arm_2d_region_t tRegion = ptItem->use_as__arm_2d_control_node_t.tRegion;
            int16_t itemWidth = tRegion.tSize.iWidth;
            int16_t currentX = ptWidget->gridPadding.left + (int16_t)columnIndex * (columnWidth + columnGap);

            if (columnWidth >= 0)
            {
                itemWidth = MIN(itemWidth, columnWidth);
            }

            tRegion.tLocation.iX = currentX;
            tRegion.tLocation.iY = currentY;
            tRegion.tSize.iWidth = MAX(0, itemWidth);
            ldWindowApplyLayoutRegion(ptItem, tRegion);
        }

        currentY += rowHeight + rowGap;
        startIndex += rowItemCount;
    }
}

static void ldWindowApplyGridLayout(ldWindow_t *ptWidget)
{
    uint16_t childCount = ldBaseGetChildCount((ldBase_t *)ptWidget);
    uint16_t visibleCount;
    uint16_t columnCount;
    uint16_t rowCount;
    arm_2d_size_t windowSize;
    int16_t innerWidth;
    int16_t innerHeight;
    int16_t columnGap;
    int16_t rowGap;
    int16_t columnStartOffset;
    int16_t rowStartOffset;
    int16_t resolvedColumnGap;
    int16_t resolvedRowGap;
    uint16_t childIndex;

    if (childCount == 0)
    {
        return;
    }

    if (ldWindowGridHasDescriptors(ptWidget) == false)
    {
        ldWindowApplyLegacyGridLayout(ptWidget);
        return;
    }

    ldBase_t *children[childCount];
    visibleCount = ldWindowCollectLayoutChildren(ptWidget, children, childCount);
    if (visibleCount == 0)
    {
        return;
    }

    columnCount = ldWindowGridGetTrackCount(ptWidget->gridColDsc);
    rowCount = ldWindowGridGetTrackCount(ptWidget->gridRowDsc);
    if ((columnCount == 0) || (rowCount == 0))
    {
        ldWindowApplyLegacyGridLayout(ptWidget);
        return;
    }

    int16_t columnSizes[columnCount];
    int16_t rowSizes[rowCount];
    int16_t columnPos[columnCount];
    int16_t rowPos[rowCount];
    uint8_t columnKinds[columnCount];
    uint8_t rowKinds[rowCount];
    int16_t columnFr[columnCount];
    int16_t rowFr[rowCount];
    ldWindowGridCell_t resolvedCells[visibleCount];

    windowSize = ptWidget->use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize;
    innerWidth = MAX(0, windowSize.iWidth - ptWidget->gridPadding.left - ptWidget->gridPadding.right);
    innerHeight = MAX(0, windowSize.iHeight - ptWidget->gridPadding.top - ptWidget->gridPadding.bottom);
    columnGap = MAX(0, ptWidget->gridColumnGap);
    rowGap = MAX(0, ptWidget->gridRowGap);
    ldWindowGridResolveCells(children, visibleCount, columnCount, rowCount, resolvedCells);

    ldWindowGridResolveTrackKinds(ptWidget->gridColDsc, columnCount, columnSizes, columnKinds, columnFr);
    ldWindowGridResolveTrackKinds(ptWidget->gridRowDsc, rowCount, rowSizes, rowKinds, rowFr);

    ldWindowGridResolveContentTracks(children, resolvedCells, visibleCount, true, columnCount, columnGap, columnKinds, columnSizes);
    ldWindowGridResolveContentTracks(children, resolvedCells, visibleCount, false, rowCount, rowGap, rowKinds, rowSizes);

    ldWindowGridResolveFrTracks(columnCount, innerWidth, columnGap, columnKinds, columnFr, columnSizes);
    ldWindowGridResolveFrTracks(rowCount, innerHeight, rowGap, rowKinds, rowFr, rowSizes);

    ldWindowGridResolveAxisAlign(ptWidget->gridColAlign,
                                 columnCount,
                                 innerWidth,
                                 columnSizes,
                                 columnGap,
                                 &columnStartOffset,
                                 &resolvedColumnGap);
    ldWindowGridResolveAxisAlign(ptWidget->gridRowAlign,
                                 rowCount,
                                 innerHeight,
                                 rowSizes,
                                 rowGap,
                                 &rowStartOffset,
                                 &resolvedRowGap);

    ldWindowGridResolveAxisPositions(columnCount, columnSizes, resolvedColumnGap, columnStartOffset, columnPos);
    ldWindowGridResolveAxisPositions(rowCount, rowSizes, resolvedRowGap, rowStartOffset, rowPos);

    for (childIndex = 0; childIndex < visibleCount; ++childIndex)
    {
        ldBase_t *ptItem = children[childIndex];
        const ldWindowGridCell_t *ptCell = &resolvedCells[childIndex];
        arm_2d_region_t tRegion = ptItem->use_as__arm_2d_control_node_t.tRegion;
        int16_t cellWidth;
        int16_t cellHeight;
        int16_t offsetX;
        int16_t offsetY;
        int16_t itemWidth;
        int16_t itemHeight;

        cellWidth = ldWindowGridGetCellSize(columnPos, columnSizes, (uint16_t)ptCell->colPos, (uint16_t)ptCell->colSpan);
        cellHeight = ldWindowGridGetCellSize(rowPos, rowSizes, (uint16_t)ptCell->rowPos, (uint16_t)ptCell->rowSpan);

        ldWindowGridResolveCellAxis(ptCell->xAlign, cellWidth, tRegion.tSize.iWidth, &offsetX, &itemWidth);
        ldWindowGridResolveCellAxis(ptCell->yAlign, cellHeight, tRegion.tSize.iHeight, &offsetY, &itemHeight);

        tRegion.tLocation.iX = ptWidget->gridPadding.left + columnPos[ptCell->colPos] + offsetX;
        tRegion.tLocation.iY = ptWidget->gridPadding.top + rowPos[ptCell->rowPos] + offsetY;
        tRegion.tSize.iWidth = itemWidth;
        tRegion.tSize.iHeight = itemHeight;
        ldWindowApplyLayoutRegion(ptItem, tRegion);
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
        case layoutGrid:
            ldWindowApplyGridLayout(ptWidget);
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

void ldWindowSetFlexTrackAlign(ldWindow_t *ptWidget, ldFlexTrackAlign_t trackAlign)
{
    assert(NULL != ptWidget);
    if (ptWidget == NULL)
    {
        return;
    }
    ptWidget->flexTrackAlign = trackAlign;
    ptWidget->layoutTpye = layoutFlex;
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

void ldWindowSetFlexGap(ldWindow_t *ptWidget, int16_t itemGap, int16_t trackGap)
{
    assert(NULL != ptWidget);
    if (ptWidget == NULL)
    {
        return;
    }
    ptWidget->flexItemGap = itemGap;
    ptWidget->flexTrackGap = trackGap;
    ptWidget->layoutTpye = layoutFlex;
    ldWindowMarkLayoutDirty(ptWidget);
}

void ldWindowSetGap(ldWindow_t *ptWidget, int16_t gap)
{
    assert(NULL != ptWidget);
    if (ptWidget == NULL)
    {
        return;
    }
    ptWidget->flexItemGap = gap;
    ptWidget->flexTrackGap = gap;
    ldWindowMarkLayoutDirty(ptWidget);
}

void ldWindowSetGridColumns(ldWindow_t *ptWidget, uint16_t columns)
{
    assert(NULL != ptWidget);
    if (ptWidget == NULL)
    {
        return;
    }
    ptWidget->gridColumns = columns > 0 ? columns : 1;
    ptWidget->layoutTpye = layoutGrid;
    ldWindowMarkLayoutDirty(ptWidget);
}

void ldWindowSetGridDscArray(ldWindow_t *ptWidget, const int16_t *gridColDsc, const int16_t *gridRowDsc)
{
    assert(NULL != ptWidget);
    if (ptWidget == NULL)
    {
        return;
    }
    ptWidget->gridColDsc = gridColDsc;
    ptWidget->gridRowDsc = gridRowDsc;
    ptWidget->layoutTpye = layoutGrid;
    ldWindowMarkLayoutDirty(ptWidget);
}

void ldWindowSetGridAlign(ldWindow_t *ptWidget, ldGridAlign_t colAlign, ldGridAlign_t rowAlign)
{
    assert(NULL != ptWidget);
    if (ptWidget == NULL)
    {
        return;
    }
    ptWidget->gridColAlign = colAlign;
    ptWidget->gridRowAlign = rowAlign;
    ptWidget->layoutTpye = layoutGrid;
    ldWindowMarkLayoutDirty(ptWidget);
}

void ldWindowSetGridGap(ldWindow_t *ptWidget, int16_t rowGap, int16_t columnGap)
{
    assert(NULL != ptWidget);
    if (ptWidget == NULL)
    {
        return;
    }
    ptWidget->gridRowGap = MAX(0, rowGap);
    ptWidget->gridColumnGap = MAX(0, columnGap);
    ptWidget->layoutTpye = layoutGrid;
    ldWindowMarkLayoutDirty(ptWidget);
}

void ldWindowSetGridPadding(ldWindow_t *ptWidget, ldPadding_t padding)
{
    assert(NULL != ptWidget);
    if (ptWidget == NULL)
    {
        return;
    }
    ptWidget->gridPadding = padding;
    ptWidget->layoutTpye = layoutGrid;
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
