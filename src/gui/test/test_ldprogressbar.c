#include <assert.h>

#include "ldProgressBar.h"
#include "ldWindow.h"

static void test_ldprogressbar_init_sets_defaults_and_links_to_parent(void)
{
    ld_scene_t scene = {0};
    ldWindow_t root = {0};
    ldProgressBar_t bar = {0};

    root.use_as__ldBase_t.widgetType = widgetTypeWindow;
    root.use_as__ldBase_t.nameId = 1;
    scene.ptNodeRoot = (arm_2d_control_node_t *)&root;

    assert(ldProgressBar_init(&scene, &bar, 2, 1, 4, 5, 60, 7) == &bar);
    assert(bar.use_as__ldBase_t.nameId == 2);
    assert(bar.use_as__ldBase_t.widgetType == widgetTypeProgressBar);
    assert(ldBaseGetParent((ldBase_t *)&bar) == (ldBase_t *)&root);
    assert(bar.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 4);
    assert(bar.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 5);
    assert(bar.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iWidth == 60);
    assert(bar.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iHeight == 7);
    assert(bar.bgColor == GLCD_COLOR_WHITE);
    assert(bar.fgColor == __RGB(0x94, 0xd2, 0x52));
    assert(bar.frameColor == __RGB(0xa5, 0xc6, 0xef));
    assert(bar.frameColorSize == 0);
    assert(bar.isHorizontal == true);
    assert(bar.isInverted == false);
    assert(bar.use_as__ldBase_t.isDirtyRegionUpdate == true);
}

static void test_ldprogressbar_setters_update_state(void)
{
    ldProgressBar_t bar = {0};
    arm_2d_tile_t tile_a = {0};
    arm_2d_tile_t tile_b = {0};
    arm_2d_tile_t tile_c = {0};
    arm_2d_tile_t tile_d = {0};
    arm_2d_tile_t tile_e = {0};
    arm_2d_tile_t tile_f = {0};

    bar.use_as__ldBase_t.isDirtyRegionUpdate = false;
    ldProgressBarSetPercent(&bar, 33.5f);
    assert(bar.permille == 335);
    assert(bar.use_as__ldBase_t.isDirtyRegionUpdate == true);

    bar.use_as__ldBase_t.isDirtyRegionUpdate = false;
    ldProgressBarSetPercent(&bar, -25.0f);
    assert(bar.permille == 750);
    assert(bar.use_as__ldBase_t.isDirtyRegionUpdate == true);

    bar.use_as__ldBase_t.isDirtyRegionUpdate = false;
    ldProgressBarSetColor(&bar, __RGB(1, 1, 1), __RGB(2, 2, 2));
    assert(bar.bgColor == __RGB(1, 1, 1));
    assert(bar.fgColor == __RGB(2, 2, 2));
    assert(bar.ptBgImgTile == NULL);
    assert(bar.ptFgImgTile == NULL);
    assert(bar.use_as__ldBase_t.isDirtyRegionUpdate == true);

    bar.use_as__ldBase_t.isDirtyRegionUpdate = false;
    ldProgressBarSetFrameColor(&bar, __RGB(3, 3, 3), 4);
    assert(bar.frameColor == __RGB(3, 3, 3));
    assert(bar.frameColorSize == 4);
    assert(bar.use_as__ldBase_t.isDirtyRegionUpdate == true);

    bar.use_as__ldBase_t.isDirtyRegionUpdate = false;
    ldProgressBarSetImage(&bar, &tile_a, &tile_b, &tile_c, &tile_d);
    assert(bar.ptBgImgTile == &tile_a);
    assert(bar.ptBgMaskTile == &tile_b);
    assert(bar.ptFgImgTile == &tile_c);
    assert(bar.ptFgMaskTile == &tile_d);
    assert(bar.use_as__ldBase_t.isDirtyRegionUpdate == true);

    bar.use_as__ldBase_t.isDirtyRegionUpdate = false;
    ldProgressBarSetFrameImage(&bar, &tile_e, &tile_f);
    assert(bar.ptFrameImgTile == &tile_e);
    assert(bar.ptFrameMaskTile == &tile_f);
    assert(bar.use_as__ldBase_t.isDirtyRegionUpdate == true);

    ldProgressBarSetHorizontal(&bar, false);
    assert(bar.isHorizontal == false);

    ldProgressBarSetInverted(&bar, true);
    assert(bar.isInverted == true);
}

int main(void)
{
    test_ldprogressbar_init_sets_defaults_and_links_to_parent();
    test_ldprogressbar_setters_update_state();
    return 0;
}
