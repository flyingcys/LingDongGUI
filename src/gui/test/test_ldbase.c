#include <assert.h>

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

    assert(ldBaseAutoVerticalGridAlign((arm_2d_region_t){{0, 0}, {80, 100}}, -5, 1, 10, 0) == 85);

    child.use_as__ldBase_t.isHidden = true;
    ldBaseMove((ldBase_t *)&child, 20, 30);
    assert(child.use_as__ldBase_t.isHidden == true);
    assert(ldBaseGetX((ldBase_t *)&child) == -80);
    assert(ldBaseGetY((ldBase_t *)&child) == -20);
}

int main(void)
{
    test_calendar_helpers();
    test_tree_and_lookup_helpers();
    test_visibility_selection_and_basic_getters();
    test_location_and_region_helpers();
    test_alignment_and_hidden_move_helpers();
    return 0;
}
