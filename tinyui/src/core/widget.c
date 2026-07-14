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
#include "core/widget.h"
#include "arm_2d.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldButton.h"
#include "../../../src/gui/ldCheckBox.h"
#include "../../../src/gui/ldComboBox.h"
#include "../../../src/gui/ldDateTime.h"
#include "../../../src/gui/ldGui.h"
#include "../../../src/gui/ldIconSlider.h"
#include "../../../src/gui/ldKeyboard.h"
#include "../../../src/gui/ldLabel.h"
#include "../../../src/gui/ldLineEdit.h"
#include "../../../src/gui/ldList.h"
#include "../../../src/gui/ldMessageBox.h"
#include "../../../src/gui/ldScrollSelecter.h"
#include "../../../src/gui/ldTable.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

static struct tinyui_widget *tinyui_obj_widget(tinyui_obj_t *obj)
{
    return (struct tinyui_widget *)(void *)obj;
}

static const struct tinyui_widget *tinyui_obj_const_widget(const tinyui_obj_t *obj)
{
    return (const struct tinyui_widget *)(const void *)obj;
}

tinyui_result_t tinyui_obj_delete(tinyui_obj_t *obj)
{
    struct tinyui_widget *widget = tinyui_obj_widget(obj);

    if (widget == 0) {
        return TINYUI_ERROR_INVALID_OBJECT;
    }
    return tinyui_widget_destroy(widget) == 0 ? TINYUI_OK : TINYUI_ERROR_INVALID_STATE;
}

tinyui_result_t tinyui_obj_get_id(const tinyui_obj_t *obj, uint16_t *id)
{
    int name_id;

    if (obj == 0) {
        return TINYUI_ERROR_INVALID_OBJECT;
    }
    if (id == 0) {
        return TINYUI_ERROR_INVALID_ARG;
    }

    name_id = tinyui_widget_get_name_id(tinyui_obj_const_widget(obj));
    if (name_id < 0 || name_id > UINT16_MAX) {
        return TINYUI_ERROR_INVALID_OBJECT;
    }
    *id = (uint16_t)name_id;
    return TINYUI_OK;
}

tinyui_obj_t *tinyui_obj_find_by_id(tinyui_obj_t *root, uint16_t id)
{
    if (root == 0 || id == 0) {
        return 0;
    }
    return (tinyui_obj_t *)tinyui_widget_find_by_name_id(
        tinyui_obj_const_widget(root),
        (int)id);
}

tinyui_obj_t *tinyui_obj_get_parent(const tinyui_obj_t *obj)
{
    return (tinyui_obj_t *)tinyui_widget_get_parent(tinyui_obj_const_widget(obj));
}

tinyui_obj_t *tinyui_obj_get_first_child(const tinyui_obj_t *obj)
{
    return (tinyui_obj_t *)tinyui_widget_get_first_child(tinyui_obj_const_widget(obj));
}

tinyui_obj_t *tinyui_obj_get_next_sibling(const tinyui_obj_t *obj)
{
    return (tinyui_obj_t *)tinyui_widget_get_next_sibling(tinyui_obj_const_widget(obj));
}

tinyui_obj_t *tinyui_obj_get_root(const tinyui_obj_t *obj)
{
    return (tinyui_obj_t *)tinyui_widget_get_root(tinyui_obj_const_widget(obj));
}

tinyui_result_t tinyui_obj_get_child_count(const tinyui_obj_t *obj, uint16_t *count)
{
    int child_count;

    if (obj == 0) {
        return TINYUI_ERROR_INVALID_OBJECT;
    }
    if (count == 0) {
        return TINYUI_ERROR_INVALID_ARG;
    }

    child_count = tinyui_widget_get_child_count(tinyui_obj_const_widget(obj));
    if (child_count < 0 || child_count > UINT16_MAX) {
        return TINYUI_ERROR_INVALID_OBJECT;
    }
    *count = (uint16_t)child_count;
    return TINYUI_OK;
}

typedef struct ldLabel_t ldLabel_t;
typedef struct ldText_t ldText_t;
typedef struct ldQRCode_t ldQRCode_t;
typedef struct ldButton_t ldButton_t;
typedef struct ldCheckBox_t ldCheckBox_t;
typedef struct ldSwitch_t ldSwitch_t;
/* ldBase_t typedef comes from ldBase.h (included above) */
typedef struct arm_2d_font_t arm_2d_font_t;
typedef struct arm_2d_control_node_t arm_2d_control_node_t;
typedef arm_2d_location_t tinyui_ld_location_t;
typedef arm_2d_region_t tinyui_ld_region_t;
typedef arm_2d_align_t tinyui_ld_align_t;
#define tinyui_ld_align_left ARM_2D_ALIGN_LEFT
#define tinyui_ld_align_right ARM_2D_ALIGN_RIGHT
#define tinyui_ld_align_center ARM_2D_ALIGN_CENTRE
#define tinyui_ld_align_top ARM_2D_ALIGN_TOP
#define tinyui_ld_align_bottom ARM_2D_ALIGN_BOTTOM
typedef enum {
    tinyui_ld_nav_up,
    tinyui_ld_nav_down,
    tinyui_ld_nav_left,
    tinyui_ld_nav_right,
    tinyui_ld_nav_enter,
    tinyui_ld_nav_back,
} tinyui_ld_nav_dir_t;

/* All ldBase_t API declarations come from ldBase.h (included above).
 * Additional widget-specific setters not in ldBase.h: */
void ldSwitchSetDisabled(ldSwitch_t *ptWidget, bool isDisabled);
void ldLabelSetText(ldLabel_t *ptWidget, uint8_t *pStr);
void ldTextSetText(ldText_t *ptWidget, uint8_t *pStr);
void ldQRCodeSetText(ldQRCode_t *ptWidget, uint8_t *pStr);
void ldButtonSetText(ldButton_t *ptWidget, uint8_t *pStr);
void ldCheckBoxSetText(ldCheckBox_t *ptWidget, arm_2d_font_t *ptFont, uint8_t *pStr);

static void tinyui_scroll_selecter_sync_native_size(ldScrollSelecter_t *scroll_selecter)
{
    int16_t height;

    if (scroll_selecter == 0 || scroll_selecter->ptFont == 0) {
        return;
    }

    height = scroll_selecter->use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iHeight;
    height = (int16_t)(height / 3 * 3);
    scroll_selecter->use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iHeight = height;
    scroll_selecter->use_as__ldBase_t.tTempRegion =
        scroll_selecter->use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion;

    scroll_selecter->itemSpace =
        height - scroll_selecter->ptFont->tCharSize.iHeight * 3;
    if (scroll_selecter->itemSpace > 0) {
        scroll_selecter->itemSpace = scroll_selecter->itemSpace >> 1;
        scroll_selecter->isEdit = true;
    } else {
        scroll_selecter->isEdit = false;
        scroll_selecter->itemSpace = height - scroll_selecter->ptFont->tCharSize.iHeight;
        if (scroll_selecter->itemSpace > 0) {
            scroll_selecter->itemSpace = scroll_selecter->itemSpace >> 1;
        } else {
            scroll_selecter->itemSpace = 0;
        }
    }
    scroll_selecter->scrollOffset =
        (int16_t)(-scroll_selecter->itemSelect *
                  (scroll_selecter->itemSpace + scroll_selecter->ptFont->tCharSize.iHeight));
}

__attribute__((weak)) void tinyui_test_on_prepare_native_depose(
    const struct tinyui_widget *widget)
{
    (void)widget;
}

static void tinyui_clear_static_font_ref(arm_2d_font_t **font)
{
    if (font != 0 && tinyui_ld_font_is_static(*font)) {
        *font = 0;
    }
}

void tinyui_widget_prepare_native_depose(struct tinyui_widget *widget)
{
    ldTable_t *table;
    unsigned int count;
    unsigned int i;

    if (widget == 0 || widget->ld_widget == 0) {
        return;
    }

    tinyui_test_on_prepare_native_depose(widget);

    switch (widget->kind) {
    case TINYUI_BACKEND_WIDGET_LABEL:
        tinyui_clear_static_font_ref(&((ldLabel_t *)widget->ld_widget)->ptFont);
        break;
    case TINYUI_BACKEND_WIDGET_BUTTON:
        tinyui_clear_static_font_ref(&((ldButton_t *)widget->ld_widget)->ptFont);
        break;
    case TINYUI_BACKEND_WIDGET_CHECKBOX:
        tinyui_clear_static_font_ref(&((ldCheckBox_t *)widget->ld_widget)->ptFont);
        break;
    case TINYUI_BACKEND_WIDGET_DATE_TIME:
        tinyui_clear_static_font_ref(&((ldDateTime_t *)widget->ld_widget)->ptFont);
        break;
    case TINYUI_BACKEND_WIDGET_LIST:
        tinyui_clear_static_font_ref(&((ldList_t *)widget->ld_widget)->ptFont);
        break;
    case TINYUI_BACKEND_WIDGET_MESSAGE_BOX:
        tinyui_clear_static_font_ref(&((ldMessageBox_t *)widget->ld_widget)->ptFont);
        break;
    case TINYUI_BACKEND_WIDGET_KEYBOARD:
        tinyui_clear_static_font_ref(&((ldKeyboard_t *)widget->ld_widget)->ptFont);
        break;
    case TINYUI_BACKEND_WIDGET_COMBO_BOX:
        tinyui_clear_static_font_ref(&((ldComboBox_t *)widget->ld_widget)->ptFont);
        break;
    case TINYUI_BACKEND_WIDGET_SCROLL_SELECTER:
        tinyui_clear_static_font_ref(&((ldScrollSelecter_t *)widget->ld_widget)->ptFont);
        break;
    case TINYUI_BACKEND_WIDGET_TEXT:
        if (((ldBase_t *)widget->ld_widget)->widgetType == widgetTypeLineEdit) {
            tinyui_clear_static_font_ref(&((ldLineEdit_t *)widget->ld_widget)->ptFont);
        }
        break;
    case TINYUI_BACKEND_WIDGET_ICON_SLIDER:
        tinyui_clear_static_font_ref(&((ldIconSlider_t *)widget->ld_widget)->ptFont);
        break;
    case TINYUI_BACKEND_WIDGET_TABLE:
        table = (ldTable_t *)widget->ld_widget;
        if (table->ptItemInfo == 0) {
            break;
        }
        count = (unsigned int)table->rowCount * (unsigned int)table->columnCount;
        for (i = 0; i < count; ++i) {
            tinyui_clear_static_font_ref(&table->ptItemInfo[i].ptFont);
        }
        break;
    default:
        break;
    }
}

static int tinyui_widget_is_valid(struct tinyui_widget *widget)
{
    return widget != 0;
}

static ldBase_t *tinyui_widget_get_ld_base(struct tinyui_widget *widget)
{
    if (!tinyui_widget_is_valid(widget) || widget->ld_widget == 0) {
        return 0;
    }

    return (ldBase_t *)widget->ld_widget;
}

/* C1: tinyui_widget_get_backend removed — backend state is now folded into
 * struct tinyui_widget directly. Callers should access widget->kind,
 * widget->ld_widget, widget->owner, etc. directly. */

static int tinyui_widget_expected_native_type(enum tinyui_backend_widget_kind kind)
{
    switch (kind) {
    case TINYUI_BACKEND_WIDGET_BACKGROUND:
        return 0;
    case TINYUI_BACKEND_WIDGET_WINDOW:
        return 1;
    case TINYUI_BACKEND_WIDGET_LABEL:
        return 19;
    case TINYUI_BACKEND_WIDGET_BUTTON:
        return 2;
    case TINYUI_BACKEND_WIDGET_CHECKBOX:
        return 7;
    case TINYUI_BACKEND_WIDGET_SWITCH:
        return 9;
    case TINYUI_BACKEND_WIDGET_SLIDER:
        return 8;
    case TINYUI_BACKEND_WIDGET_TEXT:
        return 4;
    default:
        return -1;
    }
}

static int tinyui_widget_validate_native_binding(const struct tinyui_widget *widget,
                                                 const ldBase_t *ld_base)
{
    int expected_type;

    if (widget == 0 || ld_base == 0) {
        return 0;
    }

    expected_type = tinyui_widget_expected_native_type(widget->kind);
    if (expected_type < 0) {
        return 1;
    }

    return ldBaseGetWidgetType((ldBase_t *)ld_base) == expected_type;
}

int tinyui_widget_is_kind(const struct tinyui_widget *widget,
                          enum tinyui_backend_widget_kind kind)
{
    if (widget == 0) {
        return 0;
    }

    return widget->kind == kind;
}

int tinyui_native_nav_dir_to_ld(enum tinyui_native_nav_dir dir)
{
    switch (dir) {
    case TINYUI_NATIVE_NAV_LEFT:
        return tinyui_ld_nav_left;
    case TINYUI_NATIVE_NAV_RIGHT:
        return tinyui_ld_nav_right;
    case TINYUI_NATIVE_NAV_UP:
        return tinyui_ld_nav_up;
    case TINYUI_NATIVE_NAV_DOWN:
        return tinyui_ld_nav_down;
    case TINYUI_NATIVE_NAV_ENTER:
        return tinyui_ld_nav_enter;
    case TINYUI_NATIVE_NAV_BACK:
        return tinyui_ld_nav_back;
    default:
        return tinyui_ld_nav_down;
    }
}

int tinyui_widget_claim_backend_focus(struct tinyui_widget *widget)
{
    if (widget == 0) {
        return -1;
    }

    return tinyui_widget_claim_focus(widget);
}

int tinyui_widget_release_backend_focus(struct tinyui_widget *widget)
{
    if (widget == 0) {
        return -1;
    }

    return tinyui_widget_release_focus(widget);
}

int tinyui_widget_update_value(struct tinyui_widget *widget,
                               int value,
                               tinyui_value_changed_cb cb,
                               void *user_data)
{
    if (widget == 0) {
        return -1;
    }

    widget->value = value;
    tinyui_widget_sync_ld_value(widget, value);
    (void)cb;
    (void)user_data;
    return 0;
}

struct tinyui_app *tinyui_widget_owner_app(const struct tinyui_widget *widget)
{
    if (widget == 0) {
        return 0;
    }

    return widget->owner;
}

int tinyui_widget_has_ld_binding(const struct tinyui_widget *widget)
{
    return widget != 0 && widget->ld_widget != 0;
}

struct tinyui_widget *tinyui_widget_backend_parent(const struct tinyui_widget *widget)
{
    /* C1: parent traversal via ld tree using pInfo */
    ldBase_t *ld_base;
    ldBase_t *ld_parent;

    if (widget == 0 || widget->ld_widget == 0) {
        return 0;
    }

    ld_base = (ldBase_t *)widget->ld_widget;
    ld_parent = ldBaseGetParent(ld_base);
    if (ld_parent == 0) {
        return 0;
    }

    return tinyui_app_lookup_host(widget->owner, ld_parent->nameId);
}

static int tinyui_align_to_ld_horizontal(enum tinyui_align align)
{
    switch (align) {
    case TINYUI_ALIGN_START:
        return tinyui_ld_align_left;
    case TINYUI_ALIGN_END:
        return tinyui_ld_align_right;
    case TINYUI_ALIGN_CENTER:
    case TINYUI_ALIGN_STRETCH:
    case TINYUI_ALIGN_SPACE_EVENLY:
    case TINYUI_ALIGN_SPACE_AROUND:
    case TINYUI_ALIGN_SPACE_BETWEEN:
    default:
        return tinyui_ld_align_center;
    }
}

static int tinyui_align_to_ld_vertical(enum tinyui_align align)
{
    switch (align) {
    case TINYUI_ALIGN_START:
        return tinyui_ld_align_top;
    case TINYUI_ALIGN_END:
        return tinyui_ld_align_bottom;
    case TINYUI_ALIGN_CENTER:
    case TINYUI_ALIGN_STRETCH:
    case TINYUI_ALIGN_SPACE_EVENLY:
    case TINYUI_ALIGN_SPACE_AROUND:
    case TINYUI_ALIGN_SPACE_BETWEEN:
    default:
        return tinyui_ld_align_center;
    }
}

static tinyui_ld_region_t tinyui_rect_to_ld_region(struct tinyui_rect rect)
{
    tinyui_ld_region_t region;

    region.tLocation.iX = (int16_t)rect.x;
    region.tLocation.iY = (int16_t)rect.y;
    region.tSize.iWidth = (int16_t)rect.width;
    region.tSize.iHeight = (int16_t)rect.height;
    return region;
}

static arm_2d_location_t tinyui_ld_location_to_arm(tinyui_ld_location_t location)
{
    arm_2d_location_t arm_location;

    arm_location.iX = location.iX;
    arm_location.iY = location.iY;
    return arm_location;
}

static tinyui_ld_location_t tinyui_ld_location_from_arm(arm_2d_location_t location)
{
    tinyui_ld_location_t tinyui_location;

    tinyui_location.iX = location.iX;
    tinyui_location.iY = location.iY;
    return tinyui_location;
}

static arm_2d_region_t tinyui_ld_region_to_arm(tinyui_ld_region_t region)
{
    arm_2d_region_t arm_region;

    arm_region.tLocation.iX = region.tLocation.iX;
    arm_region.tLocation.iY = region.tLocation.iY;
    arm_region.tSize.iWidth = region.tSize.iWidth;
    arm_region.tSize.iHeight = region.tSize.iHeight;
    return arm_region;
}

static tinyui_ld_region_t tinyui_ld_region_from_arm(arm_2d_region_t region)
{
    tinyui_ld_region_t tinyui_region;

    tinyui_region.tLocation.iX = region.tLocation.iX;
    tinyui_region.tLocation.iY = region.tLocation.iY;
    tinyui_region.tSize.iWidth = region.tSize.iWidth;
    tinyui_region.tSize.iHeight = region.tSize.iHeight;
    return tinyui_region;
}

static struct tinyui_rect tinyui_rect_from_ld_region(tinyui_ld_region_t region)
{
    struct tinyui_rect rect;

    rect.x = region.tLocation.iX;
    rect.y = region.tLocation.iY;
    rect.width = region.tSize.iWidth;
    rect.height = region.tSize.iHeight;
    return rect;
}

static enum tinyui_widget_type tinyui_widget_type_from_backend_kind(enum tinyui_backend_widget_kind kind)
{
    switch (kind) {
    case TINYUI_BACKEND_WIDGET_BACKGROUND:
        return TINYUI_WIDGET_TYPE_BACKGROUND;
    case TINYUI_BACKEND_WIDGET_WINDOW:
        return TINYUI_WIDGET_TYPE_WINDOW;
    case TINYUI_BACKEND_WIDGET_LABEL:
        return TINYUI_WIDGET_TYPE_LABEL;
    case TINYUI_BACKEND_WIDGET_BUTTON:
        return TINYUI_WIDGET_TYPE_BUTTON;
    case TINYUI_BACKEND_WIDGET_CHECKBOX:
        return TINYUI_WIDGET_TYPE_CHECKBOX;
    case TINYUI_BACKEND_WIDGET_SWITCH:
        return TINYUI_WIDGET_TYPE_SWITCH;
    case TINYUI_BACKEND_WIDGET_SLIDER:
        return TINYUI_WIDGET_TYPE_SLIDER;
    case TINYUI_BACKEND_WIDGET_ARC:
        return TINYUI_WIDGET_TYPE_ARC;
    case TINYUI_BACKEND_WIDGET_GAUGE:
        return TINYUI_WIDGET_TYPE_GAUGE;
    case TINYUI_BACKEND_WIDGET_ICON_SLIDER:
        return TINYUI_WIDGET_TYPE_ICON_SLIDER;
    case TINYUI_BACKEND_WIDGET_RADIAL_MENU:
        return TINYUI_WIDGET_TYPE_RADIAL_MENU;
    case TINYUI_BACKEND_WIDGET_PROGRESS_BAR:
        return TINYUI_WIDGET_TYPE_PROGRESS_BAR;
    case TINYUI_BACKEND_WIDGET_QRCODE:
        return TINYUI_WIDGET_TYPE_QRCODE;
    case TINYUI_BACKEND_WIDGET_PROGRESS_WHEEL:
        return TINYUI_WIDGET_TYPE_PROGRESS_WHEEL;
    case TINYUI_BACKEND_WIDGET_ANIMATION:
        return TINYUI_WIDGET_TYPE_ANIMATION;
    case TINYUI_BACKEND_WIDGET_LIST:
        return TINYUI_WIDGET_TYPE_LIST;
    case TINYUI_BACKEND_WIDGET_MESSAGE_BOX:
        return TINYUI_WIDGET_TYPE_MESSAGE_BOX;
    case TINYUI_BACKEND_WIDGET_DATE_TIME:
        return TINYUI_WIDGET_TYPE_DATE_TIME;
    case TINYUI_BACKEND_WIDGET_CLOCK:
        return TINYUI_WIDGET_TYPE_CLOCK;
    case TINYUI_BACKEND_WIDGET_TEXT:
        return TINYUI_WIDGET_TYPE_TEXT;
    case TINYUI_BACKEND_WIDGET_KEYBOARD:
        return TINYUI_WIDGET_TYPE_KEYBOARD;
    case TINYUI_BACKEND_WIDGET_COMBO_BOX:
        return TINYUI_WIDGET_TYPE_COMBO_BOX;
    case TINYUI_BACKEND_WIDGET_SCROLL_SELECTER:
        return TINYUI_WIDGET_TYPE_SCROLL_SELECTER;
    case TINYUI_BACKEND_WIDGET_TABLE:
        return TINYUI_WIDGET_TYPE_TABLE;
    case TINYUI_BACKEND_WIDGET_GRAPH:
        return TINYUI_WIDGET_TYPE_GRAPH;
    case TINYUI_BACKEND_WIDGET_IMAGE:
        return TINYUI_WIDGET_TYPE_IMAGE;
    case TINYUI_BACKEND_WIDGET_CALENDAR:
        return TINYUI_WIDGET_TYPE_CALENDAR;
    case TINYUI_BACKEND_WIDGET_CANVAS:
        return TINYUI_WIDGET_TYPE_CANVAS;
    default:
        return TINYUI_WIDGET_TYPE_UNKNOWN;
    }
}

/**
 * @brief Set pos of widget
 *
 * @param[in] widget Widget instance
 * @param[in] x X coordinate
 * @param[in] y Y coordinate
 * @return 0 on success, -1 on failure
 */

int tinyui_widget_set_pos(struct tinyui_widget *widget, int x, int y)
{
    ldBase_t *ld_base;

    if (!tinyui_widget_is_valid(widget)) {
        return -1;
    }

    widget->x = x;
    widget->y = y;
    ld_base = tinyui_widget_get_ld_base(widget);
    if (ld_base != 0) {
        ldBaseMove(ld_base, (int16_t)x, (int16_t)y);
    }
    return 0;
}

/**
 * @brief Set size of widget
 *
 * @param[in] widget Widget instance
 * @param[in] width Width in pixels
 * @param[in] height Height in pixels
 * @return 0 on success, -1 on failure
 */

int tinyui_widget_set_size(struct tinyui_widget *widget, int width, int height)
{
    ldBase_t *ld_base;

    if (!tinyui_widget_is_valid(widget) || width < 0 || height < 0) {
        return -1;
    }

    widget->width = width;
    widget->height = height;
    ld_base = tinyui_widget_get_ld_base(widget);
    if (ld_base != 0) {
        ldBaseSetWidth(ld_base, (int16_t)width);
        ldBaseSetHeight(ld_base, (int16_t)height);
        if (widget->kind == TINYUI_BACKEND_WIDGET_COMBO_BOX) {
            ((ldComboBox_t *)widget->ld_widget)->itemHeight = (int16_t)height;
        } else if (widget->kind == TINYUI_BACKEND_WIDGET_SCROLL_SELECTER) {
            tinyui_scroll_selecter_sync_native_size((ldScrollSelecter_t *)widget->ld_widget);
        }
    }
    return 0;
}

/**
 * @brief Set text of widget
 *
 * @param[in] widget Widget instance
 * @param[in] text Text widget instance
 * @return 0 on success, -1 on failure
 */

int tinyui_widget_set_text(struct tinyui_widget *widget, const char *text)
{
    if (!tinyui_widget_is_valid(widget) || text == 0) {
        return -1;
    }

    widget->text = text;
    return 0;
}

int tinyui_widget_set_backend_text(struct tinyui_widget *widget, const char *text)
{
    if (widget == NULL || text == NULL) {
        return -1;
    }

    widget->text = text;
    if (widget->ld_widget != NULL) {
        switch (widget->kind) {
        case TINYUI_BACKEND_WIDGET_LABEL:
            ldLabelSetText((ldLabel_t *)widget->ld_widget, (uint8_t *)text);
            break;
        case TINYUI_BACKEND_WIDGET_TEXT:
            ldTextSetText((ldText_t *)widget->ld_widget, (uint8_t *)text);
            break;
        case TINYUI_BACKEND_WIDGET_QRCODE:
            ldQRCodeSetText((ldQRCode_t *)widget->ld_widget, (uint8_t *)text);
            break;
        case TINYUI_BACKEND_WIDGET_BUTTON:
            ldButtonSetText((ldButton_t *)widget->ld_widget, (uint8_t *)text);
            break;
        case TINYUI_BACKEND_WIDGET_CHECKBOX:
        {
            ldCheckBox_t *ld_checkbox = (ldCheckBox_t *)widget->ld_widget;
            arm_2d_font_t *font = (arm_2d_font_t *)widget->font;

            if (font == NULL) {
                font = ld_checkbox->ptFont;
            }
            ldCheckBoxSetText(ld_checkbox, font, (uint8_t *)text);
            break;
        }
        default:
            break;
        }
    }

    return 0;
}

void tinyui_widget_emit_value_changed(tinyui_value_changed_cb cb,
                                      struct tinyui_widget *widget,
                                      int value,
                                      void *user_data)
{
    if (cb != 0) {
        cb(widget, value, user_data);
    }
}

void tinyui_widget_emit_event(tinyui_event_cb cb,
                              struct tinyui_widget *widget,
                              void *user_data)
{
    if (cb != 0) {
        cb(widget, user_data);
    }
}

void tinyui_widget_emit_clicked(tinyui_event_cb cb,
                                struct tinyui_widget *widget,
                                void *user_data)
{
    if (cb != 0) {
        cb(widget, user_data);
    }
}

/**
 * @brief Set style class of widget
 *
 * @param[in] widget Widget instance
 * @param[in] style_class style class
 * @return 0 on success, -1 on failure
 */

int tinyui_widget_set_style_class(struct tinyui_widget *widget, const char *style_class)
{
    if (!tinyui_widget_is_valid(widget)) {
        return -1;
    }

    widget->style_class = style_class;
    return 0;
}

/**
 * @brief Set user data of widget
 *
 * @param[in] widget Widget instance
 * @param[in] user_data User data pointer
 * @return 0 on success, -1 on failure
 */

int tinyui_widget_set_user_data(struct tinyui_widget *widget, void *user_data)
{
    if (!tinyui_widget_is_valid(widget)) {
        return -1;
    }

    widget->user_data = user_data;
    return 0;
}

/**
 * @brief Set bg color of widget
 *
 * @param[in] widget Widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int tinyui_widget_set_bg_color(struct tinyui_widget *widget, unsigned int rgb)
{
    if (!tinyui_widget_is_valid(widget)) {
        return -1;
    }

    widget->bg_color = rgb;
    return 0;
}

/**
 * @brief Set text color of widget
 *
 * @param[in] widget Widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int tinyui_widget_set_text_color(struct tinyui_widget *widget, unsigned int rgb)
{
    if (!tinyui_widget_is_valid(widget)) {
        return -1;
    }

    widget->text_color = rgb;
    return 0;
}

/**
 * @brief Set border color of widget
 *
 * @param[in] widget Widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int tinyui_widget_set_border_color(struct tinyui_widget *widget, unsigned int rgb)
{
    if (!tinyui_widget_is_valid(widget)) {
        return -1;
    }

    widget->border_color = rgb;
    return 0;
}

/**
 * @brief Set radius of widget
 *
 * @param[in] widget Widget instance
 * @param[in] radius Radius
 * @return 0 on success, -1 on failure
 */

int tinyui_widget_set_radius(struct tinyui_widget *widget, int radius)
{
    if (!tinyui_widget_is_valid(widget) || radius < 0) {
        return -1;
    }

    widget->radius = radius;
    return 0;
}

/**
 * @brief Set padding of widget
 *
 * @param[in] widget Widget instance
 * @param[in] padding padding
 * @return 0 on success, -1 on failure
 */

int tinyui_widget_set_padding(struct tinyui_widget *widget, int padding)
{
    if (!tinyui_widget_is_valid(widget) || padding < 0) {
        return -1;
    }

    if (widget->kind == TINYUI_BACKEND_WIDGET_WINDOW
        || widget->kind == TINYUI_BACKEND_WIDGET_BACKGROUND) {
        struct tinyui_window *window = (struct tinyui_window *)widget;

        if (tinyui_window_apply_uniform_padding(window, padding) != 0) {
            return -1;
        }
    }

    widget->padding = padding;
    return 0;
}

/**
 * @brief Set center of widget
 *
 * @param[in] widget Widget instance
 * @return 0 on success, -1 on failure
 */

int tinyui_widget_set_center(struct tinyui_widget *widget)
{
    ldBase_t *ld_base;

    if (!tinyui_widget_is_valid(widget)) {
        return -1;
    }

    ld_base = tinyui_widget_get_ld_base(widget);
    if (ld_base != 0) {
        ldBaseSetCenter(ld_base);
    }
    return 0;
}

/**
 * @brief Set visible of widget
 *
 * @param[in] widget Widget instance
 * @param[in] visible Visibility state
 * @return 0 on success, -1 on failure
 */

int tinyui_widget_set_visible(struct tinyui_widget *widget, int visible)
{
    ldBase_t *ld_base;

    if (!tinyui_widget_is_valid(widget)) {
        return -1;
    }

    widget->visible = visible != 0;
    if (widget->visible == 0) {
        (void)tinyui_widget_release_focus(widget);
    }
    ld_base = tinyui_widget_get_ld_base(widget);
    if (ld_base != 0) {
        ldBaseSetHidden(ld_base, widget->visible == 0);
    }
    return 0;
}

int tinyui_widget_is_hidden(struct tinyui_widget *widget)
{
    if (!tinyui_widget_is_valid(widget)) {
        return -1;
    }
    return !widget->visible;
}

/**
 * @brief Set opacity of widget
 *
 * @param[in] widget Widget instance
 * @param[in] opacity Opacity (0-255)
 * @return 0 on success, -1 on failure
 */

int tinyui_widget_set_opacity(struct tinyui_widget *widget, int opacity)
{
    ldBase_t *ld_base;

    if (!tinyui_widget_is_valid(widget) || opacity < 0 || opacity > 255) {
        return -1;
    }

    widget->opacity = opacity;
    ld_base = tinyui_widget_get_ld_base(widget);
    if (ld_base != 0) {
        ldBaseSetOpacity(ld_base, (uint8_t)opacity);
    }
    return 0;
}

/**
 * @brief Set selectable of widget
 *
 * @param[in] widget Widget instance
 * @param[in] selectable selectable
 * @return 0 on success, -1 on failure
 */

int tinyui_widget_set_selectable(struct tinyui_widget *widget, int selectable)
{
    ldBase_t *ld_base;

    if (!tinyui_widget_is_valid(widget)) {
        return -1;
    }

    widget->selectable = selectable != 0;
    ld_base = tinyui_widget_get_ld_base(widget);
    if (ld_base != 0) {
        ldBaseSetSelectable(ld_base, widget->selectable != 0);
    }
    return 0;
}

/**
 * @brief Set selected of widget
 *
 * @param[in] widget Widget instance
 * @param[in] selected selected
 * @return 0 on success, -1 on failure
 */

int tinyui_widget_set_selected(struct tinyui_widget *widget, int selected)
{
    ldBase_t *ld_base;

    if (!tinyui_widget_is_valid(widget)) {
        return -1;
    }

    widget->selected = selected != 0;
    ld_base = tinyui_widget_get_ld_base(widget);
    if (ld_base != 0) {
        ldBaseSetSelect(ld_base, widget->selected != 0);
    }
    return 0;
}

/**
 * @brief Set corner of widget
 *
 * @param[in] widget Widget instance
 * @param[in] corner corner
 * @return 0 on success, -1 on failure
 */

int tinyui_widget_set_corner(struct tinyui_widget *widget, int corner)
{
    ldBase_t *ld_base;

    if (!tinyui_widget_is_valid(widget)) {
        return -1;
    }

    widget->corner = corner != 0;
    ld_base = tinyui_widget_get_ld_base(widget);
    if (ld_base != 0) {
        ldBaseSetCorner(ld_base, widget->corner != 0);
    }
    return 0;
}

/**
 * @brief Set enabled of widget
 *
 * @param[in] widget Widget instance
 * @param[in] enabled Enable state
 * @return 0 on success, -1 on failure
 */

int tinyui_widget_set_enabled(struct tinyui_widget *widget, int enabled)
{
    ldBase_t *ld_base;

    if (!tinyui_widget_is_valid(widget)) {
        return -1;
    }

    widget->enabled = enabled != 0;
    if (widget->enabled == 0) {
        (void)tinyui_widget_release_focus(widget);
    }
    ld_base = tinyui_widget_get_ld_base(widget);
    if (ld_base != 0) {
        ldBaseSetSelectable(ld_base, widget->enabled != 0);
    }
    if (widget->kind == TINYUI_BACKEND_WIDGET_LIST && widget->ld_widget != 0) {
        ldBaseSetSelectable((ldBase_t *)widget->ld_widget, widget->enabled != 0);
    }
    if (widget->kind == TINYUI_BACKEND_WIDGET_SWITCH && widget->ld_widget != 0) {
        ldSwitchSetDisabled((ldSwitch_t *)widget->ld_widget, widget->enabled == 0);
    }
    return 0;
}

/**
 * @brief Set flex grow of widget
 *
 * @param[in] widget Widget instance
 * @param[in] grow grow
 * @return -1 on failure
 */

int tinyui_widget_set_flex_grow(struct tinyui_widget *widget, int grow)
{
    ldBase_t *ld_base;

    if (!tinyui_widget_is_valid(widget) || grow < 0) {
        return -1;
    }

    ld_base = tinyui_widget_get_ld_base(widget);
    if (ld_base == 0 || !tinyui_widget_validate_native_binding(widget, ld_base)) {
        return -1;
    }

    ldBaseSetFlexGrow(ld_base, (uint16_t)grow);
    widget->flex_grow = grow;
    return 0;
}

/**
 * @brief Set flex new track of widget
 *
 * @param[in] widget Widget instance
 * @param[in] new_track new track
 * @return -1 on failure
 */

int tinyui_widget_set_flex_new_track(struct tinyui_widget *widget, int new_track)
{
    int value;
    ldBase_t *ld_base;

    if (!tinyui_widget_is_valid(widget)) {
        return -1;
    }

    value = new_track != 0;
    ld_base = tinyui_widget_get_ld_base(widget);
    if (ld_base == 0 || !tinyui_widget_validate_native_binding(widget, ld_base)) {
        return -1;
    }

    ldBaseSetFlexNewTrack(ld_base, value);
    widget->flex_new_track = value;
    return 0;
}

/**
 * @brief Set flex min width of widget
 *
 * @param[in] widget Widget instance
 * @param[in] min_width min width
 * @return 0 on success, -1 on failure
 */

int tinyui_widget_set_flex_min_width(struct tinyui_widget *widget, int min_width)
{
    ldBase_t *ld_base;

    if (!tinyui_widget_is_valid(widget) || min_width < 0) {
        return -1;
    }

    widget->flex_min_width = min_width;
    ld_base = tinyui_widget_get_ld_base(widget);
    if (ld_base != 0) {
        ldBaseSetFlexMinWidth(ld_base, (int16_t)min_width);
    }
    return 0;
}

/**
 * @brief Set flex min height of widget
 *
 * @param[in] widget Widget instance
 * @param[in] min_height min height
 * @return 0 on success, -1 on failure
 */

int tinyui_widget_set_flex_min_height(struct tinyui_widget *widget, int min_height)
{
    ldBase_t *ld_base;

    if (!tinyui_widget_is_valid(widget) || min_height < 0) {
        return -1;
    }

    widget->flex_min_height = min_height;
    ld_base = tinyui_widget_get_ld_base(widget);
    if (ld_base != 0) {
        ldBaseSetFlexMinHeight(ld_base, (int16_t)min_height);
    }
    return 0;
}

/**
 * @brief Set flex max width of widget
 *
 * @param[in] widget Widget instance
 * @param[in] max_width max width
 * @return 0 on success, -1 on failure
 */

int tinyui_widget_set_flex_max_width(struct tinyui_widget *widget, int max_width)
{
    ldBase_t *ld_base;

    if (!tinyui_widget_is_valid(widget) || max_width < 0) {
        return -1;
    }

    widget->flex_max_width = max_width;
    ld_base = tinyui_widget_get_ld_base(widget);
    if (ld_base != 0) {
        ldBaseSetFlexMaxWidth(ld_base, (int16_t)max_width);
    }
    return 0;
}

/**
 * @brief Set flex max height of widget
 *
 * @param[in] widget Widget instance
 * @param[in] max_height max height
 * @return 0 on success, -1 on failure
 */

int tinyui_widget_set_flex_max_height(struct tinyui_widget *widget, int max_height)
{
    ldBase_t *ld_base;

    if (!tinyui_widget_is_valid(widget) || max_height < 0) {
        return -1;
    }

    widget->flex_max_height = max_height;
    ld_base = tinyui_widget_get_ld_base(widget);
    if (ld_base != 0) {
        ldBaseSetFlexMaxHeight(ld_base, (int16_t)max_height);
    }
    return 0;
}

/**
 * @brief Set ignore layout of widget
 *
 * @param[in] widget Widget instance
 * @param[in] ignore_layout ignore layout
 * @return -1 on failure
 */

int tinyui_widget_set_ignore_layout(struct tinyui_widget *widget, int ignore_layout)
{
    int value;
    ldBase_t *ld_base;

    if (!tinyui_widget_is_valid(widget)) {
        return -1;
    }

    value = ignore_layout != 0;
    ld_base = tinyui_widget_get_ld_base(widget);
    if (ld_base == 0 || !tinyui_widget_validate_native_binding(widget, ld_base)) {
        return -1;
    }

    ldBaseSetIgnoreLayout(ld_base, value);
    widget->ignore_layout = value;
    return 0;
}

/**
 * @brief Set grid cell of widget
 *
 * @param[in] widget Widget instance
 * @param[in] col col
 * @param[in] row Row index
 * @param[in] col_span Column span count
 * @param[in] row_span Row span count
 * @param[in] x_align x align
 * @param[in] y_align y align
 * @return -1 on failure
 */

int tinyui_widget_set_grid_cell(struct tinyui_widget *widget,
                                int col,
                                int row,
                                int col_span,
                                int row_span,
                                enum tinyui_align x_align,
                                enum tinyui_align y_align)
{
    ldBase_t *ld_base;
    int grid_x_align;
    int grid_y_align;

    if (!tinyui_widget_is_valid(widget) || col_span <= 0 || row_span <= 0) {
        return -1;
    }

    ld_base = tinyui_widget_get_ld_base(widget);
    if (ld_base == 0 || !tinyui_widget_validate_native_binding(widget, ld_base)) {
        return -1;
    }

    grid_x_align = tinyui_native_align_to_ld_grid((enum tinyui_native_align)x_align);
    grid_y_align = tinyui_native_align_to_ld_grid((enum tinyui_native_align)y_align);
    ldBaseSetGridCell(ld_base,
                      grid_x_align,
                      (int16_t)col,
                      (int16_t)col_span,
                      grid_y_align,
                      (int16_t)row,
                      (int16_t)row_span);
    widget->grid_col = col;
    widget->grid_row = row;
    widget->grid_col_span = col_span;
    widget->grid_row_span = row_span;
    widget->grid_x_align = x_align;
    widget->grid_y_align = y_align;
    return 0;
}

/**
 * @brief Widget: remove from parent
 *
 * @param[in] widget Widget instance
 * @return -1 on failure
 */

int tinyui_widget_remove_from_parent(struct tinyui_widget *widget)
{
    if (!tinyui_widget_is_valid(widget)) {
        return -1;
    }

    if (widget->kind == TINYUI_BACKEND_WIDGET_WINDOW
        || widget->ld_widget == 0) {
        return -1;
    }

    return tinyui_runtime_bridge_detach_from_parent(widget);
}

static void tinyui_destroy_reclaim_hosts_in_subtree(struct tinyui_app *app, ldBase_t *node);

/**
 * @brief Destroy widget widget
 *
 * @param[in] widget Widget instance
 * @return 0 on success, -1 on failure
 */

int tinyui_widget_destroy(struct tinyui_widget *widget)
{
    struct tinyui_app *owner;
    ldBase_t *ld_base;

    if (!tinyui_widget_is_valid(widget)) {
        return -1;
    }

    if (widget->ld_widget == 0) {
        return -1;
    }

    owner = widget->owner;

    /* Root window cannot be destroyed individually */
    if (owner != 0 && owner->root_window != 0 &&
        &owner->root_window->widget == widget) {
        return -1;
    }

    if (owner != 0) {
        if (owner->focus_owner == widget) {
            (void)tinyui_widget_release_focus(widget);
        }
        if (owner->editing_owner == widget) {
            (void)tinyui_widget_release_editing(widget);
        }
    }

    /* Container path: reclaim all hosts in subtree (including this one), then
     * let ldGuiDisposeNodeTree tear down the entire ld subtree. */
    if (ldBaseGetChildList((ldBase_t *)widget->ld_widget) != 0) {
        ldBase_t *ld_root = (ldBase_t *)widget->ld_widget; /* save before free */
        /* reclaim_hosts_in_subtree handles the container host itself too */
        tinyui_destroy_reclaim_hosts_in_subtree(owner, ld_root);
        if (owner != 0 && owner->ld_scene != 0) {
            ldGuiDisposeNodeTree(owner->ld_scene, ld_root);
        }
        return 0;
    }

    tinyui_app_unregister_host(owner, widget);

    /* host_cleanup + depose + free 都在 destroy_common */
    tinyui_widget_destroy_common(widget);
    return 0;
}

/**
 * @brief Get x of widget
 *
 * @param[in] widget Widget instance
 * @return -1 on failure
 */

int tinyui_widget_get_x(const struct tinyui_widget *widget)
{
    ldBase_t *ld_base = tinyui_widget_get_ld_base((struct tinyui_widget *)widget);

    if (!tinyui_widget_is_valid((struct tinyui_widget *)widget)) {
        return -1;
    }
    return ld_base != 0 ? ldBaseGetX(ld_base) : widget->x;
}

/**
 * @brief Get y of widget
 *
 * @param[in] widget Widget instance
 * @return -1 on failure
 */

int tinyui_widget_get_y(const struct tinyui_widget *widget)
{
    ldBase_t *ld_base = tinyui_widget_get_ld_base((struct tinyui_widget *)widget);

    if (!tinyui_widget_is_valid((struct tinyui_widget *)widget)) {
        return -1;
    }
    return ld_base != 0 ? ldBaseGetY(ld_base) : widget->y;
}

/**
 * @brief Get width of widget
 *
 * @param[in] widget Widget instance
 * @return -1 on failure
 */

int tinyui_widget_get_width(const struct tinyui_widget *widget)
{
    ldBase_t *ld_base = tinyui_widget_get_ld_base((struct tinyui_widget *)widget);

    if (!tinyui_widget_is_valid((struct tinyui_widget *)widget)) {
        return -1;
    }
    return ld_base != 0 ? ldBaseGetWidth(ld_base) : widget->width;
}

/**
 * @brief Get height of widget
 *
 * @param[in] widget Widget instance
 * @return -1 on failure
 */

int tinyui_widget_get_height(const struct tinyui_widget *widget)
{
    ldBase_t *ld_base = tinyui_widget_get_ld_base((struct tinyui_widget *)widget);

    if (!tinyui_widget_is_valid((struct tinyui_widget *)widget)) {
        return -1;
    }
    return ld_base != 0 ? ldBaseGetHeight(ld_base) : widget->height;
}

/**
 * @brief Get visible of widget
 *
 * @param[in] widget Widget instance
 * @return -1 on failure
 */

int tinyui_widget_get_visible(const struct tinyui_widget *widget)
{
    ldBase_t *ld_base = tinyui_widget_get_ld_base((struct tinyui_widget *)widget);

    if (!tinyui_widget_is_valid((struct tinyui_widget *)widget)) {
        return -1;
    }
    return ld_base != 0 ? (ldBaseIsHidden(ld_base) ? 0 : 1) : widget->visible;
}

/**
 * @brief Get opacity of widget
 *
 * @param[in] widget Widget instance
 * @return -1 on failure
 */

int tinyui_widget_get_opacity(const struct tinyui_widget *widget)
{
    ldBase_t *ld_base = tinyui_widget_get_ld_base((struct tinyui_widget *)widget);

    if (!tinyui_widget_is_valid((struct tinyui_widget *)widget)) {
        return -1;
    }
    return ld_base != 0 ? (int)ldBaseGetOpacity(ld_base) : widget->opacity;
}

/**
 * @brief Get selectable of widget
 *
 * @param[in] widget Widget instance
 * @return -1 on failure
 */

int tinyui_widget_get_selectable(const struct tinyui_widget *widget)
{
    ldBase_t *ld_base = tinyui_widget_get_ld_base((struct tinyui_widget *)widget);

    if (!tinyui_widget_is_valid((struct tinyui_widget *)widget)) {
        return -1;
    }
    return ld_base != 0 ? (ldBaseIsSelectable(ld_base) ? 1 : 0) : widget->selectable;
}

/**
 * @brief Get selected of widget
 *
 * @param[in] widget Widget instance
 * @return -1 on failure
 */

int tinyui_widget_get_selected(const struct tinyui_widget *widget)
{
    ldBase_t *ld_base = tinyui_widget_get_ld_base((struct tinyui_widget *)widget);

    if (!tinyui_widget_is_valid((struct tinyui_widget *)widget)) {
        return -1;
    }
    return ld_base != 0 ? (ldBaseIsSelected(ld_base) ? 1 : 0) : widget->selected;
}

/**
 * @brief Get corner of widget
 *
 * @param[in] widget Widget instance
 * @return -1 on failure
 */

int tinyui_widget_get_corner(const struct tinyui_widget *widget)
{
    ldBase_t *ld_base = tinyui_widget_get_ld_base((struct tinyui_widget *)widget);

    if (!tinyui_widget_is_valid((struct tinyui_widget *)widget)) {
        return -1;
    }
    return ld_base != 0 ? (ldBaseIsCorner(ld_base) ? 1 : 0) : widget->corner;
}

/**
 * @brief Get parent of widget
 *
 * @param[in] widget Widget instance
 * @return Pointer to the object on success, NULL on failure
 */

struct tinyui_widget *tinyui_widget_get_parent(const struct tinyui_widget *widget)
{
    ldBase_t *ld_base;
    ldBase_t *ld_parent;

    if (widget == 0 || widget->ld_widget == 0) {
        return 0;
    }

    ld_base = (ldBase_t *)widget->ld_widget;
    ld_parent = ldBaseGetParent(ld_base);
    if (ld_parent == 0) {
        return 0;
    }

    return tinyui_app_lookup_host(widget->owner, ld_parent->nameId);
}

/**
 * @brief Get first child of widget
 *
 * @param[in] widget Widget instance
 * @return Pointer to the object on success, NULL on failure
 */

struct tinyui_widget *tinyui_widget_get_first_child(const struct tinyui_widget *widget)
{
    ldBase_t *ld_base;
    ldBase_t *ld_child;

    if (widget == 0 || widget->ld_widget == 0) {
        return 0;
    }

    ld_base = (ldBase_t *)widget->ld_widget;
    ld_child = ldBaseGetChildList(ld_base);
    if (ld_child == 0) {
        return 0;
    }

    return tinyui_app_lookup_host(widget->owner, ld_child->nameId);
}

/**
 * @brief Get next sibling of widget
 *
 * @param[in] widget Widget instance
 * @return Pointer to the object on success, NULL on failure
 */

struct tinyui_widget *tinyui_widget_get_next_sibling(const struct tinyui_widget *widget)
{
    ldBase_t *ld_base;
    ldBase_t *ld_sibling;

    if (widget == 0 || widget->ld_widget == 0) {
        return 0;
    }

    ld_base = (ldBase_t *)widget->ld_widget;
    ld_sibling = ldBaseGetNextSibling(ld_base);
    if (ld_sibling == 0) {
        return 0;
    }

    return tinyui_app_lookup_host(widget->owner, ld_sibling->nameId);
}

/**
 * @brief Get root of widget
 *
 * @param[in] widget Widget instance
 * @return Pointer to the object on success, NULL on failure
 */

struct tinyui_widget *tinyui_widget_get_root(const struct tinyui_widget *widget)
{
    ldBase_t *ld_node;
    struct tinyui_widget *last_valid = 0;

    if (widget == 0 || widget->ld_widget == 0) {
        return 0;
    }

    /* Walk up the LD tree, tracking the last node with a registered host.
     * With a phantom scene root (no host), this returns the topmost
     * user-visible root rather than the phantom. */
    ld_node = (ldBase_t *)widget->ld_widget;
    while (ld_node != 0) {
        struct tinyui_widget *host = tinyui_app_lookup_host(widget->owner, ld_node->nameId);
        if (host != 0) {
            last_valid = host;
        }
        ld_node = ldBaseGetParent(ld_node);
    }
    return last_valid;
}

/**
 * @brief Get child count of widget
 *
 * @param[in] widget Widget instance
 * @return -1 on failure
 */

int tinyui_widget_get_child_count(const struct tinyui_widget *widget)
{
    ldBase_t *ld_base;

    if (widget == 0) {
        return -1;
    }

    ld_base = tinyui_widget_get_ld_base((struct tinyui_widget *)widget);
    if (ld_base != 0) {
        return (int)ldBaseGetChildCount(ld_base);
    }

    return 0;
}

/**
 * @brief Get name id of widget
 *
 * @param[in] widget Widget instance
 * @return -1 on failure
 */

int tinyui_widget_get_name_id(const struct tinyui_widget *widget)
{
    ldBase_t *ld_base;

    if (widget == 0) {
        return -1;
    }

    ld_base = tinyui_widget_get_ld_base((struct tinyui_widget *)widget);
    if (ld_base != 0) {
        return (int)ldBaseGetNameId(ld_base);
    }

    return -1;
}

/**
 * @brief Widget: find by name id
 *
 * @param[in] root root
 * @param[in] name_id Name identifier ID
 * @return Pointer to the object on success, NULL on failure
 */

static struct tinyui_widget *find_widget_by_name_id_in_ld(struct tinyui_app *app, ldBase_t *node, int name_id)
{
    struct tinyui_widget *found;
    ldBase_t *child;

    if (node == 0) {
        return 0;
    }
    if ((int)ldBaseGetNameId(node) == name_id) {
        return tinyui_app_lookup_host(app, (uint16_t)name_id);
    }
    for (child = ldBaseGetChildList(node); child != 0; child = ldBaseGetNextSibling(child)) {
        found = find_widget_by_name_id_in_ld(app, child, name_id);
        if (found != 0) {
            return found;
        }
    }
    return 0;
}

struct tinyui_widget *tinyui_widget_find_by_name_id(const struct tinyui_widget *root, int name_id)
{
    ldBase_t *ld_root;

    if (root == 0 || name_id <= 0) {
        return 0;
    }
    ld_root = tinyui_widget_get_ld_base((struct tinyui_widget *)root);
    if (ld_root == 0) {
        return 0;
    }
    return find_widget_by_name_id_in_ld(root->owner, ld_root, name_id);
}

/**
 * @brief Get type of widget
 *
 * @param[in] widget Widget instance
 */

enum tinyui_widget_type tinyui_widget_get_type(const struct tinyui_widget *widget)
{
    if (widget == 0 || widget->ld_widget == 0) {
        return TINYUI_WIDGET_TYPE_UNKNOWN;
    }

    return tinyui_widget_type_from_backend_kind(widget->kind);
}

/**
 * @brief Get absolute pos of widget
 *
 * @param[in] widget Widget instance
 * @param[in] point Point
 * @return Pointer to the object
 */

struct tinyui_point tinyui_widget_get_absolute_pos(const struct tinyui_widget *widget,
                                                   struct tinyui_point point)
{
    ldBase_t *ld_base;
    tinyui_ld_location_t location;
    struct tinyui_point result = {-1, -1};

    if (widget == 0) {
        return result;
    }

    ld_base = tinyui_widget_get_ld_base((struct tinyui_widget *)widget);
    if (ld_base == 0) {
        result.x = point.x + widget->x;
        result.y = point.y + widget->y;
        return result;
    }

    location.iX = (int16_t)point.x;
    location.iY = (int16_t)point.y;
    location = tinyui_ld_location_from_arm(
        ldBaseGetAbsoluteLocation(ld_base, tinyui_ld_location_to_arm(location)));
    result.x = location.iX;
    result.y = location.iY;
    return result;
}

/**
 * @brief Get relative pos of widget
 *
 * @param[in] widget Widget instance
 * @param[in] point Point
 * @return Pointer to the object
 */

struct tinyui_point tinyui_widget_get_relative_pos(const struct tinyui_widget *widget,
                                                   struct tinyui_point point)
{
    ldBase_t *ld_base;
    tinyui_ld_location_t location;
    struct tinyui_point result = {-1, -1};

    if (widget == 0) {
        return result;
    }

    ld_base = tinyui_widget_get_ld_base((struct tinyui_widget *)widget);
    if (ld_base == 0) {
        result.x = point.x - widget->x;
        result.y = point.y - widget->y;
        return result;
    }

    location.iX = (int16_t)point.x;
    location.iY = (int16_t)point.y;
    location = tinyui_ld_location_from_arm(
        ldBaseGetRelativeLocation(ld_base, tinyui_ld_location_to_arm(location)));
    result.x = location.iX;
    result.y = location.iY;
    return result;
}

/**
 * @brief rect align
 *
 * @param[in] parent Parent widget
 * @param[in] child Child widget
 * @param[in] x_align x align
 * @param[in] y_align y align
 * @return Pointer to the object
 */

struct tinyui_rect tinyui_rect_align(struct tinyui_rect parent,
                                     struct tinyui_rect child,
                                     enum tinyui_align x_align,
                                     enum tinyui_align y_align)
{
    tinyui_ld_region_t aligned;
    int ld_align = tinyui_align_to_ld_horizontal(x_align) | tinyui_align_to_ld_vertical(y_align);

    if (child.width < 0 || child.height < 0 || parent.width < 0 || parent.height < 0) {
        struct tinyui_rect invalid = {0, 0, -1, -1};
        return invalid;
    }

    aligned = tinyui_ld_region_from_arm(
        ldBaseGetAlignRegion(tinyui_ld_region_to_arm(tinyui_rect_to_ld_region(parent)),
                             tinyui_ld_region_to_arm(tinyui_rect_to_ld_region(child)),
                             (arm_2d_align_t)ld_align));
    return tinyui_rect_from_ld_region(aligned);
}

/**
 * @brief rect center
 *
 * @param[in] parent Parent widget
 * @param[in] child Child widget
 * @return Pointer to the object
 */

struct tinyui_rect tinyui_rect_center(struct tinyui_rect parent,
                                      struct tinyui_rect child)
{
    return tinyui_rect_align(parent, child, TINYUI_ALIGN_CENTER, TINYUI_ALIGN_CENTER);
}

/**
 * @brief vertical grid align offset
 *
 * @param[in] widget Widget instance
 * @param[in] current_offset current offset
 * @param[in] item_count item count
 * @param[in] item_height item height
 * @param[in] space Spacing
 * @return -1 on failure
 */

int tinyui_vertical_grid_align_offset(struct tinyui_rect widget,
                                      int current_offset,
                                      int item_count,
                                      int item_height,
                                      int space)
{
    if (widget.width < 0 || widget.height < 0 || item_count < 0 || item_height < 0 || space < 0) {
        return -1;
    }

    return (int)ldBaseAutoVerticalGridAlign(tinyui_ld_region_to_arm(tinyui_rect_to_ld_region(widget)),
                                            (int16_t)current_offset,
                                            (uint8_t)item_count,
                                            (uint8_t)item_height,
                                            (uint8_t)space);
}

/**
 * @brief focus reset
 *
 * @param[in] app Application instance
 * @return 0 on success, -1 on failure
 */

int tinyui_focus_reset(struct tinyui_app *app)
{
    if (app == 0) {
        return -1;
    }

    if (app->focus_owner != 0) {
        (void)tinyui_widget_release_focus(app->focus_owner);
    }
    ldBaseFocusNavigateInit();
    return 0;
}

/**
 * @brief focus navigate
 *
 * @param[in] app Application instance
 * @param[in] dir dir
 * @return 0 on success, -1 on failure
 */

int tinyui_focus_navigate(struct tinyui_app *app, enum tinyui_native_nav_dir dir)
{
    struct tinyui_widget *current;
    struct tinyui_widget *candidate;
    struct tinyui_widget *best = 0;
    struct tinyui_point current_pos;
    struct tinyui_point candidate_pos;
    int best_distance = 0x7fffffff;

    if (app == 0 ||
        (dir != TINYUI_NATIVE_NAV_LEFT &&
         dir != TINYUI_NATIVE_NAV_RIGHT &&
         dir != TINYUI_NATIVE_NAV_UP &&
         dir != TINYUI_NATIVE_NAV_DOWN &&
         dir != TINYUI_NATIVE_NAV_ENTER &&
         dir != TINYUI_NATIVE_NAV_BACK)) {
        return -1;
    }

    current = app->focus_owner;
    if (current == 0) {
        current = app->root_window != 0 ? &app->root_window->widget : 0;
        if (current == 0) {
            return -1;
        }
        candidate = tinyui_widget_get_first_child(current);
        while (candidate != 0) {
            if (candidate->visible != 0 && candidate->enabled != 0 && candidate->selectable != 0) {
                return tinyui_widget_claim_focus(candidate);
            }
            candidate = tinyui_widget_get_next_sibling(candidate);
        }
        return -1;
    }

    if (dir == TINYUI_NATIVE_NAV_ENTER) {
        candidate = tinyui_widget_get_first_child(current);
        if (candidate != 0 &&
            candidate->visible != 0 &&
            candidate->enabled != 0 &&
            candidate->selectable != 0) {
            return tinyui_widget_claim_focus(candidate);
        }
        return 0;
    }

    if (dir == TINYUI_NATIVE_NAV_BACK) {
        candidate = tinyui_widget_get_parent(current);
        if (candidate != 0 &&
            candidate->visible != 0 &&
            candidate->enabled != 0) {
            return tinyui_widget_claim_focus(candidate);
        }
        return 0;
    }

    current_pos = tinyui_widget_get_absolute_pos(current, (struct tinyui_point){0, 0});
    candidate = tinyui_widget_get_first_child(tinyui_widget_get_parent(current));
    while (candidate != 0) {
        int dx;
        int dy;
        int distance;

        if (candidate == current ||
            candidate->visible == 0 ||
            candidate->enabled == 0 ||
            candidate->selectable == 0) {
            candidate = tinyui_widget_get_next_sibling(candidate);
            continue;
        }

        candidate_pos = tinyui_widget_get_absolute_pos(candidate, (struct tinyui_point){0, 0});
        dx = candidate_pos.x - current_pos.x;
        dy = candidate_pos.y - current_pos.y;

        if ((dir == TINYUI_NATIVE_NAV_LEFT && dx >= 0) ||
            (dir == TINYUI_NATIVE_NAV_RIGHT && dx <= 0) ||
            (dir == TINYUI_NATIVE_NAV_UP && dy >= 0) ||
            (dir == TINYUI_NATIVE_NAV_DOWN && dy <= 0)) {
            candidate = tinyui_widget_get_next_sibling(candidate);
            continue;
        }

        distance = (dx < 0 ? -dx : dx) + (dy < 0 ? -dy : dy);
        if (distance < best_distance) {
            best_distance = distance;
            best = candidate;
        }
        candidate = tinyui_widget_get_next_sibling(candidate);
    }

    return best != 0 ? tinyui_widget_claim_focus(best) : 0;
}

/* ── C1-T2: color / align helpers ─────────────────────────────────────────── */

/**
 * @brief Convert RGB888 packed value to ldColor (RGB565 via __RGB macro)
 */
unsigned int tinyui_rgb_to_ld_color(unsigned int rgb888)
{
    return (unsigned int)__RGB((rgb888 >> 16) & 0xFFU, (rgb888 >> 8) & 0xFFU, rgb888 & 0xFFU);
}

/**
 * @brief Convert ldColor (RGB565) back to approximately RGB888
 */
unsigned int tinyui_ld_color_to_rgb(unsigned int color)
{
    uint32_t red   = (color >> 11) & 0x1FU;
    uint32_t green = (color >> 5)  & 0x3FU;
    uint32_t blue  = color         & 0x1FU;

    red   = (red   << 3) | (red   >> 2);
    green = (green << 2) | (green >> 4);
    blue  = (blue  << 3) | (blue  >> 2);
    return (red << 16) | (green << 8) | blue;
}

/**
 * @brief Map tinyui_align to arm_2d_align_t (horizontal axis, START/CENTER/END)
 */
int tinyui_align_to_arm2d(enum tinyui_align align)
{
    switch (align) {
    case TINYUI_ALIGN_START:
        return ARM_2D_ALIGN_LEFT;
    case TINYUI_ALIGN_END:
        return ARM_2D_ALIGN_RIGHT;
    case TINYUI_ALIGN_CENTER:
    default:
        return ARM_2D_ALIGN_CENTRE;
    }
}

/* ── C1-T3: detach / destroy_common helpers ────────────────────────────────── */

/**
 * @brief Common destroy: host_cleanup + depose ld via ptGuiFunc + free host
 */
__attribute__((weak)) void tinyui_test_capture_destroyed_widget_snapshot(
    const struct tinyui_widget *widget)
{
    (void)widget;
}

/* Depth-first: reclaim host structs in subtree while ld is still alive.
 * ldGuiDisposeNodeTree tears down the ld side afterwards. */
static void tinyui_destroy_reclaim_hosts_in_subtree(struct tinyui_app *app, ldBase_t *node)
{
    ldBase_t *child;
    ldBase_t *next;

    if (node == 0 || app == 0) {
        return;
    }

    child = ldBaseGetChildList(node);
    while (child != 0) {
        next = ldBaseGetNextSibling(child);
        tinyui_destroy_reclaim_hosts_in_subtree(app, child);
        child = next;
    }

    {
        struct tinyui_widget *host = tinyui_app_lookup_host(app, node->nameId);
        if (host != 0) {
            if (host->owner != 0) {
                if (host->owner->focus_owner == host) {
                    (void)tinyui_widget_release_focus(host);
                }
                if (host->owner->editing_owner == host) {
                    (void)tinyui_widget_release_editing(host);
                }
            }
            tinyui_widget_prepare_native_depose(host);
            if (host->host_cleanup != 0) {
                host->host_cleanup(host);
            }
            tinyui_app_free_name_id(app, host->ld_name_id);
            tinyui_app_unregister_host(app, host);
            ldFree(host);
        }
    }
}

void tinyui_widget_destroy_common(struct tinyui_widget *w)
{
    struct tinyui_app *owner;
    ldBase_t *ld_base;
    struct ld_scene_t *scene;

    if (w == 0) {
        return;
    }

    owner = w->owner;

    /* Idempotent unregister — safe whether called via destroy or rollback */
    if (owner != 0) {
        tinyui_app_unregister_host(owner, w);
        tinyui_app_free_name_id(owner, w->ld_name_id);
    }

    if (w->host_cleanup != 0) {
        w->host_cleanup(w);
    }

    tinyui_widget_prepare_native_depose(w);

    scene = (owner != 0) ? owner->ld_scene : 0;
    ld_base = (ldBase_t *)w->ld_widget;
    if (ld_base != 0 && ld_base->ptGuiFunc != 0 && ld_base->ptGuiFunc->depose != 0) {
        ld_base->ptGuiFunc->depose(scene, w->ld_widget);
    } else if (ld_base != 0) {
        ldBaseNodeRemove((arm_2d_control_node_t *)w->ld_widget);
    }

    /* Clear volatile fields before snapshot so tests see post-teardown state */
    w->owner = 0;
    w->ld_widget = 0;
    w->ld_event_bridge_scene = 0;
    w->ld_event_bridge_sender = 0;

    if (tinyui_test_capture_destroyed_widget_snapshot != 0) {
        tinyui_test_capture_destroyed_widget_snapshot(w);
    }

    ldFree(w);
}

/* ── C1-T4: tinyui_widget_create_leaf ─────────────────────────────────────── */

/**
 * @brief Generic leaf widget factory.
 *
 * Allocates @p host_size bytes for the host object, creates the backing ld
 * widget via @p ld_init_cb, attaches it to the ld tree under @p parent, and
 * binds pInfo so ld events can reach the host widget.
 */
struct tinyui_widget *tinyui_widget_create_leaf(
    struct tinyui_widget *parent,
    enum tinyui_backend_widget_kind kind,
    void *(*ld_init_cb)(void *ctx, struct ld_scene_t *scene,
                        uint16_t name_id, uint16_t parent_name_id),
    void *ctx,
    size_t host_size)
{
    struct tinyui_widget *w;
    struct tinyui_app *owner;
    uint16_t name_id;
    uint16_t parent_name_id;
    void *ld_widget;

    if (parent == 0 || ld_init_cb == 0 || host_size < sizeof(struct tinyui_widget)) {
        return 0;
    }

    owner = parent->owner;
    if (parent->ld_widget == 0 || owner == 0 || owner->ld_scene == 0) {
        return 0;
    }

    /* 1. Allocate host object (zeroed) */
    w = (struct tinyui_widget *)ldCalloc(1, host_size);
    if (w == 0) {
        return 0;
    }

    /* 2. Bind owner */
    w->owner = owner;

    /* 3. Assign name_id before calling ld_init so ld sees the correct id */
    name_id = tinyui_app_alloc_name_id(owner);

    /* 4. Obtain parent name_id (0 is acceptable for the root window) */
    parent_name_id = parent->ld_name_id;

    /* 5. Create the backing ld widget */
    ld_widget = ld_init_cb(ctx, owner->ld_scene, name_id, parent_name_id);
    if (ld_widget == 0) {
        ldFree(w);
        return 0;
    }

    /* 6. Store ld binding and kind */
    w->ld_widget  = ld_widget;
    w->ld_name_id = name_id;
    w->kind       = kind;

    /* 7. Attach to ld tree under parent
     *    Note: most ld<Xxx>_init() functions already auto-attach the new
     *    node to its parent (looked up via parent_name_id). Only fall back
     *    to manual ldBaseNodeAdd when the ld widget has no parent yet — this
     *    keeps create_leaf usable for ld widgets that don't auto-attach
     *    while avoiding the double-attach that produces a cyclic child list. */
    if (ldBaseGetParent((ldBase_t *)ld_widget) == 0
        && parent->ld_widget != 0) {
        ldBaseNodeAdd((arm_2d_control_node_t *)parent->ld_widget,
                      (arm_2d_control_node_t *)ld_widget);
    }

    /* 8. 注册宿主(ld 事件经 nameId 反查到此 widget) */
    tinyui_app_register_host(owner, w);

    /* 9. Default visibility / enabled */
    w->visible = 1;
    w->enabled = 1;

    /* 10. Bind runtime event bridge */
    if (tinyui_runtime_bridge_bind_leaf_widget(w, owner) != 0) {
        /* Roll back the partially-created leaf with no leak and no phantom
         * node: unregister host, detach from the ld tree, then depose the ld
         * widget via its generic func-table depose. */
        ldBase_t *ld_base = (ldBase_t *)ld_widget;
        tinyui_app_unregister_host(owner, w);
        ldBaseNodeRemove((arm_2d_control_node_t *)ld_widget);
        if (ld_base->ptGuiFunc != 0 && ld_base->ptGuiFunc->depose != 0) {
            ld_base->ptGuiFunc->depose(owner->ld_scene, ld_widget);
        }
        ldFree(w);
        return 0;
    }

    return w;
}
