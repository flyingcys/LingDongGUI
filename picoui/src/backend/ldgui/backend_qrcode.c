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
#include "ldQRCode.h"

#include <stdlib.h>

static struct picoui_backend_app_state *picoui_backend_qrcode_get_app_state(void *parent)
{
    struct picoui_backend_widget *parent_widget = parent;

    if (parent_widget == NULL || parent_widget->owner == NULL || parent_widget->owner->backend_app == NULL) {
        return NULL;
    }
    return (struct picoui_backend_app_state *)parent_widget->owner->backend_app;
}

static ldQRCode_t *picoui_backend_qrcode_get_ld(struct picoui_qrcode *qrcode)
{
    struct picoui_backend_widget *backend;

    if (qrcode == NULL || qrcode->widget.backend_widget == NULL) {
        return NULL;
    }

    backend = (struct picoui_backend_widget *)qrcode->widget.backend_widget;
    if (backend->kind != PICOUI_BACKEND_WIDGET_QRCODE || backend->ld_widget == NULL) {
        return NULL;
    }

    return (ldQRCode_t *)backend->ld_widget;
}

/**
 * @brief Create backend for qrcode
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 */

void *picoui_backend_create_qrcode(void *parent, const char *id)
{
    struct picoui_backend_widget *widget;
    struct picoui_backend_widget *parent_widget = parent;
    struct picoui_backend_app_state *app_state;
    ldQRCode_t *ld_qrcode;
    uint16_t name_id;
    static unsigned char empty_text[] = "";

    if (parent == 0 || id == 0) {
        return 0;
    }

    app_state = picoui_backend_qrcode_get_app_state(parent);
    if (app_state == NULL || app_state->ld_scene == NULL || parent_widget->ld_widget == NULL) {
        return 0;
    }

    widget = calloc(1, sizeof(*widget));
    if (widget == 0) {
        return 0;
    }

    name_id = ++app_state->next_ld_name_id;
    ld_qrcode = ldQRCode_init(app_state->ld_scene,
                              NULL,
                              name_id,
                              parent_widget->ld_name_id,
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
    if (ld_qrcode == NULL) {
        free(widget);
        return 0;
    }

    widget->parent = parent;
    widget->id = id;
    widget->kind = PICOUI_BACKEND_WIDGET_QRCODE;
    widget->theme = ((struct picoui_backend_widget *)parent)->theme;
    widget->ld_widget = ld_qrcode;
    widget->ld_name_id = name_id;
    widget->text = (const char *)empty_text;
    if (picoui_backend_widget_attach_child(parent, widget) != 0) {
        free(widget);
        return 0;
    }
    return widget;
}

/**
 * @brief Set text of qrcode backend
 *
 * @param[in] qrcode QR code widget instance
 * @param[in] text Text widget instance
 * @return 0 on success, -1 on failure
 */

int picoui_backend_qrcode_set_text(struct picoui_qrcode *qrcode, const char *text)
{
    ldQRCode_t *ld_qrcode = picoui_backend_qrcode_get_ld(qrcode);
    struct picoui_backend_widget *backend;

    if (ld_qrcode == NULL || text == NULL) {
        return -1;
    }

    ldQRCodeSetText(ld_qrcode, (uint8_t *)text);
    backend = (struct picoui_backend_widget *)qrcode->widget.backend_widget;
    backend->text = text;
    return 0;
}

/**
 * @brief Get text from qrcode backend
 *
 * @param[out] qrcode QR code widget instance
 */

const char *picoui_backend_qrcode_get_text(struct picoui_qrcode *qrcode)
{
    ldQRCode_t *ld_qrcode = picoui_backend_qrcode_get_ld(qrcode);

    if (ld_qrcode == NULL) {
        return NULL;
    }

    return (const char *)ld_qrcode->pStr;
}

/**
 * @brief Set qr color of qrcode backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_backend_qrcode_set_qr_color(void *backend_widget, unsigned int rgb)
{
    ldQRCode_t *ld_qrcode;

    if (backend_widget == NULL || rgb > 0xFFFFFFU) {
        return -1;
    }

    ld_qrcode = picoui_backend_qrcode_get_ld((struct picoui_qrcode *)
        ((struct picoui_backend_widget *)backend_widget)->host_widget);
    if (ld_qrcode == NULL) {
        return -1;
    }

    ld_qrcode->qrColor = (ldColor)rgb;
    return 0;
}

/**
 * @brief Set bg color of qrcode backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_backend_qrcode_set_bg_color(void *backend_widget, unsigned int rgb)
{
    ldQRCode_t *ld_qrcode;

    if (backend_widget == NULL || rgb > 0xFFFFFFU) {
        return -1;
    }

    ld_qrcode = picoui_backend_qrcode_get_ld((struct picoui_qrcode *)
        ((struct picoui_backend_widget *)backend_widget)->host_widget);
    if (ld_qrcode == NULL) {
        return -1;
    }

    ld_qrcode->bgColor = (ldColor)rgb;
    return 0;
}

/**
 * @brief Set ecc of qrcode backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] ecc ecc
 * @return 0 on success, -1 on failure
 */

int picoui_backend_qrcode_set_ecc(void *backend_widget, int ecc)
{
    ldQRCode_t *ld_qrcode;

    if (backend_widget == NULL || ecc < 0 || ecc > 3) {
        return -1;
    }

    ld_qrcode = picoui_backend_qrcode_get_ld((struct picoui_qrcode *)
        ((struct picoui_backend_widget *)backend_widget)->host_widget);
    if (ld_qrcode == NULL) {
        return -1;
    }

    ld_qrcode->qrEcc = (uint8_t)ecc;
    return 0;
}

/**
 * @brief Set max version of qrcode backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] max_version max version
 * @return 0 on success, -1 on failure
 */

int picoui_backend_qrcode_set_max_version(void *backend_widget, int max_version)
{
    ldQRCode_t *ld_qrcode;

    if (backend_widget == NULL || max_version <= 0 || max_version > 40) {
        return -1;
    }

    ld_qrcode = picoui_backend_qrcode_get_ld((struct picoui_qrcode *)
        ((struct picoui_backend_widget *)backend_widget)->host_widget);
    if (ld_qrcode == NULL) {
        return -1;
    }

    ld_qrcode->qrMaxVersion = (uint8_t)max_version;
    return 0;
}

/**
 * @brief Set zoom of qrcode backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] zoom zoom
 * @return 0 on success, -1 on failure
 */

int picoui_backend_qrcode_set_zoom(void *backend_widget, int zoom)
{
    ldQRCode_t *ld_qrcode;

    if (backend_widget == NULL || zoom <= 0) {
        return -1;
    }

    ld_qrcode = picoui_backend_qrcode_get_ld((struct picoui_qrcode *)
        ((struct picoui_backend_widget *)backend_widget)->host_widget);
    if (ld_qrcode == NULL) {
        return -1;
    }

    ld_qrcode->qrZoom = (uint8_t)zoom;
    return 0;
}
