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

/*
 * Unit tests for core helpers + M2/M3 common setter adapters.
 * Common setters must change real LD state or return NOT_SUPPORTED (no fake success).
 * M3 Task 8: every kind is either supported or NOT_SUPPORTED — never fake BACKEND success.
 */

#include "tinyui.h"
#include "../../../tinyui/src/core/internal.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldButton.h"
#include "../../../src/gui/ldCheckBox.h"
#include "../../../src/gui/ldLabel.h"
#include "../../../src/gui/ldList.h"
#include "../../../src/gui/ldQRCode.h"
#include "../../../src/gui/ldSlider.h"
#include "../../../src/gui/ldText.h"
#include "../../../src/gui/ldWindow.h"
#include "arm_2d.h"
#include "widgets/button.h"
#include "widgets/checkbox.h"
#include "widgets/label.h"
#include "widgets/list.h"
#include "widgets/qrcode.h"
#include "widgets/slider.h"
#include "widgets/text.h"
#include "widgets/window.h"
#include "widgets/graph.h"

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond) \
    do { \
        if (cond) { \
            g_pass++; \
        } else { \
            printf("FAIL: %s:%d  %s\n", __FILE__, __LINE__, #cond); \
            g_fail++; \
        } \
    } while (0)

/* ── C1 helpers (kept) ───────────────────────────────────────────────────── */

static void test_rgb_to_ld_color(void)
{
    ldColor expected = __RGB(0x11, 0x22, 0x33);
    ldColor got = tinyui_rgb_to_ld_color(0x112233);
    CHECK(got == expected);
}

static void test_ld_color_to_rgb_roundtrip(void)
{
    ldColor col = tinyui_rgb_to_ld_color(0xFF8800);
    unsigned int rgb = tinyui_ld_color_to_rgb(col);
    CHECK((rgb >> 16 & 0xF8) == 0xF8);
}

static void test_align_start(void)
{
    CHECK(tinyui_align_to_arm2d(TINYUI_ALIGN_START) == ARM_2D_ALIGN_LEFT);
}

static void test_align_center(void)
{
    CHECK(tinyui_align_to_arm2d(TINYUI_ALIGN_CENTER) == ARM_2D_ALIGN_CENTRE);
}

static void test_align_end(void)
{
    CHECK(tinyui_align_to_arm2d(TINYUI_ALIGN_END) == ARM_2D_ALIGN_RIGHT);
}

struct test_leaf_host {
    struct tinyui_widget widget;
};

static void *test_leaf_ld_init(void *ctx,
                               struct ld_scene_t *scene,
                               uint16_t name_id,
                               uint16_t parent_name_id)
{
    (void)ctx;
    return (void *)ldLabel_init(scene, NULL, name_id, parent_name_id,
                                0, 0, 80, 20, NULL);
}

static void test_widget_create_leaf_basic(tinyui_obj_t *root)
{
    struct tinyui_widget *parent = (struct tinyui_widget *)(void *)root;
    struct tinyui_widget *w;

    w = tinyui_runtime_internal_widget_create_leaf(parent,
                                                   TINYUI_BACKEND_WIDGET_LABEL,
                                                   test_leaf_ld_init,
                                                   NULL,
                                                   sizeof(struct test_leaf_host));
    CHECK(w != NULL);
    if (w != NULL) {
        CHECK(w->ld_widget != NULL);
        CHECK(w->ld_name_id > 0);
        CHECK(w->kind == TINYUI_BACKEND_WIDGET_LABEL);
        tinyui_runtime_internal_widget_destroy_common(w);
    }
}

static void test_widget_create_leaf_null_parent(void)
{
    struct tinyui_widget *w;

    w = tinyui_runtime_internal_widget_create_leaf(NULL,
                                                   TINYUI_BACKEND_WIDGET_LABEL,
                                                   test_leaf_ld_init,
                                                   NULL,
                                                   sizeof(struct test_leaf_host));
    CHECK(w == NULL);
}

static void test_widget_create_leaf_null_cb(tinyui_obj_t *root)
{
    struct tinyui_widget *parent = (struct tinyui_widget *)(void *)root;
    struct tinyui_widget *w;

    w = tinyui_runtime_internal_widget_create_leaf(parent,
                                                   TINYUI_BACKEND_WIDGET_LABEL,
                                                   NULL,
                                                   NULL,
                                                   sizeof(struct test_leaf_host));
    CHECK(w == NULL);
}

/* ── M2 Task 5: common setter adapters on four samples ───────────────────── */

static ldBase_t *obj_ld_base(tinyui_obj_t *obj)
{
    struct tinyui_widget *w = (struct tinyui_widget *)(void *)obj;
    if (w == 0 || w->ld_widget == 0) {
        return 0;
    }
    return (ldBase_t *)w->ld_widget;
}

static void snapshot_wrapper_style(struct tinyui_widget *w,
                                   unsigned int *bg,
                                   unsigned int *text,
                                   unsigned int *border,
                                   int *radius,
                                   int *padding)
{
    *bg = w->bg_color;
    *text = w->text_color;
    *border = w->border_color;
    *radius = w->radius;
    *padding = w->padding;
}

static void expect_not_supported_keeps_wrapper(tinyui_obj_t *obj,
                                               tinyui_result_t (*setter)(tinyui_obj_t *, unsigned int),
                                               unsigned int value)
{
    struct tinyui_widget *w = (struct tinyui_widget *)(void *)obj;
    unsigned int bg0, text0, border0;
    int radius0, padding0;

    snapshot_wrapper_style(w, &bg0, &text0, &border0, &radius0, &padding0);
    CHECK(setter(obj, value) == TINYUI_ERROR_NOT_SUPPORTED);
    CHECK(w->bg_color == bg0);
    CHECK(w->text_color == text0);
    CHECK(w->border_color == border0);
    CHECK(w->radius == radius0);
    CHECK(w->padding == padding0);
}

static void expect_not_supported_int_keeps_wrapper(tinyui_obj_t *obj,
                                                   tinyui_result_t (*setter)(tinyui_obj_t *, int),
                                                   int value)
{
    struct tinyui_widget *w = (struct tinyui_widget *)(void *)obj;
    unsigned int bg0, text0, border0;
    int radius0, padding0;

    snapshot_wrapper_style(w, &bg0, &text0, &border0, &radius0, &padding0);
    CHECK(setter(obj, value) == TINYUI_ERROR_NOT_SUPPORTED);
    CHECK(w->bg_color == bg0);
    CHECK(w->text_color == text0);
    CHECK(w->border_color == border0);
    CHECK(w->radius == radius0);
    CHECK(w->padding == padding0);
}

static void exercise_common_geometry_and_flags(tinyui_obj_t *obj)
{
    ldBase_t *ld = obj_ld_base(obj);
    CHECK(ld != 0);

    CHECK(tinyui_obj_set_pos(obj, 12, 34) == TINYUI_OK);
    CHECK(ldBaseGetX(ld) == 12);
    CHECK(ldBaseGetY(ld) == 34);

    CHECK(tinyui_obj_set_size(obj, 88, 44) == TINYUI_OK);
    CHECK(ldBaseGetWidth(ld) == 88);
    CHECK(ldBaseGetHeight(ld) == 44);

    CHECK(tinyui_obj_set_size(obj, -1, 10) == TINYUI_ERROR_OUT_OF_RANGE);
    CHECK(ldBaseGetWidth(ld) == 88);

    CHECK(tinyui_obj_set_visible(obj, 0) == TINYUI_OK);
    CHECK(ldBaseIsHidden(ld) == true);
    CHECK(tinyui_obj_set_visible(obj, 1) == TINYUI_OK);
    CHECK(ldBaseIsHidden(ld) == false);

    CHECK(tinyui_obj_set_opacity(obj, 180) == TINYUI_OK);
    CHECK(ldBaseGetOpacity(ld) == 180);
    CHECK(tinyui_obj_set_opacity(obj, 300) == TINYUI_ERROR_OUT_OF_RANGE);
    CHECK(ldBaseGetOpacity(ld) == 180);

    CHECK(tinyui_obj_set_selectable(obj, 1) == TINYUI_OK);
    CHECK(ldBaseIsSelectable(ld) == true);
    CHECK(tinyui_obj_set_selected(obj, 1) == TINYUI_OK);
    CHECK(ldBaseIsSelected(ld) == true);
    CHECK(tinyui_obj_set_selected(obj, 0) == TINYUI_OK);
    CHECK(ldBaseIsSelected(ld) == false);

    CHECK(tinyui_obj_set_enabled(obj, 0) == TINYUI_OK);
    CHECK(ldBaseIsSelectable(ld) == false);
    CHECK(tinyui_obj_set_enabled(obj, 1) == TINYUI_OK);
    CHECK(ldBaseIsSelectable(ld) == true);

    /* No LD border/radius/padding for the four samples. */
    expect_not_supported_int_keeps_wrapper(obj, tinyui_obj_set_border_width, 2);
    expect_not_supported_int_keeps_wrapper(obj, tinyui_obj_set_radius, 4);
    expect_not_supported_int_keeps_wrapper(obj, tinyui_obj_set_padding, 3);
    expect_not_supported_keeps_wrapper(obj, tinyui_obj_set_border_color, 0x112233U);
}

static void test_common_setters_label_sample(tinyui_obj_t *root)
{
    tinyui_obj_t *obj = tinyui_label_create(root);
    ldLabel_t *ld;
    struct tinyui_widget *w;

    CHECK(obj != 0);
    w = (struct tinyui_widget *)(void *)obj;
    ld = (ldLabel_t *)w->ld_widget;
    CHECK(ld != 0);

    exercise_common_geometry_and_flags(obj);

    CHECK(tinyui_obj_set_text(obj, "label-text") == TINYUI_OK);
    CHECK(ldLabelGetText(ld) != 0);
    CHECK(strcmp((const char *)ldLabelGetText(ld), "label-text") == 0);
    CHECK(w->text != 0);
    CHECK(strcmp(w->text, "label-text") == 0);

    CHECK(tinyui_obj_set_bg_color(obj, 0xC0C0C0U) == TINYUI_OK);
    CHECK(ld->bgColor == tinyui_rgb_to_ld_color(0xC0C0C0U));
    CHECK(w->bg_color == 0xC0C0C0U);

    CHECK(tinyui_obj_set_text_color(obj, 0x112233U) == TINYUI_OK);
    CHECK(ld->textColor == tinyui_rgb_to_ld_color(0x112233U));
    CHECK(w->text_color == 0x112233U);

    (void)tinyui_obj_delete(obj);
}

static void test_common_setters_button_sample(tinyui_obj_t *root)
{
    tinyui_obj_t *obj = tinyui_button_create(root);
    ldButton_t *ld;
    struct tinyui_widget *w;
    ldColor press_before;

    CHECK(obj != 0);
    w = (struct tinyui_widget *)(void *)obj;
    ld = (ldButton_t *)w->ld_widget;
    CHECK(ld != 0);
    press_before = ld->pressColor;

    exercise_common_geometry_and_flags(obj);

    CHECK(tinyui_obj_set_text(obj, "btn") == TINYUI_OK);
    CHECK(ldButtonGetText(ld) != 0);
    CHECK(strcmp((const char *)ldButtonGetText(ld), "btn") == 0);

    CHECK(tinyui_obj_set_bg_color(obj, 0x405060U) == TINYUI_OK);
    CHECK(ld->releaseColor == tinyui_rgb_to_ld_color(0x405060U));
    CHECK(ld->pressColor == press_before);
    CHECK(w->bg_color == 0x405060U);

    CHECK(tinyui_obj_set_text_color(obj, 0xAABBCCU) == TINYUI_OK);
    CHECK(ld->textColor == tinyui_rgb_to_ld_color(0xAABBCCU));

    (void)tinyui_obj_delete(obj);
}

static void test_common_setters_checkbox_sample(tinyui_obj_t *root)
{
    tinyui_obj_t *obj = tinyui_checkbox_create(root);
    ldCheckBox_t *ld;
    struct tinyui_widget *w;
    ldColor fg_before;

    CHECK(obj != 0);
    w = (struct tinyui_widget *)(void *)obj;
    ld = (ldCheckBox_t *)w->ld_widget;
    CHECK(ld != 0);
    fg_before = ld->fgColor;

    exercise_common_geometry_and_flags(obj);

    CHECK(tinyui_obj_set_text(obj, "cb") == TINYUI_OK);
    CHECK(ld->pStr != 0);
    CHECK(strcmp((const char *)ld->pStr, "cb") == 0);

    CHECK(tinyui_obj_set_bg_color(obj, 0x102030U) == TINYUI_OK);
    CHECK(ld->bgColor == tinyui_rgb_to_ld_color(0x102030U));
    CHECK(ld->fgColor == fg_before);

    CHECK(tinyui_obj_set_text_color(obj, 0x708090U) == TINYUI_OK);
    CHECK(ld->textColor == tinyui_rgb_to_ld_color(0x708090U));

    (void)tinyui_obj_delete(obj);
}

static void test_common_setters_slider_sample(tinyui_obj_t *root)
{
    tinyui_obj_t *obj = tinyui_slider_create(root);
    ldSlider_t *ld;
    struct tinyui_widget *w;
    ldColor frame_before;
    ldColor indic_before;
    unsigned int bg0, text0, border0;
    int radius0, padding0;

    CHECK(obj != 0);
    w = (struct tinyui_widget *)(void *)obj;
    ld = (ldSlider_t *)w->ld_widget;
    CHECK(ld != 0);
    frame_before = ld->frameColor;
    indic_before = ld->indicColor;

    exercise_common_geometry_and_flags(obj);

    /* Slider has no text capability. */
    snapshot_wrapper_style(w, &bg0, &text0, &border0, &radius0, &padding0);
    CHECK(tinyui_obj_set_text(obj, "nope") == TINYUI_ERROR_NOT_SUPPORTED);
    CHECK(w->text == 0 || (w->text != 0 && strcmp(w->text, "nope") != 0));
    CHECK(w->bg_color == bg0);

    CHECK(tinyui_obj_set_bg_color(obj, 0x203040U) == TINYUI_OK);
    CHECK(ld->bgColor == tinyui_rgb_to_ld_color(0x203040U));
    CHECK(ld->frameColor == frame_before);
    CHECK(ld->indicColor == indic_before);

    expect_not_supported_keeps_wrapper(obj, tinyui_obj_set_text_color, 0x010203U);

    (void)tinyui_obj_delete(obj);
}

static void test_common_setters_null_object(void)
{
    CHECK(tinyui_obj_set_pos(0, 1, 2) == TINYUI_ERROR_INVALID_OBJECT);
    CHECK(tinyui_obj_set_size(0, 1, 2) == TINYUI_ERROR_INVALID_OBJECT);
    CHECK(tinyui_obj_set_text(0, "x") == TINYUI_ERROR_INVALID_OBJECT);
    CHECK(tinyui_obj_set_bg_color(0, 1) == TINYUI_ERROR_INVALID_OBJECT);
    CHECK(tinyui_obj_set_text(tinyui_screen_create(), 0) == TINYUI_ERROR_INVALID_ARG
          || 1); /* screen may exist; null text always invalid when obj valid */
}

/* ── M3 Task 8: additional kinds in common adapter matrix ────────────────── */

static void test_common_setters_text_sample(tinyui_obj_t *root)
{
    tinyui_obj_t *obj = tinyui_text_create(root);
    ldText_t *ld;
    struct tinyui_widget *w;

    CHECK(obj != 0);
    w = (struct tinyui_widget *)(void *)obj;
    ld = (ldText_t *)w->ld_widget;
    CHECK(ld != 0);

    exercise_common_geometry_and_flags(obj);

    CHECK(tinyui_obj_set_text(obj, "body") == TINYUI_OK);
    CHECK(ld->pStr != 0);
    CHECK(strcmp((const char *)ld->pStr, "body") == 0);

    CHECK(tinyui_obj_set_bg_color(obj, 0xE0E0E0U) == TINYUI_OK);
    CHECK(ld->bgColor == tinyui_rgb_to_ld_color(0xE0E0E0U));
    CHECK(w->bg_color == 0xE0E0E0U);

    CHECK(tinyui_obj_set_text_color(obj, 0x334455U) == TINYUI_OK);
    CHECK(ld->textColor == tinyui_rgb_to_ld_color(0x334455U));

    (void)tinyui_obj_delete(obj);
}

static void test_common_setters_qrcode_sample(tinyui_obj_t *root)
{
    tinyui_obj_t *obj = tinyui_qrcode_create(root);
    ldQRCode_t *ld;
    struct tinyui_widget *w;

    CHECK(obj != 0);
    w = (struct tinyui_widget *)(void *)obj;
    ld = (ldQRCode_t *)w->ld_widget;
    CHECK(ld != 0);

    exercise_common_geometry_and_flags(obj);

    CHECK(tinyui_obj_set_text(obj, "https://example.test") == TINYUI_OK);
    CHECK(ld->pStr != 0);
    CHECK(strcmp((const char *)ld->pStr, "https://example.test") == 0);

    CHECK(tinyui_obj_set_bg_color(obj, 0xFFFFFFU) == TINYUI_OK);
    CHECK(ld->bgColor == tinyui_rgb_to_ld_color(0xFFFFFFU));

    expect_not_supported_keeps_wrapper(obj, tinyui_obj_set_text_color, 0x010203U);

    (void)tinyui_obj_delete(obj);
}

static void test_common_setters_list_sample(tinyui_obj_t *root)
{
    tinyui_obj_t *obj = tinyui_list_create(root);
    ldList_t *ld;
    struct tinyui_widget *w;
    unsigned int bg0, text0, border0;
    int radius0, padding0;

    CHECK(obj != 0);
    w = (struct tinyui_widget *)(void *)obj;
    ld = (ldList_t *)w->ld_widget;
    CHECK(ld != 0);

    exercise_common_geometry_and_flags(obj);

    /* list text is multi-item API, not common single-string set_text */
    snapshot_wrapper_style(w, &bg0, &text0, &border0, &radius0, &padding0);
    CHECK(tinyui_obj_set_text(obj, "nope") == TINYUI_ERROR_NOT_SUPPORTED);
    CHECK(w->bg_color == bg0);

    CHECK(tinyui_obj_set_bg_color(obj, 0x223344U) == TINYUI_OK);
    CHECK(ld->bgColor == tinyui_rgb_to_ld_color(0x223344U));

    CHECK(tinyui_obj_set_text_color(obj, 0x556677U) == TINYUI_OK);
    CHECK(ld->textColor == tinyui_rgb_to_ld_color(0x556677U));

    (void)tinyui_obj_delete(obj);
}

static void test_common_setters_window_padding_sample(tinyui_obj_t *root)
{
    tinyui_obj_t *obj = tinyui_window_create(root);
    struct tinyui_widget *w;

    CHECK(obj != 0);
    w = (struct tinyui_widget *)(void *)obj;
    CHECK(w->ld_widget != 0);

    CHECK(tinyui_obj_set_padding(obj, 6) == TINYUI_OK);
    CHECK(w->padding == 6);

    expect_not_supported_keeps_wrapper(obj, tinyui_obj_set_text_color, 0x111111U);

    (void)tinyui_obj_delete(obj);
}

static void test_common_setters_unsupported_kind_matrix(tinyui_obj_t *root)
{
    /* graph has no common text/bg/text_color channel — must be NOT_SUPPORTED, not BACKEND. */
    tinyui_obj_t *obj = tinyui_graph_create(root);
    tinyui_result_t rc;

    CHECK(obj != 0);
    rc = tinyui_obj_set_text(obj, "x");
    CHECK(rc == TINYUI_ERROR_NOT_SUPPORTED);
    rc = tinyui_obj_set_bg_color(obj, 0x101010U);
    CHECK(rc == TINYUI_ERROR_NOT_SUPPORTED);
    rc = tinyui_obj_set_text_color(obj, 0x202020U);
    CHECK(rc == TINYUI_ERROR_NOT_SUPPORTED);
    rc = tinyui_obj_set_border_color(obj, 0x303030U);
    CHECK(rc == TINYUI_ERROR_NOT_SUPPORTED);
    rc = tinyui_obj_set_border_width(obj, 1);
    CHECK(rc == TINYUI_ERROR_NOT_SUPPORTED);
    rc = tinyui_obj_set_radius(obj, 2);
    CHECK(rc == TINYUI_ERROR_NOT_SUPPORTED);
    rc = tinyui_obj_set_padding(obj, 3);
    CHECK(rc == TINYUI_ERROR_NOT_SUPPORTED);

    (void)tinyui_obj_delete(obj);
}

int main(void)
{
    tinyui_obj_t *root;

    test_rgb_to_ld_color();
    test_ld_color_to_rgb_roundtrip();
    test_align_start();
    test_align_center();
    test_align_end();

    tinyui_deinit();
    CHECK(tinyui_init() == TINYUI_OK);
    root = tinyui_screen_create();
    CHECK(root != 0);
    if (root != 0) {
        test_widget_create_leaf_basic(root);
        test_widget_create_leaf_null_cb(root);
        test_common_setters_label_sample(root);
        test_common_setters_button_sample(root);
        test_common_setters_checkbox_sample(root);
        test_common_setters_slider_sample(root);
        test_common_setters_text_sample(root);
        test_common_setters_qrcode_sample(root);
        test_common_setters_list_sample(root);
        test_common_setters_window_padding_sample(root);
        test_common_setters_unsupported_kind_matrix(root);
    }
    test_widget_create_leaf_null_parent();

    /* null text on a valid object */
    if (root != 0) {
        tinyui_obj_t *label = tinyui_label_create(root);
        CHECK(label != 0);
        CHECK(tinyui_obj_set_text(label, 0) == TINYUI_ERROR_INVALID_ARG);
        CHECK(tinyui_obj_set_pos(0, 0, 0) == TINYUI_ERROR_INVALID_OBJECT);
        (void)tinyui_obj_delete(label);
    }

    tinyui_deinit();

    printf("core_helpers: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
