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

#ifndef __LD_WINDOW_H__
#define __LD_WINDOW_H__

#ifdef __cplusplus
extern "C"
{
#endif

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-declarations"
#pragma clang diagnostic ignored "-Wmicrosoft-anon-tag"
#pragma clang diagnostic ignored "-Wpadded"
#endif



/* OOC header, please DO NOT modify  */
#ifdef __LD_WINDOW_IMPLEMENT__
#undef __LD_WINDOW_IMPLEMENT__
#define __ARM_2D_IMPL__
#elif defined(__LD_WINDOW_INHERIT__)
#undef __LD_WINDOW_INHERIT__
#define __ARM_2D_INHERIT__
#endif
#include "arm_2d_utils.h"
#include "ldBase.h"

typedef struct ldWindow_t ldWindow_t;

typedef struct ldPadding_t
{
    int16_t left;
    int16_t top;
    int16_t right;
    int16_t bottom;
}ldPadding_t;

typedef enum {
    ldFlexFlowRow = 0,
    ldFlexFlowColumn,
    ldFlexFlowRowWrap,
    ldFlexFlowColumnWrap,
    ldFlexFlowRowReverse,
    ldFlexFlowColumnReverse,
    ldFlexFlowRowWrapReverse,
    ldFlexFlowColumnWrapReverse,
} ldFlexFlow_t;

typedef enum {
    ldFlexMainAlignStart = 0,
    ldFlexMainAlignCenter,
    ldFlexMainAlignEnd,
    ldFlexMainAlignSpaceBetween,
    ldFlexMainAlignSpaceEvenly,
    ldFlexMainAlignSpaceAround,
} ldFlexMainAlign_t;

typedef enum {
    ldFlexCrossAlignStart = 0,
    ldFlexCrossAlignCenter,
    ldFlexCrossAlignEnd,
} ldFlexCrossAlign_t;

typedef enum {
    ldFlexTrackAlignStart = 0,
    ldFlexTrackAlignCenter,
    ldFlexTrackAlignEnd,
    ldFlexTrackAlignSpaceBetween,
    ldFlexTrackAlignSpaceAround,
    ldFlexTrackAlignSpaceEvenly,
} ldFlexTrackAlign_t;

struct ldWindow_t
{
    implement(ldBase_t);
    ldColor bgColor;
    arm_2d_tile_t *ptImgTile;
    arm_2d_tile_t *ptMaskTile;
    bool isTransparent:1;
    ldPadding_t *pLayoutPaddingGroup;
    ldPadding_t flexPadding;
    union {
        struct {
            int16_t flexItemGap;
            int16_t flexTrackGap;
        };
        struct {
            int16_t flexGap;
            int16_t flexGapCompatTrack;
        };
    };
    ldPadding_t gridPadding;
    int16_t gridRowGap;
    int16_t gridColumnGap;
    uint16_t gridColumns;
    ldLayoutType_t layoutTpye:3;
    bool isLayoutUpdate:1;
    ldFlexFlow_t flexFlow:3;
    ldFlexMainAlign_t flexMainAlign:3;
    ldFlexCrossAlign_t flexCrossAlign:2;
    ldFlexTrackAlign_t flexTrackAlign:3;
};

ldWindow_t* ldWindow_init(ld_scene_t *ptScene, ldWindow_t *ptWidget, uint16_t nameId, uint16_t parentNameId, int16_t x, int16_t y, int16_t width, int16_t height);
#define ldWindowInit(nameId,parentNameId,x,y,width,height) \
        ldWindow_init(ptScene,NULL,nameId,parentNameId,x,y,width,height)
void ldWindow_depose(ld_scene_t *ptScene, ldWindow_t *ptWidget);
void ldWindow_on_load(ld_scene_t *ptScene, ldWindow_t *ptWidget);
void ldWindow_on_frame_start(ld_scene_t *ptScene, ldWindow_t *ptWidget);
void ldWindow_on_frame_complete(ld_scene_t *ptScene, ldWindow_t *ptWidget);
void ldWindow_show(ld_scene_t *pScene, ldWindow_t *ptWidget, const arm_2d_tile_t *ptTile, bool bIsNewFrame);

void ldWindowSetColor(ldWindow_t *ptWidget,ldColor bgColor);
void ldWindowSetImage(ldWindow_t *ptWidget, arm_2d_tile_t* ptImgTile, arm_2d_tile_t* ptMaskTile);

void ldWindowSetLayout(ldWindow_t *ptWidget, ldLayoutType_t type);
void ldWindowSetFlexFlow(ldWindow_t *ptWidget, ldFlexFlow_t flow);
void ldWindowSetFlexAlign(ldWindow_t *ptWidget,
                          ldFlexMainAlign_t mainAlign,
                          ldFlexCrossAlign_t crossAlign);
void ldWindowSetFlexTrackAlign(ldWindow_t *ptWidget, ldFlexTrackAlign_t trackAlign);
void ldWindowSetPadding(ldWindow_t *ptWidget, ldPadding_t padding);
void ldWindowSetFlexGap(ldWindow_t *ptWidget, int16_t itemGap, int16_t trackGap);
void ldWindowSetGap(ldWindow_t *ptWidget, int16_t gap);
void ldWindowSetGridColumns(ldWindow_t *ptWidget, uint16_t columns);
void ldWindowSetGridGap(ldWindow_t *ptWidget, int16_t rowGap, int16_t columnGap);
void ldWindowSetGridPadding(ldWindow_t *ptWidget, ldPadding_t padding);
void ldWindowSetPaddingGroup(ldWindow_t *ptWidget, ldPadding_t *pPaddingGroup);//Local variables forbidden

ldColor ldWindowGetColor(ldWindow_t *ptWidget);


#if defined(__clang__)
#pragma clang diagnostic pop
#endif

#ifdef __cplusplus
}
#endif

#endif
