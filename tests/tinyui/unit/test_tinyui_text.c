/*
 * TinyUI text unit tests — M3 Task 4 L3/L4 harness.
 *
 * Validates real ldText_t fields for create/props/text/font/color/
 * transparent/background/scroll and props failure rollback.
 */

#include "tinyui.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldText.h"
#include "../../../src/misc/ldMsg.h"
#include "../../../examples/common/demo/widget/fonts/uiFonts.h"
#include "internal.h"
#include "resource/image_source.h"
#include "widgets/text.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void tinyui_text_test_fail_next_set_font(void);

static void *g_test_text_alloc_fail_once_result = (void *)1;

void *ldMalloc(uint32_t size)
{
    return malloc((size_t)size);
}

void *ldCalloc(uint32_t num, uint32_t size)
{
    if (g_test_text_alloc_fail_once_result == NULL) {
        g_test_text_alloc_fail_once_result = (void *)1;
        return NULL;
    }
    return calloc((size_t)num, (size_t)size);
}

void *ldRealloc(void *ptr, uint32_t size)
{
    if (ptr == NULL) {
        return ldMalloc(size);
    }
    return realloc(ptr, (size_t)size);
}

void ldFree(void *ptr)
{
    free(ptr);
}

static int text_has_signal_connection(ldText_t *ld_text, uint8_t signal)
{
    ldAssn_t *assn = ((ldBase_t *)ld_text)->ptAssn;

    while (assn != 0) {
        if (assn->signal == signal) {
            return 1;
        }
        assn = assn->ptNext;
    }
    return 0;
}

static void test_props_initial_values(tinyui_obj_t *root)
{
    tinyui_obj_t *text_obj = tinyui_text_create(root);
    struct tinyui_widget *backend;
    ldText_t *ld_text;
    ldBase_t *ld_base;

    assert(text_obj != 0);
    backend = (struct tinyui_widget *)(void *)text_obj;
    assert(backend->ld_widget != 0);
    assert(backend->kind == TINYUI_BACKEND_WIDGET_TEXT);
    assert(tinyui_runtime_internal_widget_has_ld_binding(backend) == 1);

    ld_text = (ldText_t *)backend->ld_widget;
    ld_base = (ldBase_t *)backend->ld_widget;
    assert(ld_text != 0);
    assert(ld_base->widgetType == widgetTypeText);
    assert(ldBaseGetWidth(ld_base) == 220);
    assert(ldBaseGetHeight(ld_base) == 48);
    assert(ld_base->opacity == 255);
    assert(ld_base->isHidden == false);
    assert(ld_text->bgColor == __RGB(255, 255, 255));
    assert(ld_text->ptFont == (arm_2d_font_t *)FONT_ARIAL_12);
    assert(ld_text->isTransparent == false);
    assert(ld_text->pStr == 0);
    assert(ld_text->scrollOffset == 0);
    assert(backend->font == 0);
}

static void test_widget_native_base_flags_round_trip_to_ldbase(tinyui_obj_t *root)
{
    tinyui_obj_t *text_obj = tinyui_text_create(root);
    struct tinyui_widget *backend;
    ldBase_t *ld_base;
    arm_2d_location_t loc;

    assert(text_obj != 0);
    backend = (struct tinyui_widget *)(void *)text_obj;
    ld_base = (ldBase_t *)backend->ld_widget;
    assert(ld_base != 0);

    assert(tinyui_obj_set_visible(text_obj, 0) == TINYUI_OK);
    assert(ld_base->isHidden == true);
    assert(tinyui_obj_set_visible(text_obj, 1) == TINYUI_OK);
    assert(ld_base->isHidden == false);

    assert(tinyui_obj_set_opacity(text_obj, 128) == TINYUI_OK);
    assert(ld_base->opacity == 128U);

    assert(tinyui_obj_set_selectable(text_obj, 1) == TINYUI_OK);
    assert(ld_base->isSelectable == true);
    assert(tinyui_obj_set_selected(text_obj, 1) == TINYUI_OK);
    assert(ld_base->isSelected == true);
    assert(tinyui_obj_set_selected(text_obj, 0) == TINYUI_OK);
    assert(ld_base->isSelected == false);

    assert(tinyui_obj_set_pos(text_obj, 12, 34) == TINYUI_OK);
    loc = ldBaseGetLocation(ld_base);
    assert(loc.iX == 12);
    assert(loc.iY == 34);

    /* corner has no tinyui_obj_* surface; common internal setter must map. */
    assert(tinyui_runtime_internal_widget_set_corner(backend, 1) == 0);
    assert(ld_base->isCorner == true);
    assert(tinyui_runtime_internal_widget_set_corner(backend, 0) == 0);
    assert(ld_base->isCorner == false);
}

static void test_text_native_r3_style_background_static_and_scroll_round_trip(tinyui_obj_t *root)
{
    tinyui_obj_t *text_obj = tinyui_text_create(root);
    struct tinyui_widget *backend;
    ldText_t *ld_text;
    char owned_buf[32];
    uint16_t pixels[4] = {0xF800U, 0x07E0U, 0x001FU, 0xFFFFU};
    tinyui_image_source_t source;

    assert(text_obj != 0);
    backend = (struct tinyui_widget *)(void *)text_obj;
    ld_text = (ldText_t *)backend->ld_widget;
    assert(ld_text != 0);

    memcpy(owned_buf, "OwnedCopy", 10);
    assert(tinyui_text_set_text(text_obj, owned_buf) == 0);
    assert(backend->text != 0);
    assert(backend->text != owned_buf);
    assert(strcmp(backend->text, "OwnedCopy") == 0);
    assert(backend->text_owned == 1);
    assert(ld_text->pStr != 0);
    assert(strcmp((const char *)ld_text->pStr, "OwnedCopy") == 0);
    assert(ld_text->_isStatic == false);
    memcpy(owned_buf, "mutated!!", 10);
    assert(strcmp((const char *)ld_text->pStr, "OwnedCopy") == 0);
    assert(strcmp(backend->text, "OwnedCopy") == 0);

    /* Multiline content must survive ownership copy into real ldText_t. */
    assert(tinyui_text_set_text(text_obj, "line1\nline2\nline3") == 0);
    assert(backend->text_owned == 1);
    assert(strcmp(backend->text, "line1\nline2\nline3") == 0);
    assert(strcmp((const char *)ld_text->pStr, "line1\nline2\nline3") == 0);
    assert(ld_text->_isStatic == false);

    assert(tinyui_text_set_static_text(text_obj, "StaticBorrow") == 0);
    assert(backend->text == (const char *)"StaticBorrow"
           || strcmp(backend->text, "StaticBorrow") == 0);
    assert(backend->text_owned == 0);
    assert(ld_text->pStr != 0);
    assert(strcmp((const char *)ld_text->pStr, "StaticBorrow") == 0);
    assert(ld_text->_isStatic == true);

    assert(tinyui_text_set_transparent(text_obj, 1) == 0);
    assert(ld_text->isTransparent == true);
    assert(tinyui_text_set_transparent(text_obj, 0) == 0);
    assert(ld_text->isTransparent == false);

    assert(tinyui_text_set_text_color(text_obj, 0x112233U) == 0);
    assert(ld_text->textColor == (ldColor)tinyui_rgb_to_ld_color(0x112233U));
    assert(backend->text_color == 0x112233U);

    assert(tinyui_text_set_bg_color(text_obj, 0xC0C0C0U) == 0);
    assert(ld_text->bgColor == (ldColor)tinyui_rgb_to_ld_color(0xC0C0C0U));
    assert(backend->bg_color == 0xC0C0C0U);
    assert(ld_text->isTransparent == false);
    assert(ld_text->ptImgTile == 0);

    assert(tinyui_image_source_from_rgb565(pixels, 2, 2, 4, 0, 0, &source) == TINYUI_OK);
    assert(tinyui_text_set_background_source(text_obj, &source) == 0);
    assert(ld_text->ptImgTile == tinyui_image_source_get_image_tile(&source));
    assert(ld_text->ptMaskTile == tinyui_image_source_get_mask_tile(&source));
    assert(tinyui_text_set_background_source(text_obj, 0) == 0);
    assert(ld_text->ptImgTile == 0);

    assert(text_has_signal_connection(ld_text, SIGNAL_PRESS) == 0);
    assert(text_has_signal_connection(ld_text, SIGNAL_HOLD_DOWN) == 0);
    assert(text_has_signal_connection(ld_text, SIGNAL_RELEASE) == 0);
    assert(tinyui_text_set_scroll_enabled(text_obj, 1) == 0);
    assert(text_has_signal_connection(ld_text, SIGNAL_PRESS) == 1);
    assert(text_has_signal_connection(ld_text, SIGNAL_HOLD_DOWN) == 1);
    assert(text_has_signal_connection(ld_text, SIGNAL_RELEASE) == 1);
    assert(tinyui_text_set_scroll_enabled(text_obj, 0) == 0);
    assert(text_has_signal_connection(ld_text, SIGNAL_PRESS) == 0);

    assert(tinyui_text_scroll_seek(text_obj, 17) == 0);
    assert(ld_text->scrollOffset == 17);
    assert(tinyui_text_scroll_move(text_obj, 5) == 0);
    assert(ld_text->scrollOffset == 22);
    assert(tinyui_text_scroll_move(text_obj, -3) == 0);
    assert(ld_text->scrollOffset == 19);
    assert(tinyui_text_scroll_move(text_obj, 200) == -1);
    assert(ld_text->scrollOffset == 19);
}

static void test_text_font_null_falls_back_to_default_contract(tinyui_obj_t *root)
{
    tinyui_obj_t *text_obj = tinyui_text_create(root);
    struct tinyui_widget *backend;
    ldText_t *ld_text;

    assert(text_obj != 0);
    backend = (struct tinyui_widget *)(void *)text_obj;
    ld_text = (ldText_t *)backend->ld_widget;
    assert(ld_text != 0);

    assert(backend->font == 0);
    assert(ld_text->ptFont == (arm_2d_font_t *)FONT_ARIAL_12);

    assert(tinyui_text_set_consumed_font(text_obj, NULL) == 0);
    assert(ld_text->ptFont == (arm_2d_font_t *)FONT_ARIAL_12);
    assert(backend->font == 0);

    assert(tinyui_text_set_font(text_obj, NULL) == 0);
    assert(ld_text->ptFont == (arm_2d_font_t *)FONT_ARIAL_12);
    assert(backend->font == 0);
}

static void test_text_font_runtime_rebind_updates_real_ldtext_and_public_cache(tinyui_obj_t *root)
{
    tinyui_font_t arial16;
    tinyui_font_t arial12;
    tinyui_obj_t *text_obj = tinyui_text_create(root);
    struct tinyui_widget *backend;
    ldText_t *ld_text;

    assert(text_obj != 0);
    backend = (struct tinyui_widget *)(void *)text_obj;
    ld_text = (ldText_t *)backend->ld_widget;
    assert(ld_text != 0);

    assert(tinyui_font_from_builtin(TINYUI_FONT_ARIAL_16_A8, &arial16) == TINYUI_OK);
    assert(tinyui_text_set_font(text_obj, &arial16) == 0);
    assert(backend->font == &arial16);
    assert(ld_text->ptFont == (arm_2d_font_t *)FONT_ARIAL_16_A8);

    assert(tinyui_font_from_builtin(TINYUI_FONT_ARIAL_12, &arial12) == TINYUI_OK);
    assert(tinyui_text_set_font(text_obj, &arial12) == 0);
    assert(backend->font == &arial12);
    assert(ld_text->ptFont == (arm_2d_font_t *)FONT_ARIAL_12);
}

static void test_text_create_with_props_pushes_ld_fields(tinyui_obj_t *root)
{
    tinyui_font_t arial16;
    tinyui_text_props_t props;
    tinyui_obj_t *text_obj;
    struct tinyui_widget *backend;
    ldText_t *ld_text;
    ldBase_t *ld_base;

    assert(tinyui_font_from_builtin(TINYUI_FONT_ARIAL_16_A8, &arial16) == TINYUI_OK);
    memset(&props, 0, sizeof(props));
    props.fields = TINYUI_TEXT_FIELD_TEXT
        | TINYUI_TEXT_FIELD_FONT
        | TINYUI_TEXT_FIELD_WIDTH
        | TINYUI_TEXT_FIELD_HEIGHT
        | TINYUI_TEXT_FIELD_BG_COLOR
        | TINYUI_TEXT_FIELD_TEXT_COLOR;
    props.text = "PropsContent";
    props.font = &arial16;
    props.width = 200;
    props.height = 40;
    props.bg_color = 0xC0C0C0U;
    props.text_color = 0x102030U;

    text_obj = tinyui_text_create_with_props(root, &props);
    assert(text_obj != 0);
    backend = (struct tinyui_widget *)(void *)text_obj;
    ld_text = (ldText_t *)backend->ld_widget;
    ld_base = (ldBase_t *)backend->ld_widget;
    assert(ld_text != 0);
    assert(backend->text != 0);
    assert(strcmp(backend->text, "PropsContent") == 0);
    assert(strcmp((const char *)ld_text->pStr, "PropsContent") == 0);
    assert(backend->font == &arial16);
    assert(ld_text->ptFont == (arm_2d_font_t *)FONT_ARIAL_16_A8);
    assert(ldBaseGetWidth(ld_base) == 200);
    assert(ldBaseGetHeight(ld_base) == 40);
    assert(ld_text->bgColor == (ldColor)tinyui_rgb_to_ld_color(0xC0C0C0U));
    assert(ld_text->textColor == (ldColor)tinyui_rgb_to_ld_color(0x102030U));
}

static void test_text_create_with_props_font_failure_rolls_back(tinyui_obj_t *root)
{
    tinyui_font_t failed_font;
    tinyui_text_props_t props;
    uint16_t child_count_before = 0;
    uint16_t child_count_after = 0;
    ldBase_t *root_ld = (ldBase_t *)((struct tinyui_widget *)(void *)root)->ld_widget;
    tinyui_obj_t *text_obj;

    assert(tinyui_font_from_builtin(TINYUI_FONT_ARIAL_16_A8, &failed_font) == TINYUI_OK);
    assert(tinyui_obj_get_child_count(root, &child_count_before) == TINYUI_OK);

    memset(&props, 0, sizeof(props));
    props.fields = TINYUI_TEXT_FIELD_TEXT | TINYUI_TEXT_FIELD_FONT;
    props.text = "Content";
    props.font = &failed_font;

    tinyui_text_test_fail_next_set_font();
    text_obj = tinyui_text_create_with_props(root, &props);
    assert(text_obj == 0);
    assert(tinyui_obj_get_child_count(root, &child_count_after) == TINYUI_OK);
    assert(child_count_after == child_count_before);
    assert(ldBaseGetChildCount(root_ld) == child_count_before);
}

static void test_text_create_with_props_unsupported_style_rolls_back(tinyui_obj_t *root)
{
    uint16_t child_count_before = 0;
    uint16_t child_count_after = 0;
    tinyui_text_props_t props;
    tinyui_obj_t *text_obj;

    assert(tinyui_obj_get_child_count(root, &child_count_before) == TINYUI_OK);

    memset(&props, 0, sizeof(props));
    props.fields = TINYUI_TEXT_FIELD_BORDER_COLOR
        | TINYUI_TEXT_FIELD_RADIUS
        | TINYUI_TEXT_FIELD_PADDING;
    props.border_color = 0x010203U;
    props.radius = 3;
    props.padding = 2;

    text_obj = tinyui_text_create_with_props(root, &props);
    assert(text_obj == 0);
    assert(tinyui_obj_get_child_count(root, &child_count_after) == TINYUI_OK);
    assert(child_count_after == child_count_before);
}

static void test_text_create_with_props_negative_size_rejected(tinyui_obj_t *root)
{
    uint16_t child_count_before = 0;
    uint16_t child_count_after = 0;
    tinyui_text_props_t props;

    assert(tinyui_obj_get_child_count(root, &child_count_before) == TINYUI_OK);
    memset(&props, 0, sizeof(props));
    props.fields = TINYUI_TEXT_FIELD_WIDTH | TINYUI_TEXT_FIELD_HEIGHT;
    props.width = -1;
    props.height = 10;
    assert(tinyui_text_create_with_props(root, &props) == 0);
    assert(tinyui_obj_get_child_count(root, &child_count_after) == TINYUI_OK);
    assert(child_count_after == child_count_before);
}

static void test_text_rejects_null_and_wrong_kind(tinyui_obj_t *root)
{
    tinyui_obj_t *button;

    assert(tinyui_text_create(0) == 0);
    assert(tinyui_text_set_static_text(0, "x") == -1);
    assert(tinyui_text_set_text(0, "x") == -1);
    assert(tinyui_text_set_font(0, 0) == -1);
    assert(tinyui_text_set_scroll_enabled(0, 1) == -1);

    button = tinyui_button_create(root);
    assert(button != 0);
    assert(tinyui_text_set_text(button, "nope") == -1);
    assert(tinyui_text_set_font(button, 0) == -1);
    assert(tinyui_text_set_bg_color(button, 0x123456U) == -1);
    (void)tinyui_obj_delete(button);
}

static void test_text_set_text_handles_alloc_failure_without_crash(tinyui_obj_t *root)
{
    tinyui_obj_t *text_obj = tinyui_text_create(root);
    struct tinyui_widget *backend;
    ldText_t *ld_text;
    const char *old_text;

    assert(text_obj != 0);
    backend = (struct tinyui_widget *)(void *)text_obj;
    ld_text = (ldText_t *)backend->ld_widget;
    assert(ld_text != 0);

    assert(tinyui_text_set_text(text_obj, "before") == 0);
    old_text = backend->text;
    assert(old_text != 0);
    assert(strcmp((const char *)ld_text->pStr, "before") == 0);

    g_test_text_alloc_fail_once_result = NULL;
    assert(tinyui_text_set_text(text_obj, "oom") == -1);
    assert(backend->text == old_text);
    assert(strcmp((const char *)ld_text->pStr, "before") == 0);
}

static void test_text_destroy_via_obj_delete(tinyui_obj_t *root)
{
    tinyui_obj_t *text_obj = tinyui_text_create(root);
    uint16_t before = 0;
    uint16_t after = 0;

    assert(text_obj != 0);
    assert(tinyui_obj_get_child_count(root, &before) == TINYUI_OK);
    assert(tinyui_obj_delete(text_obj) == TINYUI_OK);
    assert(tinyui_obj_get_child_count(root, &after) == TINYUI_OK);
    assert(after + 1U == before);
}

int main(void)
{
    tinyui_obj_t *root;

    tinyui_deinit();
    assert(tinyui_init() == TINYUI_OK);
    root = tinyui_screen_create();
    assert(root != 0);

    test_props_initial_values(root);
    test_widget_native_base_flags_round_trip_to_ldbase(root);
    test_text_native_r3_style_background_static_and_scroll_round_trip(root);
    test_text_font_null_falls_back_to_default_contract(root);
    test_text_font_runtime_rebind_updates_real_ldtext_and_public_cache(root);
    test_text_create_with_props_pushes_ld_fields(root);
    test_text_create_with_props_font_failure_rolls_back(root);
    test_text_create_with_props_unsupported_style_rolls_back(root);
    test_text_create_with_props_negative_size_rejected(root);
    test_text_rejects_null_and_wrong_kind(root);
    test_text_set_text_handles_alloc_failure_without_crash(root);
    test_text_destroy_via_obj_delete(root);

    tinyui_deinit();
    return 0;
}
