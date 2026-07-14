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
#include "internal/widget_legacy.h"
#include "../core/runtime_bridge.h"
#include "../../../src/gui/ldQRCode.h"

#include <stdlib.h>
#include <string.h>


static struct tinyui_qrcode *tinyui_qrcode_as_qrcode(tinyui_obj_t *obj)
{
    struct tinyui_widget *w = (struct tinyui_widget *)(void *)obj;
    if (w == 0 || !tinyui_runtime_internal_widget_is_kind(w, TINYUI_BACKEND_WIDGET_QRCODE)) {
        return 0;
    }
    return (struct tinyui_qrcode *)w;
}

static const struct tinyui_qrcode *tinyui_qrcode_as_qrcode_const(const tinyui_obj_t *obj)
{
    const struct tinyui_widget *w = (const struct tinyui_widget *)(const void *)obj;
    if (w == 0 || !tinyui_runtime_internal_widget_is_kind(w, TINYUI_BACKEND_WIDGET_QRCODE)) {
        return 0;
    }
    return (const struct tinyui_qrcode *)w;
}

/* ---- test seam state ---- */


static int qrcode_props_valid(const tinyui_qrcode_props_t *props)
{
    return props != 0;
}


static void *tinyui_runtime_internal_qrcode_ld_init(void *ctx,
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

tinyui_obj_t *tinyui_qrcode_create(tinyui_obj_t *parent)
{
    struct tinyui_widget *parent_w = (struct tinyui_widget *)(void *)parent;
    const char *id = "qrcode";
    if (parent_w == 0) { return 0; }

    struct tinyui_qrcode *qrcode;
    static unsigned char empty_text[] = "";
    ldQRCode_t *ld_qrcode;

    if (parent_w == 0 || id == 0 || parent_w->ld_widget == 0) {
        return 0;
    }
    qrcode = (struct tinyui_qrcode *)tinyui_runtime_internal_widget_create_leaf(parent_w,
                                                               TINYUI_BACKEND_WIDGET_QRCODE,
                                                               tinyui_runtime_internal_qrcode_ld_init,
                                                               0,
                                                               sizeof(*qrcode));
    if (qrcode == 0) {
        return 0;
    }
    ld_qrcode = (ldQRCode_t *)qrcode->widget.ld_widget;
    if (ld_qrcode == 0) {
        tinyui_runtime_internal_widget_destroy_common(&qrcode->widget);
        return 0;
    }
    qrcode->id = id;
    qrcode->qr_color = (unsigned int)ld_qrcode->qrColor;
    qrcode->bg_color = (unsigned int)ld_qrcode->bgColor;
    qrcode->ecc = (int)ld_qrcode->qrEcc;
    qrcode->max_version = (int)ld_qrcode->qrMaxVersion;
    qrcode->zoom = (int)ld_qrcode->qrZoom;
    qrcode->text = ld_qrcode->pStr != 0 ? (const char *)ld_qrcode->pStr : (const char *)empty_text;
    return (tinyui_obj_t *)qrcode;
}

/**
 * @brief q r code init
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object
 */

struct tinyui_qrcode *tinyui_runtime_internal_q_r_code_init(struct tinyui_widget *parent, const char *id)
{
    return tinyui_qrcode_create(parent);
}

/**
 * @brief Create qrcode widget with properties
 *
 * @param[in] parent Parent widget
 * @param[in] props Properties structure
 * @return Pointer to the object on success, NULL on failure
 */

tinyui_obj_t *tinyui_qrcode_create_with_props(tinyui_obj_t *parent,
                                             const tinyui_qrcode_props_t *props)
{
    tinyui_obj_t *obj;
    struct tinyui_qrcode *qrcode;

    if (props == 0) {
        return tinyui_qrcode_create(parent);
    }

    obj = tinyui_qrcode_create(parent);
    if (obj == 0) {
        return 0;
    }
    qrcode = (struct tinyui_qrcode *)(void *)obj;

    if ((props->fields & TINYUI_QRCODE_FIELD_ID) != 0) {
        /* id=0 means runtime auto-alloc; non-zero reserved for host name_id path. */
        (void)props->id;
    }
    if ((props->fields & TINYUI_QRCODE_FIELD_USER_DATA) != 0) {
        if (tinyui_runtime_internal_widget_set_user_data(&qrcode->widget, props->user_data) != 0) {
            (void)tinyui_obj_delete((tinyui_obj_t *)qrcode);
            return 0;
        }
    }
    if ((props->fields & TINYUI_QRCODE_FIELD_STYLE_CLASS) != 0) {
        if (tinyui_runtime_internal_widget_set_style_class(&qrcode->widget, props->style_class) != 0) {
            (void)tinyui_obj_delete((tinyui_obj_t *)qrcode);
            return 0;
        }
    }
    if ((props->fields & TINYUI_QRCODE_FIELD_TEXT) != 0) {
        if (tinyui_qrcode_set_text((tinyui_obj_t *)qrcode, props->text) != 0) {
            (void)tinyui_obj_delete((tinyui_obj_t *)qrcode);
            return 0;
        }
    }
    if ((props->fields & TINYUI_QRCODE_FIELD_QR_COLOR) != 0) {
        if (tinyui_qrcode_set_qr_color((tinyui_obj_t *)qrcode, props->qr_color) != 0) {
            (void)tinyui_obj_delete((tinyui_obj_t *)qrcode);
            return 0;
        }
    }
    if ((props->fields & TINYUI_QRCODE_FIELD_BG_COLOR) != 0) {
        if (tinyui_qrcode_set_bg_color((tinyui_obj_t *)qrcode, props->bg_color) != 0) {
            (void)tinyui_obj_delete((tinyui_obj_t *)qrcode);
            return 0;
        }
    }
    if ((props->fields & TINYUI_QRCODE_FIELD_ECC) != 0) {
        if (tinyui_qrcode_set_ecc((tinyui_obj_t *)qrcode, props->ecc) != 0) {
            (void)tinyui_obj_delete((tinyui_obj_t *)qrcode);
            return 0;
        }
    }
    if ((props->fields & TINYUI_QRCODE_FIELD_MAX_VERSION) != 0) {
        if (tinyui_qrcode_set_max_version((tinyui_obj_t *)qrcode, props->max_version) != 0) {
            (void)tinyui_obj_delete((tinyui_obj_t *)qrcode);
            return 0;
        }
    }
    if ((props->fields & TINYUI_QRCODE_FIELD_ZOOM) != 0) {
        if (tinyui_qrcode_set_zoom((tinyui_obj_t *)qrcode, props->zoom) != 0) {
            (void)tinyui_obj_delete((tinyui_obj_t *)qrcode);
            return 0;
        }
    }

    return obj;
}



/**
 * @brief Set text of qrcode widget
 *
 * @param[in] qrcode QR code widget instance
 * @param[in] text Text widget instance
 * @return 0 on success, -1 on failure
 */

int tinyui_qrcode_set_text(tinyui_obj_t *qrcode_obj, const char *text)
{
    struct tinyui_qrcode *qrcode = tinyui_qrcode_as_qrcode(qrcode_obj);
    if (qrcode == 0) { return -1; }

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

int tinyui_runtime_internal_q_r_code_set_text(tinyui_obj_t *qrcode_obj, const char *text)
{
    struct tinyui_qrcode *qrcode = tinyui_qrcode_as_qrcode(qrcode_obj);
    if (qrcode == 0) { return -1; }

    return tinyui_qrcode_set_text((tinyui_obj_t *)qrcode, text);
}

/**
 * @brief Get text of qrcode widget
 *
 * @param[in] qrcode QR code widget instance
 */

const char * tinyui_qrcode_get_text(const tinyui_obj_t *qrcode_obj)
{
    const struct tinyui_qrcode *qrcode = tinyui_qrcode_as_qrcode_const(qrcode_obj);
    if (qrcode == 0) { return 0; }

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

int tinyui_qrcode_set_qr_color(tinyui_obj_t *qrcode_obj, unsigned int rgb)
{
    struct tinyui_qrcode *qrcode = tinyui_qrcode_as_qrcode(qrcode_obj);
    if (qrcode == 0) { return -1; }

    if (qrcode == 0 || rgb > 0xFFFFFFU || qrcode->widget.ld_widget == 0
        || qrcode->widget.kind != TINYUI_BACKEND_WIDGET_QRCODE) {
        return -1;
    }

    ((ldQRCode_t *)qrcode->widget.ld_widget)->qrColor = (ldColor)tinyui_rgb_to_ld_color(rgb);
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

int tinyui_qrcode_set_bg_color(tinyui_obj_t *qrcode_obj, unsigned int rgb)
{
    struct tinyui_qrcode *qrcode = tinyui_qrcode_as_qrcode(qrcode_obj);
    if (qrcode == 0) { return -1; }

    if (qrcode == 0 || rgb > 0xFFFFFFU || qrcode->widget.ld_widget == 0
        || qrcode->widget.kind != TINYUI_BACKEND_WIDGET_QRCODE) {
        return -1;
    }

    ((ldQRCode_t *)qrcode->widget.ld_widget)->bgColor = (ldColor)tinyui_rgb_to_ld_color(rgb);
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

int tinyui_qrcode_set_ecc(tinyui_obj_t *qrcode_obj, int ecc)
{
    struct tinyui_qrcode *qrcode = tinyui_qrcode_as_qrcode(qrcode_obj);
    if (qrcode == 0) { return -1; }

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

int tinyui_qrcode_set_max_version(tinyui_obj_t *qrcode_obj, int max_version)
{
    struct tinyui_qrcode *qrcode = tinyui_qrcode_as_qrcode(qrcode_obj);
    if (qrcode == 0) { return -1; }

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

int tinyui_qrcode_set_zoom(tinyui_obj_t *qrcode_obj, int zoom)
{
    struct tinyui_qrcode *qrcode = tinyui_qrcode_as_qrcode(qrcode_obj);
    if (qrcode == 0) { return -1; }

    if (qrcode == 0 || zoom <= 0 || qrcode->widget.ld_widget == 0
        || qrcode->widget.kind != TINYUI_BACKEND_WIDGET_QRCODE) {
        return -1;
    }

    ((ldQRCode_t *)qrcode->widget.ld_widget)->qrZoom = (uint8_t)zoom;
    qrcode->zoom = zoom;
    return 0;
}
