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
 * Unit tests for Phase C1 core helper functions:
 *   - tinyui_rgb_to_ld_color / tinyui_ld_color_to_rgb
 *   - tinyui_align_to_arm2d
 *   - tinyui_widget_detach_from_parent (declaration check)
 *   - tinyui_widget_destroy_common     (declaration check)
 *   - tinyui_widget_create_leaf        (C1-T4)
 */

#include "tinyui.h"
#include "../../../tinyui/src/core/internal.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldLabel.h"
#include "arm_2d.h"
#include <stdint.h>
#include <stddef.h>
#include <string.h>

/* ── minimal test harness ─────────────────────────────────────────────────── */

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

/* ── C1-T2: color helpers ────────────────────────────────────────────────── */

static void test_rgb_to_ld_color(void)
{
    ldColor expected = __RGB(0x11, 0x22, 0x33);
    ldColor got = tinyui_rgb_to_ld_color(0x112233);
    CHECK(got == expected);
}

static void test_ld_color_to_rgb_roundtrip(void)
{
    /* RGB565 has 5/6/5 bit resolution so only check top bits */
    ldColor col = tinyui_rgb_to_ld_color(0xFF8800);
    unsigned int rgb = tinyui_ld_color_to_rgb(col);
    /* red channel top 5 bits should survive */
    CHECK((rgb >> 16 & 0xF8) == 0xF8);
}

/* ── C1-T2: align helpers ────────────────────────────────────────────────── */

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

/* ── C1-T4: tinyui_widget_create_leaf ────────────────────────────────────── */

/* Minimal stub host struct used for the create_leaf test */
struct test_leaf_host {
    struct tinyui_widget widget; /* MUST be first member */
};

/*
 * ld_init_cb stub: delegates to ldLabel_init with fixed geometry so the test
 * runs without real layout machinery.
 */
static void *test_leaf_ld_init(void *ctx,
                               struct ld_scene_t *scene,
                               uint16_t name_id,
                               uint16_t parent_name_id)
{
    (void)ctx;
    return (void *)ldLabel_init(scene, NULL, name_id, parent_name_id,
                                0, 0, 80, 20, NULL);
}

static struct ld_scene_t *g_test_scene = NULL;

static void test_leaf_ld_depose(void *ld_widget)
{
    if (ld_widget != NULL && g_test_scene != NULL) {
        ldLabel_depose(g_test_scene, (ldLabel_t *)ld_widget);
    }
}

static void test_widget_create_leaf_basic(struct tinyui_window *win)
{
    struct tinyui_widget *w;

    w = tinyui_widget_create_leaf(win,
                                  TINYUI_BACKEND_WIDGET_LABEL,
                                  test_leaf_ld_init,
                                  NULL,
                                  sizeof(struct test_leaf_host));
    CHECK(w != NULL);
    if (w != NULL) {
        CHECK(w->ld_widget != NULL);
        CHECK(w->ld_name_id > 0);
        CHECK(w->kind == TINYUI_BACKEND_WIDGET_LABEL);
        g_test_scene = w->owner->ld_scene;
        /* Clean up — pass a real depose so the ld widget is properly disposed */
        tinyui_widget_destroy_common(w, test_leaf_ld_depose);
        g_test_scene = NULL;
    }
}

static void test_widget_create_leaf_null_parent(void)
{
    struct tinyui_widget *w;

    w = tinyui_widget_create_leaf(NULL,
                                  TINYUI_BACKEND_WIDGET_LABEL,
                                  test_leaf_ld_init,
                                  NULL,
                                  sizeof(struct test_leaf_host));
    CHECK(w == NULL);
}

static void test_widget_create_leaf_null_cb(struct tinyui_window *win)
{
    struct tinyui_widget *w;

    w = tinyui_widget_create_leaf(win,
                                  TINYUI_BACKEND_WIDGET_LABEL,
                                  NULL,
                                  NULL,
                                  sizeof(struct test_leaf_host));
    CHECK(w == NULL);
}

/* ── main ─────────────────────────────────────────────────────────────────── */

int main(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win;

    test_rgb_to_ld_color();
    test_ld_color_to_rgb_roundtrip();
    test_align_start();
    test_align_center();
    test_align_end();

    /* C1-T4: create_leaf tests need a real app + window */
    app = tinyui_app_create();
    if (app != NULL) {
        win = tinyui_window_create(app, "root");
        if (win != NULL) {
            test_widget_create_leaf_basic(win);
            test_widget_create_leaf_null_cb(win);
        }
        tinyui_app_destroy(app);
    }
    test_widget_create_leaf_null_parent();

    printf("core_helpers: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
