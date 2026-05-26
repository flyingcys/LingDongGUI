#include <assert.h>

#include "ldLabel.h"
#include "ldWindow.h"
#include "ldWindowLayoutInternal.h"

static void test_collect_direct_children_ignores_grandchildren(void)
{
    ldWindow_t parent = {0};
    ldWindow_t child_window = {0};
    ldLabel_t child_label = {0};
    ldLabel_t grandchild = {0};
    ldBase_t *children[4] = {0};

    ldBaseNodeAdd((arm_2d_control_node_t *)&parent, (arm_2d_control_node_t *)&child_window);
    ldBaseNodeAdd((arm_2d_control_node_t *)&parent, (arm_2d_control_node_t *)&child_label);
    ldBaseNodeAdd((arm_2d_control_node_t *)&child_window, (arm_2d_control_node_t *)&grandchild);

    assert(ldWindowCollectDirectChildren((ldBase_t *)&parent, children, 4, false) == 2);
    assert(children[0] == (ldBase_t *)&child_window);
    assert(children[1] == (ldBase_t *)&child_label);
}

static void test_collect_direct_children_keeps_hidden_for_legacy(void)
{
    ldWindow_t parent = {0};
    ldLabel_t a = {0};
    ldLabel_t b = {0};
    ldBase_t *items[4] = {0};

    a.use_as__ldBase_t.isHidden = false;
    b.use_as__ldBase_t.isHidden = true;

    ldBaseNodeAdd((arm_2d_control_node_t *)&parent, (arm_2d_control_node_t *)&a);
    ldBaseNodeAdd((arm_2d_control_node_t *)&parent, (arm_2d_control_node_t *)&b);

    assert(ldWindowCollectDirectChildren((ldBase_t *)&parent, items, 4, false) == 2);
}

static void test_mark_parent_layout_dirty_marks_nearest_window(void)
{
    ldWindow_t root = {0};
    ldWindow_t container = {0};
    ldLabel_t leaf = {0};

    root.layoutTpye = layoutVertical;
    container.layoutTpye = layoutHorizontal;

    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&container);
    ldBaseNodeAdd((arm_2d_control_node_t *)&container, (arm_2d_control_node_t *)&leaf);

    root.isLayoutUpdate = false;
    container.isLayoutUpdate = false;

    ldBaseMarkParentLayoutDirty((ldBase_t *)&leaf);

    assert(container.isLayoutUpdate == true);
    assert(root.isLayoutUpdate == true);
}

static void test_set_region_marks_parent_layout_dirty(void)
{
    ldWindow_t root = {0};
    ldLabel_t child = {0};

    root.layoutTpye = layoutVertical;

    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&child);
    root.isLayoutUpdate = false;

    arm_2d_region_t region = child.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion;
    region.tSize.iWidth = 42;

    ldBaseSetRegion((ldBase_t *)&child, region);

    assert(root.isLayoutUpdate == true);
    assert(child.use_as__ldBase_t.isDirtyRegionUpdate == true);
}

static void test_set_hidden_marks_parent_layout_dirty(void)
{
    ldWindow_t root = {0};
    ldLabel_t child = {0};

    root.layoutTpye = layoutVertical;

    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&child);
    root.isLayoutUpdate = false;

    ldBaseSetHidden((ldBase_t *)&child, true);

    assert(root.isLayoutUpdate == true);
}

static void test_node_add_marks_container_layout_dirty(void)
{
    ldWindow_t root = {0};
    ldWindow_t container = {0};
    ldLabel_t child = {0};

    root.layoutTpye = layoutVertical;
    container.layoutTpye = layoutHorizontal;

    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&container);
    root.isLayoutUpdate = false;
    container.isLayoutUpdate = false;

    ldBaseNodeAdd((arm_2d_control_node_t *)&container, (arm_2d_control_node_t *)&child);

    assert(container.isLayoutUpdate == true);
    assert(root.isLayoutUpdate == true);
}

static void test_node_remove_marks_container_layout_dirty(void)
{
    ldWindow_t root = {0};
    ldWindow_t container = {0};
    ldLabel_t child = {0};

    root.layoutTpye = layoutVertical;
    container.layoutTpye = layoutHorizontal;

    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&container);
    ldBaseNodeAdd((arm_2d_control_node_t *)&container, (arm_2d_control_node_t *)&child);
    root.isLayoutUpdate = false;
    container.isLayoutUpdate = false;

    ldBaseNodeRemove((arm_2d_control_node_t *)&child);

    assert(container.isLayoutUpdate == true);
    assert(root.isLayoutUpdate == true);
}

static void test_legacy_layout_updates_direct_children_beyond_sixty_four(void)
{
    enum { child_count = 65 };

    ldWindow_t root = {0};
    ldLabel_t children[child_count] = {0};

    root.use_as__ldBase_t.widgetType = widgetTypeWindow;
    root.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iWidth = 100;
    root.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iHeight = 650;
    root.layoutTpye = layoutVertical;
    root.isLayoutUpdate = true;

    for (uint16_t i = 0; i < child_count; ++i)
    {
        children[i].use_as__ldBase_t.widgetType = widgetTypeLabel;
        children[i].use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX = -123;
        children[i].use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY = -321;
        children[i].use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iWidth = 20;
        children[i].use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iHeight = 2;
        ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&children[i]);
    }

    ldWindow_on_frame_start(NULL, &root);

    assert(children[0].use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 4);
    assert(children[child_count - 1].use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 40);
    assert(children[child_count - 1].use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 644);
}

static void test_flex_space_between_falls_back_to_start_when_no_space(void)
{
    int16_t resolved_gap = -1;
    int16_t start = ldFlexResolveMainStart(ldFlexMainAlignSpaceBetween, 100, 100, 3, 8, &resolved_gap);

    assert(start == 0);
    assert(resolved_gap == 8);
}

static void test_flex_space_between_preserves_gap_when_space_available(void)
{
    int16_t resolved_gap = -1;
    int16_t start = ldFlexResolveMainStart(ldFlexMainAlignSpaceBetween, 180, 110, 3, 10, &resolved_gap);

    assert(start == 0);
    assert(resolved_gap == 45);
}

static void test_flex_row_center_applies_gap_and_padding(void)
{
    int16_t resolved_gap = -1;
    int16_t start = ldFlexResolveMainStart(ldFlexMainAlignCenter, 180, 110, 3, 10, &resolved_gap);

    assert(start == 35);
    assert(resolved_gap == 10);
}

static void test_flex_space_between_resolves_gap_from_remaining_space(void)
{
    int16_t resolved_gap = -1;
    int16_t start = ldFlexResolveMainStart(ldFlexMainAlignSpaceBetween, 180, 90, 3, 0, &resolved_gap);

    assert(start == 0);
    assert(resolved_gap == 45);
}

static void test_flex_setters_mark_window_dirty(void)
{
    ldWindow_t window = {0};
    ldPadding_t padding = {.left = 1, .top = 2, .right = 3, .bottom = 4};

    window.use_as__ldBase_t.isDirtyRegionUpdate = false;
    window.isLayoutUpdate = false;

    ldWindowSetFlexFlow(&window, ldFlexFlowColumn);

    assert(window.layoutTpye == layoutFlex);
    assert(window.flexFlow == ldFlexFlowColumn);
    assert(window.isLayoutUpdate == true);
    assert(window.use_as__ldBase_t.isDirtyRegionUpdate == true);

    window.use_as__ldBase_t.isDirtyRegionUpdate = false;
    window.isLayoutUpdate = false;
    ldWindowSetFlexAlign(&window, ldFlexMainAlignEnd, ldFlexCrossAlignCenter);
    assert(window.flexMainAlign == ldFlexMainAlignEnd);
    assert(window.flexCrossAlign == ldFlexCrossAlignCenter);
    assert(window.isLayoutUpdate == true);
    assert(window.use_as__ldBase_t.isDirtyRegionUpdate == true);

    window.use_as__ldBase_t.isDirtyRegionUpdate = false;
    window.isLayoutUpdate = false;
    ldWindowSetPadding(&window, padding);
    assert(window.flexPadding.left == 1);
    assert(window.flexPadding.top == 2);
    assert(window.flexPadding.right == 3);
    assert(window.flexPadding.bottom == 4);
    assert(window.isLayoutUpdate == true);
    assert(window.use_as__ldBase_t.isDirtyRegionUpdate == true);

    window.use_as__ldBase_t.isDirtyRegionUpdate = false;
    window.isLayoutUpdate = false;
    ldWindowSetGap(&window, 7);
    assert(window.flexGap == 7);
    assert(window.isLayoutUpdate == true);
    assert(window.use_as__ldBase_t.isDirtyRegionUpdate == true);
}

static void test_grid_setters_mark_window_dirty(void)
{
    ldWindow_t window = {0};
    ldPadding_t padding = {.left = 2, .top = 3, .right = 4, .bottom = 5};

    window.use_as__ldBase_t.isDirtyRegionUpdate = false;
    window.isLayoutUpdate = false;
    ldWindowSetGridColumns(&window, 3);
    assert(window.layoutTpye == layoutGrid);
    assert(window.gridColumns == 3);
    assert(window.isLayoutUpdate == true);
    assert(window.use_as__ldBase_t.isDirtyRegionUpdate == true);

    window.use_as__ldBase_t.isDirtyRegionUpdate = false;
    window.isLayoutUpdate = false;
    ldWindowSetGridGap(&window, 6, 7);
    assert(window.layoutTpye == layoutGrid);
    assert(window.gridRowGap == 6);
    assert(window.gridColumnGap == 7);
    assert(window.isLayoutUpdate == true);
    assert(window.use_as__ldBase_t.isDirtyRegionUpdate == true);

    window.use_as__ldBase_t.isDirtyRegionUpdate = false;
    window.isLayoutUpdate = false;
    ldWindowSetGridPadding(&window, padding);
    assert(window.layoutTpye == layoutGrid);
    assert(window.gridPadding.left == 2);
    assert(window.gridPadding.top == 3);
    assert(window.gridPadding.right == 4);
    assert(window.gridPadding.bottom == 5);
    assert(window.isLayoutUpdate == true);
    assert(window.use_as__ldBase_t.isDirtyRegionUpdate == true);
}

static void test_flex_row_layout_skips_hidden_children_and_keeps_size(void)
{
    ldWindow_t root = {0};
    ldLabel_t a = {0};
    ldLabel_t hidden = {0};
    ldLabel_t c = {0};

    root.use_as__ldBase_t.widgetType = widgetTypeWindow;
    root.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iWidth = 200;
    root.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iHeight = 100;

    a.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion = (arm_2d_region_t){{11, 12}, {20, 10}};
    hidden.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion = (arm_2d_region_t){{123, 45}, {40, 12}};
    c.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion = (arm_2d_region_t){{13, 14}, {30, 20}};
    hidden.use_as__ldBase_t.isHidden = true;

    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&a);
    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&hidden);
    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&c);

    ldWindowSetFlexFlow(&root, ldFlexFlowRow);
    ldWindowSetFlexAlign(&root, ldFlexMainAlignCenter, ldFlexCrossAlignEnd);
    ldWindowSetPadding(&root, (ldPadding_t){.left = 10, .top = 5, .right = 20, .bottom = 15});
    ldWindowSetGap(&root, 8);

    ldWindow_on_frame_start(NULL, &root);

    assert(a.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 66);
    assert(a.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 75);
    assert(a.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iWidth == 20);
    assert(a.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iHeight == 10);

    assert(hidden.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 123);
    assert(hidden.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 45);

    assert(c.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 94);
    assert(c.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 65);
    assert(c.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iWidth == 30);
    assert(c.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iHeight == 20);
}

static void test_flex_column_layout_applies_main_and_cross_alignment(void)
{
    ldWindow_t root = {0};
    ldLabel_t a = {0};
    ldLabel_t b = {0};

    root.use_as__ldBase_t.widgetType = widgetTypeWindow;
    root.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iWidth = 100;
    root.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iHeight = 100;

    a.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion = (arm_2d_region_t){{0, 0}, {20, 10}};
    b.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion = (arm_2d_region_t){{0, 0}, {30, 20}};

    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&a);
    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&b);

    ldWindowSetFlexFlow(&root, ldFlexFlowColumn);
    ldWindowSetFlexAlign(&root, ldFlexMainAlignEnd, ldFlexCrossAlignCenter);
    ldWindowSetPadding(&root, (ldPadding_t){.left = 5, .top = 10, .right = 5, .bottom = 10});
    ldWindowSetGap(&root, 5);

    ldWindow_on_frame_start(NULL, &root);

    assert(a.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 40);
    assert(a.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 55);
    assert(b.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 35);
    assert(b.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 70);
}

static void test_grid_layout_places_visible_children_row_first(void)
{
    ldWindow_t root = {0};
    ldLabel_t a = {0};
    ldLabel_t hidden = {0};
    ldLabel_t c = {0};
    ldLabel_t d = {0};

    root.use_as__ldBase_t.widgetType = widgetTypeWindow;
    root.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iWidth = 100;
    root.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iHeight = 80;

    a.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion = (arm_2d_region_t){{-1, -1}, {50, 10}};
    hidden.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion = (arm_2d_region_t){{77, 66}, {12, 12}};
    c.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion = (arm_2d_region_t){{-2, -2}, {20, 12}};
    d.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion = (arm_2d_region_t){{-3, -3}, {15, 8}};
    hidden.use_as__ldBase_t.isHidden = true;

    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&a);
    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&hidden);
    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&c);
    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&d);

    ldWindowSetGridColumns(&root, 2);
    ldWindowSetGridGap(&root, 5, 4);
    ldWindowSetGridPadding(&root, (ldPadding_t){.left = 3, .top = 2, .right = 5, .bottom = 7});

    ldWindow_on_frame_start(NULL, &root);

    assert(a.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 3);
    assert(a.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 2);
    assert(a.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iWidth == 44);
    assert(a.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iHeight == 10);

    assert(hidden.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 77);
    assert(hidden.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 66);

    assert(c.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 51);
    assert(c.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 2);
    assert(c.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iWidth == 20);
    assert(c.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iHeight == 12);

    assert(d.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 3);
    assert(d.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 19);
    assert(d.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iWidth == 15);
    assert(d.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iHeight == 8);
}

int main(void)
{
    test_collect_direct_children_ignores_grandchildren();
    test_collect_direct_children_keeps_hidden_for_legacy();
    test_mark_parent_layout_dirty_marks_nearest_window();
    test_set_region_marks_parent_layout_dirty();
    test_set_hidden_marks_parent_layout_dirty();
    test_node_add_marks_container_layout_dirty();
    test_node_remove_marks_container_layout_dirty();
    test_legacy_layout_updates_direct_children_beyond_sixty_four();
    test_flex_space_between_falls_back_to_start_when_no_space();
    test_flex_space_between_preserves_gap_when_space_available();
    test_flex_row_center_applies_gap_and_padding();
    test_flex_space_between_resolves_gap_from_remaining_space();
    test_flex_setters_mark_window_dirty();
    test_grid_setters_mark_window_dirty();
    test_flex_row_layout_skips_hidden_children_and_keeps_size();
    test_flex_column_layout_applies_main_and_cross_alignment();
    test_grid_layout_places_visible_children_row_first();
    return 0;
}
