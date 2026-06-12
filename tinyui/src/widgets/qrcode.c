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
#include "qrcode.h"
#include "widget.h"
#include "../core/runtime_bridge.h"
#include "../backend/ldgui/backend.h"
#include "../../../src/gui/ldQRCode.h"

#include <stdlib.h>
#include <string.h>

int tinyui_runtime_bridge_unbind_host(void *backend_widget);
int tinyui_runtime_bridge_detach_from_parent(void *backend_widget);

static int tinyui_qrcode_props_are_valid(const struct picoui_qrcode_props *props)
{
    return props != 0 && props->id != 0 && props->text != 0;
}

static ldQRCode_t *tinyui_qrcode_get_ld(const struct picoui_qrcode *qrcode)
{
    struct picoui_backend_widget *backend;

    if (qrcode == 0 || qrcode->widget.backend_widget == 0) {
        return 0;
    }

    backend = (struct picoui_backend_widget *)qrcode->widget.backend_widget;
    if (backend->kind != PICOUI_BACKEND_WIDGET_QRCODE || backend->ld_widget == 0) {
        return 0;
    }

    return (ldQRCode_t *)backend->ld_widget;
}

static int tinyui_qrcode_finish_detach_after_backend_failure(
    struct picoui_backend_widget *backend)
{
    struct picoui_backend_widget *parent;
    struct picoui_backend_widget *cursor;

    if (backend == 0 || backend->parent == 0) {
        return 0;
    }

    parent = backend->parent;
    if (parent->first_child == backend) {
        parent->first_child = backend->next_sibling;
    } else {
        cursor = parent->first_child;
        while (cursor != 0 && cursor->next_sibling != backend) {
            cursor = cursor->next_sibling;
        }
        if (cursor == 0) {
            return -1;
        }
        cursor->next_sibling = backend->next_sibling;
    }

    backend->parent = 0;
    backend->next_sibling = 0;
    backend->owner = 0;
    backend->root = 0;
    return 0;
}

static void tinyui_qrcode_dispose_partial_impl(struct picoui_qrcode *qrcode)
{
    struct picoui_backend_widget *backend;
    struct picoui_backend_app_state *app_state;
    int detach_result = 0;

    if (qrcode == 0) {
        return;
    }

    backend = (struct picoui_backend_widget *)qrcode->widget.backend_widget;
    if (backend != 0) {
        app_state = tinyui_runtime_bridge_backend_state(backend->owner);
        if (backend->parent != 0) {
            detach_result = tinyui_runtime_bridge_detach_from_parent(backend);
            if (detach_result != 0) {
                detach_result = tinyui_qrcode_finish_detach_after_backend_failure(backend);
            }
        }
        (void)tinyui_runtime_bridge_unbind_host(backend);
        if (app_state != 0 && app_state->ld_scene != 0 && backend->ld_widget != 0) {
            ldQRCode_depose(app_state->ld_scene, (ldQRCode_t *)backend->ld_widget);
        }
        free(backend);
    }

    free(qrcode);
}

static struct picoui_qrcode *tinyui_qrcode_create_with_props_impl(
    struct picoui_widget *parent,
    const struct picoui_qrcode_props *props)
{
    struct picoui_qrcode *qrcode;

    if (!tinyui_qrcode_props_are_valid(props)) {
        return 0;
    }

    qrcode = picoui_qrcode_create(parent, props->id);
    if (qrcode == 0) {
        return 0;
    }

    if ((props->style_class != 0
         && picoui_widget_set_style_class(&qrcode->widget, props->style_class) != 0)
        || picoui_widget_set_user_data(&qrcode->widget, props->user_data) != 0
        || picoui_qrcode_set_text(qrcode, props->text) != 0) {
        tinyui_qrcode_dispose_partial_impl(qrcode);
        return 0;
    }

    return qrcode;
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
    struct picoui_backend_widget *backend;
    struct picoui_backend_widget *parent_backend;
    struct picoui_backend_app_state *app_state;
    ldQRCode_t *ld_qrcode;
    uint16_t name_id;
    static unsigned char empty_text[] = "";

    if (parent == 0 || id == 0 || parent->backend_widget == 0) {
        return 0;
    }

    parent_backend = (struct picoui_backend_widget *)parent->backend_widget;
    app_state = tinyui_runtime_bridge_backend_state_from_parent(parent_backend);
    if (parent_backend->ld_widget == 0 || app_state == 0 || app_state->ld_scene == 0) {
        return 0;
    }

    qrcode = calloc(1, sizeof(*qrcode));
    if (qrcode == 0) {
        return 0;
    }

    backend = calloc(1, sizeof(*backend));
    if (backend == 0) {
        free(qrcode);
        return 0;
    }

    name_id = tinyui_runtime_bridge_next_name_id(parent_backend);
    if (name_id == 0) {
        free(backend);
        free(qrcode);
        return 0;
    }
    ld_qrcode = ldQRCode_init(app_state->ld_scene,
                              NULL,
                              name_id,
                              parent_backend->ld_name_id,
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
    if (ld_qrcode == 0) {
        free(backend);
        free(qrcode);
        return 0;
    }

    if (tinyui_widget_init_child(backend,
                                         parent_backend,
                                         PICOUI_BACKEND_WIDGET_QRCODE,
                                         id,
                                         parent_backend->theme) != 0) {
        ldQRCode_depose(app_state->ld_scene, ld_qrcode);
        free(backend);
        free(qrcode);
        return 0;
    }
    backend->ld_widget = ld_qrcode;
    backend->ld_name_id = name_id;
    backend->text = (const char *)empty_text;
    if (tinyui_widget_attach_child(parent_backend, backend) != 0) {
        ldQRCode_depose(app_state->ld_scene, ld_qrcode);
        free(backend);
        free(qrcode);
        return 0;
    }

    qrcode->widget.backend_widget = backend;
    qrcode->id = id;
    qrcode->qr_color = 0x000000U;
    qrcode->bg_color = 0xFFFFFFU;
    qrcode->ecc = 0;
    qrcode->max_version = 2;
    qrcode->zoom = 4;
    qrcode->widget.visible = 1;
    qrcode->widget.enabled = 1;
    if (tinyui_runtime_bridge_bind_host(qrcode->widget.backend_widget, &qrcode->widget) != 0) {
        tinyui_qrcode_dispose_partial_impl(qrcode);
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
    return tinyui_qrcode_create_with_props_impl(parent, props);
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
    ldQRCode_t *ld_qrcode;
    struct picoui_backend_widget *backend;

    if (qrcode == 0 || text == 0) {
        return -1;
    }

    ld_qrcode = tinyui_qrcode_get_ld(qrcode);
    if (ld_qrcode == 0) {
        return -1;
    }

    ldQRCodeSetText(ld_qrcode, (uint8_t *)text);
    backend = (struct picoui_backend_widget *)qrcode->widget.backend_widget;
    backend->text = text;
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
    ldQRCode_t *ld_qrcode;

    if (qrcode == 0) {
        return 0;
    }

    ld_qrcode = tinyui_qrcode_get_ld(qrcode);
    if (ld_qrcode == 0) {
        return 0;
    }

    return (const char *)ld_qrcode->pStr;
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
    ldQRCode_t *ld_qrcode;

    if (qrcode == 0 || rgb > 0xFFFFFFU) {
        return -1;
    }

    ld_qrcode = tinyui_qrcode_get_ld(qrcode);
    if (ld_qrcode == 0) {
        return -1;
    }

    ld_qrcode->qrColor = (ldColor)rgb;
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

int picoui_qrcode_set_bg_color(struct picoui_qrcode *qrcode, unsigned int rgb)
{
    ldQRCode_t *ld_qrcode;

    if (qrcode == 0 || rgb > 0xFFFFFFU) {
        return -1;
    }

    ld_qrcode = tinyui_qrcode_get_ld(qrcode);
    if (ld_qrcode == 0) {
        return -1;
    }

    ld_qrcode->bgColor = (ldColor)rgb;
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

int picoui_qrcode_set_ecc(struct picoui_qrcode *qrcode, int ecc)
{
    ldQRCode_t *ld_qrcode;

    if (qrcode == 0 || ecc < 0 || ecc > 3) {
        return -1;
    }

    ld_qrcode = tinyui_qrcode_get_ld(qrcode);
    if (ld_qrcode == 0) {
        return -1;
    }

    ld_qrcode->qrEcc = (uint8_t)ecc;
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

int picoui_qrcode_set_max_version(struct picoui_qrcode *qrcode, int max_version)
{
    ldQRCode_t *ld_qrcode;

    if (qrcode == 0 || max_version <= 0 || max_version > 40) {
        return -1;
    }

    ld_qrcode = tinyui_qrcode_get_ld(qrcode);
    if (ld_qrcode == 0) {
        return -1;
    }

    ld_qrcode->qrMaxVersion = (uint8_t)max_version;
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

int picoui_qrcode_set_zoom(struct picoui_qrcode *qrcode, int zoom)
{
    ldQRCode_t *ld_qrcode;

    if (qrcode == 0 || zoom <= 0) {
        return -1;
    }

    ld_qrcode = tinyui_qrcode_get_ld(qrcode);
    if (ld_qrcode == 0) {
        return -1;
    }

    ld_qrcode->qrZoom = (uint8_t)zoom;
    qrcode->zoom = zoom;
    return 0;
}
