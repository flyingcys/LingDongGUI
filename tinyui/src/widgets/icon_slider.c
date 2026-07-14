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

#include "internal.h"
#include "widgets/icon_slider.h"
#include "internal/widget_legacy.h"
#include "../core/runtime_bridge.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldIconSlider.h"
#include "../../../src/misc/ldMsg.h"


static struct tinyui_icon_slider *tinyui_icon_slider_as_icon_slider(tinyui_obj_t *obj)
{
    struct tinyui_widget *w = (struct tinyui_widget *)(void *)obj;
    if (w == 0 || !tinyui_runtime_internal_widget_is_kind(w, TINYUI_BACKEND_WIDGET_ICON_SLIDER)) {
        return 0;
    }
    return (struct tinyui_icon_slider *)w;
}

static const struct tinyui_icon_slider *tinyui_icon_slider_as_icon_slider_const(const tinyui_obj_t *obj)
{
    const struct tinyui_widget *w = (const struct tinyui_widget *)(const void *)obj;
    if (w == 0 || !tinyui_runtime_internal_widget_is_kind(w, TINYUI_BACKEND_WIDGET_ICON_SLIDER)) {
        return 0;
    }
    return (const struct tinyui_icon_slider *)w;
}


extern const arm_2d_tile_t c_tileQuaterArcGRAY8;
extern const arm_2d_tile_t c_tileQuaterArcMask;
extern const arm_2d_tile_t c_tilePointerSecGRAY8;
extern const arm_2d_tile_t c_tilePointerSecMask;

static arm_2d_tile_t *const g_icon_slider_tiles[] = {
    (arm_2d_tile_t *)&c_tileQuaterArcGRAY8,
    (arm_2d_tile_t *)&c_tilePointerSecGRAY8,
    (arm_2d_tile_t *)&c_tileQuaterArcGRAY8,
    (arm_2d_tile_t *)&c_tilePointerSecGRAY8,
};

static arm_2d_tile_t *const g_icon_slider_masks[] = {
    (arm_2d_tile_t *)&c_tileQuaterArcMask,
    (arm_2d_tile_t *)&c_tilePointerSecMask,
    (arm_2d_tile_t *)&c_tileQuaterArcMask,
    (arm_2d_tile_t *)&c_tilePointerSecMask,
};

#define TINYUI_ICON_SLIDER_NATIVE_MAX_ITEMS 8
#define TINYUI_ICON_SLIDER_IMAGE_FONT_SPACE 1


struct tinyui_icon_slider_create_ctx {
    int width;
    int height;
    int icon_width;
    int icon_space;
    int columns;
    int rows;
    int pages;
};

static void *tinyui_runtime_internal_icon_slider_ld_init(void *ctx,
                                        struct ld_scene_t *scene,
                                        uint16_t name_id,
                                        uint16_t parent_name_id)
{
    const struct tinyui_icon_slider_create_ctx *create_ctx;

    create_ctx = (const struct tinyui_icon_slider_create_ctx *)ctx;
    if (scene == 0 || create_ctx == 0) {
        return 0;
    }

    return ldIconSlider_init(scene,
                             NULL,
                             name_id,
                             parent_name_id,
                             0,
                             0,
                             (int16_t)create_ctx->width,
                             (int16_t)create_ctx->height,
                             (int16_t)create_ctx->icon_width,
                             (uint8_t)create_ctx->icon_space,
                             (uint8_t)create_ctx->columns,
                             (uint8_t)create_ctx->rows,
                             (uint8_t)create_ctx->pages,
                             tinyui_resolve_ld_font(0, 12));
}


static void tinyui_icon_slider_rollback(struct tinyui_icon_slider *icon_slider)
{
    if (icon_slider == 0) {
        return;
    }
    if (icon_slider->widget.ld_widget != 0) {
        tinyui_runtime_internal_widget_destroy_common(&icon_slider->widget);
    } else {
        ldFree(icon_slider);
    }
}

static bool tinyui_icon_slider_native_slot(struct ld_scene_t *scene, ldMsg_t msg)
{
    struct tinyui_widget *w;
    struct tinyui_icon_slider *icon_slider;
    ldIconSlider_t *ld_icon_slider;
    int selected_index;
    int previous_selected_index;

    if (msg.ptSender == 0 || msg.signal != SIGNAL_CLICKED_ITEM) {
        return false;
    }

    w = tinyui_runtime_internal_widget_from_ld_scene(scene, msg.ptSender);
    if (w == 0) {
        return false;
    }

    icon_slider = (struct tinyui_icon_slider *)w;
    selected_index = (int)msg.value;
    if (selected_index < 0 || selected_index >= icon_slider->item_count) {
        return false;
    }

    previous_selected_index = icon_slider->selected_index;
    if (icon_slider->widget.visible == 0 || icon_slider->widget.enabled == 0) {
        return false;
    }

    if (tinyui_runtime_internal_widget_claim_backend_focus(w) != 0) {
        return false;
    }

    if (w->kind != TINYUI_BACKEND_WIDGET_ICON_SLIDER ||
        w->ld_widget == 0 ||
        selected_index < 0 ||
        selected_index >= (int)w->list_item_count) {
        return false;
    }
    ld_icon_slider = (ldIconSlider_t *)w->ld_widget;
    ld_icon_slider->selectIconOrPage = (uint8_t)selected_index;
    ld_icon_slider->isWaitMove = true;
    w->value = selected_index;

    icon_slider->selected_index = selected_index;
    if (previous_selected_index == selected_index) {
        return false;
    }
    if (icon_slider->cb != 0) {
        icon_slider->cb(icon_slider, selected_index, icon_slider->user_data);
    }
    return false;
}

static int tinyui_icon_slider_props_are_valid(const tinyui_icon_slider_props_t *props)
{
    return props != 0 &&
           props->width >= 0 &&
           props->height >= 0 &&
           props->icon_width >= 0 &&
           props->icon_space >= 0 &&
           props->columns >= 0 &&
           props->rows >= 0 &&
           props->pages >= 0;
}

static void tinyui_icon_slider_apply_native_layout(ldIconSlider_t *ld_icon_slider,
                                                   int width,
                                                   int height,
                                                   int icon_width,
                                                   int icon_space,
                                                   int columns,
                                                   int rows,
                                                   int pages)
{
    ldBaseSetWidth((ldBase_t *)ld_icon_slider, (int16_t)width);
    ldBaseSetHeight((ldBase_t *)ld_icon_slider, (int16_t)height);
    ld_icon_slider->iconWidth = (int16_t)icon_width;
    ld_icon_slider->iconSpace = (uint8_t)icon_space;
    ld_icon_slider->columnCount = (uint8_t)columns;
    ld_icon_slider->rowCount = (uint8_t)rows;
    ld_icon_slider->pageMax = (uint8_t)pages;
    ld_icon_slider->iconMax = (uint16_t)(rows * columns * pages);
    ld_icon_slider->scrollOffset = 0;
    ld_icon_slider->selectIconOrPage = 0;
    ld_icon_slider->isWaitMove = false;

    if (rows == 1) {
        if (columns == 1) {
            ld_icon_slider->hasVerticalBorder = false;
            ld_icon_slider->hasHorizontalBorder = false;
        } else {
            ld_icon_slider->hasVerticalBorder = true;
            ld_icon_slider->hasHorizontalBorder = false;
        }
    } else if (columns == 1) {
        ld_icon_slider->hasVerticalBorder = false;
        ld_icon_slider->hasHorizontalBorder = true;
    } else {
        ld_icon_slider->hasVerticalBorder = true;
        ld_icon_slider->hasHorizontalBorder = true;
    }

    ld_icon_slider->isScrollEn = true;
    if (pages == 1 && (rows == 1 || columns == 1)) {
        if (rows < columns) {
            ld_icon_slider->isHorizontalScroll = true;
            if (width >= ld_icon_slider->iconMax * (icon_width + icon_space)) {
                ld_icon_slider->isScrollEn = false;
            }
        } else {
            ld_icon_slider->isHorizontalScroll = false;
            if (height >= ld_icon_slider->iconMax
                * (icon_width + icon_space + ld_icon_slider->ptFont->tCharSize.iHeight
                   + TINYUI_ICON_SLIDER_IMAGE_FONT_SPACE)) {
                ld_icon_slider->isScrollEn = false;
            }
        }
    }
}

static struct tinyui_icon_slider *tinyui_icon_slider_create_with_backend_config(struct tinyui_widget *parent,
                                                                                const char *id,
                                                                                int width,
                                                                                int height,
                                                                                int icon_width,
                                                                                int icon_space,
                                                                                int columns,
                                                                                int rows,
                                                                                int pages)
{
    struct tinyui_icon_slider *icon_slider;
    ldIconSlider_t *ld_icon_slider;
    struct tinyui_icon_slider_create_ctx create_ctx;

    if (parent == 0 || id == 0) {
        return 0;
    }

    if (width <= 0 || height <= 0 || icon_width <= 0) {
        return 0;
    }
    if (icon_space < 0) {
        icon_space = 0;
    }
    if (columns <= 0) {
        columns = 1;
    }
    if (rows <= 0) {
        rows = 1;
    }
    if (pages <= 0) {
        pages = 1;
    }

    if (((struct tinyui_widget *)(void *)parent)->ld_widget == 0 || ((struct tinyui_widget *)(void *)parent)->owner == 0 || ((struct tinyui_widget *)(void *)parent)->owner->ld_scene == 0) {
        return 0;
    }

    create_ctx.width = width;
    create_ctx.height = height;
    create_ctx.icon_width = icon_width;
    create_ctx.icon_space = icon_space;
    create_ctx.columns = columns;
    create_ctx.rows = rows;
    create_ctx.pages = pages;

    icon_slider = (struct tinyui_icon_slider *)tinyui_runtime_internal_widget_create_leaf(parent,
                                                                         TINYUI_BACKEND_WIDGET_ICON_SLIDER,
                                                                         tinyui_runtime_internal_icon_slider_ld_init,
                                                                         &create_ctx,
                                                                         sizeof(*icon_slider));
    if (icon_slider == 0) {
        return 0;
    }

    icon_slider->widget.value = -1;
    icon_slider->widget.visible = 1;
    icon_slider->widget.enabled = 1;

    icon_slider->id = id;
    icon_slider->selected_index = -1;
    icon_slider->horizontal = 1;
    icon_slider->icon_width = icon_width;
    icon_slider->icon_space = icon_space;
    icon_slider->columns = columns;
    icon_slider->rows = rows;
    icon_slider->pages = pages;

    ld_icon_slider = (ldIconSlider_t *)icon_slider->widget.ld_widget;
    if (!ldMsgConnect(ld_icon_slider, SIGNAL_CLICKED_ITEM, tinyui_icon_slider_native_slot)) {
        tinyui_icon_slider_rollback(icon_slider);
        return 0;
    }

    return icon_slider;
}

tinyui_obj_t *tinyui_icon_slider_create(tinyui_obj_t *parent)
{
    struct tinyui_widget *parent_w = (struct tinyui_widget *)(void *)parent;
    const char *id = "icon_slider";
    if (parent_w == 0) { return 0; }

    return tinyui_icon_slider_create_with_backend_config(parent_w, id, 220, 86, 48, 4, 4, 1, 2);
}

struct tinyui_icon_slider *tinyui_runtime_internal_icon_slider_init(struct tinyui_widget *parent, const char *id)
{
    return tinyui_icon_slider_create(parent);
}

tinyui_obj_t *tinyui_icon_slider_create_with_props(tinyui_obj_t *parent,
                                             const tinyui_icon_slider_props_t *props)
{
    tinyui_obj_t *obj;
    struct tinyui_icon_slider *icon_slider;

    if (props == 0) {
        return tinyui_icon_slider_create(parent);
    }

    obj = tinyui_icon_slider_create(parent);
    if (obj == 0) {
        return 0;
    }
    icon_slider = (struct tinyui_icon_slider *)(void *)obj;

    if ((props->fields & TINYUI_ICON_SLIDER_FIELD_ID) != 0) {
        (void)props->id;
    }
    if ((props->fields & TINYUI_ICON_SLIDER_FIELD_USER_DATA) != 0) {
    if (tinyui_runtime_internal_widget_set_user_data(&icon_slider->widget, props->user_data) != 0) {
        tinyui_icon_slider_rollback(icon_slider);
        return 0;
    }
    }
    if ((props->fields & TINYUI_ICON_SLIDER_FIELD_STYLE_CLASS) != 0) {
    if (tinyui_runtime_internal_widget_set_style_class(&icon_slider->widget, props->style_class) != 0) {
        tinyui_icon_slider_rollback(icon_slider);
        return 0;
    }
    }
    if ((props->fields & TINYUI_ICON_SLIDER_FIELD_WIDTH) != 0 ||
        (props->fields & TINYUI_ICON_SLIDER_FIELD_HEIGHT) != 0 ||
        (props->fields & TINYUI_ICON_SLIDER_FIELD_ICON_WIDTH) != 0 ||
        (props->fields & TINYUI_ICON_SLIDER_FIELD_ICON_SPACE) != 0 ||
        (props->fields & TINYUI_ICON_SLIDER_FIELD_COLUMNS) != 0 ||
        (props->fields & TINYUI_ICON_SLIDER_FIELD_ROWS) != 0 ||
        (props->fields & TINYUI_ICON_SLIDER_FIELD_PAGES) != 0) {
        int w = ((props->fields & TINYUI_ICON_SLIDER_FIELD_WIDTH) != 0) ? props->width : tinyui_runtime_internal_widget_get_width(&icon_slider->widget);
        int h = ((props->fields & TINYUI_ICON_SLIDER_FIELD_HEIGHT) != 0) ? props->height : tinyui_runtime_internal_widget_get_height(&icon_slider->widget);
        int iw = ((props->fields & TINYUI_ICON_SLIDER_FIELD_ICON_WIDTH) != 0) ? props->icon_width : icon_slider->icon_width;
        int is = ((props->fields & TINYUI_ICON_SLIDER_FIELD_ICON_SPACE) != 0) ? props->icon_space : icon_slider->icon_space;
        int cols = ((props->fields & TINYUI_ICON_SLIDER_FIELD_COLUMNS) != 0) ? props->columns : icon_slider->columns;
        int rows = ((props->fields & TINYUI_ICON_SLIDER_FIELD_ROWS) != 0) ? props->rows : icon_slider->rows;
        int pages = ((props->fields & TINYUI_ICON_SLIDER_FIELD_PAGES) != 0) ? props->pages : icon_slider->pages;
        if (w < 0) { w = 0; }
        if (h < 0) { h = 0; }
            if (tinyui_icon_slider_set_layout((tinyui_obj_t *)icon_slider, w, h, iw, is, cols, rows, pages) != 0) {
                tinyui_icon_slider_rollback(icon_slider);
                return 0;
            }
    }
    if ((props->fields & TINYUI_ICON_SLIDER_FIELD_HORIZONTAL) != 0) {
    if (tinyui_icon_slider_set_horizontal((tinyui_obj_t *)icon_slider, props->horizontal) != 0) {
        tinyui_icon_slider_rollback(icon_slider);
        return 0;
    }
    }

    return obj;
}



int tinyui_icon_slider_add_item(tinyui_obj_t *icon_slider_obj, const char *id, const char *text)
{
    struct tinyui_icon_slider *icon_slider = tinyui_icon_slider_as_icon_slider(icon_slider_obj);
    if (icon_slider == 0) { return -1; }

    ldIconSlider_t *ld_icon_slider;
    int index;

    if (icon_slider == 0 || id == 0 || text == 0
        || icon_slider->widget.ld_widget == 0
        || icon_slider->widget.list_item_count >= TINYUI_ICON_SLIDER_NATIVE_MAX_ITEMS) {
        return -1;
    }

    ld_icon_slider = (ldIconSlider_t *)icon_slider->widget.ld_widget;

    index = icon_slider->widget.list_item_count;
    ldIconSliderAddIcon(ld_icon_slider,
                        g_icon_slider_tiles[index % 4],
                        g_icon_slider_masks[index % 4],
                        (const uint8_t *)text);
    icon_slider->widget.list_item_count++;

    index = icon_slider->item_count++;
    icon_slider->items[index].id = id;
    icon_slider->items[index].text = text;
    return 0;
}

int tinyui_icon_slider_add_item_with_source(tinyui_obj_t *icon_slider_obj, const char *id, const char *text, struct tinyui_image_source *source)
{
    struct tinyui_icon_slider *icon_slider = tinyui_icon_slider_as_icon_slider(icon_slider_obj);
    if (icon_slider == 0) { return -1; }

    ldIconSlider_t *ld_icon_slider;
    int index;

    if (icon_slider == 0
        || id == 0
        || text == 0
        || source == 0
        || tinyui_image_source_get_image_tile(source) == 0
        || tinyui_image_source_get_mask_tile(source) == 0
        || icon_slider->widget.ld_widget == 0
        || icon_slider->widget.list_item_count >= TINYUI_ICON_SLIDER_NATIVE_MAX_ITEMS) {
        return -1;
    }

    ld_icon_slider = (ldIconSlider_t *)icon_slider->widget.ld_widget;

    index = icon_slider->widget.list_item_count;
    ldIconSliderAddIcon(ld_icon_slider,
                        tinyui_image_source_get_image_tile(source),
                        tinyui_image_source_get_mask_tile(source),
                        (const uint8_t *)text);
    icon_slider->widget.list_item_count++;

    index = icon_slider->item_count++;
    icon_slider->items[index].id = id;
    icon_slider->items[index].text = text;
    icon_slider->item_sources[index] = source;
    return 0;
}

int tinyui_icon_slider_add_icon(tinyui_obj_t *icon_slider_obj, const char *id, const char *text, struct tinyui_image_source *source)
{
    struct tinyui_icon_slider *icon_slider = tinyui_icon_slider_as_icon_slider(icon_slider_obj);
    if (icon_slider == 0) { return -1; }

    return tinyui_icon_slider_add_item_with_source((tinyui_obj_t *)icon_slider, id, text, source);
}

int tinyui_icon_slider_set_layout(tinyui_obj_t *icon_slider_obj, int width, int height, int icon_width, int icon_space, int columns, int rows, int pages)
{
    struct tinyui_icon_slider *icon_slider = tinyui_icon_slider_as_icon_slider(icon_slider_obj);
    if (icon_slider == 0) { return -1; }

    ldIconSlider_t *ld_icon_slider;
    int icon_max;

    if (icon_slider == 0 || width <= 0 || height <= 0 || icon_width <= 0
        || icon_space < 0 || columns <= 0 || rows <= 0 || pages <= 0
        || icon_slider->widget.ld_widget == 0
        || icon_slider->widget.kind != TINYUI_BACKEND_WIDGET_ICON_SLIDER
        || icon_slider->widget.list_item_count != 0) {
        return -1;
    }

    icon_max = columns * rows * pages;
    if (icon_max <= 0 || icon_max > TINYUI_ICON_SLIDER_NATIVE_MAX_ITEMS) {
        return -1;
    }

    ld_icon_slider = (ldIconSlider_t *)icon_slider->widget.ld_widget;
    tinyui_icon_slider_apply_native_layout(ld_icon_slider,
                                           width,
                                           height,
                                           icon_width,
                                           icon_space,
                                           columns,
                                           rows,
                                           pages);
    icon_slider->icon_width = icon_width;
    icon_slider->icon_space = icon_space;
    icon_slider->columns = columns;
    icon_slider->rows = rows;
    icon_slider->pages = pages;
    icon_slider->selected_index = -1;
    icon_slider->widget.value = -1;
    return 0;
}

int tinyui_icon_slider_set_selected_index(tinyui_obj_t *icon_slider_obj, int index)
{
    struct tinyui_icon_slider *icon_slider = tinyui_icon_slider_as_icon_slider(icon_slider_obj);
    if (icon_slider == 0) { return -1; }

    ldIconSlider_t *ld_icon_slider;

    if (icon_slider == 0 || index < 0 || index >= icon_slider->item_count
        || icon_slider->widget.ld_widget == 0) {
        return -1;
    }

    ld_icon_slider = (ldIconSlider_t *)icon_slider->widget.ld_widget;
    ld_icon_slider->selectIconOrPage = (uint8_t)index;
    ld_icon_slider->isWaitMove = true;
    icon_slider->widget.value = index;
    icon_slider->selected_index = index;
    return 0;
}

int tinyui_icon_slider_get_selected_index(const tinyui_obj_t *icon_slider_obj)
{
    const struct tinyui_icon_slider *icon_slider = tinyui_icon_slider_as_icon_slider_const(icon_slider_obj);
    if (icon_slider == 0) { return -1; }

    ldIconSlider_t *ld_icon_slider;

    if (icon_slider == 0 || icon_slider->widget.ld_widget == 0) {
        return -1;
    }

    ld_icon_slider = (ldIconSlider_t *)icon_slider->widget.ld_widget;
    return (int)ld_icon_slider->selectIconOrPage;
}

int tinyui_icon_slider_set_horizontal(tinyui_obj_t *icon_slider_obj, int horizontal)
{
    struct tinyui_icon_slider *icon_slider = tinyui_icon_slider_as_icon_slider(icon_slider_obj);
    if (icon_slider == 0) { return -1; }

    if (icon_slider == 0 || icon_slider->widget.ld_widget == 0) {
        return -1;
    }

    ldIconSliderSetHorizontalScroll((ldIconSlider_t *)icon_slider->widget.ld_widget, horizontal != 0);
    icon_slider->horizontal = horizontal != 0 ? 1 : 0;
    return 0;
}

int tinyui_icon_slider_set_horizontal_scroll(tinyui_obj_t *icon_slider_obj, int horizontal)
{
    struct tinyui_icon_slider *icon_slider = tinyui_icon_slider_as_icon_slider(icon_slider_obj);
    if (icon_slider == 0) { return -1; }

    return tinyui_icon_slider_set_horizontal((tinyui_obj_t *)icon_slider, horizontal);
}

int tinyui_icon_slider_get_horizontal(const tinyui_obj_t *icon_slider_obj, int *horizontal)
{
    const struct tinyui_icon_slider *icon_slider = tinyui_icon_slider_as_icon_slider_const(icon_slider_obj);
    if (icon_slider == 0) { return -1; }

    ldIconSlider_t *ld_icon_slider;

    if (icon_slider == 0 || horizontal == 0 || icon_slider->widget.ld_widget == 0) {
        return -1;
    }

    ld_icon_slider = (ldIconSlider_t *)icon_slider->widget.ld_widget;
    *horizontal = ld_icon_slider->isHorizontalScroll ? 1 : 0;
    return 0;
}

int tinyui_icon_slider_set_speed(tinyui_obj_t *icon_slider_obj, int speed)
{
    struct tinyui_icon_slider *icon_slider = tinyui_icon_slider_as_icon_slider(icon_slider_obj);
    if (icon_slider == 0) { return -1; }

    if (icon_slider == 0 || speed <= 0 || icon_slider->widget.ld_widget == 0) {
        return -1;
    }

    ldIconSliderSetSpeed((ldIconSlider_t *)icon_slider->widget.ld_widget, (uint8_t)speed);
    icon_slider->speed = speed;
    return 0;
}

void tinyui_icon_slider_set_on_selected(tinyui_obj_t *icon_slider_obj, void (*callback)(tinyui_obj_t *icon_slider,
                                                         int index,
                                                         void *user_data), void *user_data)
{
    struct tinyui_icon_slider *icon_slider = tinyui_icon_slider_as_icon_slider(icon_slider_obj);
    if (icon_slider == 0) { return; }

    if (icon_slider == 0) {
        return;
    }

    icon_slider->cb = (void (*)(struct tinyui_icon_slider *, int, void *))callback;
    icon_slider->user_data = user_data;
}
