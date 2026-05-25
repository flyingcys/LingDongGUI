#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define __ARM_2D_HELPER_CONTROL_INHERIT__
#include "ldWindow.h"
#include "ldLabel.h"
#include "ldSwitch.h"
#include "ldWindowLayoutInternal.h"

static arm_2d_err_t test_preorder_enum_init(arm_2d_control_enumerator_t *ptThis,
                                            const arm_2d_control_node_t *ptRoot);
static arm_2d_control_node_t *test_preorder_enum_get_next_node(arm_2d_control_enumerator_t *ptThis);
static arm_2d_err_t test_preorder_enum_depose(arm_2d_control_enumerator_t *ptThis);

const arm_2d_control_enumeration_policy_t ARM_2D_CONTROL_ENUMERATION_POLICY_PREORDER_TRAVERSAL = {
    .fnInit = test_preorder_enum_init,
    .fnDepose = test_preorder_enum_depose,
    .fnGetNextNode = test_preorder_enum_get_next_node,
};
const arm_2d_tile_t c_tileWhiteDotMask = {0};
const arm_2d_tile_t c_tileCircleMask = {0};

void VT_enter_global_mutex(void) {}
void VT_leave_global_mutex(void) {}

static arm_2d_err_t test_preorder_enum_init(arm_2d_control_enumerator_t *ptThis,
                                            const arm_2d_control_node_t *ptRoot)
{
    assert(ptThis != NULL);

    ptThis->ptPolicy = &ARM_2D_CONTROL_ENUMERATION_POLICY_PREORDER_TRAVERSAL;
    ptThis->ptRoot = (arm_2d_control_node_t *)ptRoot;
    ptThis->ptCurrent = (arm_2d_control_node_t *)ptRoot;
    ptThis->Preorder.bFirstNode = true;
    return ARM_2D_ERR_NONE;
}

static arm_2d_control_node_t *test_preorder_enum_get_next_node(arm_2d_control_enumerator_t *ptThis)
{
    arm_2d_control_node_t *ptCurrent;
    ldBase_t *ptBaseCurrent;

    assert(ptThis != NULL);
    ptCurrent = ptThis->ptCurrent;
    if (ptCurrent == NULL)
    {
        return NULL;
    }

    if (ptThis->Preorder.bFirstNode)
    {
        ptThis->Preorder.bFirstNode = false;
        return ptCurrent;
    }

    ptBaseCurrent = (ldBase_t *)ptCurrent;
    if (ptBaseCurrent->use_as__arm_2d_control_node_t.ptChildList != NULL)
    {
        ptThis->ptCurrent = (arm_2d_control_node_t *)ptBaseCurrent->use_as__arm_2d_control_node_t.ptChildList;
        return ptThis->ptCurrent;
    }

    while ((ptCurrent != NULL)
        && (ptCurrent != ptThis->ptRoot)
        && (((ldBase_t *)ptCurrent)->use_as__arm_2d_control_node_t.ptNext == NULL))
    {
        ptCurrent = (arm_2d_control_node_t *)((ldBase_t *)ptCurrent)->use_as__arm_2d_control_node_t.ptParent;
    }

    if ((ptCurrent == NULL)
        || (((ldBase_t *)ptCurrent)->use_as__arm_2d_control_node_t.ptNext == NULL))
    {
        ptThis->ptCurrent = NULL;
        return NULL;
    }

    ptThis->ptCurrent = (arm_2d_control_node_t *)((ldBase_t *)ptCurrent)->use_as__arm_2d_control_node_t.ptNext;
    return ptThis->ptCurrent;
}

static arm_2d_err_t test_preorder_enum_depose(arm_2d_control_enumerator_t *ptThis)
{
    (void)ptThis;
    return ARM_2D_ERR_NONE;
}

arm_2d_err_t arm_2d_helper_control_enum_init(arm_2d_control_enumerator_t *ptThis,
                                             const arm_2d_control_enumeration_policy_t *ptPolicy,
                                             const arm_2d_control_node_t *ptRoot)
{
    assert(ptPolicy != NULL);
    return ptPolicy->fnInit(ptThis, ptRoot);
}

arm_2d_control_node_t *arm_2d_helper_control_enum_get_next_node(arm_2d_control_enumerator_t *ptThis)
{
    assert(ptThis != NULL);
    return ptThis->ptPolicy->fnGetNextNode(ptThis);
}

arm_2d_err_t arm_2d_helper_control_enum_depose(arm_2d_control_enumerator_t *ptThis)
{
    assert(ptThis != NULL);
    return ptThis->ptPolicy->fnDepose(ptThis);
}

bool __arm_2d_helper_pfb_is_region_active0(const arm_2d_tile_t *ptTarget,
                                           const arm_2d_region_t *ptRegion,
                                           bool bConsiderDryRun)
{
    (void)ptTarget;
    (void)ptRegion;
    (void)bConsiderDryRun;
    return true;
}

void __draw_round_corner_box(const arm_2d_tile_t *ptTarget,
                             const arm_2d_region_t *ptRegion,
                             COLOUR_INT tColour,
                             uint8_t chOpacity,
                             const arm_2d_tile_t *ptCircleMask)
{
    (void)ptTarget;
    (void)ptRegion;
    (void)tColour;
    (void)chOpacity;
    (void)ptCircleMask;
}

void __draw_round_corner_image(const arm_2d_tile_t *ptSource,
                               const arm_2d_tile_t *ptTarget,
                               const arm_2d_region_t *ptRegion,
                               bool bIsNewFrame,
                               uint8_t chOpacity,
                               const arm_2d_tile_t *ptCircleMask)
{
    (void)ptSource;
    (void)ptTarget;
    (void)ptRegion;
    (void)bIsNewFrame;
    (void)chOpacity;
    (void)ptCircleMask;
}

void __draw_round_corner_border(const arm_2d_tile_t *ptTarget,
                                const arm_2d_region_t *ptRegion,
                                COLOUR_INT tColour,
                                arm_2d_border_opacity_t opacity,
                                arm_2d_corner_opacity_t cornerOpacity,
                                const arm_2d_tile_t *ptCircleMask)
{
    (void)ptTarget;
    (void)ptRegion;
    (void)tColour;
    (void)opacity;
    (void)cornerOpacity;
    (void)ptCircleMask;
}

void arm_2d_helper_draw_box(const arm_2d_tile_t *ptTarget,
                            const arm_2d_region_t *ptRegion,
                            int16_t iBorderWidth,
                            COLOUR_INT tColour,
                            uint8_t chOpacity)
{
    (void)ptTarget;
    (void)ptRegion;
    (void)iBorderWidth;
    (void)tColour;
    (void)chOpacity;
}

int64_t arm_2d_helper_get_system_timestamp(void)
{
    return 0;
}

int64_t arm_2d_helper_convert_ticks_to_ms(int64_t lTick)
{
    return lTick;
}

int64_t arm_2d_helper_convert_ms_to_ticks(int64_t lMS)
{
    return lMS;
}

int8_t arm_lcd_text_set_char_spacing(int8_t chNewSpacing)
{
    return chNewSpacing;
}

void arm_lcd_text_set_target_framebuffer(const arm_2d_tile_t *ptFrameBuffer)
{
    (void)ptFrameBuffer;
}

void arm_lcd_text_set_draw_region(arm_2d_region_t *ptRegion)
{
    (void)ptRegion;
}

void arm_lcd_text_set_colour(COLOUR_INT wForeground, COLOUR_INT wBackground)
{
    (void)wForeground;
    (void)wBackground;
}

arm_2d_err_t arm_lcd_text_set_font(const arm_2d_font_t *ptFont)
{
    (void)ptFont;
    return ARM_2D_ERR_NONE;
}

void arm_lcd_text_set_opacity(uint8_t chOpacity)
{
    (void)chOpacity;
}

void arm_lcd_text_location(int16_t iRow, uint16_t iColumn)
{
    (void)iRow;
    (void)iColumn;
}

void arm_lcd_puts_label(const char *pchString, arm_2d_align_t tAlign)
{
    (void)pchString;
    (void)tAlign;
}

float32_t arm_sin_f32(float32_t x)
{
    return sinf(x);
}

float32_t arm_cos_f32(float32_t x)
{
    return cosf(x);
}

q31_t arm_sin_q31(q31_t x)
{
    (void)x;
    return 0;
}

q31_t arm_cos_q31(q31_t x)
{
    (void)x;
    return 0;
}

void ldGuiUpdateScene(void) {}

void *pvPortMalloc(size_t xWantedSize)
{
    return malloc(xWantedSize);
}

void vPortFree(void *pv)
{
    free(pv);
}

size_t xPortGetFreeHeapSize(void)
{
    return 0;
}

void *pvPortRealloc(uint8_t *srcaddr, size_t xWantedSize)
{
    return realloc(srcaddr, xWantedSize);
}

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

#define LD_FLEX_FLOW_ROW_WRAP ((ldFlexFlow_t)2)
#define LD_FLEX_FLOW_COLUMN_WRAP ((ldFlexFlow_t)3)
#define LD_FLEX_FLOW_ROW_REVERSE ((ldFlexFlow_t)4)
#define LD_FLEX_FLOW_COLUMN_REVERSE ((ldFlexFlow_t)5)
#define LD_FLEX_FLOW_ROW_WRAP_REVERSE ((ldFlexFlow_t)6)
#define LD_FLEX_FLOW_COLUMN_WRAP_REVERSE ((ldFlexFlow_t)7)
#define LD_FLEX_MAIN_ALIGN_SPACE_AROUND ((ldFlexMainAlign_t)4)
#define LD_FLEX_MAIN_ALIGN_SPACE_EVENLY ((ldFlexMainAlign_t)5)

#define LD_FLEX_TRACK_ALIGN_START 0
#define LD_FLEX_TRACK_ALIGN_CENTER 1
#define LD_FLEX_TRACK_ALIGN_END 2
#define LD_FLEX_TRACK_ALIGN_SPACE_BETWEEN 3
#define LD_FLEX_TRACK_ALIGN_SPACE_AROUND 4
#define LD_FLEX_TRACK_ALIGN_SPACE_EVENLY 5

#if defined(LDGUI_FLEX_API_CONTRACT)
void ldWindowSetFlexTrackAlign(ldWindow_t *ptWidget, int trackAlign);
void ldBaseSetFlexGrow(ldBase_t *ptWidget, uint8_t grow);
void ldBaseSetFlexNewTrack(ldBase_t *ptWidget, bool inNewTrack);
void ldBaseSetIgnoreLayout(ldBase_t *ptWidget, bool ignoreLayout);
#endif

static void set_widget_region(ldBase_t *ptWidget, int16_t x, int16_t y, int16_t width, int16_t height)
{
    ptWidget->use_as__arm_2d_control_node_t.tRegion = (arm_2d_region_t){{x, y}, {width, height}};
}

static void init_window_region(ldWindow_t *ptWindow, int16_t width, int16_t height)
{
    ptWindow->use_as__ldBase_t.widgetType = widgetTypeWindow;
    set_widget_region((ldBase_t *)ptWindow, 0, 0, width, height);
}

static void init_focus_runtime_scene(ld_scene_t *ptScene,
                                     ldWindow_t *ptRoot,
                                     ldLabel_t *ptLeft,
                                     ldSwitch_t *ptSwitch,
                                     ldLabel_t *ptRight)
{
    memset(ptScene, 0, sizeof(*ptScene));
    memset(ptRoot, 0, sizeof(*ptRoot));
    memset(ptLeft, 0, sizeof(*ptLeft));
    memset(ptSwitch, 0, sizeof(*ptSwitch));
    memset(ptRight, 0, sizeof(*ptRight));

    init_window_region(ptRoot, 160, 60);
    ptRoot->use_as__ldBase_t.nameId = 0;
    ptLeft->use_as__ldBase_t.widgetType = widgetTypeLabel;
    ptSwitch->use_as__ldBase_t.widgetType = widgetTypeSwitch;
    ptRight->use_as__ldBase_t.widgetType = widgetTypeLabel;

    set_widget_region((ldBase_t *)ptLeft, 0, 0, 20, 20);
    set_widget_region((ldBase_t *)ptSwitch, 40, 0, 44, 24);
    set_widget_region((ldBase_t *)ptRight, 100, 0, 20, 20);

    ldBaseSetSelectable((ldBase_t *)ptLeft, true);
    ldBaseSetSelectable((ldBase_t *)ptSwitch, true);
    ldBaseSetSelectable((ldBase_t *)ptRight, true);

    ldBaseNodeAdd((arm_2d_control_node_t *)ptRoot, (arm_2d_control_node_t *)ptLeft);
    ldBaseNodeAdd((arm_2d_control_node_t *)ptRoot, (arm_2d_control_node_t *)ptSwitch);
    ldBaseNodeAdd((arm_2d_control_node_t *)ptRoot, (arm_2d_control_node_t *)ptRight);

    ptScene->ptNodeRoot = (arm_2d_control_node_t *)ptRoot;
    ptScene->ptMsgQueue = NULL;
    ldBaseFocusNavigateInit();
}

static void init_focus_runtime_cross_scene(ld_scene_t *ptScene,
                                           ldWindow_t *ptRoot,
                                           ldLabel_t *ptLeft,
                                           ldLabel_t *ptUp,
                                           ldSwitch_t *ptSwitch,
                                           ldLabel_t *ptRight,
                                           ldLabel_t *ptDown)
{
    memset(ptScene, 0, sizeof(*ptScene));
    memset(ptRoot, 0, sizeof(*ptRoot));
    memset(ptLeft, 0, sizeof(*ptLeft));
    memset(ptUp, 0, sizeof(*ptUp));
    memset(ptSwitch, 0, sizeof(*ptSwitch));
    memset(ptRight, 0, sizeof(*ptRight));
    memset(ptDown, 0, sizeof(*ptDown));

    init_window_region(ptRoot, 180, 120);
    ptRoot->use_as__ldBase_t.nameId = 0;
    ptLeft->use_as__ldBase_t.widgetType = widgetTypeLabel;
    ptUp->use_as__ldBase_t.widgetType = widgetTypeLabel;
    ptSwitch->use_as__ldBase_t.widgetType = widgetTypeSwitch;
    ptRight->use_as__ldBase_t.widgetType = widgetTypeLabel;
    ptDown->use_as__ldBase_t.widgetType = widgetTypeLabel;

    set_widget_region((ldBase_t *)ptLeft, 0, 40, 20, 20);
    set_widget_region((ldBase_t *)ptUp, 40, 0, 20, 20);
    set_widget_region((ldBase_t *)ptSwitch, 40, 40, 44, 24);
    set_widget_region((ldBase_t *)ptRight, 120, 40, 20, 20);
    set_widget_region((ldBase_t *)ptDown, 40, 88, 20, 20);

    ldBaseSetSelectable((ldBase_t *)ptLeft, true);
    ldBaseSetSelectable((ldBase_t *)ptUp, true);
    ldBaseSetSelectable((ldBase_t *)ptSwitch, true);
    ldBaseSetSelectable((ldBase_t *)ptRight, true);
    ldBaseSetSelectable((ldBase_t *)ptDown, true);

    ldBaseNodeAdd((arm_2d_control_node_t *)ptRoot, (arm_2d_control_node_t *)ptLeft);
    ldBaseNodeAdd((arm_2d_control_node_t *)ptRoot, (arm_2d_control_node_t *)ptUp);
    ldBaseNodeAdd((arm_2d_control_node_t *)ptRoot, (arm_2d_control_node_t *)ptSwitch);
    ldBaseNodeAdd((arm_2d_control_node_t *)ptRoot, (arm_2d_control_node_t *)ptRight);
    ldBaseNodeAdd((arm_2d_control_node_t *)ptRoot, (arm_2d_control_node_t *)ptDown);

    ptScene->ptNodeRoot = (arm_2d_control_node_t *)ptRoot;
    ptScene->ptMsgQueue = NULL;
    ldBaseFocusNavigateInit();
}

static void test_focus_navigation_switch_consumes_only_when_value_changes(void)
{
    ld_scene_t scene;
    ldWindow_t root;
    ldLabel_t left;
    ldSwitch_t sw;
    ldLabel_t right;

    init_focus_runtime_scene(&scene, &root, &left, &sw, &right);

    ldBaseFocusNavigate(&scene, NAV_RIGHT);
    assert(ldBaseIsSelected((ldBase_t *)&sw) == true);
    assert(ldBaseIsSelected((ldBase_t *)&right) == false);

    ldBaseFocusNavigate(&scene, NAV_RIGHT);
    assert(ldSwitchIsChecked(&sw) == true);
    assert(ldBaseIsSelected((ldBase_t *)&sw) == true);

    ldBaseFocusNavigate(&scene, NAV_RIGHT);
    assert(ldSwitchIsChecked(&sw) == true);
    assert(ldBaseIsSelected((ldBase_t *)&sw) == false);
    assert(ldBaseIsSelected((ldBase_t *)&right) == true);
}

static void test_focus_navigation_switch_releases_noop_and_disabled_directions(void)
{
    ld_scene_t scene;
    ldWindow_t root;
    ldLabel_t left;
    ldSwitch_t sw;
    ldLabel_t right;

    init_focus_runtime_scene(&scene, &root, &left, &sw, &right);

    ldBaseFocusNavigate(&scene, NAV_RIGHT);
    ldBaseFocusNavigate(&scene, NAV_RIGHT);
    assert(ldSwitchIsChecked(&sw) == true);
    assert(ldBaseIsSelected((ldBase_t *)&sw) == true);

    ldBaseFocusNavigate(&scene, NAV_LEFT);
    assert(ldSwitchIsChecked(&sw) == false);
    assert(ldBaseIsSelected((ldBase_t *)&sw) == true);

    ldBaseFocusNavigate(&scene, NAV_LEFT);
    assert(ldBaseIsSelected((ldBase_t *)&left) == true);
    assert(ldBaseIsSelected((ldBase_t *)&sw) == false);

    init_focus_runtime_scene(&scene, &root, &left, &sw, &right);
    ldBaseFocusNavigate(&scene, NAV_RIGHT);
    ldSwitchSetDisabled(&sw, true);

    ldBaseFocusNavigate(&scene, NAV_RIGHT);
    assert(ldSwitchIsChecked(&sw) == false);
    assert(ldBaseIsSelected((ldBase_t *)&right) == true);
}

static void test_focus_navigation_switch_releases_all_remaining_noop_directions(void)
{
    ld_scene_t scene;
    ldWindow_t root;
    ldLabel_t left;
    ldLabel_t up;
    ldSwitch_t sw;
    ldLabel_t right;
    ldLabel_t down;

    init_focus_runtime_cross_scene(&scene, &root, &left, &up, &sw, &right, &down);
    ldBaseFocusNavigate(&scene, NAV_RIGHT);
    assert(ldBaseIsSelected((ldBase_t *)&sw) == true);

    ldBaseFocusNavigate(&scene, NAV_LEFT);
    assert(ldSwitchIsChecked(&sw) == false);
    assert(ldBaseIsSelected((ldBase_t *)&left) == true);

    init_focus_runtime_cross_scene(&scene, &root, &left, &up, &sw, &right, &down);
    ldBaseFocusNavigate(&scene, NAV_RIGHT);
    ldBaseFocusNavigate(&scene, NAV_DOWN);
    assert(ldSwitchIsChecked(&sw) == false);
    assert(ldBaseIsSelected((ldBase_t *)&down) == true);

    init_focus_runtime_cross_scene(&scene, &root, &left, &up, &sw, &right, &down);
    ldBaseFocusNavigate(&scene, NAV_RIGHT);
    ldBaseFocusNavigate(&scene, NAV_RIGHT);
    assert(ldSwitchIsChecked(&sw) == true);
    assert(ldBaseIsSelected((ldBase_t *)&sw) == true);

    ldBaseFocusNavigate(&scene, NAV_UP);
    assert(ldSwitchIsChecked(&sw) == true);
    assert(ldBaseIsSelected((ldBase_t *)&up) == true);
}

static void test_focus_navigation_disabled_switch_releases_all_directions(void)
{
    ld_scene_t scene;
    ldWindow_t root;
    ldLabel_t left;
    ldLabel_t up;
    ldSwitch_t sw;
    ldLabel_t right;
    ldLabel_t down;

    init_focus_runtime_cross_scene(&scene, &root, &left, &up, &sw, &right, &down);
    ldBaseFocusNavigate(&scene, NAV_RIGHT);
    ldSwitchSetDisabled(&sw, true);

    ldBaseFocusNavigate(&scene, NAV_LEFT);
    assert(ldBaseIsSelected((ldBase_t *)&left) == true);

    init_focus_runtime_cross_scene(&scene, &root, &left, &up, &sw, &right, &down);
    ldBaseFocusNavigate(&scene, NAV_RIGHT);
    ldSwitchSetDisabled(&sw, true);
    ldBaseFocusNavigate(&scene, NAV_UP);
    assert(ldBaseIsSelected((ldBase_t *)&up) == true);

    init_focus_runtime_cross_scene(&scene, &root, &left, &up, &sw, &right, &down);
    ldBaseFocusNavigate(&scene, NAV_RIGHT);
    ldSwitchSetDisabled(&sw, true);
    ldBaseFocusNavigate(&scene, NAV_RIGHT);
    assert(ldBaseIsSelected((ldBase_t *)&right) == true);

    init_focus_runtime_cross_scene(&scene, &root, &left, &up, &sw, &right, &down);
    ldBaseFocusNavigate(&scene, NAV_RIGHT);
    ldSwitchSetDisabled(&sw, true);
    ldBaseFocusNavigate(&scene, NAV_DOWN);
    assert(ldBaseIsSelected((ldBase_t *)&down) == true);
    assert(ldSwitchIsChecked(&sw) == false);
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

static void test_flex_space_evenly_resolves_start_and_gap(void)
{
    int16_t resolvedGap = -1;
    int16_t start = ldFlexResolveMainStart(LD_FLEX_MAIN_ALIGN_SPACE_EVENLY, 150, 30, 3, 0, &resolvedGap);

    assert(start == 30);
    assert(resolvedGap == 30);
}

static void test_flex_space_around_resolves_start_and_gap(void)
{
    int16_t resolvedGap = -1;
    int16_t start = ldFlexResolveMainStart(LD_FLEX_MAIN_ALIGN_SPACE_AROUND, 150, 30, 3, 0, &resolvedGap);

    assert(start == 20);
    assert(resolvedGap == 40);
}

static void test_flex_space_around_resolves_outer_padding_and_gap(void)
{
    int16_t resolvedGap = -1;
    int16_t start = ldFlexResolveMainStart(ldFlexMainAlignSpaceAround, 200, 80, 3, 10, &resolvedGap);

    assert(start == 20);
    assert(resolvedGap == 50);
}

static void test_flex_space_evenly_resolves_uniform_spacing(void)
{
    int16_t resolvedGap = -1;
    int16_t start = ldFlexResolveMainStart(ldFlexMainAlignSpaceEvenly, 220, 80, 3, 10, &resolvedGap);

    assert(start == 35);
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
    assert(window.flexItemGap == 7);
    assert(window.flexTrackGap == 7);
    assert(window.isLayoutUpdate == true);
    assert(window.use_as__ldBase_t.isDirtyRegionUpdate == true);
}

static void test_flex_extended_setters_mark_window_dirty(void)
{
    ldWindow_t window = {0};

    window.use_as__ldBase_t.isDirtyRegionUpdate = false;
    window.isLayoutUpdate = false;
    ldWindowSetFlexTrackAlign(&window, ldFlexTrackAlignSpaceAround);
    assert(window.flexTrackAlign == ldFlexTrackAlignSpaceAround);
    assert(window.isLayoutUpdate == true);
    assert(window.use_as__ldBase_t.isDirtyRegionUpdate == true);

    window.use_as__ldBase_t.isDirtyRegionUpdate = false;
    window.isLayoutUpdate = false;
    ldWindowSetFlexGap(&window, 7, 9);
    assert(window.flexItemGap == 7);
    assert(window.flexTrackGap == 9);
    assert(window.isLayoutUpdate == true);
    assert(window.use_as__ldBase_t.isDirtyRegionUpdate == true);

    window.use_as__ldBase_t.isDirtyRegionUpdate = false;
    window.isLayoutUpdate = false;
    ldWindowSetGap(&window, 5);
    assert(window.flexItemGap == 5);
    assert(window.flexTrackGap == 5);
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

static void test_flex_row_wrap_creates_second_track(void)
{
    ldWindow_t root = {0};
    ldLabel_t a = {0};
    ldLabel_t b = {0};
    ldLabel_t c = {0};

    root.use_as__ldBase_t.widgetType = widgetTypeWindow;
    root.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize = (arm_2d_size_t){100, 80};

    a.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize = (arm_2d_size_t){40, 10};
    b.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize = (arm_2d_size_t){40, 12};
    c.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize = (arm_2d_size_t){40, 14};

    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&a);
    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&b);
    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&c);

    ldWindowSetFlexFlow(&root, ldFlexFlowRowWrap);
    ldWindowSetFlexGap(&root, 10, 6);
    ldWindowSetPadding(&root, (ldPadding_t){.left = 5, .top = 4, .right = 5, .bottom = 4});

    ldWindow_on_frame_start(NULL, &root);

    assert(a.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 5);
    assert(a.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 4);
    assert(b.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 55);
    assert(b.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 4);
    assert(c.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 5);
    assert(c.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 22);
}

static void test_flex_column_wrap_creates_second_track(void)
{
    ldWindow_t root = {0};
    ldLabel_t a = {0};
    ldLabel_t b = {0};
    ldLabel_t c = {0};

    root.use_as__ldBase_t.widgetType = widgetTypeWindow;
    root.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize = (arm_2d_size_t){90, 90};

    a.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize = (arm_2d_size_t){20, 30};
    b.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize = (arm_2d_size_t){24, 30};
    c.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize = (arm_2d_size_t){28, 30};

    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&a);
    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&b);
    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&c);

    ldWindowSetFlexFlow(&root, ldFlexFlowColumnWrap);
    ldWindowSetFlexGap(&root, 8, 10);
    ldWindowSetPadding(&root, (ldPadding_t){.left = 4, .top = 5, .right = 4, .bottom = 5});

    ldWindow_on_frame_start(NULL, &root);

    assert(a.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 4);
    assert(a.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 5);
    assert(b.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 4);
    assert(b.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 43);
    assert(c.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 38);
    assert(c.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 5);
}

static void test_flex_row_reverse_places_items_from_end(void)
{
    ldWindow_t root = {0};
    ldLabel_t a = {0};
    ldLabel_t b = {0};

    root.use_as__ldBase_t.widgetType = widgetTypeWindow;
    root.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize = (arm_2d_size_t){120, 50};

    a.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize = (arm_2d_size_t){20, 10};
    b.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize = (arm_2d_size_t){30, 10};

    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&a);
    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&b);

    ldWindowSetFlexFlow(&root, ldFlexFlowRowReverse);
    ldWindowSetGap(&root, 10);

    ldWindow_on_frame_start(NULL, &root);

    assert(a.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 60);
    assert(b.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 90);
}

static void test_flex_column_reverse_places_items_from_end(void)
{
    ldWindow_t root = {0};
    ldLabel_t a = {0};
    ldLabel_t b = {0};

    root.use_as__ldBase_t.widgetType = widgetTypeWindow;
    root.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize = (arm_2d_size_t){80, 120};

    a.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize = (arm_2d_size_t){20, 20};
    b.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize = (arm_2d_size_t){30, 30};

    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&a);
    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&b);

    ldWindowSetFlexFlow(&root, ldFlexFlowColumnReverse);
    ldWindowSetGap(&root, 10);

    ldWindow_on_frame_start(NULL, &root);

    assert(a.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 60);
    assert(b.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 90);
}

static void test_flex_track_align_centers_tracks(void)
{
    ldWindow_t root = {0};
    ldLabel_t a = {0};
    ldLabel_t b = {0};
    ldLabel_t c = {0};

    root.use_as__ldBase_t.widgetType = widgetTypeWindow;
    root.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize = (arm_2d_size_t){90, 100};

    a.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize = (arm_2d_size_t){40, 10};
    b.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize = (arm_2d_size_t){40, 12};
    c.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize = (arm_2d_size_t){40, 14};

    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&a);
    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&b);
    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&c);

    ldWindowSetFlexFlow(&root, ldFlexFlowRowWrap);
    ldWindowSetFlexGap(&root, 10, 8);
    ldWindowSetFlexTrackAlign(&root, ldFlexTrackAlignCenter);

    ldWindow_on_frame_start(NULL, &root);

    assert(a.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 33);
    assert(b.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 33);
    assert(c.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 53);
}

static void test_base_flex_child_setters_mark_parent_layout_dirty(void)
{
    ldWindow_t root = {0};
    ldLabel_t child = {0};

    root.layoutTpye = layoutFlex;
    root.use_as__ldBase_t.widgetType = widgetTypeWindow;
    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&child);

    root.isLayoutUpdate = false;
    ldBaseSetFlexGrow((ldBase_t *)&child, 2);
    assert(child.use_as__ldBase_t.flexGrow == 2);
    assert(root.isLayoutUpdate == true);

    root.isLayoutUpdate = false;
    ldBaseSetFlexNewTrack((ldBase_t *)&child, true);
    assert(child.use_as__ldBase_t.flexInNewTrack == true);
    assert(root.isLayoutUpdate == true);

    root.isLayoutUpdate = false;
    ldBaseSetIgnoreLayout((ldBase_t *)&child, true);
    assert(child.use_as__ldBase_t.ignoreLayout == true);
    assert(root.isLayoutUpdate == true);
}

static void test_flex_grow_distributes_remaining_space_by_weight(void)
{
    ldWindow_t root = {0};
    ldLabel_t a = {0};
    ldLabel_t b = {0};

    root.use_as__ldBase_t.widgetType = widgetTypeWindow;
    root.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize = (arm_2d_size_t){120, 40};

    a.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize = (arm_2d_size_t){20, 10};
    b.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize = (arm_2d_size_t){20, 10};

    ldBaseSetFlexGrow((ldBase_t *)&a, 1);
    ldBaseSetFlexGrow((ldBase_t *)&b, 2);

    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&a);
    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&b);

    ldWindowSetFlexFlow(&root, ldFlexFlowRow);
    ldWindowSetGap(&root, 0);

    ldWindow_on_frame_start(NULL, &root);

    assert(a.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iWidth == 46);
    assert(b.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iWidth == 74);
    assert(a.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 0);
    assert(b.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 46);
}

static void test_flex_grow_reset_restores_basis_size(void)
{
    ldWindow_t root = {0};
    ldLabel_t a = {0};
    ldLabel_t b = {0};

    root.use_as__ldBase_t.widgetType = widgetTypeWindow;
    root.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize = (arm_2d_size_t){120, 40};

    a.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize = (arm_2d_size_t){20, 10};
    b.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize = (arm_2d_size_t){20, 10};

    ldBaseSetFlexGrow((ldBase_t *)&a, 1);
    ldBaseSetFlexGrow((ldBase_t *)&b, 2);
    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&a);
    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&b);
    ldWindowSetFlexFlow(&root, ldFlexFlowRow);
    ldWindowSetGap(&root, 0);

    ldWindow_on_frame_start(NULL, &root);
    assert(a.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iWidth == 46);
    assert(b.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iWidth == 74);

    ldBaseSetFlexGrow((ldBase_t *)&a, 0);
    ldBaseSetFlexGrow((ldBase_t *)&b, 0);
    ldWindow_on_frame_start(NULL, &root);

    assert(a.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iWidth == 20);
    assert(b.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iWidth == 20);
    assert(a.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 0);
    assert(b.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 20);
}

static void test_flex_wrap_and_grow_expand_last_track_without_overflow(void)
{
    ldWindow_t root = {0};
    ldLabel_t a = {0};
    ldLabel_t b = {0};
    ldLabel_t c = {0};

    init_window_region(&root, 100, 60);
    set_widget_region((ldBase_t *)&a, 0, 0, 40, 10);
    set_widget_region((ldBase_t *)&b, 0, 0, 40, 10);
    set_widget_region((ldBase_t *)&c, 0, 0, 20, 10);

    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&a);
    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&b);
    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&c);

    ldWindowSetFlexFlow(&root, LD_FLEX_FLOW_ROW_WRAP);
    ldWindowSetFlexAlign(&root, ldFlexMainAlignStart, ldFlexCrossAlignStart);
    ldWindowSetGap(&root, 5);
    ldBaseSetFlexGrow((ldBase_t *)&c, 1);

    ldWindow_on_frame_start(NULL, &root);

    assert(a.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 0);
    assert(a.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 0);
    assert(b.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 45);
    assert(b.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 0);
    assert(c.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 0);
    assert(c.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 15);
    assert(c.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iWidth == 100);
}

static void test_flex_min_size_hook_affects_wrap_capacity(void)
{
    ldWindow_t root = {0};
    ldLabel_t a = {0};
    ldLabel_t b = {0};

    init_window_region(&root, 100, 60);
    set_widget_region((ldBase_t *)&a, 0, 0, 20, 10);
    set_widget_region((ldBase_t *)&b, 0, 0, 50, 10);

    a.use_as__ldBase_t.flexMinSize = (arm_2d_size_t){60, 10};
    a.use_as__ldBase_t.hasFlexMinWidth = true;

    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&a);
    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&b);

    ldWindowSetFlexFlow(&root, ldFlexFlowRowWrap);
    ldWindowSetFlexAlign(&root, ldFlexMainAlignStart, ldFlexCrossAlignStart);
    ldWindowSetGap(&root, 5);

    ldWindow_on_frame_start(NULL, &root);

    assert(a.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iWidth == 60);
    assert(a.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 0);
    assert(a.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 0);
    assert(b.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 0);
    assert(b.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 15);
}

static void test_flex_max_size_hook_caps_grow_without_overflow(void)
{
    ldWindow_t root = {0};
    ldLabel_t a = {0};
    ldLabel_t b = {0};

    init_window_region(&root, 140, 40);
    set_widget_region((ldBase_t *)&a, 0, 0, 20, 10);
    set_widget_region((ldBase_t *)&b, 0, 0, 20, 10);

    a.use_as__ldBase_t.flexMaxSize = (arm_2d_size_t){40, 10};
    a.use_as__ldBase_t.hasFlexMaxWidth = true;

    ldBaseSetFlexGrow((ldBase_t *)&a, 1);
    ldBaseSetFlexGrow((ldBase_t *)&b, 1);
    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&a);
    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&b);

    ldWindowSetFlexFlow(&root, ldFlexFlowRow);
    ldWindowSetFlexAlign(&root, ldFlexMainAlignStart, ldFlexCrossAlignStart);
    ldWindowSetGap(&root, 10);

    ldWindow_on_frame_start(NULL, &root);

    assert(a.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iWidth == 40);
    assert(b.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iWidth == 90);
    assert(a.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 0);
    assert(b.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 50);
    assert((b.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX +
            b.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iWidth) == 140);
}

static void test_flex_new_track_forces_wrap_before_capacity_runs_out(void)
{
    ldWindow_t root = {0};
    ldLabel_t a = {0};
    ldLabel_t b = {0};
    ldLabel_t c = {0};

    root.use_as__ldBase_t.widgetType = widgetTypeWindow;
    root.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize = (arm_2d_size_t){200, 90};

    a.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize = (arm_2d_size_t){40, 10};
    b.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize = (arm_2d_size_t){40, 12};
    c.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize = (arm_2d_size_t){40, 14};

    ldBaseSetFlexNewTrack((ldBase_t *)&c, true);

    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&a);
    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&b);
    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&c);

    ldWindowSetFlexFlow(&root, ldFlexFlowRowWrap);
    ldWindowSetFlexGap(&root, 10, 6);

    ldWindow_on_frame_start(NULL, &root);

    assert(a.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 0);
    assert(b.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 0);
    assert(c.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 18);
}

static void test_flex_ignore_layout_keeps_manual_position_and_skips_slot(void)
{
    ldWindow_t root = {0};
    ldLabel_t a = {0};
    ldLabel_t ignored = {0};
    ldLabel_t c = {0};

    root.use_as__ldBase_t.widgetType = widgetTypeWindow;
    root.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize = (arm_2d_size_t){120, 50};

    a.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion = (arm_2d_region_t){{0, 0}, {20, 10}};
    ignored.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion = (arm_2d_region_t){{77, 19}, {15, 8}};
    c.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion = (arm_2d_region_t){{0, 0}, {20, 10}};

    ldBaseSetIgnoreLayout((ldBase_t *)&ignored, true);

    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&a);
    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&ignored);
    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&c);

    ldWindowSetFlexFlow(&root, ldFlexFlowRow);
    ldWindowSetGap(&root, 10);

    ldWindow_on_frame_start(NULL, &root);

    assert(a.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 0);
    assert(c.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 30);
    assert(ignored.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 77);
    assert(ignored.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 19);
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

static void test_grid_layout_ignore_layout_keeps_manual_coordinates_and_skips_slot(void)
{
    ldWindow_t root = {0};
    ldLabel_t a = {0};
    ldLabel_t overlay = {0};
    ldLabel_t b = {0};

    init_window_region(&root, 100, 80);
    set_widget_region((ldBase_t *)&a, -1, -1, 50, 10);
    set_widget_region((ldBase_t *)&overlay, 77, 19, 14, 14);
    set_widget_region((ldBase_t *)&b, -2, -2, 20, 12);

    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&a);
    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&overlay);
    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&b);

    ldWindowSetGridColumns(&root, 2);
    ldWindowSetGridGap(&root, 5, 4);
    ldWindowSetGridPadding(&root, (ldPadding_t){.left = 3, .top = 2, .right = 5, .bottom = 7});
    ldBaseSetIgnoreLayout((ldBase_t *)&overlay, true);

    ldWindow_on_frame_start(NULL, &root);

    assert(a.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 3);
    assert(a.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 2);
    assert(overlay.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 77);
    assert(overlay.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 19);
    assert(overlay.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iWidth == 14);
    assert(overlay.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iHeight == 14);
    assert(b.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 51);
    assert(b.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 2);
}

static void test_flex_row_wrap_starts_new_track_when_main_axis_overflows(void)
{
    ldWindow_t root = {0};
    ldLabel_t a = {0};
    ldLabel_t b = {0};
    ldLabel_t c = {0};

    init_window_region(&root, 100, 60);
    set_widget_region((ldBase_t *)&a, 0, 0, 40, 10);
    set_widget_region((ldBase_t *)&b, 0, 0, 40, 10);
    set_widget_region((ldBase_t *)&c, 0, 0, 40, 10);

    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&a);
    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&b);
    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&c);

    ldWindowSetFlexFlow(&root, LD_FLEX_FLOW_ROW_WRAP);
    ldWindowSetFlexAlign(&root, ldFlexMainAlignStart, ldFlexCrossAlignStart);
    ldWindowSetGap(&root, 5);

    ldWindow_on_frame_start(NULL, &root);

    assert(a.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 0);
    assert(a.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 0);
    assert(b.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 45);
    assert(b.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 0);
    assert(c.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 0);
    assert(c.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 15);
}

static void test_flex_column_wrap_starts_new_track_when_main_axis_overflows(void)
{
    ldWindow_t root = {0};
    ldLabel_t a = {0};
    ldLabel_t b = {0};
    ldLabel_t c = {0};

    init_window_region(&root, 80, 60);
    set_widget_region((ldBase_t *)&a, 0, 0, 30, 20);
    set_widget_region((ldBase_t *)&b, 0, 0, 30, 20);
    set_widget_region((ldBase_t *)&c, 0, 0, 30, 20);

    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&a);
    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&b);
    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&c);

    ldWindowSetFlexFlow(&root, LD_FLEX_FLOW_COLUMN_WRAP);
    ldWindowSetFlexAlign(&root, ldFlexMainAlignStart, ldFlexCrossAlignStart);
    ldWindowSetGap(&root, 5);

    ldWindow_on_frame_start(NULL, &root);

    assert(a.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 0);
    assert(a.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 0);
    assert(b.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 0);
    assert(b.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 25);
    assert(c.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 35);
    assert(c.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 0);
}

static void test_flex_row_reverse_places_first_child_from_right_edge(void)
{
    ldWindow_t root = {0};
    ldLabel_t a = {0};
    ldLabel_t b = {0};

    init_window_region(&root, 120, 40);
    set_widget_region((ldBase_t *)&a, 0, 0, 20, 10);
    set_widget_region((ldBase_t *)&b, 0, 0, 30, 10);

    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&a);
    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&b);

    ldWindowSetFlexFlow(&root, LD_FLEX_FLOW_ROW_REVERSE);
    ldWindowSetFlexAlign(&root, ldFlexMainAlignStart, ldFlexCrossAlignStart);
    ldWindowSetGap(&root, 10);

    ldWindow_on_frame_start(NULL, &root);

    assert(a.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 100);
    assert(a.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 0);
    assert(b.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 60);
    assert(b.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 0);
}

static void test_flex_column_reverse_places_first_child_from_bottom_edge(void)
{
    ldWindow_t root = {0};
    ldLabel_t a = {0};
    ldLabel_t b = {0};

    init_window_region(&root, 40, 100);
    set_widget_region((ldBase_t *)&a, 0, 0, 20, 10);
    set_widget_region((ldBase_t *)&b, 0, 0, 20, 20);

    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&a);
    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&b);

    ldWindowSetFlexFlow(&root, LD_FLEX_FLOW_COLUMN_REVERSE);
    ldWindowSetFlexAlign(&root, ldFlexMainAlignStart, ldFlexCrossAlignStart);
    ldWindowSetGap(&root, 5);

    ldWindow_on_frame_start(NULL, &root);

    assert(a.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 0);
    assert(a.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 90);
    assert(b.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 0);
    assert(b.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 65);
}

static void test_flex_row_wrap_reverse_stacks_tracks_from_bottom(void)
{
    ldWindow_t root = {0};
    ldLabel_t a = {0};
    ldLabel_t b = {0};
    ldLabel_t c = {0};

    init_window_region(&root, 100, 60);
    set_widget_region((ldBase_t *)&a, 0, 0, 40, 10);
    set_widget_region((ldBase_t *)&b, 0, 0, 40, 10);
    set_widget_region((ldBase_t *)&c, 0, 0, 40, 10);

    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&a);
    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&b);
    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&c);

    ldWindowSetFlexFlow(&root, LD_FLEX_FLOW_ROW_WRAP_REVERSE);
    ldWindowSetFlexAlign(&root, ldFlexMainAlignStart, ldFlexCrossAlignStart);
    ldWindowSetGap(&root, 5);

    ldWindow_on_frame_start(NULL, &root);

    assert(c.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 60);
    assert(c.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 50);
    assert(b.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 15);
    assert(b.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 50);
    assert(a.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 60);
    assert(a.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 35);
}

static void test_flex_column_wrap_reverse_stacks_tracks_from_right(void)
{
    ldWindow_t root = {0};
    ldLabel_t a = {0};
    ldLabel_t b = {0};
    ldLabel_t c = {0};

    init_window_region(&root, 80, 60);
    set_widget_region((ldBase_t *)&a, 0, 0, 30, 20);
    set_widget_region((ldBase_t *)&b, 0, 0, 30, 20);
    set_widget_region((ldBase_t *)&c, 0, 0, 30, 20);

    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&a);
    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&b);
    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&c);

    ldWindowSetFlexFlow(&root, LD_FLEX_FLOW_COLUMN_WRAP_REVERSE);
    ldWindowSetFlexAlign(&root, ldFlexMainAlignStart, ldFlexCrossAlignStart);
    ldWindowSetGap(&root, 5);

    ldWindow_on_frame_start(NULL, &root);

    assert(c.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 50);
    assert(c.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 40);
    assert(b.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 50);
    assert(b.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 15);
    assert(a.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 15);
    assert(a.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 40);
}

static void test_hidden_and_ignore_layout_have_different_effects(void)
{
    ldWindow_t root = {0};
    ldLabel_t visible = {0};
    ldLabel_t hidden = {0};
    ldLabel_t overlay = {0};

    init_window_region(&root, 120, 40);
    set_widget_region((ldBase_t *)&visible, 0, 0, 30, 10);
    set_widget_region((ldBase_t *)&hidden, 10, 10, 30, 10);
    set_widget_region((ldBase_t *)&overlay, 77, 11, 15, 15);

    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&visible);
    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&hidden);
    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&overlay);

    ldWindowSetFlexFlow(&root, ldFlexFlowRow);
    ldWindowSetGap(&root, 5);
    ldBaseSetHidden((ldBase_t *)&hidden, true);
    ldBaseSetIgnoreLayout((ldBase_t *)&overlay, true);

    ldWindow_on_frame_start(NULL, &root);

    assert(visible.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 0);
    assert(hidden.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX != 10);
    assert(hidden.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY != 10);
    assert(overlay.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 77);
    assert(overlay.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 11);
}

#if defined(LDGUI_FLEX_API_CONTRACT)
static void test_flex_track_align_centers_tracks_in_cross_axis(void)
{
    ldWindow_t root = {0};
    ldLabel_t a = {0};
    ldLabel_t b = {0};
    ldLabel_t c = {0};

    init_window_region(&root, 100, 70);
    set_widget_region((ldBase_t *)&a, 0, 0, 40, 10);
    set_widget_region((ldBase_t *)&b, 0, 0, 40, 10);
    set_widget_region((ldBase_t *)&c, 0, 0, 40, 10);

    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&a);
    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&b);
    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&c);

    ldWindowSetFlexFlow(&root, LD_FLEX_FLOW_ROW_WRAP);
    ldWindowSetFlexAlign(&root, ldFlexMainAlignStart, ldFlexCrossAlignStart);
    ldWindowSetFlexTrackAlign(&root, LD_FLEX_TRACK_ALIGN_CENTER);
    ldWindowSetGap(&root, 5);

    ldWindow_on_frame_start(NULL, &root);

    assert(a.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 22);
    assert(b.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 22);
    assert(c.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 37);
}

static void test_flex_grow_distributes_remaining_main_size(void)
{
    ldWindow_t root = {0};
    ldLabel_t a = {0};
    ldLabel_t b = {0};

    init_window_region(&root, 140, 40);
    set_widget_region((ldBase_t *)&a, 0, 0, 20, 10);
    set_widget_region((ldBase_t *)&b, 0, 0, 20, 10);

    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&a);
    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&b);

    ldWindowSetFlexFlow(&root, ldFlexFlowRow);
    ldWindowSetFlexAlign(&root, ldFlexMainAlignStart, ldFlexCrossAlignStart);
    ldWindowSetGap(&root, 10);
    ldBaseSetFlexGrow((ldBase_t *)&a, 1);
    ldBaseSetFlexGrow((ldBase_t *)&b, 2);

    ldWindow_on_frame_start(NULL, &root);

    assert(a.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iWidth == 50);
    assert(b.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iWidth == 80);
}

static void test_flex_new_track_forces_wrapping_even_when_space_remains(void)
{
    ldWindow_t root = {0};
    ldLabel_t a = {0};
    ldLabel_t b = {0};
    ldLabel_t c = {0};

    init_window_region(&root, 120, 60);
    set_widget_region((ldBase_t *)&a, 0, 0, 30, 10);
    set_widget_region((ldBase_t *)&b, 0, 0, 30, 10);
    set_widget_region((ldBase_t *)&c, 0, 0, 30, 10);

    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&a);
    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&b);
    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&c);

    ldWindowSetFlexFlow(&root, LD_FLEX_FLOW_ROW_WRAP);
    ldWindowSetFlexAlign(&root, ldFlexMainAlignStart, ldFlexCrossAlignStart);
    ldWindowSetGap(&root, 10);
    ldBaseSetFlexNewTrack((ldBase_t *)&b, true);

    ldWindow_on_frame_start(NULL, &root);

    assert(a.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 0);
    assert(a.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 0);
    assert(b.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 0);
    assert(b.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 20);
    assert(c.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 40);
    assert(c.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 20);
}

static void test_flex_ignore_layout_keeps_manual_coordinates_and_skips_slot(void)
{
    ldWindow_t root = {0};
    ldLabel_t a = {0};
    ldLabel_t overlay = {0};
    ldLabel_t b = {0};

    init_window_region(&root, 120, 40);
    set_widget_region((ldBase_t *)&a, 0, 0, 30, 10);
    set_widget_region((ldBase_t *)&overlay, 77, 11, 15, 15);
    set_widget_region((ldBase_t *)&b, 0, 0, 30, 10);

    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&a);
    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&overlay);
    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&b);

    ldWindowSetFlexFlow(&root, ldFlexFlowRow);
    ldWindowSetFlexAlign(&root, ldFlexMainAlignStart, ldFlexCrossAlignStart);
    ldWindowSetGap(&root, 5);
    ldBaseSetIgnoreLayout((ldBase_t *)&overlay, true);

    ldWindow_on_frame_start(NULL, &root);

    assert(a.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 0);
    assert(a.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 0);
    assert(overlay.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 77);
    assert(overlay.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 11);
    assert(b.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 35);
    assert(b.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 0);
}

static void test_flex_child_metadata_setters_mark_parent_layout_dirty(void)
{
    ldWindow_t root = {0};
    ldLabel_t child = {0};

    init_window_region(&root, 80, 40);
    root.layoutTpye = layoutFlex;
    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&child);

    root.isLayoutUpdate = false;
    ldBaseSetFlexGrow((ldBase_t *)&child, 2);
    assert(root.isLayoutUpdate == true);

    root.isLayoutUpdate = false;
    ldBaseSetFlexNewTrack((ldBase_t *)&child, true);
    assert(root.isLayoutUpdate == true);

    root.isLayoutUpdate = false;
    ldBaseSetIgnoreLayout((ldBase_t *)&child, true);
    assert(root.isLayoutUpdate == true);
}
#endif

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

static void test_grid_descriptor_layout_auto_places_visible_children(void)
{
    ldWindow_t root = {0};
    ldLabel_t a = {0};
    ldLabel_t hidden = {0};
    ldLabel_t c = {0};
    ldLabel_t d = {0};
    static const int16_t col_dsc[] = {30, 30, LD_GRID_TEMPLATE_LAST};
    static const int16_t row_dsc[] = {20, 20, LD_GRID_TEMPLATE_LAST};

    init_window_region(&root, 80, 60);
    set_widget_region((ldBase_t *)&a, -1, -1, 10, 8);
    set_widget_region((ldBase_t *)&hidden, 77, 66, 12, 12);
    set_widget_region((ldBase_t *)&c, -2, -2, 10, 8);
    set_widget_region((ldBase_t *)&d, -3, -3, 10, 8);
    hidden.use_as__ldBase_t.isHidden = true;

    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&a);
    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&hidden);
    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&c);
    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&d);

    ldWindowSetGridDscArray(&root, col_dsc, row_dsc);
    ldWindowSetGridGap(&root, 4, 6);

    ldWindow_on_frame_start(NULL, &root);

    assert(a.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 0);
    assert(a.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 0);
    assert(hidden.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 77);
    assert(hidden.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 66);
    assert(c.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 36);
    assert(c.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 0);
    assert(d.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 0);
    assert(d.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 24);
}

static void test_grid_descriptor_layout_ignore_layout_keeps_manual_coordinates_and_skips_auto_cell(void)
{
    ldWindow_t root = {0};
    ldLabel_t a = {0};
    ldLabel_t overlay = {0};
    ldLabel_t c = {0};
    ldLabel_t d = {0};
    static const int16_t col_dsc[] = {30, 30, LD_GRID_TEMPLATE_LAST};
    static const int16_t row_dsc[] = {20, 20, LD_GRID_TEMPLATE_LAST};

    init_window_region(&root, 80, 60);
    set_widget_region((ldBase_t *)&a, -1, -1, 10, 8);
    set_widget_region((ldBase_t *)&overlay, 61, 33, 12, 12);
    set_widget_region((ldBase_t *)&c, -2, -2, 10, 8);
    set_widget_region((ldBase_t *)&d, -3, -3, 10, 8);

    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&a);
    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&overlay);
    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&c);
    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&d);

    ldWindowSetGridDscArray(&root, col_dsc, row_dsc);
    ldWindowSetGridGap(&root, 4, 6);
    ldBaseSetIgnoreLayout((ldBase_t *)&overlay, true);

    ldWindow_on_frame_start(NULL, &root);

    assert(a.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 0);
    assert(a.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 0);
    assert(overlay.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 61);
    assert(overlay.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 33);
    assert(overlay.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iWidth == 12);
    assert(overlay.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iHeight == 12);
    assert(c.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 36);
    assert(c.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 0);
    assert(d.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 0);
    assert(d.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 24);
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

static void test_grid_layout_space_between_distributes_remaining_gap(void)
{
    ldWindow_t root = {0};
    ldLabel_t left = {0};
    ldLabel_t right = {0};
    static const int16_t col_dsc[] = {20, 20, LD_GRID_TEMPLATE_LAST};
    static const int16_t row_dsc[] = {12, LD_GRID_TEMPLATE_LAST};

    init_window_region(&root, 100, 20);
    set_widget_region((ldBase_t *)&left, 0, 0, 10, 8);
    set_widget_region((ldBase_t *)&right, 0, 0, 10, 8);

    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&left);
    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&right);

    ldWindowSetGridDscArray(&root, col_dsc, row_dsc);
    ldWindowSetGridAlign(&root, ldGridAlignSpaceBetween, ldGridAlignStart);
    ldBaseSetGridCell((ldBase_t *)&left, ldGridAlignStart, 0, 1, ldGridAlignStart, 0, 1);
    ldBaseSetGridCell((ldBase_t *)&right, ldGridAlignStart, 1, 1, ldGridAlignStart, 0, 1);

    ldWindow_on_frame_start(NULL, &root);

    assert(left.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 0);
    assert(right.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 80);
}

static void test_grid_layout_space_around_distributes_outer_spacing(void)
{
    ldWindow_t root = {0};
    ldLabel_t left = {0};
    ldLabel_t right = {0};
    static const int16_t col_dsc[] = {20, 20, LD_GRID_TEMPLATE_LAST};
    static const int16_t row_dsc[] = {12, LD_GRID_TEMPLATE_LAST};

    init_window_region(&root, 100, 20);
    set_widget_region((ldBase_t *)&left, 0, 0, 10, 8);
    set_widget_region((ldBase_t *)&right, 0, 0, 10, 8);

    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&left);
    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&right);

    ldWindowSetGridDscArray(&root, col_dsc, row_dsc);
    ldWindowSetGridAlign(&root, ldGridAlignSpaceAround, ldGridAlignStart);
    ldBaseSetGridCell((ldBase_t *)&left, ldGridAlignStart, 0, 1, ldGridAlignStart, 0, 1);
    ldBaseSetGridCell((ldBase_t *)&right, ldGridAlignStart, 1, 1, ldGridAlignStart, 0, 1);

    ldWindow_on_frame_start(NULL, &root);

    assert(left.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 15);
    assert(right.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 65);
}

static void test_grid_layout_space_evenly_distributes_uniform_spacing(void)
{
    ldWindow_t root = {0};
    ldLabel_t left = {0};
    ldLabel_t right = {0};
    static const int16_t col_dsc[] = {20, 20, LD_GRID_TEMPLATE_LAST};
    static const int16_t row_dsc[] = {12, LD_GRID_TEMPLATE_LAST};

    init_window_region(&root, 100, 20);
    set_widget_region((ldBase_t *)&left, 0, 0, 10, 8);
    set_widget_region((ldBase_t *)&right, 0, 0, 10, 8);

    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&left);
    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&right);

    ldWindowSetGridDscArray(&root, col_dsc, row_dsc);
    ldWindowSetGridAlign(&root, ldGridAlignSpaceEvenly, ldGridAlignStart);
    ldBaseSetGridCell((ldBase_t *)&left, ldGridAlignStart, 0, 1, ldGridAlignStart, 0, 1);
    ldBaseSetGridCell((ldBase_t *)&right, ldGridAlignStart, 1, 1, ldGridAlignStart, 0, 1);

    ldWindow_on_frame_start(NULL, &root);

    assert(left.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 20);
    assert(right.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 60);
}

static void test_grid_layout_stretch_expands_track_sizes(void)
{
    ldWindow_t root = {0};
    ldLabel_t left = {0};
    ldLabel_t right = {0};
    static const int16_t col_dsc[] = {20, 20, LD_GRID_TEMPLATE_LAST};
    static const int16_t row_dsc[] = {12, LD_GRID_TEMPLATE_LAST};

    init_window_region(&root, 100, 20);
    set_widget_region((ldBase_t *)&left, 0, 0, 10, 8);
    set_widget_region((ldBase_t *)&right, 0, 0, 10, 8);

    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&left);
    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&right);

    ldWindowSetGridDscArray(&root, col_dsc, row_dsc);
    ldWindowSetGridAlign(&root, ldGridAlignStretch, ldGridAlignStart);
    ldBaseSetGridCell((ldBase_t *)&left, ldGridAlignStretch, 0, 1, ldGridAlignStretch, 0, 1);
    ldBaseSetGridCell((ldBase_t *)&right, ldGridAlignStretch, 1, 1, ldGridAlignStretch, 0, 1);

    ldWindow_on_frame_start(NULL, &root);

    assert(left.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 0);
    assert(left.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iWidth == 50);
    assert(right.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 50);
    assert(right.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iWidth == 50);
}

static void test_grid_layout_content_track_uses_largest_visible_child(void)
{
    ldWindow_t root = {0};
    ldLabel_t small = {0};
    ldLabel_t large = {0};
    static const int16_t col_dsc[] = {LD_GRID_CONTENT, LD_GRID_FR(1), LD_GRID_TEMPLATE_LAST};
    static const int16_t row_dsc[] = {20, LD_GRID_TEMPLATE_LAST};

    init_window_region(&root, 160, 40);
    set_widget_region((ldBase_t *)&small, 0, 0, 24, 8);
    set_widget_region((ldBase_t *)&large, 0, 0, 58, 8);

    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&small);
    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&large);

    ldWindowSetGridDscArray(&root, col_dsc, row_dsc);
    ldBaseSetGridCell((ldBase_t *)&small, ldGridAlignStart, 0, 1, ldGridAlignStart, 0, 1);
    ldBaseSetGridCell((ldBase_t *)&large, ldGridAlignStart, 0, 1, ldGridAlignStart, 0, 1);

    ldWindow_on_frame_start(NULL, &root);

    assert(large.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 0);
    assert(large.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iWidth == 58);
}

static void test_grid_layout_auto_placement_skips_occupied_spans(void)
{
    ldWindow_t root = {0};
    ldLabel_t span = {0};
    ldLabel_t auto_a = {0};
    ldLabel_t auto_b = {0};
    static const int16_t col_dsc[] = {30, 30, 30, LD_GRID_TEMPLATE_LAST};
    static const int16_t row_dsc[] = {18, 18, LD_GRID_TEMPLATE_LAST};

    init_window_region(&root, 120, 60);
    set_widget_region((ldBase_t *)&span, 0, 0, 12, 8);
    set_widget_region((ldBase_t *)&auto_a, 0, 0, 12, 8);
    set_widget_region((ldBase_t *)&auto_b, 0, 0, 12, 8);

    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&span);
    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&auto_a);
    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&auto_b);

    ldWindowSetGridDscArray(&root, col_dsc, row_dsc);
    ldWindowSetGridGap(&root, 4, 6);
    ldBaseSetGridCell((ldBase_t *)&span, ldGridAlignStretch, 0, 2, ldGridAlignStretch, 0, 1);

    ldWindow_on_frame_start(NULL, &root);

    assert(auto_a.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 72);
    assert(auto_a.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 0);
    assert(auto_b.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 0);
    assert(auto_b.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 22);
}

static void test_grid_layout_span_with_fr_tracks_preserves_expected_width(void)
{
    ldWindow_t root = {0};
    ldLabel_t child = {0};
    static const int16_t col_dsc[] = {40, LD_GRID_FR(1), LD_GRID_FR(1), LD_GRID_TEMPLATE_LAST};
    static const int16_t row_dsc[] = {20, LD_GRID_TEMPLATE_LAST};

    init_window_region(&root, 160, 40);
    set_widget_region((ldBase_t *)&child, 0, 0, 10, 8);

    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&child);
    ldWindowSetGridDscArray(&root, col_dsc, row_dsc);
    ldWindowSetGridGap(&root, 0, 4);
    ldBaseSetGridCell((ldBase_t *)&child, ldGridAlignStretch, 1, 2, ldGridAlignStretch, 0, 1);

    ldWindow_on_frame_start(NULL, &root);

    assert(child.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 44);
    assert(child.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iWidth == 116);
}

static void test_grid_layout_container_center_keeps_track_order_and_offsets(void)
{
    ldWindow_t root = {0};
    ldLabel_t left = {0};
    ldLabel_t right = {0};
    static const int16_t col_dsc[] = {20, 20, LD_GRID_TEMPLATE_LAST};
    static const int16_t row_dsc[] = {12, LD_GRID_TEMPLATE_LAST};

    init_window_region(&root, 90, 20);
    set_widget_region((ldBase_t *)&left, 0, 0, 10, 8);
    set_widget_region((ldBase_t *)&right, 0, 0, 10, 8);

    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&left);
    ldBaseNodeAdd((arm_2d_control_node_t *)&root, (arm_2d_control_node_t *)&right);

    ldWindowSetGridDscArray(&root, col_dsc, row_dsc);
    ldWindowSetGridGap(&root, 0, 10);
    ldWindowSetGridAlign(&root, ldGridAlignCenter, ldGridAlignStart);
    ldBaseSetGridCell((ldBase_t *)&left, ldGridAlignStart, 0, 1, ldGridAlignStart, 0, 1);
    ldBaseSetGridCell((ldBase_t *)&right, ldGridAlignStart, 1, 1, ldGridAlignStart, 0, 1);

    ldWindow_on_frame_start(NULL, &root);

    assert(left.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 20);
    assert(right.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 50);
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
    test_flex_space_around_resolves_outer_padding_and_gap();
    test_flex_space_evenly_resolves_uniform_spacing();
    test_flex_setters_mark_window_dirty();
    test_flex_extended_setters_mark_window_dirty();
    test_grid_setters_mark_window_dirty();
    test_grid_descriptor_setters_mark_window_dirty();
    test_grid_cell_setter_marks_parent_dirty_and_stores_metadata();
    test_flex_row_layout_skips_hidden_children_and_keeps_size();
    test_flex_column_layout_applies_main_and_cross_alignment();
    test_flex_row_wrap_creates_second_track();
    test_flex_column_wrap_creates_second_track();
    test_flex_row_reverse_places_items_from_end();
    test_flex_column_reverse_places_items_from_end();
    test_flex_row_wrap_reverse_stacks_tracks_from_bottom();
    test_flex_column_wrap_reverse_stacks_tracks_from_right();
    test_flex_track_align_centers_tracks();
    test_base_flex_child_setters_mark_parent_layout_dirty();
    test_flex_grow_distributes_remaining_space_by_weight();
    test_flex_grow_reset_restores_basis_size();
    test_flex_wrap_and_grow_expand_last_track_without_overflow();
    test_flex_min_size_hook_affects_wrap_capacity();
    test_flex_max_size_hook_caps_grow_without_overflow();
    test_flex_new_track_forces_wrap_before_capacity_runs_out();
    test_flex_ignore_layout_keeps_manual_position_and_skips_slot();
    test_hidden_and_ignore_layout_have_different_effects();
    test_grid_layout_places_visible_children_row_first();
    test_grid_layout_ignore_layout_keeps_manual_coordinates_and_skips_slot();
    test_grid_layout_uses_fixed_tracks_and_explicit_cells();
    test_grid_layout_resolves_fr_and_cell_alignment();
    test_grid_layout_resolves_content_tracks_and_hidden_children();
    test_grid_descriptor_layout_auto_places_visible_children();
    test_grid_descriptor_layout_ignore_layout_keeps_manual_coordinates_and_skips_auto_cell();
    test_grid_layout_supports_span_and_container_alignment();
    test_grid_layout_space_between_distributes_remaining_gap();
    test_grid_layout_space_around_distributes_outer_spacing();
    test_grid_layout_space_evenly_distributes_uniform_spacing();
    test_grid_layout_stretch_expands_track_sizes();
    test_grid_layout_content_track_uses_largest_visible_child();
    test_grid_layout_auto_placement_skips_occupied_spans();
    test_grid_layout_span_with_fr_tracks_preserves_expected_width();
    test_grid_layout_container_center_keeps_track_order_and_offsets();
    test_grid_layout_clamps_invalid_cell_settings();
    test_focus_navigation_switch_consumes_only_when_value_changes();
    test_focus_navigation_switch_releases_noop_and_disabled_directions();
    test_focus_navigation_switch_releases_all_remaining_noop_directions();
    test_focus_navigation_disabled_switch_releases_all_directions();
    return 0;
}
