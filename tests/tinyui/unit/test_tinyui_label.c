/*
 * TinyUI label unit tests — M2 Task 5 L3/L4 harness.
 *
 * Validates real ldLabel_t fields for create/props/text/font/color/
 * transparent/align/background and props failure rollback.
 */

#include "tinyui.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldLabel.h"
#include "../../../examples/common/demo/widget/fonts/uiFonts.h"
#include "internal.h"
#include "resource/image_source.h"
#include "widgets/label.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_label_create_and_ld_mapping(tinyui_obj_t *root)
{
    tinyui_obj_t *label_obj = tinyui_label_create(root);
    struct tinyui_widget *backend;
    ldBase_t *ld_base;

    assert(label_obj != 0);
    backend = (struct tinyui_widget *)(void *)label_obj;
    assert(backend->ld_widget != 0);
    assert(backend->kind == TINYUI_BACKEND_WIDGET_LABEL);
    ld_base = (ldBase_t *)backend->ld_widget;
    assert(ld_base->widgetType == widgetTypeLabel);
}

static void test_label_rejects_null_parent(void)
{
    assert(tinyui_label_create(0) == 0);
}

static void test_label_set_text_round_trip(tinyui_obj_t *root)
{
    tinyui_obj_t *label_obj = tinyui_label_create(root);
    struct tinyui_widget *backend;
    ldLabel_t *ld_label;
    char buf[32];

    assert(label_obj != 0);
    backend = (struct tinyui_widget *)(void *)label_obj;
    ld_label = (ldLabel_t *)backend->ld_widget;
    assert(ld_label != 0);
    assert(ldLabelGetFont(ld_label) == (arm_2d_font_t *)FONT_ARIAL_12);

    memcpy(buf, "Hello TINYUI", 13);
    assert(tinyui_label_set_text(label_obj, buf) == 0);
    assert(tinyui_label_get_text(label_obj) != 0);
    assert(strcmp(tinyui_label_get_text(label_obj), "Hello TINYUI") == 0);
    assert(tinyui_label_get_text(label_obj) != buf);
    assert(ldLabelGetText(ld_label) != 0);
    assert(strcmp((const char *)ldLabelGetText(ld_label), "Hello TINYUI") == 0);
    assert(backend->text != 0);
    assert(strcmp(backend->text, "Hello TINYUI") == 0);

    /* label_set_text is only a thin forwarder to the common text path. */
    memcpy(buf, "mutated", 8);
    assert(strcmp(tinyui_label_get_text(label_obj), "Hello TINYUI") == 0);

    assert(tinyui_label_set_text(0, "x") == -1);
    assert(tinyui_label_set_text(label_obj, 0) == -1);
}

static void test_label_set_font_maps_public_font_to_legacy_font(tinyui_obj_t *root)
{
    tinyui_font_t arial16;
    tinyui_obj_t *label_obj = tinyui_label_create(root);
    struct tinyui_widget *backend;
    ldLabel_t *ld_label;

    assert(label_obj != 0);
    backend = (struct tinyui_widget *)(void *)label_obj;
    ld_label = (ldLabel_t *)backend->ld_widget;
    assert(ld_label != 0);
    assert(tinyui_font_from_builtin(TINYUI_FONT_ARIAL_16_A8, &arial16) == TINYUI_OK);
    assert(tinyui_label_set_font(label_obj, &arial16) == 0);
    assert(backend->font == &arial16);
    assert(ldLabelGetFont(ld_label) == (arm_2d_font_t *)FONT_ARIAL_16_A8);
}

static void test_label_colors_transparent_align_background(tinyui_obj_t *root)
{
    tinyui_obj_t *label_obj = tinyui_label_create(root);
    ldLabel_t *ld_label;
    unsigned int rgb = 0;
    int transparent = -1;
    enum tinyui_align align = (enum tinyui_align)99;
    uint16_t pixels[4] = {0xF800U, 0x07E0U, 0x001FU, 0xFFFFU};
    tinyui_image_source_t source;

    assert(label_obj != 0);
    ld_label = (ldLabel_t *)((struct tinyui_widget *)(void *)label_obj)->ld_widget;
    assert(ld_label != 0);

    assert(tinyui_label_set_bg_color(label_obj, 0xC0C0C0U) == 0);
    assert(ld_label->bgColor == GLCD_COLOR_LIGHT_GREY);
    assert(tinyui_label_get_bg_color(label_obj, &rgb) == 0);
    assert((rgb & 0xF8FCF8U) == (0xC0C0C0U & 0xF8FCF8U)
           || ld_label->bgColor == tinyui_rgb_to_ld_color(0xC0C0C0U));

    assert(tinyui_label_set_text_color(label_obj, 0x112233U) == 0);
    assert(ld_label->textColor == tinyui_rgb_to_ld_color(0x112233U));
    assert(tinyui_label_get_text_color(label_obj, &rgb) == 0);

    assert(tinyui_label_set_transparent(label_obj, 1) == 0);
    assert(ldLabelGetTransparent(ld_label) == true);
    assert(tinyui_label_get_transparent(label_obj, &transparent) == 0);
    assert(transparent == 1);
    assert(tinyui_label_set_transparent(label_obj, 0) == 0);
    assert(ldLabelGetTransparent(ld_label) == false);

    assert(tinyui_label_set_align(label_obj, TINYUI_ALIGN_END) == 0);
    assert((ldLabelGetAlign(ld_label) & (ARM_2D_ALIGN_LEFT | ARM_2D_ALIGN_RIGHT))
           == ARM_2D_ALIGN_RIGHT);
    assert(tinyui_label_get_align(label_obj, &align) == 0);
    assert(align == TINYUI_ALIGN_END);
    assert(tinyui_label_set_align(label_obj, (enum tinyui_align)99) == -1);

    assert(tinyui_label_set_text_align(label_obj, TINYUI_ALIGN_START, TINYUI_ALIGN_END) == 0);
    assert(ldLabelGetAlign(ld_label) == (ARM_2D_ALIGN_LEFT | ARM_2D_ALIGN_BOTTOM));
    assert(tinyui_label_set_text_align(label_obj, TINYUI_ALIGN_START, TINYUI_ALIGN_CENTER) == 0);
    assert(ldLabelGetAlign(ld_label) == ARM_2D_ALIGN_MIDDLE_LEFT);

    assert(tinyui_image_source_from_rgb565(pixels, 2, 2, 4, 0, 0, &source) == TINYUI_OK);
    assert(tinyui_label_set_background_source(label_obj, &source) == 0);
    assert(ld_label->ptImgTile != 0);
    assert(tinyui_label_set_background_source(label_obj, 0) == 0);
    assert(ld_label->ptImgTile == 0);
}

static void test_label_create_with_props_pushes_ld_fields(tinyui_obj_t *root)
{
    tinyui_font_t arial16;
    tinyui_label_props_t props;
    tinyui_obj_t *label_obj;
    struct tinyui_widget *backend;
    ldLabel_t *ld_label;
    ldBase_t *ld_base;

    assert(tinyui_font_from_builtin(TINYUI_FONT_ARIAL_16_A8, &arial16) == TINYUI_OK);
    memset(&props, 0, sizeof(props));
    props.fields = TINYUI_LABEL_FIELD_TEXT
        | TINYUI_LABEL_FIELD_FONT
        | TINYUI_LABEL_FIELD_WIDTH
        | TINYUI_LABEL_FIELD_HEIGHT
        | TINYUI_LABEL_FIELD_BG_COLOR
        | TINYUI_LABEL_FIELD_TEXT_COLOR
        | TINYUI_LABEL_FIELD_TRANSPARENT
        | TINYUI_LABEL_FIELD_ALIGN;
    props.text = "PropsTest";
    props.font = &arial16;
    props.width = 200;
    props.height = 30;
    props.bg_color = 0xC0C0C0U;
    props.text_color = 0x102030U;
    props.transparent = 1;
    props.align = TINYUI_ALIGN_END;

    label_obj = tinyui_label_create_with_props(root, &props);
    assert(label_obj != 0);
    backend = (struct tinyui_widget *)(void *)label_obj;
    ld_label = (ldLabel_t *)backend->ld_widget;
    ld_base = (ldBase_t *)backend->ld_widget;
    assert(ld_label != 0);
    assert(strcmp((const char *)ldLabelGetText(ld_label), "PropsTest") == 0);
    assert(ldLabelGetFont(ld_label) == (arm_2d_font_t *)FONT_ARIAL_16_A8);
    assert(ldBaseGetWidth(ld_base) == 200);
    assert(ldBaseGetHeight(ld_base) == 30);
    assert(ld_label->bgColor == tinyui_rgb_to_ld_color(0xC0C0C0U));
    assert(ld_label->textColor == tinyui_rgb_to_ld_color(0x102030U));
    assert(ldLabelGetTransparent(ld_label) == true);
    assert((ldLabelGetAlign(ld_label) & (ARM_2D_ALIGN_LEFT | ARM_2D_ALIGN_RIGHT))
           == ARM_2D_ALIGN_RIGHT);
}

static void test_label_create_with_props_invalid_align_rolls_back(tinyui_obj_t *root)
{
    ldBase_t *root_ld = (ldBase_t *)((struct tinyui_widget *)(void *)root)->ld_widget;
    uint16_t child_count_before = 0;
    uint16_t child_count_after = 0;
    tinyui_label_props_t props;
    tinyui_obj_t *label_obj;

    assert(tinyui_obj_get_child_count(root, &child_count_before) == TINYUI_OK);

    memset(&props, 0, sizeof(props));
    props.fields = TINYUI_LABEL_FIELD_TEXT | TINYUI_LABEL_FIELD_ALIGN;
    props.text = "bad";
    props.align = (enum tinyui_align)99;

    label_obj = tinyui_label_create_with_props(root, &props);
    assert(label_obj == 0);
    assert(tinyui_obj_get_child_count(root, &child_count_after) == TINYUI_OK);
    assert(child_count_after == child_count_before);
    assert(ldBaseGetChildCount(root_ld) == child_count_before);
}

static void test_label_create_with_props_unsupported_style_rolls_back(tinyui_obj_t *root)
{
    uint16_t child_count_before = 0;
    uint16_t child_count_after = 0;
    tinyui_label_props_t props;
    tinyui_obj_t *label_obj;

    assert(tinyui_obj_get_child_count(root, &child_count_before) == TINYUI_OK);

    memset(&props, 0, sizeof(props));
    props.fields = TINYUI_LABEL_FIELD_BORDER_COLOR | TINYUI_LABEL_FIELD_RADIUS
        | TINYUI_LABEL_FIELD_PADDING;
    props.border_color = 0x010203U;
    props.radius = 3;
    props.padding = 2;

    label_obj = tinyui_label_create_with_props(root, &props);
    assert(label_obj == 0);
    assert(tinyui_obj_get_child_count(root, &child_count_after) == TINYUI_OK);
    assert(child_count_after == child_count_before);
}

static void test_label_create_with_props_negative_size_rejected(tinyui_obj_t *root)
{
    uint16_t child_count_before = 0;
    uint16_t child_count_after = 0;
    tinyui_label_props_t props;

    assert(tinyui_obj_get_child_count(root, &child_count_before) == TINYUI_OK);
    memset(&props, 0, sizeof(props));
    props.fields = TINYUI_LABEL_FIELD_WIDTH | TINYUI_LABEL_FIELD_HEIGHT;
    props.width = -1;
    props.height = 10;
    assert(tinyui_label_create_with_props(root, &props) == 0);
    assert(tinyui_obj_get_child_count(root, &child_count_after) == TINYUI_OK);
    assert(child_count_after == child_count_before);
}

static void test_label_wrong_kind_rejected(tinyui_obj_t *root)
{
    tinyui_obj_t *button = tinyui_button_create(root);

    assert(button != 0);
    assert(tinyui_label_set_text(button, "nope") == -1);
    assert(tinyui_label_set_font(button, 0) == -1);
    assert(tinyui_label_set_bg_color(button, 0x123456U) == -1);
    assert(tinyui_label_get_text(button) == 0);
    (void)tinyui_obj_delete(button);
}

static void test_label_destroy_via_obj_delete(tinyui_obj_t *root)
{
    tinyui_obj_t *label_obj = tinyui_label_create(root);
    uint16_t before = 0;
    uint16_t after = 0;

    assert(label_obj != 0);
    assert(tinyui_obj_get_child_count(root, &before) == TINYUI_OK);
    assert(tinyui_obj_delete(label_obj) == TINYUI_OK);
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

    test_label_create_and_ld_mapping(root);
    test_label_rejects_null_parent();
    test_label_set_text_round_trip(root);
    test_label_set_font_maps_public_font_to_legacy_font(root);
    test_label_colors_transparent_align_background(root);
    test_label_create_with_props_pushes_ld_fields(root);
    test_label_create_with_props_invalid_align_rolls_back(root);
    test_label_create_with_props_unsupported_style_rolls_back(root);
    test_label_create_with_props_negative_size_rejected(root);
    test_label_wrong_kind_rejected(root);
    test_label_destroy_via_obj_delete(root);

    tinyui_deinit();
    return 0;
}
