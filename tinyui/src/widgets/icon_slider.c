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
#include "core/widget.h"
#include "../core/runtime_bridge.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldIconSlider.h"
#include "../../../src/misc/ldMsg.h"


extern const arm_2d_a1_font_t ARM_2D_FONT_6x8;
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


struct tinyui_icon_slider_create_ctx {
    int width;
    int height;
    int icon_width;
    int icon_space;
    int columns;
    int rows;
    int pages;
};

static void *tinyui_icon_slider_ld_init(void *ctx,
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
                             (arm_2d_font_t *)&ARM_2D_FONT_6x8);
}


static void tinyui_icon_slider_rollback(struct tinyui_icon_slider *icon_slider)
{
    if (icon_slider == 0) {
        return;
    }
    if (icon_slider->widget.ld_widget != 0) {
        tinyui_widget_destroy_common(&icon_slider->widget);
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

    w = tinyui_widget_from_ld_scene(scene, msg.ptSender);
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

    if (tinyui_widget_claim_backend_focus(w) != 0) {
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

static int tinyui_icon_slider_props_are_valid(const struct tinyui_icon_slider_props *props)
{
    return props != 0 &&
           props->id != 0 &&
           props->width >= 0 &&
           props->height >= 0 &&
           props->icon_width >= 0 &&
           props->icon_space >= 0 &&
           props->columns >= 0 &&
           props->rows >= 0 &&
           props->pages >= 0;
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

    if (parent->ld_widget == 0 || parent->owner == 0 || parent->owner->ld_scene == 0) {
        return 0;
    }

    create_ctx.width = width;
    create_ctx.height = height;
    create_ctx.icon_width = icon_width;
    create_ctx.icon_space = icon_space;
    create_ctx.columns = columns;
    create_ctx.rows = rows;
    create_ctx.pages = pages;

    icon_slider = (struct tinyui_icon_slider *)tinyui_widget_create_leaf(parent,
                                                                         TINYUI_BACKEND_WIDGET_ICON_SLIDER,
                                                                         tinyui_icon_slider_ld_init,
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

struct tinyui_icon_slider *tinyui_icon_slider_create(struct tinyui_widget *parent, const char *id)
{
    return tinyui_icon_slider_create_with_backend_config(parent, id, 220, 86, 48, 4, 4, 1, 2);
}

struct tinyui_icon_slider *tinyui_icon_slider_init(struct tinyui_widget *parent, const char *id)
{
    return tinyui_icon_slider_create(parent, id);
}

struct tinyui_icon_slider *tinyui_icon_slider_create_with_props(
    struct tinyui_widget *parent,
    const struct tinyui_icon_slider_props *props
)
{
    struct tinyui_icon_slider *icon_slider;

    if (!tinyui_icon_slider_props_are_valid(props)) {
        return 0;
    }

    icon_slider = tinyui_icon_slider_create_with_backend_config(parent,
                                                                props->id,
                                                                props->width > 0 ? props->width : 220,
                                                                props->height > 0 ? props->height : 86,
                                                                props->icon_width > 0 ? props->icon_width : 48,
                                                                props->icon_space,
                                                                props->columns > 0 ? props->columns : 4,
                                                                props->rows > 0 ? props->rows : 1,
                                                                props->pages > 0 ? props->pages : 2);
    if (icon_slider == 0) {
        return 0;
    }

    if (tinyui_widget_set_user_data(&icon_slider->widget, props->user_data) != 0 ||
        (props->style_class != 0 &&
         tinyui_widget_set_style_class(&icon_slider->widget, props->style_class) != 0) ||
        tinyui_icon_slider_set_horizontal(icon_slider, props->horizontal != 0) != 0) {
        tinyui_icon_slider_rollback(icon_slider);
        return 0;
    }
    return icon_slider;
}

int tinyui_icon_slider_add_item(struct tinyui_icon_slider *icon_slider, const char *id, const char *text)
{
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

int tinyui_icon_slider_add_item_with_source(struct tinyui_icon_slider *icon_slider,
                                            const char *id,
                                            const char *text,
                                            struct tinyui_image_source *source)
{
    ldIconSlider_t *ld_icon_slider;
    int index;

    if (icon_slider == 0
        || id == 0
        || text == 0
        || source == 0
        || source->img_tile == 0
        || source->mask_tile == 0
        || icon_slider->widget.ld_widget == 0
        || icon_slider->widget.list_item_count >= TINYUI_ICON_SLIDER_NATIVE_MAX_ITEMS) {
        return -1;
    }

    ld_icon_slider = (ldIconSlider_t *)icon_slider->widget.ld_widget;

    index = icon_slider->widget.list_item_count;
    ldIconSliderAddIcon(ld_icon_slider,
                        source->img_tile,
                        source->mask_tile,
                        (const uint8_t *)text);
    icon_slider->widget.list_item_count++;

    index = icon_slider->item_count++;
    icon_slider->items[index].id = id;
    icon_slider->items[index].text = text;
    icon_slider->item_sources[index] = source;
    return 0;
}

int tinyui_icon_slider_add_icon(struct tinyui_icon_slider *icon_slider,
                                const char *id,
                                const char *text,
                                struct tinyui_image_source *source)
{
    return tinyui_icon_slider_add_item_with_source(icon_slider, id, text, source);
}

int tinyui_icon_slider_set_selected_index(struct tinyui_icon_slider *icon_slider, int index)
{
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

int tinyui_icon_slider_get_selected_index(const struct tinyui_icon_slider *icon_slider)
{
    ldIconSlider_t *ld_icon_slider;

    if (icon_slider == 0 || icon_slider->widget.ld_widget == 0) {
        return -1;
    }

    ld_icon_slider = (ldIconSlider_t *)icon_slider->widget.ld_widget;
    return (int)ld_icon_slider->selectIconOrPage;
}

int tinyui_icon_slider_set_horizontal(struct tinyui_icon_slider *icon_slider, int horizontal)
{
    if (icon_slider == 0 || icon_slider->widget.ld_widget == 0) {
        return -1;
    }

    ldIconSliderSetHorizontalScroll((ldIconSlider_t *)icon_slider->widget.ld_widget, horizontal != 0);
    icon_slider->horizontal = horizontal != 0 ? 1 : 0;
    return 0;
}

int tinyui_icon_slider_set_horizontal_scroll(struct tinyui_icon_slider *icon_slider, int horizontal)
{
    return tinyui_icon_slider_set_horizontal(icon_slider, horizontal);
}

int tinyui_icon_slider_get_horizontal(const struct tinyui_icon_slider *icon_slider, int *horizontal)
{
    ldIconSlider_t *ld_icon_slider;

    if (icon_slider == 0 || horizontal == 0 || icon_slider->widget.ld_widget == 0) {
        return -1;
    }

    ld_icon_slider = (ldIconSlider_t *)icon_slider->widget.ld_widget;
    *horizontal = ld_icon_slider->isHorizontalScroll ? 1 : 0;
    return 0;
}

int tinyui_icon_slider_set_speed(struct tinyui_icon_slider *icon_slider, int speed)
{
    if (icon_slider == 0 || speed <= 0 || icon_slider->widget.ld_widget == 0) {
        return -1;
    }

    ldIconSliderSetSpeed((ldIconSlider_t *)icon_slider->widget.ld_widget, (uint8_t)speed);
    icon_slider->speed = speed;
    return 0;
}

void tinyui_icon_slider_set_on_selected(struct tinyui_icon_slider *icon_slider,
                                        void (*callback)(struct tinyui_icon_slider *icon_slider,
                                                         int index,
                                                         void *user_data),
                                        void *user_data)
{
    if (icon_slider == 0) {
        return;
    }

    icon_slider->cb = callback;
    icon_slider->user_data = user_data;
}
