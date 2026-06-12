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

#ifndef TINYUI_QRCODE_H
#define TINYUI_QRCODE_H

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

struct picoui_qrcode *picoui_qrcode_create(struct picoui_widget *parent, const char *id);
struct picoui_qrcode *picoui_qrcode_create_with_props(struct picoui_widget *parent,
                                                      const struct picoui_qrcode_props *props);
struct picoui_qrcode *picoui_q_r_code_init(struct picoui_widget *parent, const char *id);
int picoui_qrcode_set_text(struct picoui_qrcode *qrcode, const char *text);
const char *picoui_qrcode_get_text(const struct picoui_qrcode *qrcode);
int picoui_q_r_code_set_text(struct picoui_qrcode *qrcode, const char *text);
int picoui_qrcode_set_qr_color(struct picoui_qrcode *qrcode, unsigned int rgb);
int picoui_qrcode_set_bg_color(struct picoui_qrcode *qrcode, unsigned int rgb);
int picoui_qrcode_set_ecc(struct picoui_qrcode *qrcode, int ecc);
int picoui_qrcode_set_max_version(struct picoui_qrcode *qrcode, int max_version);
int picoui_qrcode_set_zoom(struct picoui_qrcode *qrcode, int zoom);

#endif
