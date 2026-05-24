#include <assert.h>

#include "ldWindow.h"
#include "ldLabel.h"
#include "ldWindowLayoutInternal.h"

arm_2d_region_t *arm_2d_helper_control_get_absolute_region(arm_2d_control_node_t *ptNode,
                                                           arm_2d_region_t *ptOutRegion,
                                                           bool bClip)
{
    (void)bClip;

    assert(ptNode != NULL);
    assert(ptOutRegion != NULL);

    *ptOutRegion = ptNode->tRegion;
    return ptOutRegion;
}

bool arm_2d_op_wait_async(arm_2d_op_core_t *ptOP)
{
    (void)ptOP;
    return true;
}

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
    int16_t resolvedGap = -1;
    int16_t start = ldFlexResolveMainStart(ldFlexMainAlignSpaceBetween, 100, 100, 3, 8, &resolvedGap);

    assert(start == 0);
    assert(resolvedGap == 8);
}

static void test_flex_space_between_preserves_gap_when_space_available(void)
{
    int16_t resolvedGap = -1;
    int16_t start = ldFlexResolveMainStart(ldFlexMainAlignSpaceBetween, 180, 110, 3, 10, &resolvedGap);

    assert(start == 0);
    assert(resolvedGap == 45);
}

static void test_flex_row_center_applies_gap_and_padding(void)
{
    int16_t resolvedGap = -1;
    int16_t start = ldFlexResolveMainStart(ldFlexMainAlignCenter, 180, 110, 3, 10, &resolvedGap);

    assert(start == 35);
    assert(resolvedGap == 10);
}

static void test_flex_space_between_resolves_gap_from_remaining_space(void)
{
    int16_t resolvedGap = -1;
    int16_t start = ldFlexResolveMainStart(ldFlexMainAlignSpaceBetween, 180, 90, 3, 0, &resolvedGap);

    assert(start == 0);
    assert(resolvedGap == 45);
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

static void test_grid_descriptor_setters_mark_window_dirty(void)
{
    ldWindow_t window = {0};
    static const int16_t col_dsc[] = {20, LD_GRID_FR(1), LD_GRID_TEMPLATE_LAST};
    static const int16_t row_dsc[] = {12, LD_GRID_CONTENT, LD_GRID_TEMPLATE_LAST};

    window.use_as__ldBase_t.isDirtyRegionUpdate = false;
    window.isLayoutUpdate = false;
    ldWindowSetGridDscArray(&window, col_dsc, row_dsc);
    assert(window.layoutTpye == layoutGrid);
    assert(window.gridColDsc == col_dsc);
    assert(window.gridRowDsc == row_dsc);
    assert(window.isLayoutUpdate == true);
    assert(window.use_as__ldBase_t.isDirtyRegionUpdate == true);

    window.use_as__ldBase_t.isDirtyRegionUpdate = false;
    window.isLayoutUpdate = false;
    ldWindowSetGridAlign(&window, ldGridAlignCenter, ldGridAlignSpaceBetween);
    assert(window.gridColAlign == ldGridAlignCenter);
    assert(window.gridRowAlign == ldGridAlignSpaceBetween);
    assert(window.layoutTpye == layoutGrid);
    assert(window.isLayoutUpdate == true);
    assert(window.use_as__ldBase_t.isDirtyRegionUpdate == true);
}

static void test_grid_cell_setter_marks_parent_dirty_and_stores_metadata(void)
{
    ldWindow_t root = {0};
    ldLabel_t child = {0};

    root.layoutTpye = layoutGrid;
    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&child);
    root.isLayoutUpdate = false;

    ldBaseSetGridCell((ldBase_t *)&child,
                      ldGridAlignCenter,
                      2,
                      3,
                      ldGridAlignStretch,
                      1,
                      4);

    assert(child.use_as__ldBase_t.gridColPos == 2);
    assert(child.use_as__ldBase_t.gridColSpan == 3);
    assert(child.use_as__ldBase_t.gridRowPos == 1);
    assert(child.use_as__ldBase_t.gridRowSpan == 4);
    assert(child.use_as__ldBase_t.gridCellXAlign == ldGridAlignCenter);
    assert(child.use_as__ldBase_t.gridCellYAlign == ldGridAlignStretch);
    assert(root.isLayoutUpdate == true);
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
    assert(hidden.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iWidth == 12);
    assert(hidden.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iHeight == 12);

    assert(c.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 51);
    assert(c.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 2);
    assert(c.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iWidth == 20);
    assert(c.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iHeight == 12);

    assert(d.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 3);
    assert(d.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 19);
    assert(d.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iWidth == 15);
    assert(d.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iHeight == 8);
}

static void test_grid_layout_uses_fixed_tracks_and_explicit_cells(void)
{
    ldWindow_t root = {0};
    ldLabel_t a = {0};
    ldLabel_t b = {0};
    static const int16_t col_dsc[] = {30, 40, LD_GRID_TEMPLATE_LAST};
    static const int16_t row_dsc[] = {20, 25, LD_GRID_TEMPLATE_LAST};

    root.use_as__ldBase_t.widgetType = widgetTypeWindow;
    root.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize = (arm_2d_size_t){120, 80};

    a.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion = (arm_2d_region_t){{-1, -1}, {10, 8}};
    b.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion = (arm_2d_region_t){{-2, -2}, {18, 12}};

    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&a);
    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&b);

    ldWindowSetGridDscArray(&root, col_dsc, row_dsc);
    ldWindowSetGridGap(&root, 5, 7);
    ldWindowSetGridPadding(&root, (ldPadding_t){.left = 4, .top = 6, .right = 8, .bottom = 10});
    ldBaseSetGridCell((ldBase_t *)&a, ldGridAlignStart, 1, 1, ldGridAlignStart, 0, 1);
    ldBaseSetGridCell((ldBase_t *)&b, ldGridAlignEnd, 0, 1, ldGridAlignEnd, 1, 1);

    ldWindow_on_frame_start(NULL, &root);

    assert(a.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 41);
    assert(a.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 6);
    assert(a.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iWidth == 10);
    assert(a.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iHeight == 8);

    assert(b.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 16);
    assert(b.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 44);
    assert(b.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iWidth == 18);
    assert(b.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iHeight == 12);
}

static void test_grid_layout_resolves_fr_and_cell_alignment(void)
{
    ldWindow_t root = {0};
    ldLabel_t center = {0};
    ldLabel_t stretch = {0};
    static const int16_t col_dsc[] = {40, LD_GRID_FR(1), LD_GRID_FR(2), LD_GRID_TEMPLATE_LAST};
    static const int16_t row_dsc[] = {30, LD_GRID_TEMPLATE_LAST};

    root.use_as__ldBase_t.widgetType = widgetTypeWindow;
    root.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize = (arm_2d_size_t){160, 60};

    center.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion = (arm_2d_region_t){{0, 0}, {10, 8}};
    stretch.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion = (arm_2d_region_t){{0, 0}, {5, 6}};

    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&center);
    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&stretch);

    ldWindowSetGridDscArray(&root, col_dsc, row_dsc);
    ldWindowSetGridGap(&root, 0, 5);
    ldWindowSetGridPadding(&root, (ldPadding_t){.left = 10, .top = 6, .right = 10, .bottom = 6});
    ldBaseSetGridCell((ldBase_t *)&center, ldGridAlignCenter, 1, 1, ldGridAlignCenter, 0, 1);
    ldBaseSetGridCell((ldBase_t *)&stretch, ldGridAlignStretch, 2, 1, ldGridAlignStretch, 0, 1);

    ldWindow_on_frame_start(NULL, &root);

    assert(center.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 65);
    assert(center.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 17);
    assert(center.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iWidth == 10);
    assert(center.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iHeight == 8);

    assert(stretch.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 90);
    assert(stretch.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 6);
    assert(stretch.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iWidth == 60);
    assert(stretch.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iHeight == 30);
}

static void test_grid_layout_resolves_content_tracks_and_hidden_children(void)
{
    ldWindow_t root = {0};
    ldLabel_t visible = {0};
    ldLabel_t hidden = {0};
    ldLabel_t fr_child = {0};
    static const int16_t col_dsc[] = {LD_GRID_CONTENT, LD_GRID_FR(1), LD_GRID_TEMPLATE_LAST};
    static const int16_t row_dsc[] = {LD_GRID_CONTENT, LD_GRID_TEMPLATE_LAST};

    root.use_as__ldBase_t.widgetType = widgetTypeWindow;
    root.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize = (arm_2d_size_t){150, 50};

    visible.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion = (arm_2d_region_t){{0, 0}, {26, 12}};
    hidden.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion = (arm_2d_region_t){{0, 0}, {90, 30}};
    fr_child.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion = (arm_2d_region_t){{0, 0}, {12, 10}};
    hidden.use_as__ldBase_t.isHidden = true;

    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&visible);
    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&hidden);
    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&fr_child);

    ldWindowSetGridDscArray(&root, col_dsc, row_dsc);
    ldWindowSetGridGap(&root, 0, 4);
    ldWindowSetGridPadding(&root, (ldPadding_t){.left = 6, .top = 5, .right = 6, .bottom = 5});
    ldBaseSetGridCell((ldBase_t *)&visible, ldGridAlignStart, 0, 1, ldGridAlignStart, 0, 1);
    ldBaseSetGridCell((ldBase_t *)&hidden, ldGridAlignStart, 0, 1, ldGridAlignStart, 0, 1);
    ldBaseSetGridCell((ldBase_t *)&fr_child, ldGridAlignEnd, 1, 1, ldGridAlignEnd, 0, 1);

    ldWindow_on_frame_start(NULL, &root);

    assert(visible.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 6);
    assert(visible.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 5);
    assert(fr_child.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 132);
    assert(fr_child.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 7);
}

static void test_grid_layout_supports_span_and_container_alignment(void)
{
    ldWindow_t root = {0};
    ldLabel_t span = {0};
    ldLabel_t centered = {0};
    static const int16_t col_dsc[] = {20, 20, 20, LD_GRID_TEMPLATE_LAST};
    static const int16_t row_dsc[] = {15, 15, LD_GRID_TEMPLATE_LAST};

    root.use_as__ldBase_t.widgetType = widgetTypeWindow;
    root.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize = (arm_2d_size_t){110, 70};

    span.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion = (arm_2d_region_t){{0, 0}, {5, 5}};
    centered.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion = (arm_2d_region_t){{0, 0}, {8, 6}};

    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&span);
    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&centered);

    ldWindowSetGridDscArray(&root, col_dsc, row_dsc);
    ldWindowSetGridGap(&root, 5, 5);
    ldWindowSetGridAlign(&root, ldGridAlignCenter, ldGridAlignCenter);
    ldBaseSetGridCell((ldBase_t *)&span, ldGridAlignStretch, 0, 2, ldGridAlignStretch, 0, 2);
    ldBaseSetGridCell((ldBase_t *)&centered, ldGridAlignCenter, 2, 1, ldGridAlignCenter, 1, 1);

    ldWindow_on_frame_start(NULL, &root);

    assert(span.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 20);
    assert(span.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 17);
    assert(span.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iWidth == 45);
    assert(span.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iHeight == 35);

    assert(centered.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 76);
    assert(centered.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 41);
    assert(centered.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iWidth == 8);
    assert(centered.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iHeight == 6);
}

static void test_grid_layout_clamps_invalid_cell_settings(void)
{
    ldWindow_t root = {0};
    ldLabel_t child = {0};
    static const int16_t col_dsc[] = {30, 20, LD_GRID_TEMPLATE_LAST};
    static const int16_t row_dsc[] = {16, 24, LD_GRID_TEMPLATE_LAST};

    root.use_as__ldBase_t.widgetType = widgetTypeWindow;
    root.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize = (arm_2d_size_t){70, 60};

    child.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion = (arm_2d_region_t){{0, 0}, {6, 4}};
    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&child);

    ldWindowSetGridDscArray(&root, col_dsc, row_dsc);
    ldWindowSetGridGap(&root, 4, 6);
    ldBaseSetGridCell((ldBase_t *)&child,
                      ldGridAlignStretch,
                      -3,
                      99,
                      ldGridAlignStretch,
                      -4,
                      88);

    ldWindow_on_frame_start(NULL, &root);

    assert(child.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 0);
    assert(child.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 0);
    assert(child.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iWidth == 56);
    assert(child.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iHeight == 44);
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
    test_grid_descriptor_setters_mark_window_dirty();
    test_grid_cell_setter_marks_parent_dirty_and_stores_metadata();
    test_flex_row_layout_skips_hidden_children_and_keeps_size();
    test_flex_column_layout_applies_main_and_cross_alignment();
    test_grid_layout_places_visible_children_row_first();
    test_grid_layout_uses_fixed_tracks_and_explicit_cells();
    test_grid_layout_resolves_fr_and_cell_alignment();
    test_grid_layout_resolves_content_tracks_and_hidden_children();
    test_grid_layout_supports_span_and_container_alignment();
    test_grid_layout_clamps_invalid_cell_settings();
    return 0;
}
