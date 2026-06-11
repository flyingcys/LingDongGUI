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
extern const arm_2d_a1_font_t ARM_2D_FONT_6x8;

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
