#include "internal.h"
#include "internal/runtime_pools.h"
#include "resource/image_source.h"

#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldWindow.h"

#include <stdio.h>
#include <stdint.h>

/* arm_2d_tile_t is the private tile storage target for image/mask descriptors. */
#include "arm_2d.h"

static int assert_size_le(const char *name, size_t actual, size_t limit)
{
    if (actual > limit) {
        fprintf(stderr,
                "%s size regression: expected <= %zu, got %zu\n",
                name,
                limit,
                actual);
        return 1;
    }
    return 0;
}

static int assert_size_ge(const char *name, size_t actual, size_t limit)
{
    if (actual < limit) {
        fprintf(stderr,
                "%s size regression: expected >= %zu, got %zu\n",
                name,
                limit,
                actual);
        return 1;
    }
    return 0;
}

int main(void)
{
    int failed = 0;
    size_t timer_pool_bytes;
    size_t event_pool_bytes;
    size_t bookkeeping_bytes;
    size_t pool_sum_bytes;
    size_t image_source_bytes;
    size_t font_bytes;
    size_t image_private_bytes;
    size_t mask_private_bytes;

#define PRINT_SIZE(name, type) printf("TINYUI_SIZEOF_%s=%zu\n", name, sizeof(type))
    PRINT_SIZE("WIDGET_WRAPPER", struct tinyui_widget);
    PRINT_SIZE("WINDOW_WRAPPER", struct tinyui_window);
    PRINT_SIZE("BACKGROUND_WRAPPER", struct tinyui_background);
    PRINT_SIZE("CANVAS_WRAPPER", struct tinyui_canvas);
    PRINT_SIZE("LABEL_WRAPPER", struct tinyui_label);
    PRINT_SIZE("BUTTON_WRAPPER", struct tinyui_button);
    PRINT_SIZE("KEYBOARD_WRAPPER", struct tinyui_keyboard);
    PRINT_SIZE("CHECKBOX_WRAPPER", struct tinyui_checkbox);
    PRINT_SIZE("SWITCH_WRAPPER", struct tinyui_switch);
    PRINT_SIZE("SLIDER_WRAPPER", struct tinyui_slider);
    PRINT_SIZE("PROGRESS_BAR_WRAPPER", struct tinyui_progress_bar);
    PRINT_SIZE("ANIMATION_WRAPPER", struct tinyui_animation);
    PRINT_SIZE("ARC_WRAPPER", struct tinyui_arc);
    PRINT_SIZE("GAUGE_WRAPPER", struct tinyui_gauge);
    PRINT_SIZE("ICON_SLIDER_WRAPPER", struct tinyui_icon_slider);
    PRINT_SIZE("RADIAL_MENU_WRAPPER", struct tinyui_radial_menu);
    PRINT_SIZE("QRCODE_WRAPPER", struct tinyui_qrcode);
    PRINT_SIZE("PROGRESS_WHEEL_WRAPPER", struct tinyui_progress_wheel);
    PRINT_SIZE("DATE_TIME_WRAPPER", struct tinyui_date_time);
    PRINT_SIZE("CALENDAR_WRAPPER", struct tinyui_calendar);
    PRINT_SIZE("CLOCK_WRAPPER", struct tinyui_clock);
    PRINT_SIZE("LIST_WRAPPER", struct tinyui_list);
    PRINT_SIZE("MESSAGE_BOX_WRAPPER", struct tinyui_message_box);
    PRINT_SIZE("TEXT_WRAPPER", struct tinyui_text);
    PRINT_SIZE("LINE_EDIT_WRAPPER", struct tinyui_line_edit);
    PRINT_SIZE("COMBO_BOX_WRAPPER", struct tinyui_combo_box);
    PRINT_SIZE("SCROLL_SELECTER_WRAPPER", struct tinyui_scroll_selecter);
    PRINT_SIZE("IMAGE_WRAPPER", struct tinyui_image);
    PRINT_SIZE("GRAPH_WRAPPER", struct tinyui_graph);
    PRINT_SIZE("TABLE_WRAPPER", struct tinyui_table);
    printf("TINYUI_SIZEOF_SWITCH_WRAPPER_DELTA=%zu\n",
           sizeof(struct tinyui_switch) - sizeof(struct tinyui_widget));
    /* ldWindow_t is the concrete backend container used by TinyUI roots. */
    PRINT_SIZE("BACKEND_WIDGET_LD_WINDOW", ldWindow_t);
    PRINT_SIZE("LEGACY_TIMER_NODE", struct tinyui_app_timer);
    PRINT_SIZE("LEGACY_EVENT_CALLBACK_STORAGE", xBtnInfo_t);
    PRINT_SIZE("RUNTIME_BOOKKEEPING", struct tinyui_app);
#undef PRINT_SIZE

    timer_pool_bytes = sizeof(struct tinyui_timer_pool);
    event_pool_bytes = sizeof(struct tinyui_event_callback_pool);
    bookkeeping_bytes = sizeof(struct tinyui_runtime_bookkeeping);
    pool_sum_bytes = timer_pool_bytes + event_pool_bytes + bookkeeping_bytes;
    image_source_bytes = sizeof(struct tinyui_image_source);
    font_bytes = sizeof(struct tinyui_font);
    image_private_bytes = sizeof(((struct tinyui_image_source *)0)->_image_private);
    mask_private_bytes = sizeof(((struct tinyui_image_source *)0)->_mask_private);

    /* Use non-SIZEOF markers so the M0 object-overhead parser ignores extras. */
    printf("TINYUI_POOL_TIMER_BYTES=%zu\n", timer_pool_bytes);
    printf("TINYUI_POOL_EVENT_CALLBACK_BYTES=%zu\n", event_pool_bytes);
    printf("TINYUI_POOL_BOOKKEEPING_BYTES=%zu\n", bookkeeping_bytes);
    printf("TINYUI_POOL_SUM_BYTES=%zu\n", pool_sum_bytes);
    printf("TINYUI_DESC_IMAGE_SOURCE_BYTES=%zu\n", image_source_bytes);
    printf("TINYUI_DESC_FONT_BYTES=%zu\n", font_bytes);
    printf("TINYUI_DESC_IMAGE_PRIVATE_BYTES=%zu\n", image_private_bytes);
    printf("TINYUI_DESC_MASK_PRIVATE_BYTES=%zu\n", mask_private_bytes);
    printf("TINYUI_DESC_ARM_2D_TILE_BYTES=%zu\n", sizeof(arm_2d_tile_t));

    /* Hard absolute budgets for base wrapper and backend container. */
    failed |= assert_size_le("widget wrapper", sizeof(struct tinyui_widget), 192U);
    failed |= assert_size_le("backend widget", sizeof(ldWindow_t), 512U);

    /* M2 sample wrappers: keep absolute ceilings aligned with M0 baseline + 8 B. */
    failed |= assert_size_le("label wrapper", sizeof(struct tinyui_label), 208U);
    failed |= assert_size_le("button wrapper", sizeof(struct tinyui_button), 288U);
    failed |= assert_size_le("checkbox wrapper", sizeof(struct tinyui_checkbox), 232U);
    failed |= assert_size_le("slider wrapper", sizeof(struct tinyui_slider), 240U);

    /* Private tile storage must hold one arm_2d_tile_t on every host ABI. */
    failed |= assert_size_ge("image private storage",
                             image_private_bytes,
                             sizeof(arm_2d_tile_t));
    failed |= assert_size_ge("mask private storage",
                             mask_private_bytes,
                             sizeof(arm_2d_tile_t));
    failed |= assert_size_ge("image private storage",
                             image_private_bytes,
                             6U * sizeof(uintptr_t));
    failed |= assert_size_ge("mask private storage",
                             mask_private_bytes,
                             6U * sizeof(uintptr_t));

    /*
     * 32-bit hard RAM budget is only enforceable on a 32-bit ABI. Host 64-bit
     * sizes are printed for diagnostics and must not be recorded as a pass of
     * the 1024 B / 80 B / 16 B gates.
     */
#if defined(UINTPTR_MAX) && defined(UINT32_MAX) && (UINTPTR_MAX == UINT32_MAX)
    failed |= assert_size_le("32-bit runtime pool sum", pool_sum_bytes, 1024U);
    failed |= assert_size_le("32-bit image source descriptor",
                             image_source_bytes,
                             80U);
    failed |= assert_size_le("32-bit font descriptor", font_bytes, 16U);
    printf("TINYUI_ABI32_POOL_BUDGET_OK sum=%zu\n", pool_sum_bytes);
    printf("TINYUI_ABI32_DESC_BUDGET_OK image=%zu font=%zu\n",
           image_source_bytes,
           font_bytes);
#else
    printf("TINYUI_ABI32_POOL_BUDGET_HOST_ONLY sum=%zu (not 32-bit ABI)\n",
           pool_sum_bytes);
    printf("TINYUI_ABI32_DESC_BUDGET_HOST_ONLY image=%zu font=%zu (not 32-bit ABI)\n",
           image_source_bytes,
           font_bytes);
#endif

    return failed;
}
