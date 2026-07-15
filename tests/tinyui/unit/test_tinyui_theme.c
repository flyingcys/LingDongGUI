/*
 * TinyUI theme/style unit tests — M3 Task 6.
 *
 * 验证调用者持有的 theme/style value descriptor：
 * - 完整预检（fields/part/state/metric/font）
 * - 即时 apply，不保存 style 指针
 * - theme 借用指针；apply 只作用目标对象，不遍历树
 * - apply 路径零 heap 分配
 */

#include "tinyui.h"
#include "internal.h"
#include "core/obj.h"
#include "core/result.h"
#include "core/runtime.h"
#include "style/style.h"
#include "theme/theme.h"
#include "widgets/label.h"
#include "resource/font.h"

#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldLabel.h"
#include "tinyui_test_support.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

void tinyui_internal_theme_reset(void);

/* 覆盖弱符号 ldMalloc 族，统计 apply 期间分配。 */
void *ldMalloc(uint32_t size)
{
    return tinyui_test_allocator_malloc(size);
}

void *ldCalloc(uint32_t num, uint32_t size)
{
    return tinyui_test_allocator_calloc(num, size);
}

void *ldRealloc(void *ptr, uint32_t size)
{
    return tinyui_test_allocator_realloc(ptr, size);
}

void ldFree(void *ptr)
{
    tinyui_test_allocator_free(ptr);
}

static unsigned int test_rgb_to_ld(unsigned int rgb)
{
    return (unsigned int)__RGB((rgb >> 16) & 0xFFU, (rgb >> 8) & 0xFFU, rgb & 0xFFU);
}

static struct tinyui_widget *as_widget(tinyui_obj_t *obj)
{
    return (struct tinyui_widget *)(void *)obj;
}

static ldLabel_t *label_ld(tinyui_obj_t *obj)
{
    struct tinyui_widget *w = as_widget(obj);

    assert(w != 0);
    assert(w->ld_widget != 0);
    return (ldLabel_t *)w->ld_widget;
}

static void assert_zero_alloc_delta(const struct tinyui_test_allocator_stats *before,
                                    const struct tinyui_test_allocator_stats *after)
{
    assert(after->alloc_calls == before->alloc_calls);
    assert(after->calloc_calls == before->calloc_calls);
    assert(after->realloc_calls == before->realloc_calls);
    assert(after->bytes_requested == before->bytes_requested);
}

static void test_apply_style_rejects_null_and_range(tinyui_obj_t *root)
{
    tinyui_obj_t *label = tinyui_label_create(root);
    tinyui_style_t style;

    assert(label != 0);
    memset(&style, 0, sizeof(style));
    style.fields = TINYUI_STYLE_BG_COLOR;
    style.bg_color = UINT32_C(0x112233);

    assert(tinyui_obj_apply_style(0, TINYUI_PART_MAIN, TINYUI_STATE_DEFAULT, &style)
           == TINYUI_ERROR_INVALID_OBJECT);
    assert(tinyui_obj_apply_style(label, TINYUI_PART_MAIN, TINYUI_STATE_DEFAULT, 0)
           == TINYUI_ERROR_INVALID_ARG);
    assert(tinyui_obj_apply_style(label, (tinyui_part_t)99, TINYUI_STATE_DEFAULT, &style)
           == TINYUI_ERROR_OUT_OF_RANGE);
    assert(tinyui_obj_apply_style(label, TINYUI_PART_MAIN, (tinyui_state_t)99, &style)
           == TINYUI_ERROR_OUT_OF_RANGE);

    (void)tinyui_obj_delete(label);
}

static void test_apply_style_rejects_invalid_fields_before_mutation(tinyui_obj_t *root)
{
    tinyui_obj_t *label = tinyui_label_create(root);
    ldLabel_t *ld;
    tinyui_style_t style;
    unsigned int bg_before;
    unsigned int text_before;

    assert(label != 0);
    ld = label_ld(label);
    bg_before = ld->bgColor;
    text_before = ld->textColor;

    memset(&style, 0, sizeof(style));
    style.fields = UINT32_C(1) << 8; /* 非法 bit */
    style.bg_color = UINT32_C(0xAABBCC);
    assert(tinyui_obj_apply_style(label, TINYUI_PART_MAIN, TINYUI_STATE_DEFAULT, &style)
           == TINYUI_ERROR_INVALID_ARG);
    assert(ld->bgColor == bg_before);
    assert(ld->textColor == text_before);

    memset(&style, 0, sizeof(style));
    style.fields = TINYUI_STYLE_BORDER_WIDTH;
    style.border_width = -1;
    assert(tinyui_obj_apply_style(label, TINYUI_PART_MAIN, TINYUI_STATE_DEFAULT, &style)
           == TINYUI_ERROR_INVALID_ARG);
    assert(ld->bgColor == bg_before);

    memset(&style, 0, sizeof(style));
    style.fields = TINYUI_STYLE_RADIUS;
    style.radius = -2;
    assert(tinyui_obj_apply_style(label, TINYUI_PART_MAIN, TINYUI_STATE_DEFAULT, &style)
           == TINYUI_ERROR_INVALID_ARG);

    memset(&style, 0, sizeof(style));
    style.fields = TINYUI_STYLE_PADDING;
    style.padding = -3;
    assert(tinyui_obj_apply_style(label, TINYUI_PART_MAIN, TINYUI_STATE_DEFAULT, &style)
           == TINYUI_ERROR_INVALID_ARG);

    memset(&style, 0, sizeof(style));
    style.fields = TINYUI_STYLE_FONT;
    style.font = 0;
    assert(tinyui_obj_apply_style(label, TINYUI_PART_MAIN, TINYUI_STATE_DEFAULT, &style)
           == TINYUI_ERROR_INVALID_ARG);
    assert(ld->bgColor == bg_before);
    assert(ld->textColor == text_before);

    (void)tinyui_obj_delete(label);
}

static void test_apply_style_unsupported_part_state_does_not_mutate(tinyui_obj_t *root)
{
    tinyui_obj_t *label = tinyui_label_create(root);
    ldLabel_t *ld;
    tinyui_style_t style;
    unsigned int bg_before;
    unsigned int text_before;

    assert(label != 0);
    ld = label_ld(label);
    bg_before = ld->bgColor;
    text_before = ld->textColor;

    memset(&style, 0, sizeof(style));
    style.fields = TINYUI_STYLE_BG_COLOR | TINYUI_STYLE_TEXT_COLOR;
    style.bg_color = UINT32_C(0x101010);
    style.text_color = UINT32_C(0x202020);

    assert(tinyui_obj_apply_style(label, TINYUI_PART_TEXT, TINYUI_STATE_DEFAULT, &style)
           == TINYUI_ERROR_NOT_SUPPORTED);
    assert(ld->bgColor == bg_before);
    assert(ld->textColor == text_before);

    assert(tinyui_obj_apply_style(label, TINYUI_PART_MAIN, TINYUI_STATE_PRESSED, &style)
           == TINYUI_ERROR_NOT_SUPPORTED);
    assert(ld->bgColor == bg_before);
    assert(ld->textColor == text_before);

    assert(tinyui_obj_apply_style(label, TINYUI_PART_KNOB, TINYUI_STATE_FOCUSED, &style)
           == TINYUI_ERROR_NOT_SUPPORTED);
    assert(ld->bgColor == bg_before);
    assert(ld->textColor == text_before);

    (void)tinyui_obj_delete(label);
}

static void test_apply_style_unsupported_fields_do_not_mutate(tinyui_obj_t *root)
{
    tinyui_obj_t *label = tinyui_label_create(root);
    ldLabel_t *ld;
    tinyui_style_t style;
    tinyui_font_t font;
    unsigned int bg_before;
    unsigned int text_before;

    assert(label != 0);
    ld = label_ld(label);
    bg_before = ld->bgColor;
    text_before = ld->textColor;
    assert(tinyui_font_from_builtin(TINYUI_FONT_ARIAL_12, &font) == TINYUI_OK);

    memset(&style, 0, sizeof(style));
    style.fields = TINYUI_STYLE_BG_COLOR | TINYUI_STYLE_FONT;
    style.bg_color = UINT32_C(0x334455);
    style.font = &font;
    assert(tinyui_obj_apply_style(label, TINYUI_PART_MAIN, TINYUI_STATE_DEFAULT, &style)
           == TINYUI_ERROR_NOT_SUPPORTED);
    assert(ld->bgColor == bg_before);
    assert(ld->textColor == text_before);

    memset(&style, 0, sizeof(style));
    style.fields = TINYUI_STYLE_BORDER_COLOR | TINYUI_STYLE_BORDER_WIDTH
                   | TINYUI_STYLE_RADIUS;
    style.border_color = UINT32_C(0x010203);
    style.border_width = 1;
    style.radius = 2;
    assert(tinyui_obj_apply_style(label, TINYUI_PART_MAIN, TINYUI_STATE_DEFAULT, &style)
           == TINYUI_ERROR_NOT_SUPPORTED);
    assert(ld->bgColor == bg_before);

    tinyui_font_deinit(&font);
    (void)tinyui_obj_delete(label);
}

static void test_apply_style_main_default_maps_bg_text_opacity(tinyui_obj_t *root)
{
    tinyui_obj_t *label = tinyui_label_create(root);
    ldLabel_t *ld;
    tinyui_style_t style;
    struct tinyui_test_allocator_stats before;
    struct tinyui_test_allocator_stats after;

    assert(label != 0);
    ld = label_ld(label);

    memset(&style, 0, sizeof(style));
    style.fields = TINYUI_STYLE_BG_COLOR | TINYUI_STYLE_TEXT_COLOR | TINYUI_STYLE_OPACITY;
    style.bg_color = UINT32_C(0x112233);
    style.text_color = UINT32_C(0x445566);
    style.opacity = UINT8_C(200);

    before = tinyui_test_allocator_snapshot();
    assert(tinyui_obj_apply_style(label, TINYUI_PART_MAIN, TINYUI_STATE_DEFAULT, &style)
           == TINYUI_OK);
    after = tinyui_test_allocator_snapshot();
    assert_zero_alloc_delta(&before, &after);

    assert(ld->bgColor == test_rgb_to_ld(0x112233U));
    assert(ld->textColor == test_rgb_to_ld(0x445566U));
    assert(as_widget(label)->bg_color == 0x112233U);
    assert(as_widget(label)->text_color == 0x445566U);
    assert(as_widget(label)->opacity == 200);
    assert(((ldBase_t *)ld)->opacity == 200);

    (void)tinyui_obj_delete(label);
}

static void test_apply_style_does_not_retain_descriptor_pointer(tinyui_obj_t *root)
{
    tinyui_obj_t *label = tinyui_label_create(root);
    ldLabel_t *ld;
    unsigned int bg_after;
    unsigned int text_after;

    assert(label != 0);
    ld = label_ld(label);

    {
        tinyui_style_t style;

        memset(&style, 0, sizeof(style));
        style.fields = TINYUI_STYLE_BG_COLOR | TINYUI_STYLE_TEXT_COLOR;
        style.bg_color = UINT32_C(0x0A0B0C);
        style.text_color = UINT32_C(0x0D0E0F);
        assert(tinyui_obj_apply_style(label, TINYUI_PART_MAIN, TINYUI_STATE_DEFAULT, &style)
               == TINYUI_OK);
        /* 栈上 descriptor 即将销毁；对象不得依赖该指针。 */
        memset(&style, 0xA5, sizeof(style));
    }

    bg_after = ld->bgColor;
    text_after = ld->textColor;
    assert(bg_after == test_rgb_to_ld(0x0A0B0CU));
    assert(text_after == test_rgb_to_ld(0x0D0E0FU));

    (void)tinyui_obj_delete(label);
}

static void test_theme_set_get_is_caller_owned_borrow(void)
{
    tinyui_theme_t theme = {0};
    tinyui_theme_t other = {0};

    theme.colors[TINYUI_COLOR_BG] = UINT32_C(0x102030);
    theme.metrics[TINYUI_METRIC_PADDING] = 4;
    other.colors[TINYUI_COLOR_BG] = UINT32_C(0x405060);
    other.metrics[TINYUI_METRIC_PADDING] = 8;

    assert(tinyui_theme_set(0) == TINYUI_ERROR_INVALID_ARG);
    assert(tinyui_theme_set(&theme) == TINYUI_OK);
    assert(tinyui_theme_get() == &theme);
    assert(tinyui_theme_get()->colors[TINYUI_COLOR_BG] == UINT32_C(0x102030));

    /* 替换指针，不复制内容。 */
    assert(tinyui_theme_set(&other) == TINYUI_OK);
    assert(tinyui_theme_get() == &other);
    assert(tinyui_theme_get()->metrics[TINYUI_METRIC_PADDING] == 8);

    theme.colors[TINYUI_COLOR_BG] = UINT32_C(0xFFFFFF);
    assert(tinyui_theme_get()->colors[TINYUI_COLOR_BG] == UINT32_C(0x405060));
}

static void test_theme_apply_requires_set_theme_and_valid_obj(tinyui_obj_t *root)
{
    tinyui_theme_t theme = {0};
    tinyui_obj_t *label = tinyui_label_create(root);

    assert(label != 0);
    tinyui_internal_theme_reset();
    assert(tinyui_theme_get() == 0);
    assert(tinyui_theme_apply(label) == TINYUI_ERROR_INVALID_STATE);
    assert(tinyui_theme_apply(0) == TINYUI_ERROR_INVALID_OBJECT);

    theme.colors[TINYUI_COLOR_BG] = UINT32_C(0x111111);
    theme.colors[TINYUI_COLOR_TEXT_PRIMARY] = UINT32_C(0x222222);
    assert(tinyui_theme_set(&theme) == TINYUI_OK);
    assert(tinyui_theme_apply(0) == TINYUI_ERROR_INVALID_OBJECT);

    (void)tinyui_obj_delete(label);
}

static void test_theme_apply_maps_colors_without_tree_walk(tinyui_obj_t *root)
{
    tinyui_theme_t theme = {0};
    tinyui_obj_t *label_a = tinyui_label_create(root);
    tinyui_obj_t *label_b = tinyui_label_create(root);
    ldLabel_t *ld_a;
    ldLabel_t *ld_b;
    unsigned int b_bg_before;
    unsigned int b_text_before;
    struct tinyui_test_allocator_stats before;
    struct tinyui_test_allocator_stats after;

    assert(label_a != 0);
    assert(label_b != 0);
    ld_a = label_ld(label_a);
    ld_b = label_ld(label_b);
    b_bg_before = ld_b->bgColor;
    b_text_before = ld_b->textColor;

    theme.colors[TINYUI_COLOR_BG] = UINT32_C(0xABCDEF);
    theme.colors[TINYUI_COLOR_TEXT_PRIMARY] = UINT32_C(0x123456);
    theme.colors[TINYUI_COLOR_PANEL] = UINT32_C(0x654321);
    theme.colors[TINYUI_COLOR_BORDER] = UINT32_C(0x010101);
    theme.metrics[TINYUI_METRIC_PADDING] = 5;
    theme.metrics[TINYUI_METRIC_RADIUS] = 3;
    theme.metrics[TINYUI_METRIC_BORDER_WIDTH] = 1;
    theme.metrics[TINYUI_METRIC_CONTROL_HEIGHT] = 24;
    assert(tinyui_theme_set(&theme) == TINYUI_OK);

    before = tinyui_test_allocator_snapshot();
    assert(tinyui_theme_apply(label_a) == TINYUI_OK);
    after = tinyui_test_allocator_snapshot();
    assert_zero_alloc_delta(&before, &after);

    assert(ld_a->bgColor == test_rgb_to_ld(0xABCDEFU));
    assert(ld_a->textColor == test_rgb_to_ld(0x123456U));
    /* 替换/显式 apply 不遍历旧树：label_b 保持原样。 */
    assert(ld_b->bgColor == b_bg_before);
    assert(ld_b->textColor == b_text_before);

    /* 新对象不自动应用 theme（create 路径不在 Task 6 写面）。 */
    {
        tinyui_obj_t *label_c = tinyui_label_create(root);
        ldLabel_t *ld_c = label_ld(label_c);
        unsigned int c_bg = ld_c->bgColor;
        unsigned int c_text = ld_c->textColor;

        assert(c_bg != test_rgb_to_ld(0xABCDEFU)
               || c_text != test_rgb_to_ld(0x123456U)
               || c_bg == ld_b->bgColor);
        (void)tinyui_obj_delete(label_c);
        (void)c_bg;
        (void)c_text;
    }

    (void)tinyui_obj_delete(label_a);
    (void)tinyui_obj_delete(label_b);
}

static void test_theme_replace_does_not_mutate_existing_without_apply(tinyui_obj_t *root)
{
    tinyui_theme_t theme1 = {0};
    tinyui_theme_t theme2 = {0};
    tinyui_obj_t *label = tinyui_label_create(root);
    ldLabel_t *ld;
    unsigned int bg_after_first;

    assert(label != 0);
    ld = label_ld(label);

    theme1.colors[TINYUI_COLOR_BG] = UINT32_C(0x101010);
    theme1.colors[TINYUI_COLOR_TEXT_PRIMARY] = UINT32_C(0x202020);
    theme2.colors[TINYUI_COLOR_BG] = UINT32_C(0x303030);
    theme2.colors[TINYUI_COLOR_TEXT_PRIMARY] = UINT32_C(0x404040);

    assert(tinyui_theme_set(&theme1) == TINYUI_OK);
    assert(tinyui_theme_apply(label) == TINYUI_OK);
    bg_after_first = ld->bgColor;
    assert(bg_after_first == test_rgb_to_ld(0x101010U));

    assert(tinyui_theme_set(&theme2) == TINYUI_OK);
    assert(tinyui_theme_get() == &theme2);
    /* 仅替换 theme 指针，不自动刷新已有对象。 */
    assert(ld->bgColor == bg_after_first);

    assert(tinyui_theme_apply(label) == TINYUI_OK);
    assert(ld->bgColor == test_rgb_to_ld(0x303030U));
    assert(ld->textColor == test_rgb_to_ld(0x404040U));

    (void)tinyui_obj_delete(label);
}

static void test_theme_source_has_no_legacy_static_helpers(void)
{
    const char *path = tinyui_test_repo_path_from_file(__FILE__, "tinyui/src/theme/theme.c");

    assert(tinyui_test_source_lacks_function_definition(path, "tinyui_theme_rgb_to_ld_color") == 1);
    assert(tinyui_test_source_lacks_function_definition(path, "tinyui_theme_apply_widget_style") == 1);
    assert(tinyui_test_source_lacks_function_definition(path, "tinyui_theme_create") == 1);
    assert(tinyui_test_source_lacks_function_definition(path, "tinyui_theme_destroy") == 1);
}

int main(void)
{
    tinyui_obj_t *root;

    tinyui_deinit();
    tinyui_internal_theme_reset();
    tinyui_test_allocator_reset();

    assert(tinyui_init() == TINYUI_OK);
    root = tinyui_screen_create();
    assert(root != 0);

    test_theme_source_has_no_legacy_static_helpers();
    test_apply_style_rejects_null_and_range(root);
    test_apply_style_rejects_invalid_fields_before_mutation(root);
    test_apply_style_unsupported_part_state_does_not_mutate(root);
    test_apply_style_unsupported_fields_do_not_mutate(root);
    test_apply_style_main_default_maps_bg_text_opacity(root);
    test_apply_style_does_not_retain_descriptor_pointer(root);
    test_theme_set_get_is_caller_owned_borrow();
    test_theme_apply_requires_set_theme_and_valid_obj(root);
    test_theme_apply_maps_colors_without_tree_walk(root);
    test_theme_replace_does_not_mutate_existing_without_apply(root);

    tinyui_deinit();
    tinyui_internal_theme_reset();
    return 0;
}
