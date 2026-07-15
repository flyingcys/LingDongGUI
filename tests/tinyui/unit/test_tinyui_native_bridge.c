#include "tinyui.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/porting/ldConfig.h"
#include "internal.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

extern int tinyui_runtime_bridge_has_scene(const struct tinyui_app *app);
extern int16_t tinyui_runtime_bridge_map_pointer_axis(int value, int window_extent, int target_extent);
extern int tinyui_runtime_bridge_bridge_pointer_from_port(struct tinyui_app *app,
                                                          int window_width,
                                                          int window_height);
extern int tinyui_runtime_bridge_commit_pointer_event(struct tinyui_app *app,
                                                      int window_width,
                                                      int window_height,
                                                      int x,
                                                      int y,
                                                      int pressed);
extern bool ldCfgTouchGetPoint(int16_t *x, int16_t *y);

static const char *test_repo_path(const char *relative_path)
{
    static char path[2048];
    char base[2048];
    char *tests_dir;

    snprintf(base, sizeof(base), "%s", __FILE__);
    tests_dir = strstr(base, "tests/tinyui/unit/");
    assert(tests_dir != 0);
    *tests_dir = '\0';
    snprintf(path, sizeof(path), "%s%s", base, relative_path);
    return path;
}

static void reset_touch_probe(void)
{
    ldCfgTouchSetPoint(-1, -1, false);
}

static void assert_source_lacks_function_definition(const char *path, const char *function_name)
{
    char command[1024];

    snprintf(command,
             sizeof(command),
             "rg -n \"^[[:space:]]*static[[:space:]].*%s[[:space:]]*\\(\" %s >/dev/null",
             function_name,
             path);
    if (system(command) == 0) {
        fprintf(stderr, "unexpected backend-local function still present: %s in %s\n", function_name, path);
        abort();
    }
}

static void test_native_image_preserves_tile_and_mask_pointers(void)
{
    void *tile = (void *)(uintptr_t)0x1000U;
    void *mask = (void *)(uintptr_t)0x2000U;
    struct tinyui_native_image image = tinyui_native_image_wrap(tile, mask, 0x123456U);

    assert(image.tile == tile);
    assert(image.mask == mask);
    assert(image.mask_color == 0x123456U);
}

static void test_native_font_preserves_font_pointer(void)
{
    void *font = (void *)(uintptr_t)0x3000U;
    struct tinyui_native_font native_font = tinyui_native_font_wrap(font);

    assert(native_font.font == font);
}

static void test_native_align_maps_all_ldgrid_align_values(void)
{
    assert(tinyui_native_align_to_ld_grid(TINYUI_NATIVE_ALIGN_START) == ldGridAlignStart);
    assert(tinyui_native_align_to_ld_grid(TINYUI_NATIVE_ALIGN_CENTER) == ldGridAlignCenter);
    assert(tinyui_native_align_to_ld_grid(TINYUI_NATIVE_ALIGN_END) == ldGridAlignEnd);
    assert(tinyui_native_align_to_ld_grid(TINYUI_NATIVE_ALIGN_STRETCH) == ldGridAlignStretch);
    assert(tinyui_native_align_to_ld_grid(TINYUI_NATIVE_ALIGN_SPACE_EVENLY) == ldGridAlignSpaceEvenly);
    assert(tinyui_native_align_to_ld_grid(TINYUI_NATIVE_ALIGN_SPACE_AROUND) == ldGridAlignSpaceAround);
    assert(tinyui_native_align_to_ld_grid(TINYUI_NATIVE_ALIGN_SPACE_BETWEEN) == ldGridAlignSpaceBetween);
}

static void test_native_nav_dir_maps_all_ld_nav_values(void)
{
    assert(tinyui_native_nav_dir_to_ld(TINYUI_NATIVE_NAV_LEFT) == NAV_LEFT);
    assert(tinyui_native_nav_dir_to_ld(TINYUI_NATIVE_NAV_RIGHT) == NAV_RIGHT);
    assert(tinyui_native_nav_dir_to_ld(TINYUI_NATIVE_NAV_UP) == NAV_UP);
    assert(tinyui_native_nav_dir_to_ld(TINYUI_NATIVE_NAV_DOWN) == NAV_DOWN);
    assert(tinyui_native_nav_dir_to_ld(TINYUI_NATIVE_NAV_ENTER) == NAV_ENTER);
    assert(tinyui_native_nav_dir_to_ld(TINYUI_NATIVE_NAV_BACK) == NAV_BACK);
}

static void test_internal_native_helpers_no_longer_use_tinyui_prefix(void)
{
    assert_source_lacks_function_definition(
        test_repo_path("tinyui/src/core/native.c"),
        "tinyui_native_align_to_ld_grid");
    assert_source_lacks_function_definition(
        test_repo_path("tinyui/src/core/native.c"),
        "tinyui_native_signal_to_ld");
    assert_source_lacks_function_definition(
        test_repo_path("tinyui/src/core/native.c"),
        "tinyui_native_readback_policy_to_backend");
    assert_source_lacks_function_definition(
        test_repo_path("tinyui/src/core/widget.c"),
        "tinyui_native_nav_dir_to_ld");
}

static void test_runtime_bridge_reports_scene_presence(void)
{
    struct tinyui_app *app = tinyui_runtime_internal_app_create();

    assert(app != NULL);
    assert(tinyui_runtime_bridge_has_scene(app) == 1);
    tinyui_runtime_internal_app_destroy(app);
}

static void test_runtime_bridge_initializes_legacy_dirty_region(void)
{
    struct tinyui_app *app = tinyui_runtime_internal_app_create();

    assert(app != NULL);
    assert(app->ld_scene != NULL);
    assert(app->ld_scene->use_as__arm_2d_scene_t.ptDirtyRegion ==
           &app->ld_scene->tDirtyRegionItem);

    tinyui_runtime_internal_app_destroy(app);
}

static void test_runtime_bridge_pointer_axis_clamps_into_ld_touch_range(void)
{
    (void)tinyui_runtime_bridge_map_pointer_axis(100, 640, 480);
    assert(tinyui_runtime_bridge_map_pointer_axis(-12, 640, 480) == 0);
    assert(tinyui_runtime_bridge_map_pointer_axis(123, 640, 480) == 123);
    assert(tinyui_runtime_bridge_map_pointer_axis(99999, 640, 480) == 32767);
}

static void test_runtime_bridge_pointer_commit_updates_input_state_and_ld_touch(void)
{
    struct tinyui_app *app = tinyui_runtime_internal_app_create();
    int x = 0;
    int y = 0;
    int pressed = 0;
    int16_t touch_x = -1;
    int16_t touch_y = -1;

    assert(app != NULL);
    reset_touch_probe();

    assert(tinyui_runtime_bridge_commit_pointer_event(app, 640, 480, 25, 35, 1) == 0);
    assert(tinyui_input_get_pointer(app, &x, &y, &pressed) == 0);
    assert(x == 25);
    assert(y == 35);
    assert(pressed == 1);
    assert(ldCfgTouchGetPoint(&touch_x, &touch_y) == true);
    assert(touch_x == 25);
    assert(touch_y == 35);

    reset_touch_probe();
    assert(tinyui_runtime_bridge_commit_pointer_event(app, 640, 480, -5, 50000, 0) == 0);
    assert(tinyui_input_get_pointer(app, &x, &y, &pressed) == 0);
    assert(x == 0);
    assert(y == 32767);
    assert(pressed == 0);
    touch_x = 123;
    touch_y = 456;
    assert(ldCfgTouchGetPoint(&touch_x, &touch_y) == false);
    assert(touch_x == -1);
    assert(touch_y == -1);

    tinyui_runtime_internal_app_destroy(app);
}

static void test_runtime_bridge_pointer_bridge_reads_existing_input_state(void)
{
    struct tinyui_app *app = tinyui_runtime_internal_app_create();
    int16_t touch_x = -1;
    int16_t touch_y = -1;

    assert(app != NULL);
    reset_touch_probe();

    assert(tinyui_input_push_pointer(app, 44, 66, 1) == 0);
    assert(tinyui_runtime_bridge_bridge_pointer_from_port(app, 800, 600) == 0);
    assert(ldCfgTouchGetPoint(&touch_x, &touch_y) == true);
    assert(touch_x == 44);
    assert(touch_y == 66);

    tinyui_runtime_internal_app_destroy(app);
}

static void test_runtime_bridge_pointer_helpers_reject_null_app(void)
{
    int16_t touch_x = 777;
    int16_t touch_y = 888;

    reset_touch_probe();
    assert(tinyui_runtime_bridge_bridge_pointer_from_port(NULL, 640, 480) == -1);
    assert(tinyui_runtime_bridge_commit_pointer_event(NULL, 640, 480, 1, 2, 1) == -1);
    assert(ldCfgTouchGetPoint(&touch_x, &touch_y) == false);
    assert(touch_x == -1);
    assert(touch_y == -1);
}

static void test_backend_app_no_longer_defines_pointer_bridge_helpers_locally(void)
{
    const char *test_runtime_bridge_source = test_repo_path("tinyui/src/core/runtime_bridge.c");

    assert_source_lacks_function_definition(test_runtime_bridge_source, "tinyui_backend_push_pointer_to_port");
    assert_source_lacks_function_definition(test_runtime_bridge_source, "tinyui_backend_bridge_pointer_from_port");
    assert_source_lacks_function_definition(test_runtime_bridge_source, "tinyui_backend_commit_pointer_event");
    assert_source_lacks_function_definition(test_runtime_bridge_source, "tinyui_backend_map_pointer_axis");
}

int main(void)
{
    test_native_image_preserves_tile_and_mask_pointers();
    test_native_font_preserves_font_pointer();
    test_native_align_maps_all_ldgrid_align_values();
    test_native_nav_dir_maps_all_ld_nav_values();
    test_internal_native_helpers_no_longer_use_tinyui_prefix();
    test_runtime_bridge_reports_scene_presence();
    test_runtime_bridge_initializes_legacy_dirty_region();
    test_runtime_bridge_pointer_axis_clamps_into_ld_touch_range();
    test_runtime_bridge_pointer_commit_updates_input_state_and_ld_touch();
    test_runtime_bridge_pointer_bridge_reads_existing_input_state();
    test_runtime_bridge_pointer_helpers_reject_null_app();
    test_backend_app_no_longer_defines_pointer_bridge_helpers_locally();
    return 0;
}
