#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "ldBase.h"
#include "ldLabel.h"
#include "ldWindow.h"

extern void ld_test_set_system_timestamp(int64_t timestamp);

static void init_named_root(ldWindow_t *root, uint16_t name_id, int16_t width, int16_t height)
{
    *root = (ldWindow_t){0};
    root->use_as__ldBase_t.widgetType = widgetTypeWindow;
    root->use_as__ldBase_t.nameId = name_id;
    root->use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iWidth = width;
    root->use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iHeight = height;
}

static void test_calendar_helpers(void)
{
    uint8_t hour = 0;
    uint8_t minute = 0;
    uint8_t second = 0;
    uint16_t year = 0;
    uint8_t month = 0;
    uint8_t day = 0;

    ld_test_set_system_timestamp(90061000LL);
    ldBaseGetTime(&hour, &minute, &second);
    assert(hour == 1);
    assert(minute == 1);
    assert(second == 1);

    ldBaseGetDate(&year, &month, &day);
    assert(year == 1970);
    assert(month == 1);
    assert(day == 2);

    assert(ldBaseGetWeek(1970, 1, 1) == 4);
    assert(ldBaseGetWeek(1970, 1, 4) == 0);
}

static void test_tree_and_lookup_helpers(void)
{
    ldWindow_t root = {0};
    ldWindow_t container = {0};
    ldLabel_t leaf_a = {0};
    ldLabel_t leaf_b = {0};

    init_named_root(&root, 10, 100, 60);
    container.use_as__ldBase_t.nameId = 11;
    leaf_a.use_as__ldBase_t.nameId = 12;
    leaf_b.use_as__ldBase_t.nameId = 13;

    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&container);
    ldBaseNodeAdd((arm_2d_control_node_t *)&container, (arm_2d_control_node_t *)&leaf_a);
    ldBaseNodeAdd((arm_2d_control_node_t *)&container, (arm_2d_control_node_t *)&leaf_b);

    assert(ldBaseGetWidget((arm_2d_control_node_t *)&root, 12) == &leaf_a);
    assert(ldBaseGetWidget((arm_2d_control_node_t *)&root, 99) == NULL);
    assert(ldBaseGetParent((ldBase_t *)&container) == (ldBase_t *)&root);
    assert(ldBaseGetChildList((ldBase_t *)&container) == (ldBase_t *)&leaf_a);
    assert(ldBaseGetNextSibling((ldBase_t *)&leaf_a) == (ldBase_t *)&leaf_b);
    assert(ldBaseGetChildCount((ldBase_t *)&container) == 2);
    assert(ldBaseGetRootNode((arm_2d_control_node_t *)&leaf_b) == (arm_2d_control_node_t *)&root);
}

static void test_visibility_selection_and_basic_getters(void)
{
    ldWindow_t root = {0};
    ldLabel_t child = {0};

    init_named_root(&root, 1, 100, 50);
    child.use_as__ldBase_t.widgetType = widgetTypeLabel;
    child.use_as__ldBase_t.nameId = 2;
    child.use_as__ldBase_t.opacity = 10;

    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&child);

    assert(ldBaseGetWidgetType((ldBase_t *)&child) == widgetTypeLabel);
    assert(ldBaseGetNameId((ldBase_t *)&child) == 2);
    assert(ldBaseGetOpacity((ldBase_t *)&child) == 10);
    assert(ldBaseIsHidden((ldBase_t *)&child) == false);
    assert(ldBaseIsSelectable((ldBase_t *)&child) == false);
    assert(ldBaseIsSelected((ldBase_t *)&child) == false);
    assert(ldBaseIsCorner((ldBase_t *)&child) == false);

    ldBaseSetOpacity((ldBase_t *)&child, 123);
    assert(ldBaseGetOpacity((ldBase_t *)&child) == 123);
    assert(child.use_as__ldBase_t.isDirtyRegionUpdate == true);

    child.use_as__ldBase_t.isDirtyRegionUpdate = false;
    ldBaseSetSelectable((ldBase_t *)&child, true);
    assert(ldBaseIsSelectable((ldBase_t *)&child) == true);
    ldBaseSetSelect((ldBase_t *)&child, true);
    assert(ldBaseIsSelected((ldBase_t *)&child) == true);
    assert(child.use_as__ldBase_t.isDirtyRegionUpdate == true);

    child.use_as__ldBase_t.isDirtyRegionUpdate = false;
    ldBaseSetCorner((ldBase_t *)&child, true);
    assert(ldBaseIsCorner((ldBase_t *)&child) == true);
    assert(child.use_as__ldBase_t.isDirtyRegionUpdate == true);

    child.use_as__ldBase_t.isDirtyRegionUpdate = false;
    ldBaseSetHidden((ldBase_t *)&child, true);
    assert(ldBaseIsHidden((ldBase_t *)&child) == true);
    assert(child.use_as__ldBase_t.isDirtyRegionUpdate == true);

    child.use_as__ldBase_t.isDirtyRegionUpdate = false;
    ldBaseSetHidden((ldBase_t *)&child, false);
    assert(ldBaseIsHidden((ldBase_t *)&child) == false);
    assert(child.use_as__ldBase_t.isDirtyRegionUpdate == true);
}

static void test_location_and_region_helpers(void)
{
    ldWindow_t root = {0};
    ldWindow_t container = {0};
    ldLabel_t child = {0};

    init_named_root(&root, 1, 100, 80);
    root.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation = (arm_2d_location_t){10, 20};
    container.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation = (arm_2d_location_t){5, 6};
    container.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize = (arm_2d_size_t){60, 40};
    child.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation = (arm_2d_location_t){2, 3};
    child.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize = (arm_2d_size_t){20, 10};

    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&container);
    ldBaseNodeAdd((arm_2d_control_node_t *)&container, (arm_2d_control_node_t *)&child);

    assert(ldBaseGetRelativeLocation((ldBase_t *)&child, (arm_2d_location_t){18, 30}).iX == 1);
    assert(ldBaseGetRelativeLocation((ldBase_t *)&child, (arm_2d_location_t){18, 30}).iY == 1);
    assert(ldBaseGetAbsoluteLocation((ldBase_t *)&child, (arm_2d_location_t){1, 1}).iX == 18);
    assert(ldBaseGetAbsoluteLocation((ldBase_t *)&child, (arm_2d_location_t){1, 1}).iY == 30);

    child.use_as__ldBase_t.isDirtyRegionUpdate = false;
    ldBaseResize((ldBase_t *)&child, (arm_2d_size_t){30, 12});
    assert(ldBaseGetWidth((ldBase_t *)&child) == 30);
    assert(ldBaseGetHeight((ldBase_t *)&child) == 12);
    assert(child.use_as__ldBase_t.isDirtyRegionUpdate == true);

    child.use_as__ldBase_t.isDirtyRegionUpdate = false;
    ldBaseSetX((ldBase_t *)&child, 7);
    ldBaseSetY((ldBase_t *)&child, 8);
    ldBaseSetWidth((ldBase_t *)&child, 40);
    ldBaseSetHeight((ldBase_t *)&child, 22);
    assert(ldBaseGetX((ldBase_t *)&child) == 7);
    assert(ldBaseGetY((ldBase_t *)&child) == 8);
    assert(ldBaseGetWidth((ldBase_t *)&child) == 40);
    assert(ldBaseGetHeight((ldBase_t *)&child) == 22);

    ldBaseSetRegion((ldBase_t *)&child, (arm_2d_region_t){{1, 2}, {3, 4}});
    assert(ldBaseGetRegion((ldBase_t *)&child).tLocation.iX == 1);
    assert(ldBaseGetLocation((ldBase_t *)&child).iY == 2);
    assert(ldBaseGetSize((ldBase_t *)&child).iWidth == 3);

    ldBaseSetCenter((ldBase_t *)&child);
    assert(ldBaseGetX((ldBase_t *)&child) == 29);
    assert(ldBaseGetY((ldBase_t *)&child) == 20);
}

static void test_alignment_and_hidden_move_helpers(void)
{
    ldWindow_t root = {0};
    ldLabel_t child = {0};
    arm_2d_region_t parent = {{10, 20}, {100, 60}};
    arm_2d_region_t region = {{0, 0}, {20, 10}};
    arm_2d_region_t aligned;

    init_named_root(&root, 1, 100, 50);
    child.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion = (arm_2d_region_t){{5, 6}, {10, 12}};
    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&child);

    aligned = ldBaseGetAlignRegion(parent, region, ARM_2D_ALIGN_RIGHT | ARM_2D_ALIGN_BOTTOM);
    assert(aligned.tLocation.iX == 80);
    assert(aligned.tLocation.iY == 50);

    aligned = ldBaseAlignRegionCenter(parent, region);
    assert(aligned.tLocation.iX == 50);
    assert(aligned.tLocation.iY == 45);

    assert(ldBaseAutoVerticalGridAlign((arm_2d_region_t){{0, 0}, {80, 100}}, -5, 1, 10, 0) == 90);

    child.use_as__ldBase_t.isHidden = true;
    ldBaseMove((ldBase_t *)&child, 20, 30);
    assert(child.use_as__ldBase_t.isHidden == true);
    assert(ldBaseGetX((ldBase_t *)&child) == -80);
    assert(ldBaseGetY((ldBase_t *)&child) == -20);
}

static arm_2d_tile_t make_rgb565_tile(uint16_t *buffer, int16_t width, int16_t height)
{
    arm_2d_tile_t tile = {0};

    tile.bIsRoot = true;
    tile.tInfo.tColourInfo.chScheme = ARM_2D_COLOUR_RGB565;
    tile.tRegion.tSize.iWidth = width;
    tile.tRegion.tSize.iHeight = height;
    tile.phwBuffer = buffer;
    return tile;
}

static uint16_t pixel_at(const uint16_t *buffer, int16_t stride, int16_t x, int16_t y)
{
    return buffer[(y * stride) + x];
}

static void assert_region_border_clean(const uint16_t *buffer,
                                       int16_t stride,
                                       arm_2d_region_t region,
                                       uint16_t bg)
{
    int16_t min_x = (int16_t)(region.tLocation.iX - 1);
    int16_t max_x = (int16_t)(region.tLocation.iX + region.tSize.iWidth);
    int16_t min_y = (int16_t)(region.tLocation.iY - 1);
    int16_t max_y = (int16_t)(region.tLocation.iY + region.tSize.iHeight);

    for (int16_t x = min_x; x <= max_x; ++x)
    {
        assert(pixel_at(buffer, stride, x, min_y) == bg);
        assert(pixel_at(buffer, stride, x, max_y) == bg);
    }
    for (int16_t y = min_y; y <= max_y; ++y)
    {
        assert(pixel_at(buffer, stride, min_x, y) == bg);
        assert(pixel_at(buffer, stride, max_x, y) == bg);
    }
}

static void assert_capsule_region_has_clean_clip(const uint16_t *buffer,
                                                 int16_t stride,
                                                 arm_2d_region_t region,
                                                 uint16_t bg)
{
    /*
     * ldArm2dDrawCircle expands its internal draw region beyond radius * 2.
     * The capsule contract requires passing the current cap/shape region so
     * that antialiasing never bleeds outside the requested capsule bounds.
     */
    assert_region_border_clean(buffer, stride, region, bg);
}

static void draw_capsule_case(arm_2d_region_t region, uint16_t fg, uint16_t bg, uint16_t *buffer, size_t count)
{
    arm_2d_tile_t tile;

    for (size_t i = 0; i < count; ++i)
    {
        buffer[i] = bg;
    }

    tile = make_rgb565_tile(buffer, 96, 96);
    ldBaseDrawCapsule(&tile, &region, fg, 255);
    arm_2d_op_wait_async(NULL);
}

static void test_draw_capsule_rejects_circle_primitive_bleed_without_cap_clip(void)
{
    enum { WIDTH = 96, HEIGHT = 96 };
    static uint16_t buffer[WIDTH * HEIGHT];
    const uint16_t bg = 0x4444;
    const uint16_t fg = 0xFFE0;
    arm_2d_region_t region = {{9, 10}, {40, 20}};

    draw_capsule_case(region, fg, bg, buffer, sizeof(buffer) / sizeof(buffer[0]));

    assert_capsule_region_has_clean_clip(buffer, WIDTH, region, bg);
    assert(pixel_at(buffer, WIDTH, 9, 20) != bg);
    assert(pixel_at(buffer, WIDTH, 48, 20) != bg);
}

static void test_draw_capsule_even_horizontal_keeps_full_endpoints_and_clipped_caps(void)
{
    enum { WIDTH = 96, HEIGHT = 96 };
    static uint16_t buffer[WIDTH * HEIGHT];
    const uint16_t bg = 0x1111;
    const uint16_t fg = 0x07E0;
    arm_2d_region_t region = {{17, 19}, {60, 30}};

    draw_capsule_case(region, fg, bg, buffer, sizeof(buffer) / sizeof(buffer[0]));

    assert_capsule_region_has_clean_clip(buffer, WIDTH, region, bg);
    /* These midpoint samples fail if even-size caps regress to (diameter - 1) / 2. */
    assert(pixel_at(buffer, WIDTH, 17, 34) != bg);
    assert(pixel_at(buffer, WIDTH, 76, 34) != bg);
    assert(pixel_at(buffer, WIDTH, 32, 19) != bg);
    assert(pixel_at(buffer, WIDTH, 61, 48) != bg);
    assert(pixel_at(buffer, WIDTH, 17, 19) == bg);
    assert(pixel_at(buffer, WIDTH, 76, 19) == bg);
    assert(pixel_at(buffer, WIDTH, 17, 48) == bg);
    assert(pixel_at(buffer, WIDTH, 76, 48) == bg);
}

static void test_draw_capsule_even_vertical_keeps_full_endpoints_and_clipped_caps(void)
{
    enum { WIDTH = 96, HEIGHT = 96 };
    static uint16_t buffer[WIDTH * HEIGHT];
    const uint16_t bg = 0x2222;
    const uint16_t fg = 0x001F;
    arm_2d_region_t region = {{23, 11}, {30, 60}};

    draw_capsule_case(region, fg, bg, buffer, sizeof(buffer) / sizeof(buffer[0]));

    assert_capsule_region_has_clean_clip(buffer, WIDTH, region, bg);
    /* These midpoint samples fail if even-size caps regress to (diameter - 1) / 2. */
    assert(pixel_at(buffer, WIDTH, 38, 11) != bg);
    assert(pixel_at(buffer, WIDTH, 38, 70) != bg);
    assert(pixel_at(buffer, WIDTH, 23, 26) != bg);
    assert(pixel_at(buffer, WIDTH, 52, 55) != bg);
    assert(pixel_at(buffer, WIDTH, 23, 11) == bg);
    assert(pixel_at(buffer, WIDTH, 52, 11) == bg);
    assert(pixel_at(buffer, WIDTH, 23, 70) == bg);
    assert(pixel_at(buffer, WIDTH, 52, 70) == bg);
}

static void test_draw_capsule_square_degenerates_to_clipped_circle(void)
{
    enum { WIDTH = 96, HEIGHT = 96 };
    static uint16_t buffer[WIDTH * HEIGHT];
    const uint16_t bg = 0x3333;
    const uint16_t fg = 0xF800;
    arm_2d_region_t region = {{31, 37}, {24, 24}};

    draw_capsule_case(region, fg, bg, buffer, sizeof(buffer) / sizeof(buffer[0]));

    assert_capsule_region_has_clean_clip(buffer, WIDTH, region, bg);
    assert(pixel_at(buffer, WIDTH, 43, 37) != bg);
    assert(pixel_at(buffer, WIDTH, 43, 60) != bg);
    assert(pixel_at(buffer, WIDTH, 31, 49) != bg);
    assert(pixel_at(buffer, WIDTH, 54, 49) != bg);
    assert(pixel_at(buffer, WIDTH, 31, 37) == bg);
    assert(pixel_at(buffer, WIDTH, 54, 37) == bg);
    assert(pixel_at(buffer, WIDTH, 31, 60) == bg);
    assert(pixel_at(buffer, WIDTH, 54, 60) == bg);
}

int main(void)
{
    test_calendar_helpers();
    test_tree_and_lookup_helpers();
    test_visibility_selection_and_basic_getters();
    test_location_and_region_helpers();
    test_alignment_and_hidden_move_helpers();
    test_draw_capsule_rejects_circle_primitive_bleed_without_cap_clip();
    test_draw_capsule_even_horizontal_keeps_full_endpoints_and_clipped_caps();
    test_draw_capsule_even_vertical_keeps_full_endpoints_and_clipped_caps();
    test_draw_capsule_square_degenerates_to_clipped_circle();
    return 0;
}
