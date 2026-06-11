#include "picoui/picoui.h"
#include "../../../src/gui/ldBase.h"
#include "internal.h"

#include <assert.h>
#include <stdint.h>

extern int picoui_runtime_bridge_has_scene(const struct picoui_app *app);

static void test_native_image_preserves_tile_and_mask_pointers(void)
{
    void *tile = (void *)(uintptr_t)0x1000U;
    void *mask = (void *)(uintptr_t)0x2000U;
    struct picoui_native_image image = picoui_native_image_wrap(tile, mask, 0x123456U);

    assert(image.tile == tile);
    assert(image.mask == mask);
    assert(image.mask_color == 0x123456U);
}

static void test_native_font_preserves_font_pointer(void)
{
    void *font = (void *)(uintptr_t)0x3000U;
    struct picoui_native_font native_font = picoui_native_font_wrap(font);

    assert(native_font.font == font);
}

static void test_native_align_maps_all_ldgrid_align_values(void)
{
    assert(picoui_native_align_to_ld_grid(PICOUI_NATIVE_ALIGN_START) == ldGridAlignStart);
    assert(picoui_native_align_to_ld_grid(PICOUI_NATIVE_ALIGN_CENTER) == ldGridAlignCenter);
    assert(picoui_native_align_to_ld_grid(PICOUI_NATIVE_ALIGN_END) == ldGridAlignEnd);
    assert(picoui_native_align_to_ld_grid(PICOUI_NATIVE_ALIGN_STRETCH) == ldGridAlignStretch);
    assert(picoui_native_align_to_ld_grid(PICOUI_NATIVE_ALIGN_SPACE_EVENLY) == ldGridAlignSpaceEvenly);
    assert(picoui_native_align_to_ld_grid(PICOUI_NATIVE_ALIGN_SPACE_AROUND) == ldGridAlignSpaceAround);
    assert(picoui_native_align_to_ld_grid(PICOUI_NATIVE_ALIGN_SPACE_BETWEEN) == ldGridAlignSpaceBetween);
}

static void test_native_nav_dir_maps_all_ld_nav_values(void)
{
    assert(picoui_native_nav_dir_to_ld(PICOUI_NATIVE_NAV_LEFT) == NAV_LEFT);
    assert(picoui_native_nav_dir_to_ld(PICOUI_NATIVE_NAV_RIGHT) == NAV_RIGHT);
    assert(picoui_native_nav_dir_to_ld(PICOUI_NATIVE_NAV_UP) == NAV_UP);
    assert(picoui_native_nav_dir_to_ld(PICOUI_NATIVE_NAV_DOWN) == NAV_DOWN);
    assert(picoui_native_nav_dir_to_ld(PICOUI_NATIVE_NAV_ENTER) == NAV_ENTER);
    assert(picoui_native_nav_dir_to_ld(PICOUI_NATIVE_NAV_BACK) == NAV_BACK);
}

static void test_native_signal_maps_all_ld_signal_values(void)
{
    assert(picoui_native_signal_to_ld(PICOUI_NATIVE_SIGNAL_NONE) == SIGNAL_NO_OPERATION);
    assert(picoui_native_signal_to_ld(PICOUI_NATIVE_SIGNAL_PRESS) == SIGNAL_PRESS);
    assert(picoui_native_signal_to_ld(PICOUI_NATIVE_SIGNAL_HOLD_DOWN) == SIGNAL_HOLD_DOWN);
    assert(picoui_native_signal_to_ld(PICOUI_NATIVE_SIGNAL_RELEASE) == SIGNAL_RELEASE);
    assert(picoui_native_signal_to_ld(PICOUI_NATIVE_SIGNAL_CLICKED_ITEM) == SIGNAL_CLICKED_ITEM);
    assert(picoui_native_signal_to_ld(PICOUI_NATIVE_SIGNAL_FINISHED) == SIGNAL_FINISHED);
    assert(picoui_native_signal_to_ld(PICOUI_NATIVE_SIGNAL_VALUE_CHANGED) == SIGNAL_VALUE_CHANGED);
}

static void test_native_readback_policy_maps_backend_truth_modes(void)
{
    assert(picoui_native_readback_policy_to_backend(PICOUI_NATIVE_READBACK_NOT_APPLICABLE)
           == PICOUI_BACKEND_DATA_TRUTH_NOT_APPLICABLE);
    assert(picoui_native_readback_policy_to_backend(PICOUI_NATIVE_READBACK_BACKEND_FIELD)
           == PICOUI_BACKEND_DATA_TRUTH_BACKEND_VALUE);
    assert(picoui_native_readback_policy_to_backend(PICOUI_NATIVE_READBACK_BACKEND_COMMITTED)
           == PICOUI_BACKEND_DATA_TRUTH_BACKEND_VALUE);
}

static void test_runtime_bridge_reports_scene_presence(void)
{
    struct picoui_app *app = picoui_app_create();

    assert(app != NULL);
    assert(picoui_runtime_bridge_has_scene(app) == 1);
    picoui_app_destroy(app);
}

int main(void)
{
    test_native_image_preserves_tile_and_mask_pointers();
    test_native_font_preserves_font_pointer();
    test_native_align_maps_all_ldgrid_align_values();
    test_native_nav_dir_maps_all_ld_nav_values();
    test_native_signal_maps_all_ld_signal_values();
    test_native_readback_policy_maps_backend_truth_modes();
    test_runtime_bridge_reports_scene_presence();
    return 0;
}
