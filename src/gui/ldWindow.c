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

static int16_t ldWindowResolveGridCellSize(ldGridAlign_t align, int16_t cellSize, int16_t itemSize)
{
    if (align == ldGridAlignStretch)
    {
        return MAX(0, cellSize);
    }
    return MAX(0, MIN(cellSize, itemSize));
}

static int16_t ldWindowResolveGridCellStart(ldGridAlign_t align, int16_t cellSize, int16_t itemSize)
{
    int16_t remain = MAX(0, cellSize - itemSize);

    switch (align)
    {
    case ldGridAlignCenter:
        return remain / 2;
    case ldGridAlignEnd:
        return remain;
    default:
        return 0;
    }
}

static uint16_t ldWindowCountGridTracks(const int16_t *pDsc)
{
    uint16_t count = 0;

    if (pDsc == NULL)
    {
        return 0;
    }

    while (pDsc[count] != LD_GRID_TEMPLATE_LAST)
    {
        count++;
    }

    return count;
}

static bool ldWindowGridHasDescriptors(const ldWindow_t *ptWidget)
{
    return (ptWidget != NULL) &&
           (ptWidget->gridColDsc != NULL) &&
           (ptWidget->gridRowDsc != NULL) &&
           (ldWindowCountGridTracks(ptWidget->gridColDsc) > 0) &&
           (ldWindowCountGridTracks(ptWidget->gridRowDsc) > 0);
}

static bool ldWindowGridTrackIsFr(int16_t dsc)
{
    return dsc < 0;
}

static bool ldWindowGridTrackIsContent(int16_t dsc)
{
    return dsc == LD_GRID_CONTENT;
}

static int16_t ldWindowGridTrackFrWeight(int16_t dsc)
{
    return dsc < 0 ? (int16_t)(-dsc) : 0;
}

static void ldWindowNormalizeGridCell(const ldBase_t *ptItem,
                                      uint16_t colCount,
                                      uint16_t rowCount,
                                      uint16_t *pColPos,
                                      uint16_t *pColSpan,
                                      uint16_t *pRowPos,
                                      uint16_t *pRowSpan)
{
    uint16_t colPos = 0;
    uint16_t rowPos = 0;
    uint16_t colSpan = 1;
    uint16_t rowSpan = 1;

    if (ptItem != NULL)
    {
        colPos = ptItem->gridColPos;
        rowPos = ptItem->gridRowPos;
        colSpan = ptItem->gridColSpan > 0 ? ptItem->gridColSpan : 1;
        rowSpan = ptItem->gridRowSpan > 0 ? ptItem->gridRowSpan : 1;
    }

    if (colCount == 0)
    {
        colPos = 0;
        colSpan = 0;
    }
    else
    {
        if (colPos >= colCount)
        {
            colPos = colCount - 1;
        }
        if (colSpan > (colCount - colPos))
        {
            colSpan = colCount - colPos;
        }
    }

    if (rowCount == 0)
    {
        rowPos = 0;
        rowSpan = 0;
    }
    else
    {
        if (rowPos >= rowCount)
        {
            rowPos = rowCount - 1;
        }
        if (rowSpan > (rowCount - rowPos))
        {
            rowSpan = rowCount - rowPos;
        }
    }

    *pColPos = colPos;
    *pColSpan = colSpan;
    *pRowPos = rowPos;
    *pRowSpan = rowSpan;
}

static int32_t ldWindowGridMeasureSpanRequirement(const int16_t *pDsc,
                                                  const int16_t *pTrackSizes,
                                                  uint16_t pos,
                                                  uint16_t span,
                                                  int16_t gap,
                                                  int16_t itemSize)
{
    int32_t used = 0;
    uint16_t contentCount = 0;
    uint16_t index;

    if (span == 0)
    {
        return 0;
    }

    used += (int32_t)gap * (span - 1);

    for (index = 0; index < span; ++index)
    {
        int16_t dsc = pDsc[pos + index];

        if (ldWindowGridTrackIsFr(dsc))
        {
            return 0;
        }
        if (ldWindowGridTrackIsContent(dsc))
        {
            contentCount++;
        }
        else
        {
            used += pTrackSizes[pos + index];
        }
    }

    if ((contentCount == 0) || (itemSize <= used))
    {
        return 0;
    }

    return itemSize - used;
}

static void ldWindowGridMeasureContentTracks(const int16_t *pDsc,
                                             int16_t *pTrackSizes,
                                             ldBase_t **children,
                                             uint16_t childCount,
                                             bool isColumnAxis,
                                             int16_t gap,
                                             uint16_t colCount,
                                             uint16_t rowCount)
{
    uint16_t childIndex;

    for (childIndex = 0; childIndex < childCount; ++childIndex)
    {
        ldBase_t *ptItem = children[childIndex];
        arm_2d_size_t itemSize = ptItem->use_as__arm_2d_control_node_t.tRegion.tSize;
        uint16_t colPos;
        uint16_t colSpan;
        uint16_t rowPos;
        uint16_t rowSpan;
        uint16_t pos;
        uint16_t span;
        int32_t required;
        uint16_t contentCount = 0;
        uint16_t index;
        int16_t perTrack;
        int16_t remainder;

        ldWindowNormalizeGridCell(ptItem, colCount, rowCount, &colPos, &colSpan, &rowPos, &rowSpan);
        if (isColumnAxis)
        {
            pos = colPos;
            span = colSpan;
            required = ldWindowGridMeasureSpanRequirement(pDsc, pTrackSizes, pos, span, gap, itemSize.iWidth);
        }
        else
        {
            pos = rowPos;
            span = rowSpan;
            required = ldWindowGridMeasureSpanRequirement(pDsc, pTrackSizes, pos, span, gap, itemSize.iHeight);
        }

        if (required <= 0)
        {
            continue;
        }

        for (index = 0; index < span; ++index)
        {
            if (ldWindowGridTrackIsContent(pDsc[pos + index]))
            {
                contentCount++;
            }
        }
        if (contentCount == 0)
        {
            continue;
        }

        perTrack = (int16_t)(required / contentCount);
        remainder = (int16_t)(required % contentCount);
        for (index = 0; index < span; ++index)
        {
            uint16_t trackIndex = pos + index;
            int16_t candidate;

            if (!ldWindowGridTrackIsContent(pDsc[trackIndex]))
            {
                continue;
            }
            candidate = perTrack;
            if (remainder > 0)
            {
                candidate++;
                remainder--;
            }
            pTrackSizes[trackIndex] = MAX(pTrackSizes[trackIndex], candidate);
        }
    }
}

static void ldWindowGridResolveFrTracks(const int16_t *pDsc,
                                        int16_t *pTrackSizes,
                                        uint16_t trackCount,
                                        int16_t gap,
                                        int16_t innerSize)
{
    int32_t used = (int32_t)gap * MAX(0, (int16_t)trackCount - 1);
    int32_t remaining;
    int32_t totalWeight = 0;
    uint16_t index;
    int16_t lastFr = -1;

    for (index = 0; index < trackCount; ++index)
    {
        if (ldWindowGridTrackIsFr(pDsc[index]))
        {
            totalWeight += ldWindowGridTrackFrWeight(pDsc[index]);
            lastFr = (int16_t)index;
        }
        else
        {
            used += pTrackSizes[index];
        }
    }

    remaining = MAX(0, innerSize - used);
    if ((remaining <= 0) || (totalWeight <= 0))
    {
        return;
    }

    for (index = 0; index < trackCount; ++index)
    {
        if (!ldWindowGridTrackIsFr(pDsc[index]))
        {
            continue;
        }

        if ((int16_t)index == lastFr)
        {
            pTrackSizes[index] = MAX(0, (int16_t)remaining);
            break;
        }

        pTrackSizes[index] = (int16_t)((remaining * ldWindowGridTrackFrWeight(pDsc[index])) / totalWeight);
        remaining -= pTrackSizes[index];
        totalWeight -= ldWindowGridTrackFrWeight(pDsc[index]);
    }
}

static void ldWindowGridApplyContainerAlignment(ldGridAlign_t align,
                                                int16_t innerSize,
                                                int16_t baseGap,
                                                int16_t *pTrackSizes,
                                                int16_t *pTrackPos,
                                                uint16_t trackCount)
{
    int32_t contentSize = (int32_t)baseGap * MAX(0, (int16_t)trackCount - 1);
    int32_t remain;
    int16_t offset = 0;
    int16_t gap = baseGap;
    uint16_t index;

    for (index = 0; index < trackCount; ++index)
    {
        contentSize += pTrackSizes[index];
    }

    remain = innerSize - contentSize;
    if (remain > 0)
    {
        switch (align)
        {
        case ldGridAlignCenter:
            offset = (int16_t)(remain / 2);
            break;
        case ldGridAlignEnd:
            offset = (int16_t)remain;
            break;
        case ldGridAlignSpaceEvenly:
            if (trackCount > 0)
            {
                gap += (int16_t)(remain / (trackCount + 1));
                offset = gap - baseGap;
            }
            break;
        case ldGridAlignSpaceAround:
            if (trackCount > 0)
            {
                int16_t extra = (int16_t)(remain / trackCount);
                gap += extra;
                offset = extra / 2;
            }
            break;
        case ldGridAlignSpaceBetween:
            if (trackCount > 1)
            {
                gap += (int16_t)(remain / (trackCount - 1));
            }
            break;
        case ldGridAlignStretch:
            if (trackCount > 0)
            {
                int16_t bonus = (int16_t)(remain / trackCount);
                int16_t extra = (int16_t)(remain % trackCount);
                for (index = 0; index < trackCount; ++index)
                {
                    pTrackSizes[index] += bonus;
                    if (extra > 0)
                    {
                        pTrackSizes[index]++;
                        extra--;
                    }
                }
            }
            break;
        default:
            break;
        }
    }

    for (index = 0; index < trackCount; ++index)
    {
        pTrackPos[index] = offset;
        offset += pTrackSizes[index] + gap;
    }
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
    visibleCount = ldWindowCollectDirectChildren((ldBase_t *)ptWidget, children, childCount, true);
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

    if (childCount == 0)
    {
        return;
    }

    if (!ldWindowGridHasDescriptors(ptWidget))
    {
        ldWindowApplyLegacyGridLayout(ptWidget);
        return;
    }

    ldBase_t *children[childCount];
    uint16_t visibleCount = ldWindowCollectDirectChildren((ldBase_t *)ptWidget, children, childCount, true);
    uint16_t colCount = ldWindowCountGridTracks(ptWidget->gridColDsc);
    uint16_t rowCount = ldWindowCountGridTracks(ptWidget->gridRowDsc);
    arm_2d_size_t windowSize = ptWidget->use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize;
    int16_t innerWidth = MAX(0, windowSize.iWidth - ptWidget->gridPadding.left - ptWidget->gridPadding.right);
    int16_t innerHeight = MAX(0, windowSize.iHeight - ptWidget->gridPadding.top - ptWidget->gridPadding.bottom);

    if ((visibleCount == 0) || (colCount == 0) || (rowCount == 0))
    {
        return;
    }

    int16_t colSizes[colCount];
    int16_t colPos[colCount];
    int16_t rowSizes[rowCount];
    int16_t rowPos[rowCount];
    uint16_t index;

    for (index = 0; index < colCount; ++index)
    {
        int16_t dsc = ptWidget->gridColDsc[index];
        colSizes[index] = (dsc >= 0) && !ldWindowGridTrackIsContent(dsc) ? dsc : 0;
    }
    for (index = 0; index < rowCount; ++index)
    {
        int16_t dsc = ptWidget->gridRowDsc[index];
        rowSizes[index] = (dsc >= 0) && !ldWindowGridTrackIsContent(dsc) ? dsc : 0;
    }

    ldWindowGridMeasureContentTracks(ptWidget->gridColDsc,
                                     colSizes,
                                     children,
                                     visibleCount,
                                     true,
                                     ptWidget->gridColumnGap,
                                     colCount,
                                     rowCount);
    ldWindowGridMeasureContentTracks(ptWidget->gridRowDsc,
                                     rowSizes,
                                     children,
                                     visibleCount,
                                     false,
                                     ptWidget->gridRowGap,
                                     colCount,
                                     rowCount);

    ldWindowGridResolveFrTracks(ptWidget->gridColDsc, colSizes, colCount, ptWidget->gridColumnGap, innerWidth);
    ldWindowGridResolveFrTracks(ptWidget->gridRowDsc, rowSizes, rowCount, ptWidget->gridRowGap, innerHeight);
    ldWindowGridApplyContainerAlignment(ptWidget->gridColAlign,
                                        innerWidth,
                                        ptWidget->gridColumnGap,
                                        colSizes,
                                        colPos,
                                        colCount);
    ldWindowGridApplyContainerAlignment(ptWidget->gridRowAlign,
                                        innerHeight,
                                        ptWidget->gridRowGap,
                                        rowSizes,
                                        rowPos,
                                        rowCount);

    for (index = 0; index < visibleCount; ++index)
    {
        ldBase_t *ptItem = children[index];
        arm_2d_region_t tRegion = ptItem->use_as__arm_2d_control_node_t.tRegion;
        uint16_t colIndex;
        uint16_t rowIndex;
        uint16_t colStart;
        uint16_t colSpan;
        uint16_t rowStart;
        uint16_t rowSpan;
        int16_t cellWidth = 0;
        int16_t cellHeight = 0;
        int16_t itemWidth;
        int16_t itemHeight;

        ldWindowNormalizeGridCell(ptItem, colCount, rowCount, &colStart, &colSpan, &rowStart, &rowSpan);

        for (colIndex = 0; colIndex < colSpan; ++colIndex)
        {
            cellWidth += colSizes[colStart + colIndex];
        }
        cellWidth += (int16_t)(MAX(0, (int16_t)colSpan - 1) * ptWidget->gridColumnGap);
        for (rowIndex = 0; rowIndex < rowSpan; ++rowIndex)
        {
            cellHeight += rowSizes[rowStart + rowIndex];
        }
        cellHeight += (int16_t)(MAX(0, (int16_t)rowSpan - 1) * ptWidget->gridRowGap);

        itemWidth = ldWindowResolveGridCellSize(ptItem->gridCellXAlign, cellWidth, tRegion.tSize.iWidth);
        itemHeight = ldWindowResolveGridCellSize(ptItem->gridCellYAlign, cellHeight, tRegion.tSize.iHeight);
        tRegion.tLocation.iX = ptWidget->gridPadding.left +
                               colPos[colStart] +
                               ldWindowResolveGridCellStart(ptItem->gridCellXAlign, cellWidth, itemWidth);
        tRegion.tLocation.iY = ptWidget->gridPadding.top +
                               rowPos[rowStart] +
                               ldWindowResolveGridCellStart(ptItem->gridCellYAlign, cellHeight, itemHeight);
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

void ldWindowSetGridGap(ldWindow_t *ptWidget, int16_t rowGap, int16_t columnGap)
{
    assert(NULL != ptWidget);
    if (ptWidget == NULL)
    {
        return;
    }
    ptWidget->gridRowGap = rowGap;
    ptWidget->gridColumnGap = columnGap;
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

void ldWindowSetGridDscArray(ldWindow_t *ptWidget, const int16_t *pColDsc, const int16_t *pRowDsc)
{
    assert(NULL != ptWidget);
    if (ptWidget == NULL)
    {
        return;
    }
    ptWidget->gridColDsc = pColDsc;
    ptWidget->gridRowDsc = pRowDsc;
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
