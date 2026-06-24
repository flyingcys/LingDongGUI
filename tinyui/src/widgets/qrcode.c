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
#include "widgets/qrcode.h"
#include "core/widget.h"
#include "../core/runtime_bridge.h"
#include "../../../src/gui/ldQRCode.h"

#include <stdlib.h>
#include <string.h>

/* ---- test seam state ---- */


static int qrcode_props_valid(const struct tinyui_qrcode_props *props)
{
    return props != 0 && props->id != 0 && props->text != 0;
}

static void *tinyui_qrcode_ld_init(void *ctx,
                                   struct ld_scene_t *scene,
                                   uint16_t name_id,
                                   uint16_t parent_name_id)
{
    static unsigned char empty_text[] = "";

    (void)ctx;
    return ldQRCode_init(scene,
                         NULL,
                         name_id,
                         parent_name_id,
                         0,
                         0,
                         128,
                         128,
                         empty_text,
                         GLCD_COLOR_BLACK,
                         GLCD_COLOR_WHITE,
                         QR_ECC_7,
                         2,
                         4);
}

/**
 * @brief Create qrcode widget
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object on success, NULL on failure
 */

struct tinyui_qrcode *tinyui_qrcode_create(struct tinyui_widget *parent, const char *id)
{
    struct tinyui_qrcode *qrcode;
    static unsigned char empty_text[] = "";

    if (parent == 0 || id == 0 || parent->ld_widget == 0) {
        return 0;
    }
    qrcode = (struct tinyui_qrcode *)tinyui_widget_create_leaf(parent,
                                                               TINYUI_BACKEND_WIDGET_QRCODE,
                                                               tinyui_qrcode_ld_init,
                                                               0,
                                                               sizeof(*qrcode));
    if (qrcode == 0) {
        return 0;
    }
    qrcode->id = id;
    qrcode->qr_color = 0x000000U;
    qrcode->bg_color = 0xFFFFFFU;
    qrcode->ecc = 0;
    qrcode->max_version = 2;
    qrcode->zoom = 4;
    qrcode->text = (const char *)empty_text;
    return qrcode;
}

/**
 * @brief q r code init
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object
 */

struct tinyui_qrcode *tinyui_q_r_code_init(struct tinyui_widget *parent, const char *id)
{
    return tinyui_qrcode_create(parent, id);
}

/**
 * @brief Create qrcode widget with properties
 *
 * @param[in] parent Parent widget
 * @param[in] props Properties structure
 * @return Pointer to the object on success, NULL on failure
 */

struct tinyui_qrcode *tinyui_qrcode_create_with_props(struct tinyui_widget *parent,
                                                      const struct tinyui_qrcode_props *props)
{
    struct tinyui_qrcode *qrcode;

    if (!qrcode_props_valid(props)) {
        return 0;
    }

    qrcode = tinyui_qrcode_create(parent, props->id);
    if (qrcode == 0) {
        return 0;
    }

    if ((props->style_class != 0
         && tinyui_widget_set_style_class(&qrcode->widget, props->style_class) != 0)
        || tinyui_widget_set_user_data(&qrcode->widget, props->user_data) != 0
        || tinyui_qrcode_set_text(qrcode, props->text) != 0) {
        tinyui_widget_destroy_common(&qrcode->widget);
        return 0;
    }

    return qrcode;
}

/**
 * @brief Set text of qrcode widget
 *
 * @param[in] qrcode QR code widget instance
 * @param[in] text Text widget instance
 * @return 0 on success, -1 on failure
 */

int tinyui_qrcode_set_text(struct tinyui_qrcode *qrcode, const char *text)
{
    if (qrcode == 0 || text == 0 || qrcode->widget.ld_widget == 0
        || qrcode->widget.kind != TINYUI_BACKEND_WIDGET_QRCODE) {
        return -1;
    }

    ldQRCodeSetText((ldQRCode_t *)qrcode->widget.ld_widget, (uint8_t *)text);
    qrcode->text = text;
    return 0;
}

/**
 * @brief Set text of q r code widget
 *
 * @param[in] qrcode QR code widget instance
 * @param[in] text Text widget instance
 * @return 0 on success, -1 on failure
 */

int tinyui_q_r_code_set_text(struct tinyui_qrcode *qrcode, const char *text)
{
    return tinyui_qrcode_set_text(qrcode, text);
}

/**
 * @brief Get text of qrcode widget
 *
 * @param[in] qrcode QR code widget instance
 */

const char *tinyui_qrcode_get_text(const struct tinyui_qrcode *qrcode)
{
    if (qrcode == 0 || qrcode->widget.ld_widget == 0
        || qrcode->widget.kind != TINYUI_BACKEND_WIDGET_QRCODE) {
        return 0;
    }

    return (const char *)((ldQRCode_t *)qrcode->widget.ld_widget)->pStr;
}

/**
 * @brief Set qr color of qrcode widget
 *
 * @param[in] qrcode QR code widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int tinyui_qrcode_set_qr_color(struct tinyui_qrcode *qrcode, unsigned int rgb)
{
    if (qrcode == 0 || rgb > 0xFFFFFFU || qrcode->widget.ld_widget == 0
        || qrcode->widget.kind != TINYUI_BACKEND_WIDGET_QRCODE) {
        return -1;
    }

    ((ldQRCode_t *)qrcode->widget.ld_widget)->qrColor = (ldColor)rgb;
    qrcode->qr_color = rgb;
    return 0;
}

/**
 * @brief Set bg color of qrcode widget
 *
 * @param[in] qrcode QR code widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int tinyui_qrcode_set_bg_color(struct tinyui_qrcode *qrcode, unsigned int rgb)
{
    if (qrcode == 0 || rgb > 0xFFFFFFU || qrcode->widget.ld_widget == 0
        || qrcode->widget.kind != TINYUI_BACKEND_WIDGET_QRCODE) {
        return -1;
    }

    ((ldQRCode_t *)qrcode->widget.ld_widget)->bgColor = (ldColor)rgb;
    qrcode->bg_color = rgb;
    return 0;
}

/**
 * @brief Set ecc of qrcode widget
 *
 * @param[in] qrcode QR code widget instance
 * @param[in] ecc ecc
 * @return 0 on success, -1 on failure
 */

int tinyui_qrcode_set_ecc(struct tinyui_qrcode *qrcode, int ecc)
{
    if (qrcode == 0 || ecc < 0 || ecc > 3 || qrcode->widget.ld_widget == 0
        || qrcode->widget.kind != TINYUI_BACKEND_WIDGET_QRCODE) {
        return -1;
    }

    ((ldQRCode_t *)qrcode->widget.ld_widget)->qrEcc = (uint8_t)ecc;
    qrcode->ecc = ecc;
    return 0;
}

/**
 * @brief Set max version of qrcode widget
 *
 * @param[in] qrcode QR code widget instance
 * @param[in] max_version max version
 * @return 0 on success, -1 on failure
 */

int tinyui_qrcode_set_max_version(struct tinyui_qrcode *qrcode, int max_version)
{
    if (qrcode == 0 || max_version <= 0 || max_version > 40
        || qrcode->widget.ld_widget == 0
        || qrcode->widget.kind != TINYUI_BACKEND_WIDGET_QRCODE) {
        return -1;
    }

    ((ldQRCode_t *)qrcode->widget.ld_widget)->qrMaxVersion = (uint8_t)max_version;
    qrcode->max_version = max_version;
    return 0;
}

/**
 * @brief Set zoom of qrcode widget
 *
 * @param[in] qrcode QR code widget instance
 * @param[in] zoom zoom
 * @return 0 on success, -1 on failure
 */

int tinyui_qrcode_set_zoom(struct tinyui_qrcode *qrcode, int zoom)
{
    if (qrcode == 0 || zoom <= 0 || qrcode->widget.ld_widget == 0
        || qrcode->widget.kind != TINYUI_BACKEND_WIDGET_QRCODE) {
        return -1;
    }

    ((ldQRCode_t *)qrcode->widget.ld_widget)->qrZoom = (uint8_t)zoom;
    qrcode->zoom = zoom;
    return 0;
}
