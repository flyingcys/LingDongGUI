#include <assert.h>
#include <string.h>

#include "ldLabel.h"
#include "ldWindow.h"
#include "ldWindow.h"

static void test_ldlabel_init_sets_defaults_and_links_to_parent(void)
{
    ld_scene_t scene = {0};
    ldWindow_t root = {0};
    ldLabel_t label = {0};
    arm_2d_font_t font = {0};

    root.use_as__ldBase_t.widgetType = widgetTypeWindow;
    root.use_as__ldBase_t.nameId = 1;
    scene.ptNodeRoot = (arm_2d_control_node_t *)&root;

    assert(ldLabel_init(&scene, &label, 2, 1, 10, 11, 30, 40, &font) == &label);
    assert(label.use_as__ldBase_t.nameId == 2);
    assert(label.use_as__ldBase_t.widgetType == widgetTypeLabel);
    assert(ldBaseGetParent((ldBase_t *)&label) == (ldBase_t *)&root);
    assert(label.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 10);
    assert(label.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 11);
    assert(label.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iWidth == 30);
    assert(label.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iHeight == 40);
    assert(label.tAlign == ARM_2D_ALIGN_CENTRE);
    assert(label.ptFont == &font);
    assert(label.bgColor == GLCD_COLOR_WHITE);
    assert(label.textColor == GLCD_COLOR_BLACK);
    assert(label.use_as__ldBase_t.isDirtyRegionUpdate == true);
}

static void test_ldlabel_setters_update_state_and_dirty_flag(void)
{
    ldLabel_t label = {0};
    arm_2d_font_t font = {0};
    uint8_t text[] = "hello";

    label.use_as__ldBase_t.isDirtyRegionUpdate = false;
    ldLabelSetTransparent(&label, true);
    assert(ldLabelGetTransparent(&label) == true);
    assert(label.use_as__ldBase_t.isDirtyRegionUpdate == true);

    label.use_as__ldBase_t.isDirtyRegionUpdate = false;
    ldLabelSetText(&label, text);
    assert(label.use_as__ldBase_t.isDirtyRegionUpdate == true);
    assert(label.pStr != NULL);
    assert(strcmp((char *)ldLabelGetText(&label), "hello") == 0);
    assert(label.pStr != text);

    label.use_as__ldBase_t.isDirtyRegionUpdate = false;
    ldLabelSetTextColor(&label, __RGB(1, 2, 3));
    assert(ldLabelGetTextColor(&label) == __RGB(1, 2, 3));
    assert(label.use_as__ldBase_t.isDirtyRegionUpdate == true);

    label.use_as__ldBase_t.isDirtyRegionUpdate = false;
    ldLabelSetAlign(&label, ARM_2D_ALIGN_LEFT);
    assert(ldLabelGetAlign(&label) == ARM_2D_ALIGN_LEFT);
    assert(label.use_as__ldBase_t.isDirtyRegionUpdate == true);

    label.use_as__ldBase_t.isDirtyRegionUpdate = false;
    ldLabelSetBackgroundColor(&label, __RGB(4, 5, 6));
    assert(ldLabelGetBackgroundColor(&label) == __RGB(4, 5, 6));
    assert(label.isTransparent == false);
    assert(label.ptImgTile == NULL);
    assert(label.use_as__ldBase_t.isDirtyRegionUpdate == true);

    label.use_as__ldBase_t.isDirtyRegionUpdate = false;
    ldLabelSetFont(&label, &font);
    assert(ldLabelGetFont(&label) == &font);
    assert(label.use_as__ldBase_t.isDirtyRegionUpdate == true);

    ldLabel_depose(NULL, &label);
}

int main(void)
{
    test_ldlabel_init_sets_defaults_and_links_to_parent();
    test_ldlabel_setters_update_state_and_dirty_flag();
    return 0;
}
