/*
 * Copyright (c) 2009-2024 Arm Limited. All rights reserved.
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

#ifndef LD_ARM_2D_OPCODE_COMMON_H
#define LD_ARM_2D_OPCODE_COMMON_H

#ifdef   __cplusplus
extern "C" {
#endif

#include "arm_2d.h"

enum {
    LD_ARM_2D_OP_IDX_USER_DRAW_LINE = __ARM_2D_OP_IDX_USER_OP_START,
    LD_ARM_2D_OP_IDX_USER_DRAW_CIRCLE,
};

#ifdef   __cplusplus
}
#endif

#endif
