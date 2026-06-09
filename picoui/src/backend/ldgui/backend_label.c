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

#include "backend.h"
#include "internal.h"
#include "ldButton.h"
#include "ldLabel.h"
#include "ldQRCode.h"
#include "ldText.h"

#include <stdlib.h>

int picoui_backend_text_set_font(void *backend_widget, const void *font);
extern const arm_2d_a1_font_t ARM_2D_FONT_6x8;

static ldColor picoui_backend_rgb_to_ld_color(unsigned int rgb)
{
    return __RGB((rgb >> 16) & 0xFFU, (rgb >> 8) & 0xFFU, rgb & 0xFFU);
}

static unsigned int picoui_backend_ld_color_to_rgb(ldColor color)
{
    uint32_t red = ((uint32_t)color >> 11) & 0x1FU;
    uint32_t green = ((uint32_t)color >> 5) & 0x3FU;
    uint32_t blue = (uint32_t)color & 0x1FU;

    red = (red << 3) | (red >> 2);
    green = (green << 2) | (green >> 4);
    blue = (blue << 3) | (blue >> 2);
    return (unsigned int)((red << 16) | (green << 8) | blue);
}

static arm_2d_align_t picoui_backend_map_label_align(enum picoui_align align)
{
    switch (align) {
    case PICOUI_ALIGN_START:
        return ARM_2D_ALIGN_LEFT;
    case PICOUI_ALIGN_END:
        return ARM_2D_ALIGN_RIGHT;
    case PICOUI_ALIGN_CENTER:
        return ARM_2D_ALIGN_CENTRE;
    default:
        return ARM_2D_ALIGN_CENTRE;
    }
}

static enum picoui_align picoui_backend_unmap_label_align(arm_2d_align_t align)
{
    switch (align & (ARM_2D_ALIGN_LEFT | ARM_2D_ALIGN_RIGHT)) {
    case ARM_2D_ALIGN_LEFT:
        return PICOUI_ALIGN_START;
    case ARM_2D_ALIGN_RIGHT:
        return PICOUI_ALIGN_END;
    default:
        return PICOUI_ALIGN_CENTER;
    }
}

/**
 * @brief Create backend for label
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 */

void *picoui_backend_create_label(void *parent, const char *id)
{
    (void)parent;
    (void)id;
    return 0;
}

/**
 * @brief set: text
 *
 * @param[in] backend_widget backend widget
 * @param[in] text Text widget instance
 * @return 0 on success, -1 on failure
 */

int picoui_backend_set_text(void *backend_widget, const char *text)
{
    struct picoui_backend_widget *widget = backend_widget;

    if (widget == 0 || text == 0) {
        return -1;
    }

    widget->text = text;
    if (widget->ld_widget != NULL) {
        switch (widget->kind) {
        case PICOUI_BACKEND_WIDGET_LABEL:
            ldLabelSetText((ldLabel_t *)widget->ld_widget, (uint8_t *)text);
            break;
        case PICOUI_BACKEND_WIDGET_TEXT:
            ldTextSetText((ldText_t *)widget->ld_widget, (uint8_t *)text);
            break;
        case PICOUI_BACKEND_WIDGET_QRCODE:
            ldQRCodeSetText((ldQRCode_t *)widget->ld_widget, (uint8_t *)text);
            break;
        case PICOUI_BACKEND_WIDGET_BUTTON:
            ldButtonSetText((ldButton_t *)widget->ld_widget, (uint8_t *)text);
            break;
        default:
            break;
        }
    }
    return 0;
}

/**
 * @brief Set style class of widget backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] style_class style class
 * @return 0 on success, -1 on failure
 */

int picoui_backend_widget_set_style_class(void *backend_widget, const char *style_class)
{
    struct picoui_backend_widget *widget = backend_widget;

    if (widget == 0) {
        return -1;
    }

    widget->style_class = style_class;
    return 0;
}

/**
 * @brief Set font of widget backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] font font
 * @return 0 on success, -1 on failure
 */

int picoui_backend_widget_set_font(void *backend_widget, const void *font)
{
    struct picoui_backend_widget *widget = backend_widget;
    ldLabel_t *ld_label;

    if (widget == 0) {
        return -1;
    }

    if (widget->kind == PICOUI_BACKEND_WIDGET_TEXT) {
        return picoui_backend_text_set_font(backend_widget, font);
    }

    widget->font = font;
    if (widget->kind == PICOUI_BACKEND_WIDGET_LABEL && widget->ld_widget != NULL) {
        ld_label = (ldLabel_t *)widget->ld_widget;
        if (font != NULL) {
            ldLabelSetFont(ld_label, (arm_2d_font_t *)font);
        } else {
            ldLabelSetFont(ld_label, (arm_2d_font_t *)&ARM_2D_FONT_6x8);
        }
    }
    return 0;
}

/**
 * @brief Set user data of widget backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] user_data User data pointer
 * @return 0 on success, -1 on failure
 */

int picoui_backend_widget_set_user_data(void *backend_widget, void *user_data)
{
    struct picoui_backend_widget *widget = backend_widget;

    if (widget == 0) {
        return -1;
    }

    widget->user_data = user_data;
    return 0;
}

static ldLabel_t *picoui_backend_label_get_ld(struct picoui_label *label)
{
    struct picoui_backend_widget *backend;

    if (label == NULL || label->widget.backend_widget == NULL) {
        return NULL;
    }

    backend = (struct picoui_backend_widget *)label->widget.backend_widget;
    if (backend->kind != PICOUI_BACKEND_WIDGET_LABEL || backend->ld_widget == NULL) {
        return NULL;
    }

    return (ldLabel_t *)backend->ld_widget;
}

/**
 * @brief Get text from label backend
 *
 * @param[out] label Label widget instance
 */

const char *picoui_backend_label_get_text(struct picoui_label *label)
{
    ldLabel_t *ld_label = picoui_backend_label_get_ld(label);

    if (ld_label == NULL) {
        return NULL;
    }

    return (const char *)ldLabelGetText(ld_label);
}

/**
 * @brief Set text color of label backend
 *
 * @param[in] label Label widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_backend_label_set_text_color(struct picoui_label *label, unsigned int rgb)
{
    ldLabel_t *ld_label = picoui_backend_label_get_ld(label);

    if (ld_label == NULL) {
        return -1;
    }

    ldLabelSetTextColor(ld_label, picoui_backend_rgb_to_ld_color(rgb));
    return 0;
}

/**
 * @brief Get text color from label backend
 *
 * @param[out] label Label widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_backend_label_get_text_color(struct picoui_label *label, unsigned int *rgb)
{
    ldLabel_t *ld_label = picoui_backend_label_get_ld(label);

    if (ld_label == NULL || rgb == NULL) {
        return -1;
    }

    *rgb = picoui_backend_ld_color_to_rgb(ldLabelGetTextColor(ld_label));
    return 0;
}

/**
 * @brief Set bg color of label backend
 *
 * @param[in] label Label widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_backend_label_set_bg_color(struct picoui_label *label, unsigned int rgb)
{
    ldLabel_t *ld_label = picoui_backend_label_get_ld(label);

    if (ld_label == NULL) {
        return -1;
    }

    ldLabelSetBackgroundColor(ld_label, picoui_backend_rgb_to_ld_color(rgb));
    return 0;
}

/**
 * @brief Get bg color from label backend
 *
 * @param[out] label Label widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_backend_label_get_bg_color(struct picoui_label *label, unsigned int *rgb)
{
    ldLabel_t *ld_label = picoui_backend_label_get_ld(label);

    if (ld_label == NULL || rgb == NULL) {
        return -1;
    }

    *rgb = picoui_backend_ld_color_to_rgb(ldLabelGetBackgroundColor(ld_label));
    return 0;
}

/**
 * @brief Set transparent of label backend
 *
 * @param[in] label Label widget instance
 * @param[in] transparent transparent
 * @return 0 on success, -1 on failure
 */

int picoui_backend_label_set_transparent(struct picoui_label *label, int transparent)
{
    ldLabel_t *ld_label = picoui_backend_label_get_ld(label);

    if (ld_label == NULL) {
        return -1;
    }

    ldLabelSetTransparent(ld_label, transparent != 0);
    return 0;
}

/**
 * @brief Get transparent from label backend
 *
 * @param[out] label Label widget instance
 * @param[in] transparent transparent
 * @return 0 on success, -1 on failure
 */

int picoui_backend_label_get_transparent(struct picoui_label *label, int *transparent)
{
    ldLabel_t *ld_label = picoui_backend_label_get_ld(label);

    if (ld_label == NULL || transparent == NULL) {
        return -1;
    }

    *transparent = ldLabelGetTransparent(ld_label) ? 1 : 0;
    return 0;
}

/**
 * @brief Set align of label backend
 *
 * @param[in] label Label widget instance
 * @param[in] align align
 * @return 0 on success, -1 on failure
 */

int picoui_backend_label_set_align(struct picoui_label *label, enum picoui_align align)
{
    ldLabel_t *ld_label = picoui_backend_label_get_ld(label);

    if (ld_label == NULL) {
        return -1;
    }

    if (align != PICOUI_ALIGN_START && align != PICOUI_ALIGN_CENTER && align != PICOUI_ALIGN_END) {
        return -1;
    }

    ldLabelSetAlign(ld_label, picoui_backend_map_label_align(align));
    return 0;
}

/**
 * @brief Get align from label backend
 *
 * @param[out] label Label widget instance
 * @param[out] align align
 * @return 0 on success, -1 on failure
 */

int picoui_backend_label_get_align(struct picoui_label *label, enum picoui_align *align)
{
    ldLabel_t *ld_label = picoui_backend_label_get_ld(label);

    if (ld_label == NULL || align == NULL) {
        return -1;
    }

    *align = picoui_backend_unmap_label_align(ldLabelGetAlign(ld_label));
    return 0;
}

/**
 * @brief Set background source of label backend
 *
 * @param[in] label Label widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_backend_label_set_background_source(struct picoui_label *label,
                                               struct picoui_image_source *source)
{
    ldLabel_t *ld_label = picoui_backend_label_get_ld(label);

    if (ld_label == NULL || (source != NULL && source->img_tile == NULL)) {
        return -1;
    }

    ldLabelSetBackgroundImage(ld_label,
                              source != NULL ? source->img_tile : NULL,
                              source != NULL ? source->mask_tile : NULL);
    return 0;
}
