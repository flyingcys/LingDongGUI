/*
 * Copyright (c) 2023-2025 Ou Jianbo (59935554@qq.com). All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __LD_CANVAS_H__
#define __LD_CANVAS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "ldBase.h"

#define LD_CANVAS_MAX_COMMANDS 64

typedef enum {
    ldCanvasCommandFillRect = 0,
    ldCanvasCommandDrawLine,
    ldCanvasCommandDrawImage,
    ldCanvasCommandDrawImageScale,
    ldCanvasCommandDrawText,
} ldCanvasCommandKind_t;

typedef struct {
    ldCanvasCommandKind_t kind;
    arm_2d_region_t region;
    int16_t x1;
    int16_t y1;
    uint8_t lineSize;
    ldColor color0;
    ldColor color1;
    uint8_t opacity0;
    uint8_t opacity1;
    float scale;
    arm_2d_align_t align;
    uint8_t *pStr;
    arm_2d_font_t *ptFont;
    arm_2d_tile_t *ptImgTile;
    arm_2d_tile_t *ptMaskTile;
} ldCanvasCommand_t;

typedef struct ldCanvas_t ldCanvas_t;

struct ldCanvas_t {
    implement(ldBase_t);
    ldCanvasCommand_t commands[LD_CANVAS_MAX_COMMANDS];
    uint8_t commandCount;
};

ldCanvas_t *ldCanvas_init(ld_scene_t *ptScene,
                          ldCanvas_t *ptWidget,
                          uint16_t nameId,
                          uint16_t parentNameId,
                          int16_t x,
                          int16_t y,
                          int16_t width,
                          int16_t height);
void ldCanvas_depose(ld_scene_t *ptScene, ldCanvas_t *ptWidget);
void ldCanvas_on_load(ld_scene_t *ptScene, ldCanvas_t *ptWidget);
void ldCanvas_on_frame_start(ld_scene_t *ptScene, ldCanvas_t *ptWidget);
void ldCanvas_on_frame_complete(ld_scene_t *ptScene, ldCanvas_t *ptWidget);
void ldCanvas_show(ld_scene_t *ptScene, ldCanvas_t *ptWidget, const arm_2d_tile_t *ptTile, bool bIsNewFrame);
void ldCanvasClear(ldCanvas_t *ptWidget);
int ldCanvasPushCommand(ldCanvas_t *ptWidget, const ldCanvasCommand_t *command);

#ifdef __cplusplus
}
#endif

#endif
