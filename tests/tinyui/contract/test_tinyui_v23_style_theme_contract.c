#include "core/obj.h"
#include "core/result.h"
#include "core/runtime.h"
#include "style/style.h"
#include "theme/theme.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

_Static_assert(TINYUI_STYLE_BG_COLOR == (UINT32_C(1) << 0),
               "style field order changed");
_Static_assert(TINYUI_STYLE_FONT == (UINT32_C(1) << 7),
               "style field mask must have eight bits");
_Static_assert(TINYUI_PART_MAIN == 0 && TINYUI_PART_TRACK == 4,
               "style part ABI changed");
_Static_assert(TINYUI_STATE_DEFAULT == 0 && TINYUI_STATE_FOCUSED == 4,
               "style state ABI changed");
_Static_assert(sizeof(((tinyui_theme_t *)0)->colors) ==
                   TINYUI_COLOR_COUNT * sizeof(uint32_t),
               "theme colors must be a fixed value array");
_Static_assert(sizeof(((tinyui_theme_t *)0)->metrics) ==
                   TINYUI_METRIC_COUNT * sizeof(int16_t),
               "theme metrics must be a fixed value array");

static void test_style_is_value_owned_by_caller(void)
{
    tinyui_style_t style = {
        .fields = TINYUI_STYLE_BG_COLOR | TINYUI_STYLE_TEXT_COLOR |
                  TINYUI_STYLE_BORDER_COLOR | TINYUI_STYLE_BORDER_WIDTH |
                  TINYUI_STYLE_RADIUS | TINYUI_STYLE_PADDING |
                  TINYUI_STYLE_OPACITY | TINYUI_STYLE_FONT,
        .bg_color = UINT32_C(0x112233),
        .text_color = UINT32_C(0x445566),
        .border_color = UINT32_C(0x778899),
        .border_width = 1,
        .radius = 2,
        .padding = 3,
        .opacity = UINT8_C(255),
        .font = (const tinyui_font_t *)(uintptr_t)1,
    };

    assert(tinyui_obj_apply_style(0, TINYUI_PART_MAIN,
                                  TINYUI_STATE_DEFAULT, &style) ==
           TINYUI_ERROR_INVALID_OBJECT);
    assert(tinyui_obj_apply_style((tinyui_obj_t *)(uintptr_t)1,
                                  TINYUI_PART_MAIN,
                                  TINYUI_STATE_DEFAULT, 0) ==
           TINYUI_ERROR_INVALID_ARG);
    assert(tinyui_obj_apply_style((tinyui_obj_t *)(uintptr_t)1,
                                  (tinyui_part_t)99,
                                  TINYUI_STATE_DEFAULT, &style) ==
           TINYUI_ERROR_OUT_OF_RANGE);
    assert(tinyui_obj_apply_style((tinyui_obj_t *)(uintptr_t)1,
                                  TINYUI_PART_MAIN,
                                  (tinyui_state_t)99, &style) ==
           TINYUI_ERROR_OUT_OF_RANGE);
    assert(tinyui_obj_apply_style((tinyui_obj_t *)(uintptr_t)1,
                                  TINYUI_PART_MAIN,
                                  TINYUI_STATE_DEFAULT, &style) ==
           TINYUI_ERROR_NOT_SUPPORTED);
}

static void test_theme_is_a_caller_owned_descriptor(void)
{
    tinyui_theme_t theme = {0};

    theme.colors[TINYUI_COLOR_BG] = UINT32_C(0x102030);
    theme.metrics[TINYUI_METRIC_PADDING] = 4;
    assert(tinyui_theme_set(0) == TINYUI_ERROR_INVALID_ARG);
    assert(tinyui_theme_set(&theme) == TINYUI_OK);
    assert(tinyui_theme_get() == &theme);
    assert(tinyui_theme_get()->colors[TINYUI_COLOR_BG] == UINT32_C(0x102030));

    tinyui_deinit();
    assert(tinyui_theme_get() == 0);
    assert(tinyui_theme_set(&theme) == TINYUI_OK);
    assert(tinyui_theme_apply(0) == TINYUI_ERROR_INVALID_OBJECT);
    assert(tinyui_theme_apply((tinyui_obj_t *)(uintptr_t)1) ==
           TINYUI_ERROR_NOT_SUPPORTED);
}

static void test_diagnostics_returns_static_text(void)
{
#if defined(TINYUI_ENABLE_DIAGNOSTICS) && TINYUI_ENABLE_DIAGNOSTICS
    assert(strcmp(tinyui_last_error_message(), "TINYUI_ERROR_NOT_SUPPORTED") == 0);
    const char *first = tinyui_last_error_message();
    const char *second = tinyui_last_error_message();

    assert(first != 0);
    assert(second == first);
#endif
}

int main(void)
{
    test_style_is_value_owned_by_caller();
    test_theme_is_a_caller_owned_descriptor();
    test_diagnostics_returns_static_text();
    return 0;
}
