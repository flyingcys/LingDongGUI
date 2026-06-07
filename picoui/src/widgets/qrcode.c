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
#include "picoui/qrcode.h"
#include "picoui/widget.h"

#include <stdlib.h>

void picoui_native_qrcode_reset_render_state(struct picoui_qrcode *qrcode);

int picoui_backend_qrcode_set_text(struct picoui_qrcode *qrcode, const char *text);
int picoui_backend_qrcode_set_qr_color(void *backend_widget, unsigned int rgb);
int picoui_backend_qrcode_set_bg_color(void *backend_widget, unsigned int rgb);
int picoui_backend_qrcode_set_ecc(void *backend_widget, int ecc);
int picoui_backend_qrcode_set_max_version(void *backend_widget, int max_version);
int picoui_backend_qrcode_set_zoom(void *backend_widget, int zoom);

static int picoui_qrcode_props_are_valid(const struct picoui_qrcode_props *props)
{
    return props != 0 && props->id != 0 && props->text != 0;
}

/**
 * @brief Create qrcode widget
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_qrcode *picoui_qrcode_create(struct picoui_widget *parent, const char *id)
{
    struct picoui_qrcode *qrcode;

    if (parent == 0 || id == 0 || parent->backend_widget == 0) {
        return 0;
    }

    qrcode = calloc(1, sizeof(*qrcode));
    if (qrcode == 0) {
        return 0;
    }

    qrcode->widget.backend_widget = picoui_backend_create_qrcode(parent->backend_widget, id);
    if (qrcode->widget.backend_widget == 0) {
        free(qrcode);
        return 0;
    }

    qrcode->id = id;
    qrcode->text = "";
    qrcode->qr_color = 0x000000U;
    qrcode->bg_color = 0xFFFFFFU;
    qrcode->ecc = 0;
    qrcode->max_version = 2;
    qrcode->zoom = 4;
    qrcode->widget.visible = 1;
    qrcode->widget.enabled = 1;
    if (picoui_backend_widget_bind_host(qrcode->widget.backend_widget, &qrcode->widget) != 0) {
        free(qrcode);
        return 0;
    }
    return qrcode;
}

/**
 * @brief q r code init
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object
 */

struct picoui_qrcode *picoui_q_r_code_init(struct picoui_widget *parent, const char *id)
{
    return picoui_qrcode_create(parent, id);
}

/**
 * @brief Create qrcode widget with properties
 *
 * @param[in] parent Parent widget
 * @param[in] props Properties structure
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_qrcode *picoui_qrcode_create_with_props(struct picoui_widget *parent,
                                                      const struct picoui_qrcode_props *props)
{
    struct picoui_qrcode *qrcode;

    if (!picoui_qrcode_props_are_valid(props)) {
        return 0;
    }

    qrcode = picoui_qrcode_create(parent, props->id);
    if (qrcode == 0) {
        return 0;
    }

    if (props->style_class != 0
        && picoui_widget_set_style_class(&qrcode->widget, props->style_class) != 0) {
        free(qrcode);
        return 0;
    }
    if (picoui_widget_set_user_data(&qrcode->widget, props->user_data) != 0
        || picoui_qrcode_set_text(qrcode, props->text) != 0) {
        free(qrcode);
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

int picoui_qrcode_set_text(struct picoui_qrcode *qrcode, const char *text)
{
    if (qrcode == 0 || text == 0) {
        return -1;
    }

    if (picoui_backend_qrcode_set_text(qrcode, text) != 0) {
        return -1;
    }
    qrcode->text = text;
    picoui_native_qrcode_reset_render_state(qrcode);
    return 0;
}

/**
 * @brief Set text of q r code widget
 *
 * @param[in] qrcode QR code widget instance
 * @param[in] text Text widget instance
 * @return 0 on success, -1 on failure
 */

int picoui_q_r_code_set_text(struct picoui_qrcode *qrcode, const char *text)
{
    return picoui_qrcode_set_text(qrcode, text);
}

/**
 * @brief Get text of qrcode widget
 *
 * @param[in] qrcode QR code widget instance
 */

const char *picoui_qrcode_get_text(const struct picoui_qrcode *qrcode)
{
    if (qrcode == 0) {
        return 0;
    }

    return qrcode->text;
}

/**
 * @brief Set qr color of qrcode widget
 *
 * @param[in] qrcode QR code widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_qrcode_set_qr_color(struct picoui_qrcode *qrcode, unsigned int rgb)
{
    if (qrcode == 0 || rgb > 0xFFFFFFU) {
        return -1;
    }

    if (picoui_backend_qrcode_set_qr_color(qrcode->widget.backend_widget, rgb) != 0) {
        return -1;
    }

    qrcode->qr_color = rgb;
    picoui_native_qrcode_reset_render_state(qrcode);
    return 0;
}

/**
 * @brief Set bg color of qrcode widget
 *
 * @param[in] qrcode QR code widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_qrcode_set_bg_color(struct picoui_qrcode *qrcode, unsigned int rgb)
{
    if (qrcode == 0 || rgb > 0xFFFFFFU) {
        return -1;
    }

    if (picoui_backend_qrcode_set_bg_color(qrcode->widget.backend_widget, rgb) != 0) {
        return -1;
    }

    qrcode->bg_color = rgb;
    picoui_native_qrcode_reset_render_state(qrcode);
    return 0;
}

/**
 * @brief Set ecc of qrcode widget
 *
 * @param[in] qrcode QR code widget instance
 * @param[in] ecc ecc
 * @return 0 on success, -1 on failure
 */

int picoui_qrcode_set_ecc(struct picoui_qrcode *qrcode, int ecc)
{
    if (qrcode == 0 || ecc < 0 || ecc > 3) {
        return -1;
    }

    if (picoui_backend_qrcode_set_ecc(qrcode->widget.backend_widget, ecc) != 0) {
        return -1;
    }
    qrcode->ecc = ecc;
    picoui_native_qrcode_reset_render_state(qrcode);
    return 0;
}

/**
 * @brief Set max version of qrcode widget
 *
 * @param[in] qrcode QR code widget instance
 * @param[in] max_version max version
 * @return 0 on success, -1 on failure
 */

int picoui_qrcode_set_max_version(struct picoui_qrcode *qrcode, int max_version)
{
    if (qrcode == 0 || max_version <= 0 || max_version > 40) {
        return -1;
    }

    if (picoui_backend_qrcode_set_max_version(qrcode->widget.backend_widget, max_version) != 0) {
        return -1;
    }

    qrcode->max_version = max_version;
    picoui_native_qrcode_reset_render_state(qrcode);
    return 0;
}

/**
 * @brief Set zoom of qrcode widget
 *
 * @param[in] qrcode QR code widget instance
 * @param[in] zoom zoom
 * @return 0 on success, -1 on failure
 */

int picoui_qrcode_set_zoom(struct picoui_qrcode *qrcode, int zoom)
{
    if (qrcode == 0 || zoom <= 0) {
        return -1;
    }

    if (picoui_backend_qrcode_set_zoom(qrcode->widget.backend_widget, zoom) != 0) {
        return -1;
    }

    qrcode->zoom = zoom;
    picoui_native_qrcode_reset_render_state(qrcode);
    return 0;
}
