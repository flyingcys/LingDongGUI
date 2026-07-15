/*
 * Copyright (c) 2023-2026 flyingcys (flyingcys@gmail.com). All rights reserved.
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

#include "core/obj.h"
#include "internal.h"
#include "style/style.h"
#include "theme/theme.h"

#include <stdint.h>

/* 调用者持有的 theme 借用指针；set 只替换指针，不复制、不分配。 */
static const tinyui_theme_t *g_tinyui_theme;

extern void tinyui_runtime_set_last_result(tinyui_result_t result);

static tinyui_result_t tinyui_theme_return(tinyui_result_t result)
{
    tinyui_runtime_set_last_result(result);
    return result;
}

void tinyui_internal_theme_reset(void)
{
    g_tinyui_theme = 0;
}

static int tinyui_style_fields_are_valid(const tinyui_style_t *style)
{
    const uint32_t all_fields = UINT32_C(0xff);

    if ((style->fields & ~all_fields) != 0) {
        return 0;
    }
    if ((style->fields & TINYUI_STYLE_BORDER_WIDTH) != 0 && style->border_width < 0) {
        return 0;
    }
    if ((style->fields & TINYUI_STYLE_RADIUS) != 0 && style->radius < 0) {
        return 0;
    }
    if ((style->fields & TINYUI_STYLE_PADDING) != 0 && style->padding < 0) {
        return 0;
    }
    if ((style->fields & TINYUI_STYLE_FONT) != 0 && style->font == 0) {
        return 0;
    }
    return 1;
}

/* 固定支持矩阵：当前仅 MAIN + DEFAULT 进入 common setter 映射。 */
static int tinyui_style_part_state_supported(tinyui_part_t part, tinyui_state_t state)
{
    return part == TINYUI_PART_MAIN && state == TINYUI_STATE_DEFAULT;
}

/*
 * 任意 kind 都可能经 common setter 表达的字段并集。
 * 其外的字段（FONT、border、radius 等）在解引用对象前即可拒绝。
 */
static uint32_t tinyui_style_globally_applyable_fields(void)
{
    return TINYUI_STYLE_BG_COLOR | TINYUI_STYLE_TEXT_COLOR | TINYUI_STYLE_OPACITY
           | TINYUI_STYLE_PADDING;
}

/*
 * 与 tinyui_obj_set_* 当前能力对齐的静态矩阵（修改前预检，避免半更新）。
 */
static uint32_t tinyui_style_supported_fields_for_kind(uint8_t kind)
{
    uint32_t fields = TINYUI_STYLE_OPACITY;

    switch (kind) {
    case TINYUI_BACKEND_WIDGET_LABEL:
    case TINYUI_BACKEND_WIDGET_BUTTON:
    case TINYUI_BACKEND_WIDGET_CHECKBOX:
        fields |= TINYUI_STYLE_BG_COLOR | TINYUI_STYLE_TEXT_COLOR;
        break;
    case TINYUI_BACKEND_WIDGET_SLIDER:
        fields |= TINYUI_STYLE_BG_COLOR;
        break;
    case TINYUI_BACKEND_WIDGET_WINDOW:
    case TINYUI_BACKEND_WIDGET_BACKGROUND:
        fields |= TINYUI_STYLE_PADDING;
        break;
    default:
        break;
    }
    return fields;
}

tinyui_result_t tinyui_obj_apply_style(tinyui_obj_t *obj,
                                       tinyui_part_t part,
                                       tinyui_state_t state,
                                       const tinyui_style_t *style)
{
    struct tinyui_widget *widget;
    uint32_t supported;
    tinyui_result_t rc;

    if (obj == 0) {
        return tinyui_theme_return(TINYUI_ERROR_INVALID_OBJECT);
    }
    if (style == 0) {
        return tinyui_theme_return(TINYUI_ERROR_INVALID_ARG);
    }
    if (part < TINYUI_PART_MAIN || part > TINYUI_PART_TRACK
        || state < TINYUI_STATE_DEFAULT || state > TINYUI_STATE_FOCUSED) {
        return tinyui_theme_return(TINYUI_ERROR_OUT_OF_RANGE);
    }
    if (!tinyui_style_fields_are_valid(style)) {
        return tinyui_theme_return(TINYUI_ERROR_INVALID_ARG);
    }
    if (style->fields == 0) {
        return tinyui_theme_return(TINYUI_ERROR_INVALID_ARG);
    }
    if (!tinyui_style_part_state_supported(part, state)) {
        return tinyui_theme_return(TINYUI_ERROR_NOT_SUPPORTED);
    }

    /* 不依赖对象内容的字段拒绝：在任何解引用之前返回。 */
    if ((style->fields & ~tinyui_style_globally_applyable_fields()) != 0) {
        return tinyui_theme_return(TINYUI_ERROR_NOT_SUPPORTED);
    }

    /*
     * Without a live runtime there is no safe way to interpret an opaque
     * tinyui_obj_t pointer; refuse before any host/widget dereference.
     * Contract probes may pass non-null sentinels with only applyable fields.
     */
    if (tinyui_runtime_internal_app_current() == 0) {
        return tinyui_theme_return(TINYUI_ERROR_NOT_SUPPORTED);
    }

    widget = (struct tinyui_widget *)(void *)obj;
    if (widget->deleting != 0) {
        return tinyui_theme_return(TINYUI_ERROR_INVALID_STATE);
    }

    supported = tinyui_style_supported_fields_for_kind(widget->kind);
    if ((style->fields & ~supported) != 0) {
        return tinyui_theme_return(TINYUI_ERROR_NOT_SUPPORTED);
    }

    /* 固定顺序：bg → text → border → width → radius → padding → opacity → font。
     * 当前矩阵内只会出现其中子集；不保存 style 指针。 */
    if ((style->fields & TINYUI_STYLE_BG_COLOR) != 0) {
        rc = tinyui_obj_set_bg_color(obj, style->bg_color);
        if (rc != TINYUI_OK) {
            return tinyui_theme_return(rc);
        }
    }
    if ((style->fields & TINYUI_STYLE_TEXT_COLOR) != 0) {
        rc = tinyui_obj_set_text_color(obj, style->text_color);
        if (rc != TINYUI_OK) {
            return tinyui_theme_return(rc);
        }
    }
    if ((style->fields & TINYUI_STYLE_BORDER_COLOR) != 0) {
        rc = tinyui_obj_set_border_color(obj, style->border_color);
        if (rc != TINYUI_OK) {
            return tinyui_theme_return(rc);
        }
    }
    if ((style->fields & TINYUI_STYLE_BORDER_WIDTH) != 0) {
        rc = tinyui_obj_set_border_width(obj, style->border_width);
        if (rc != TINYUI_OK) {
            return tinyui_theme_return(rc);
        }
    }
    if ((style->fields & TINYUI_STYLE_RADIUS) != 0) {
        rc = tinyui_obj_set_radius(obj, style->radius);
        if (rc != TINYUI_OK) {
            return tinyui_theme_return(rc);
        }
    }
    if ((style->fields & TINYUI_STYLE_PADDING) != 0) {
        rc = tinyui_obj_set_padding(obj, style->padding);
        if (rc != TINYUI_OK) {
            return tinyui_theme_return(rc);
        }
    }
    if ((style->fields & TINYUI_STYLE_OPACITY) != 0) {
        rc = tinyui_obj_set_opacity(obj, (int)style->opacity);
        if (rc != TINYUI_OK) {
            return tinyui_theme_return(rc);
        }
    }

    return tinyui_theme_return(TINYUI_OK);
}

tinyui_result_t tinyui_theme_set(const tinyui_theme_t *theme)
{
    if (theme == 0) {
        return tinyui_theme_return(TINYUI_ERROR_INVALID_ARG);
    }

    g_tinyui_theme = theme;
    return tinyui_theme_return(TINYUI_OK);
}

const tinyui_theme_t *tinyui_theme_get(void)
{
    return g_tinyui_theme;
}

tinyui_result_t tinyui_theme_apply(tinyui_obj_t *obj)
{
    tinyui_style_t style;

    if (obj == 0) {
        return tinyui_theme_return(TINYUI_ERROR_INVALID_OBJECT);
    }
    if (g_tinyui_theme == 0) {
        return tinyui_theme_return(TINYUI_ERROR_INVALID_STATE);
    }

    /* 栈上临时 descriptor；只应用目标对象，不遍历 tree、不分配。 */
    style.fields = TINYUI_STYLE_BG_COLOR | TINYUI_STYLE_TEXT_COLOR;
    style.bg_color = g_tinyui_theme->colors[TINYUI_COLOR_BG];
    style.text_color = g_tinyui_theme->colors[TINYUI_COLOR_TEXT_PRIMARY];
    style.border_color = 0;
    style.border_width = 0;
    style.radius = 0;
    style.padding = 0;
    style.opacity = 0;
    style.font = 0;

    return tinyui_obj_apply_style(obj, TINYUI_PART_MAIN, TINYUI_STATE_DEFAULT, &style);
}
