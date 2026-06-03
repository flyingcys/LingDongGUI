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

#ifndef PICOUI_QRCODE_H
#define PICOUI_QRCODE_H

struct picoui_widget;
struct picoui_qrcode;

struct picoui_qrcode_props {
    const char *id;
    const char *style_class;
    void *user_data;
    const char *text;
    unsigned int qr_color;
    unsigned int bg_color;
    int ecc;
    int max_version;
    int zoom;
};

/**
 * @brief Create qrcode widget
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_qrcode *picoui_qrcode_create(struct picoui_widget *parent, const char *id);

/**
 * @brief Create qrcode widget with properties
 *
 * @param[in] parent Parent widget
 * @param[in] props Properties structure
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_qrcode *picoui_qrcode_create_with_props(struct picoui_widget *parent,
                                                      const struct picoui_qrcode_props *props);

/**
 * @brief q r code init
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_qrcode *picoui_q_r_code_init(struct picoui_widget *parent, const char *id);

/**
 * @brief Set text of qrcode widget
 *
 * @param[in] qrcode QR code widget instance
 * @param[in] text Text widget instance
 * @return 0 on success, -1 on failure
 */

int picoui_qrcode_set_text(struct picoui_qrcode *qrcode, const char *text);

/**
 * @brief Get text of qrcode widget
 *
 * @param[in] qrcode QR code widget instance
 */

const char *picoui_qrcode_get_text(const struct picoui_qrcode *qrcode);

/**
 * @brief Set text of q r code widget
 *
 * @param[in] qrcode QR code widget instance
 * @param[in] text Text widget instance
 * @return 0 on success, -1 on failure
 */

int picoui_q_r_code_set_text(struct picoui_qrcode *qrcode, const char *text);

/**
 * @brief Set qr color of qrcode widget
 *
 * @param[in] qrcode QR code widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_qrcode_set_qr_color(struct picoui_qrcode *qrcode, unsigned int rgb);

/**
 * @brief Set bg color of qrcode widget
 *
 * @param[in] qrcode QR code widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_qrcode_set_bg_color(struct picoui_qrcode *qrcode, unsigned int rgb);

/**
 * @brief Set ecc of qrcode widget
 *
 * @param[in] qrcode QR code widget instance
 * @param[in] ecc ecc
 * @return 0 on success, -1 on failure
 */

int picoui_qrcode_set_ecc(struct picoui_qrcode *qrcode, int ecc);

/**
 * @brief Set max version of qrcode widget
 *
 * @param[in] qrcode QR code widget instance
 * @param[in] max_version max version
 * @return 0 on success, -1 on failure
 */

int picoui_qrcode_set_max_version(struct picoui_qrcode *qrcode, int max_version);

/**
 * @brief Set zoom of qrcode widget
 *
 * @param[in] qrcode QR code widget instance
 * @param[in] zoom zoom
 * @return 0 on success, -1 on failure
 */

int picoui_qrcode_set_zoom(struct picoui_qrcode *qrcode, int zoom);

#endif
