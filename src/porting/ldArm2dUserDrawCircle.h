/*
 * Copyright (C) 2024 Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef LD_ARM_2D_USER_DRAW_CIRCLE_H
#define LD_ARM_2D_USER_DRAW_CIRCLE_H

#include "arm_2d_helper.h"

#ifdef   __cplusplus
extern "C" {
#endif

#if defined(__clang__)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wunknown-warning-option"
#   pragma clang diagnostic ignored "-Wreserved-identifier"
#   pragma clang diagnostic ignored "-Wdeclaration-after-statement"
#   pragma clang diagnostic ignored "-Wsign-conversion"
#   pragma clang diagnostic ignored "-Wpadded"
#   pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
#   pragma clang diagnostic ignored "-Wmissing-declarations"
#endif

typedef struct ld_arm_2d_user_draw_circle_api_params_t {
    arm_2d_location_t *ptPivot;
    int16_t iRadius;
    bool bAntiAlias;
} ld_arm_2d_user_draw_circle_api_params_t;

typedef struct ld_arm_2d_user_draw_circle_descriptor_t {
    implement(arm_2d_op_t);
    ld_arm_2d_user_draw_circle_api_params_t tParams;
    arm_2d_location_t tPivot;
    arm_2d_region_t tDrawRegion;
    uint8_t chOpacity;
    COLOUR_TYPE_T tForeground;
} ld_arm_2d_user_draw_circle_descriptor_t;

extern
const __arm_2d_op_info_t LD_ARM_2D_OP_USER_DRAW_CIRCLE;

extern
ARM_NONNULL(2, 4)
arm_fsm_rt_t ldArm2dDrawCircle(ld_arm_2d_user_draw_circle_descriptor_t *ptOP,
                               const arm_2d_tile_t *ptTarget,
                               const arm_2d_region_t *ptRegion,
                               const ld_arm_2d_user_draw_circle_api_params_t *ptParams,
                               arm_2d_color_rgb565_t tColour,
                               uint8_t chOpacity);

#if defined(__clang__)
#   pragma clang diagnostic pop
#endif

#ifdef   __cplusplus
}
#endif

#endif
