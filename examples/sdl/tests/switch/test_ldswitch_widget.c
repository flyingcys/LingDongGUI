#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "arm_2d_helper_shape.h"

#include "../../../../src/gui/ldSwitch.c"

static uint32_t g_emit_count;
static uint8_t g_last_signal;
static uint64_t g_last_value;
static uint32_t g_color_call_count;
static uint32_t g_image_call_count;
static ldColor g_color_calls[8];

const arm_2d_tile_t c_tileWhiteDotMask = {0};
const arm_2d_tile_t c_tileWhiteDotMask2 = {0};
const arm_2d_tile_t c_tileCircleMask = {0};
const arm_2d_tile_t c_tileCircleMask2 = {0};

void *ldMalloc(uint32_t size)
{
    return malloc(size);
}

void *ldCalloc(uint32_t num, uint32_t size)
{
    return calloc(num, size);
}

void ldFree(void *p)
{
    free(p);
}

void *ldBaseGetWidget(arm_2d_control_node_t *ptNodeRoot, uint16_t nameId)
{
    (void)nameId;
    return ptNodeRoot;
}

void ldBaseNodeAdd(arm_2d_control_node_t *parent, arm_2d_control_node_t *child)
{
    (void)parent;
    (void)child;
}

void ldBaseNodeRemove(arm_2d_control_node_t *ptNode)
{
    (void)ptNode;
}

bool ldMsgConnect(void *ptSender, uint8_t signal, assnFunc pFunc)
{
    (void)ptSender;
    (void)signal;
    (void)pFunc;
    return true;
}

void ldMsgDelConnect(void *ptSender)
{
    (void)ptSender;
}

bool ldMsgEmit(xQueue_t *ptQueue, void *ptSender, uint8_t signal, uint64_t value)
{
    (void)ptQueue;
    (void)ptSender;
    g_emit_count++;
    g_last_signal = signal;
    g_last_value = value;
    return true;
}

bool ldBaseIsHidden(ldBase_t *ptWidget)
{
    (void)ptWidget;
    return false;
}

void ldBaseColor(arm_2d_tile_t *ptTile, arm_2d_region_t *ptRegion, ldColor color, uint8_t opacity)
{
    (void)ptTile;
    (void)ptRegion;
    (void)opacity;
    if (g_color_call_count < (sizeof(g_color_calls) / sizeof(g_color_calls[0])))
    {
        g_color_calls[g_color_call_count] = color;
    }
    g_color_call_count++;
}

void ldBaseImage(arm_2d_tile_t *ptTile,
                 arm_2d_region_t *ptRegion,
                 arm_2d_tile_t *ptImgTile,
                 arm_2d_tile_t *ptMaskTile,
                 ldColor color,
                 uint8_t opacity)
{
    (void)ptTile;
    (void)ptRegion;
    (void)ptImgTile;
    (void)ptMaskTile;
    (void)color;
    (void)opacity;
    g_image_call_count++;
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

void __draw_round_corner_border(const arm_2d_tile_t *ptTarget,
                                const arm_2d_region_t *ptRegion,
                                COLOUR_INT tColour,
                                arm_2d_border_opacity_t Opacity,
                                arm_2d_corner_opacity_t CornerOpacity,
                                const arm_2d_tile_t *ptCircleMask)
{
    (void)ptTarget;
    (void)ptRegion;
    (void)tColour;
    (void)Opacity;
    (void)CornerOpacity;
    (void)ptCircleMask;
}

arm_2d_region_t *arm_2d_get_default_region(void)
{
    static arm_2d_region_t region = {0};
    return &region;
}

arm_2d_tile_t *arm_2d_get_default_frame_buffer(void)
{
    return NULL;
}

arm_2d_tile_t *__arm_2d_tile_generate_child(const arm_2d_tile_t *ptParentTile,
                                            const arm_2d_region_t *ptRegion,
                                            arm_2d_tile_t *ptOutput,
                                            bool bClipRegion,
                                            bool bValidateBeforeReturn)
{
    (void)bClipRegion;
    (void)bValidateBeforeReturn;
    assert(ptOutput != NULL);
    memset(ptOutput, 0, sizeof(*ptOutput));
    (void)ptParentTile;
    ptOutput->tRegion = *ptRegion;
    return ptOutput;
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

bool __arm_2d_helper_pfb_is_region_active1(const arm_2d_tile_t *ptTarget,
                                           const arm_2d_region_t *ptRegion,
                                           bool bConsiderDryRun,
                                           const arm_2d_tile_t **pptScreen)
{
    (void)ptTarget;
    (void)ptRegion;
    (void)bConsiderDryRun;
    *pptScreen = ptTarget;
    return true;
}

static void reset_emit_probe(void)
{
    g_emit_count = 0;
    g_last_signal = 0;
    g_last_value = 0;
}

static void reset_render_probe(void)
{
    g_color_call_count = 0;
    g_image_call_count = 0;
    memset(g_color_calls, 0, sizeof(g_color_calls));
}

static void test_set_checked_before_first_frame_jumps_to_target_and_emits(void)
{
    ldSwitch_t widget = {0};
    ld_scene_t scene = {0};
    xQueue_t queue = {0};

    scene.ptMsgQueue = &queue;
    reset_emit_probe();

    _ldSwitchSetChecked(&scene, &widget, true);

    assert(widget.isChecked == true);
    assert(widget.animProgress == 1000);
    assert(widget.isAnimating == false);
    assert(g_emit_count == 1);
    assert(g_last_signal == SIGNAL_VALUE_CHANGED);
    assert(g_last_value == 1);
}

static void test_repeated_same_value_does_not_emit_or_restart_animation(void)
{
    ldSwitch_t widget = {0};
    ld_scene_t scene = {0};
    xQueue_t queue = {0};
    uint16_t previous_elapsed;
    bool previous_animating;

    scene.ptMsgQueue = &queue;
    reset_emit_probe();
    _ldSwitchSetChecked(&scene, &widget, true);

    widget.animElapsedMs = 77;
    widget.isAnimating = true;
    previous_elapsed = widget.animElapsedMs;
    previous_animating = widget.isAnimating;

    _ldSwitchSetChecked(&scene, &widget, true);

    assert(g_emit_count == 1);
    assert(widget.animElapsedMs == previous_elapsed);
    assert(widget.isAnimating == previous_animating);
}

static void test_disabled_press_release_does_not_toggle_or_emit(void)
{
    ldSwitch_t widget = {0};
    ld_scene_t scene = {0};
    ldMsg_t msg = {0};

    reset_emit_probe();
    widget.isChecked = false;
    ldSwitchSetDisabled(&widget, true);

    msg.ptSender = &widget;
    msg.signal = SIGNAL_PRESS;
    assert(slotSwitchProcess(&scene, msg) == false);
    msg.signal = SIGNAL_RELEASE;
    assert(slotSwitchProcess(&scene, msg) == false);

    assert(widget.isChecked == false);
    assert(g_emit_count == 0);
}

static void test_auto_direction_uses_tall_region_as_vertical(void)
{
    ldSwitch_t widget = {0};

    widget.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iWidth = 30;
    widget.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iHeight = 60;

    ldSwitchSetDirection(&widget, LD_SWITCH_DIRECTION_AUTO);

    assert(ldSwitchIsHorizontal(&widget) == false);
}

static void test_set_checked_after_rendered_frame_starts_animation(void)
{
    ldSwitch_t widget = {0};
    ld_scene_t scene = {0};
    xQueue_t queue = {0};

    scene.ptMsgQueue = &queue;
    widget.hasRenderedFrame = true;
    reset_emit_probe();

    _ldSwitchSetChecked(&scene, &widget, true);

    assert(widget.isChecked == true);
    assert(widget.animProgress == 0);
    assert(widget.animTargetProgress == 1000);
    assert(widget.animElapsedMs == 0);
    assert(widget.isAnimating == true);
    assert(g_emit_count == 1);
}

static void test_navigate_enter_toggles_and_emits_once(void)
{
    ldSwitch_t widget = {0};
    ld_scene_t scene = {0};
    xQueue_t queue = {0};

    scene.ptMsgQueue = &queue;
    reset_emit_probe();

    ldSwitchNavigate(&scene, &widget, NAV_ENTER);

    assert(widget.isChecked == true);
    assert(widget.animProgress == 1000);
    assert(widget.isAnimating == false);
    assert(g_emit_count == 1);
    assert(g_last_signal == SIGNAL_VALUE_CHANGED);
    assert(g_last_value == 1);
}

static void test_navigate_direction_sets_on_and_off_without_reemitting_same_value(void)
{
    ldSwitch_t widget = {0};
    ld_scene_t scene = {0};
    xQueue_t queue = {0};

    scene.ptMsgQueue = &queue;
    reset_emit_probe();

    ldSwitchNavigate(&scene, &widget, NAV_RIGHT);
    assert(widget.isChecked == true);
    assert(g_emit_count == 1);
    assert(g_last_value == 1);

    ldSwitchNavigate(&scene, &widget, NAV_UP);
    assert(widget.isChecked == true);
    assert(g_emit_count == 1);

    ldSwitchNavigate(&scene, &widget, NAV_LEFT);
    assert(widget.isChecked == false);
    assert(g_emit_count == 2);
    assert(g_last_value == 0);

    ldSwitchNavigate(&scene, &widget, NAV_DOWN);
    assert(widget.isChecked == false);
    assert(g_emit_count == 2);
}

static void test_disabled_navigation_does_not_toggle_or_emit(void)
{
    ldSwitch_t widget = {0};
    ld_scene_t scene = {0};
    xQueue_t queue = {0};

    scene.ptMsgQueue = &queue;
    ldSwitchSetDisabled(&widget, true);
    reset_emit_probe();

    ldSwitchNavigate(&scene, &widget, NAV_ENTER);
    ldSwitchNavigate(&scene, &widget, NAV_RIGHT);

    assert(widget.isChecked == false);
    assert(g_emit_count == 0);
}

static void test_navigation_consumption_prefers_value_change(void)
{
    ldSwitch_t switchOff = {0};
    ldSwitch_t switchOn = {.isChecked = true};

    assert(ldSwitchCanNavigate(&switchOff, NAV_ENTER) == true);
    assert(ldSwitchCanNavigate(&switchOff, NAV_RIGHT) == true);
    assert(ldSwitchCanNavigate(&switchOff, NAV_UP) == true);
    assert(ldSwitchCanNavigate(&switchOff, NAV_LEFT) == false);
    assert(ldSwitchCanNavigate(&switchOff, NAV_DOWN) == false);

    assert(ldSwitchCanNavigate(&switchOn, NAV_ENTER) == true);
    assert(ldSwitchCanNavigate(&switchOn, NAV_LEFT) == true);
    assert(ldSwitchCanNavigate(&switchOn, NAV_DOWN) == true);
    assert(ldSwitchCanNavigate(&switchOn, NAV_RIGHT) == false);
    assert(ldSwitchCanNavigate(&switchOn, NAV_UP) == false);
}

static void test_disabled_switch_does_not_consume_navigation(void)
{
    ldSwitch_t widget = {.isDisabled = true, .isChecked = true};

    assert(ldSwitchCanNavigate(&widget, NAV_ENTER) == false);
    assert(ldSwitchCanNavigate(&widget, NAV_RIGHT) == false);
    assert(ldSwitchCanNavigate(&widget, NAV_UP) == false);
    assert(ldSwitchCanNavigate(&widget, NAV_LEFT) == false);
    assert(ldSwitchCanNavigate(&widget, NAV_DOWN) == false);
}

static void test_show_falls_back_per_layer_when_image_or_mask_missing(void)
{
    ldSwitch_t widget = {0};
    arm_2d_tile_t frame = {0};
    arm_2d_tile_t img = {0};
    arm_2d_tile_t mask = {0};

    widget.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iWidth = 44;
    widget.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iHeight = 24;
    widget.use_as__ldBase_t.opacity = 255;
    widget.offTrackColor = __RGB(1, 2, 3);
    widget.onTrackColor = __RGB(4, 5, 6);
    widget.knobColor = __RGB(7, 8, 9);
    widget.borderColor = __RGB(10, 11, 12);
    widget.direction = LD_SWITCH_DIRECTION_HORIZONTAL;
    widget.animProgress = 1000;
    ldSwitchSetImage(&widget, &img, NULL, &img, &mask, NULL, &mask);
    reset_render_probe();

    ldSwitch_show(NULL, &widget, &frame, true);

    assert(g_image_call_count == 1);
    assert(g_color_call_count >= 2);
    assert(g_color_calls[0] == widget.offTrackColor);
    assert(g_color_calls[g_color_call_count - 1] == widget.knobColor);
}

static void test_pressed_knob_uses_visible_highlight_not_border_color(void)
{
    ldSwitch_t widget = {0};
    arm_2d_tile_t frame = {0};

    widget.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iWidth = 44;
    widget.use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iHeight = 24;
    widget.use_as__ldBase_t.opacity = 255;
    widget.offTrackColor = __RGB(1, 2, 3);
    widget.onTrackColor = __RGB(4, 5, 6);
    widget.knobColor = __RGB(7, 8, 9);
    widget.borderColor = __RGB(10, 11, 12);
    widget.direction = LD_SWITCH_DIRECTION_HORIZONTAL;
    widget.isPressed = true;
    widget.animProgress = 1000;
    reset_render_probe();

    ldSwitch_show(NULL, &widget, &frame, true);

    assert(g_color_call_count >= 3);
    assert(g_color_calls[g_color_call_count - 1] != widget.borderColor);
    assert(g_color_calls[g_color_call_count - 1] != widget.knobColor);
}

int main(void)
{
    test_set_checked_before_first_frame_jumps_to_target_and_emits();
    test_repeated_same_value_does_not_emit_or_restart_animation();
    test_disabled_press_release_does_not_toggle_or_emit();
    test_auto_direction_uses_tall_region_as_vertical();
    test_set_checked_after_rendered_frame_starts_animation();
    test_navigate_enter_toggles_and_emits_once();
    test_navigate_direction_sets_on_and_off_without_reemitting_same_value();
    test_disabled_navigation_does_not_toggle_or_emit();
    test_navigation_consumption_prefers_value_change();
    test_disabled_switch_does_not_consume_navigation();
    test_show_falls_back_per_layer_when_image_or_mask_missing();
    test_pressed_knob_uses_visible_highlight_not_border_color();
    return 0;
}
