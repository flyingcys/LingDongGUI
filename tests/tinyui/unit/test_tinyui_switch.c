/*
 * TinyUI switch unit tests — M3 Task 2 L4 harness.
 *
 * Validates real ldSwitch_t fields for checked/direction/navigation/image skin,
 * unified VALUE_CHANGED from LD signals, and programmatic setters that do not
 * fabricate user events.
 */

#include "tinyui.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldSwitch.h"
#include "internal.h"
#include "resource/image_source.h"
#include "widgets/switch.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static int g_value_changed_count;
static int32_t g_last_value;
static tinyui_obj_t *g_last_target;
static void *g_last_user_data;

static void reset_event_fixture(void)
{
    g_value_changed_count = 0;
    g_last_value = -999;
    g_last_target = 0;
    g_last_user_data = 0;
}

static void on_value_changed(const tinyui_event_t *event)
{
    assert(event != 0);
    g_value_changed_count += 1;
    g_last_value = event->data.value;
    g_last_target = event->target;
    g_last_user_data = event->user_data;
    assert(event->code == TINYUI_EVENT_VALUE_CHANGED);
}

static void inject_switch_toggle(tinyui_obj_t *switch_obj, int checked)
{
    struct tinyui_widget *widget = (struct tinyui_widget *)(void *)switch_obj;

    assert(widget != 0);
    assert(widget->ld_widget != 0);
    assert(tinyui_runtime_internal_widget_dispatch_native_signal(
               widget, SIGNAL_VALUE_CHANGED, (uint64_t)(checked != 0)) == 0);
}

static void bind_test_tiles(tinyui_image_source_t *source,
                            arm_2d_tile_t *img_tile,
                            arm_2d_tile_t *mask_tile)
{
    assert(source != 0);
    memset(source, 0, sizeof(*source));
    source->kind = TINYUI_IMAGE_SOURCE_RGB565_MEMORY;
    if (img_tile != 0) {
        source->width = (uint16_t)img_tile->tRegion.tSize.iWidth;
        source->height = (uint16_t)img_tile->tRegion.tSize.iHeight;
        memcpy(source->_image_private, img_tile, sizeof(*img_tile));
    }
    if (mask_tile != 0) {
        memcpy(source->_mask_private, mask_tile, sizeof(*mask_tile));
    }
}

static void assert_switch_has_bound_images(tinyui_obj_t *sw,
                                           const tinyui_image_source_t *expected_off,
                                           const tinyui_image_source_t *expected_on,
                                           const tinyui_image_source_t *expected_knob)
{
    struct tinyui_widget *backend;
    const ldSwitch_t *ld_switch;

    assert(sw != 0);
    backend = (struct tinyui_widget *)(void *)sw;
    assert(backend->ld_widget != 0);
    ld_switch = (const ldSwitch_t *)backend->ld_widget;
    assert(ld_switch != 0);
    assert(ld_switch->ptOffImgTile
           == (expected_off != 0 ? tinyui_image_source_get_image_tile(expected_off) : 0));
    assert(ld_switch->ptOffMaskTile
           == (expected_off != 0 ? tinyui_image_source_get_mask_tile(expected_off) : 0));
    assert(ld_switch->ptOnImgTile
           == (expected_on != 0 ? tinyui_image_source_get_image_tile(expected_on) : 0));
    assert(ld_switch->ptOnMaskTile
           == (expected_on != 0 ? tinyui_image_source_get_mask_tile(expected_on) : 0));
    assert(ld_switch->ptKnobImgTile
           == (expected_knob != 0 ? tinyui_image_source_get_image_tile(expected_knob) : 0));
    assert(ld_switch->ptKnobMaskTile
           == (expected_knob != 0 ? tinyui_image_source_get_mask_tile(expected_knob) : 0));
}

static void test_switch_create_and_ld_mapping(tinyui_obj_t *root)
{
    tinyui_obj_t *sw = tinyui_switch_create(root);
    struct tinyui_widget *backend;
    ldBase_t *ld_base;
    ldSwitch_t *ld_switch;

    assert(sw != 0);
    backend = (struct tinyui_widget *)(void *)sw;
    assert(backend->ld_widget != 0);
    assert(backend->kind == TINYUI_BACKEND_WIDGET_SWITCH);
    assert(backend->ld_name_id != 0);
    assert(backend->ld_event_bridge_scene != 0);
    assert(backend->ld_event_bridge_sender == backend->ld_widget);
    ld_base = (ldBase_t *)backend->ld_widget;
    assert(ld_base != 0);
    assert(ld_base->widgetType == widgetTypeSwitch);
    assert(ld_base->use_as__arm_2d_control_node_t.tRegion.tSize.iWidth == 48);
    assert(ld_base->use_as__arm_2d_control_node_t.tRegion.tSize.iHeight == 24);
    ld_switch = (ldSwitch_t *)backend->ld_widget;
    assert(ld_switch->offTrackColor == __RGB(224, 224, 224));
    assert(ld_switch->onTrackColor == __RGB(33, 150, 243));
    assert(ld_switch->knobColor == GLCD_COLOR_WHITE);
    assert(ld_switch->borderColor == GLCD_COLOR_WHITE);
}

static void test_switch_create_with_props_pushes_all_fields(tinyui_obj_t *root)
{
    arm_2d_tile_t off_tile = {0};
    arm_2d_tile_t off_mask = {0};
    arm_2d_tile_t on_tile = {0};
    arm_2d_tile_t on_mask = {0};
    arm_2d_tile_t knob_tile = {0};
    arm_2d_tile_t knob_mask = {0};
    tinyui_image_source_t off_source;
    tinyui_image_source_t on_source;
    tinyui_image_source_t knob_source;
    tinyui_switch_props_t props;
    tinyui_obj_t *sw;
    struct tinyui_widget *backend;
    ldSwitch_t *ld_switch;
    int horizontal = -1;
    int direction = -1;
    int disabled = -1;

    bind_test_tiles(&off_source, &off_tile, &off_mask);
    bind_test_tiles(&on_source, &on_tile, &on_mask);
    bind_test_tiles(&knob_source, &knob_tile, &knob_mask);

    memset(&props, 0, sizeof(props));
    props.fields = TINYUI_SWITCH_FIELD_CHECKED
        | TINYUI_SWITCH_FIELD_OFF_SOURCE
        | TINYUI_SWITCH_FIELD_ON_SOURCE
        | TINYUI_SWITCH_FIELD_KNOB_SOURCE
        | TINYUI_SWITCH_FIELD_HORIZONTAL
        | TINYUI_SWITCH_FIELD_DIRECTION
        | TINYUI_SWITCH_FIELD_DISABLED;
    props.checked = 1;
    props.off_source = &off_source;
    props.on_source = &on_source;
    props.knob_source = &knob_source;
    props.horizontal = 0;
    props.direction = 2;
    props.disabled = 1;

    sw = tinyui_switch_create_with_props(root, &props);
    assert(sw != 0);
    backend = (struct tinyui_widget *)(void *)sw;
    ld_switch = (ldSwitch_t *)backend->ld_widget;
    assert(ld_switch != 0);
    assert(ld_switch->isChecked == true);
    assert(tinyui_switch_is_checked(sw) == 1);
    assert_switch_has_bound_images(sw, &off_source, &on_source, &knob_source);
    assert(tinyui_switch_get_horizontal(sw, &horizontal) == 0);
    assert(horizontal == 0);
    assert(ld_switch->isHorizontal == false);
    assert(tinyui_switch_get_direction(sw, &direction) == 0);
    assert(direction == 2);
    assert(ld_switch->direction == LD_SWITCH_DIRECTION_VERTICAL);
    assert(tinyui_switch_get_disabled(sw, &disabled) == 0);
    assert(disabled == 1);
    assert(ld_switch->isDisabled == true);
    assert(backend->enabled == 0);
}

static void test_switch_set_checked_reads_ld_and_no_fake_event(tinyui_obj_t *root)
{
    tinyui_obj_t *sw = tinyui_switch_create(root);
    struct tinyui_widget *backend;
    ldSwitch_t *ld_switch;
    tinyui_event_handle_t handle = 0U;
    int cookie = 44;

    assert(sw != 0);
    backend = (struct tinyui_widget *)(void *)sw;
    ld_switch = (ldSwitch_t *)backend->ld_widget;
    assert(ld_switch != 0);

    reset_event_fixture();
    assert(tinyui_obj_add_event_cb(sw,
                                   TINYUI_EVENT_MASK(TINYUI_EVENT_VALUE_CHANGED),
                                   on_value_changed,
                                   &cookie,
                                   &handle) == TINYUI_OK);

    assert(tinyui_switch_set_checked(sw, 1) == 0);
    assert(ld_switch->isChecked == true);
    assert(ld_switch->animProgress == 1000U);
    assert(tinyui_switch_is_checked(sw) == 1);
    assert(g_value_changed_count == 0);

    assert(tinyui_switch_set_checked(sw, 0) == 0);
    assert(ld_switch->isChecked == false);
    assert(ld_switch->animProgress == 0U);
    assert(tinyui_switch_is_checked(sw) == 0);
    assert(g_value_changed_count == 0);

    /* getter must follow LD, not stale wrapper cache */
    ld_switch->isChecked = true;
    ld_switch->animProgress = 1000U;
    assert(tinyui_switch_is_checked(sw) == 1);
    ld_switch->isChecked = false;
    ld_switch->animProgress = 0U;
    assert(tinyui_switch_is_checked(sw) == 0);
}

static void test_switch_direction_navigation_image_skin_round_trip(tinyui_obj_t *root)
{
    tinyui_obj_t *sw = tinyui_switch_create(root);
    arm_2d_tile_t off_tile = {0};
    arm_2d_tile_t off_mask = {0};
    arm_2d_tile_t on_tile = {0};
    arm_2d_tile_t on_mask = {0};
    arm_2d_tile_t knob_tile = {0};
    arm_2d_tile_t knob_mask = {0};
    tinyui_image_source_t off_source;
    tinyui_image_source_t on_source;
    tinyui_image_source_t knob_source;
    ldSwitch_t *ld_switch;
    int horizontal = -1;
    int direction = -1;
    int can_nav = -1;

    assert(sw != 0);
    ld_switch = (ldSwitch_t *)((struct tinyui_widget *)(void *)sw)->ld_widget;
    assert(ld_switch != 0);

    bind_test_tiles(&off_source, &off_tile, &off_mask);
    bind_test_tiles(&on_source, &on_tile, &on_mask);
    bind_test_tiles(&knob_source, &knob_tile, &knob_mask);

    assert(tinyui_switch_set_off_source(sw, &off_source) == 0);
    assert(tinyui_switch_set_on_source(sw, &on_source) == 0);
    assert(tinyui_switch_set_knob_source(sw, &knob_source) == 0);
    assert_switch_has_bound_images(sw, &off_source, &on_source, &knob_source);

    assert(tinyui_switch_set_horizontal(sw, 0) == 0);
    assert(tinyui_switch_get_horizontal(sw, &horizontal) == 0);
    assert(horizontal == 0);
    assert(ld_switch->isHorizontal == false);

    assert(tinyui_switch_set_direction(sw, 1) == 0);
    assert(tinyui_switch_get_direction(sw, &direction) == 0);
    assert(direction == 1);
    assert(ld_switch->direction == LD_SWITCH_DIRECTION_HORIZONTAL);

    assert(tinyui_switch_set_checked(sw, 0) == 0);
    assert(tinyui_switch_can_navigate(sw, 4, &can_nav) == 0);
    assert(can_nav == 1);
    assert(tinyui_switch_navigate(sw, 4) == 0);
    assert(ld_switch->isChecked == true);
    assert(tinyui_switch_is_checked(sw) == 1);
}

static void test_switch_real_toggle_emits_value_changed_once(tinyui_obj_t *root)
{
    tinyui_obj_t *sw = tinyui_switch_create(root);
    tinyui_event_handle_t handle = 0U;
    int cookie = 77;
    ldSwitch_t *ld_switch;

    assert(sw != 0);
    ld_switch = (ldSwitch_t *)((struct tinyui_widget *)(void *)sw)->ld_widget;
    assert(ld_switch != 0);
    assert(ld_switch->isChecked == false);

    reset_event_fixture();
    assert(tinyui_obj_add_event_cb(sw,
                                   TINYUI_EVENT_MASK(TINYUI_EVENT_VALUE_CHANGED),
                                   on_value_changed,
                                   &cookie,
                                   &handle) == TINYUI_OK);

    inject_switch_toggle(sw, 1);
    assert(g_value_changed_count == 1);
    assert(g_last_value == 1);
    assert(g_last_target == sw);
    assert(g_last_user_data == &cookie);
    assert(ld_switch->isChecked == true);
    assert(tinyui_switch_is_checked(sw) == 1);

    inject_switch_toggle(sw, 1);
    assert(g_value_changed_count == 1);

    inject_switch_toggle(sw, 0);
    assert(g_value_changed_count == 2);
    assert(g_last_value == 0);
    assert(ld_switch->isChecked == false);
    assert(tinyui_switch_is_checked(sw) == 0);
}

static void test_switch_set_on_toggled_forwards_unified_pool(tinyui_obj_t *root)
{
    tinyui_obj_t *sw = tinyui_switch_create(root);
    int cookie = 88;

    assert(sw != 0);
    reset_event_fixture();
    assert(tinyui_switch_set_on_toggled(sw, on_value_changed, &cookie) == 0);
    inject_switch_toggle(sw, 1);
    assert(g_value_changed_count == 1);
    assert(g_last_value == 1);
    assert(g_last_user_data == &cookie);

    reset_event_fixture();
    assert(tinyui_switch_set_on_toggled(sw, 0, 0) == 0);
    inject_switch_toggle(sw, 0);
    assert(g_value_changed_count == 0);
}

static void test_switch_props_reject_legacy_on_toggled(tinyui_obj_t *root)
{
    tinyui_switch_props_t props;
    ldBase_t *root_ld = (ldBase_t *)((struct tinyui_widget *)(void *)root)->ld_widget;
    ldBase_t *child_before = ldBaseGetChildList(root_ld);

    memset(&props, 0, sizeof(props));
    props.fields = TINYUI_SWITCH_FIELD_ON_TOGGLED | TINYUI_SWITCH_FIELD_CHECKED;
    props.checked = 1;
    props.on_toggled = (tinyui_value_changed_cb)(void *)0x1;

    assert(tinyui_switch_create_with_props(root, &props) == 0);
    assert(ldBaseGetChildList(root_ld) == child_before);
}

static void test_switch_rejects_null_and_invalid(tinyui_obj_t *root)
{
    tinyui_obj_t *sw = tinyui_switch_create(root);
    tinyui_image_source_t empty_source;
    int horizontal = -1;
    int direction = -1;
    int disabled = -1;
    int can_nav = -1;

    assert(sw != 0);
    memset(&empty_source, 0, sizeof(empty_source));

    assert(tinyui_switch_create(0) == 0);
    assert(tinyui_switch_set_checked(0, 1) == -1);
    assert(tinyui_switch_is_checked(0) == -1);
    assert(tinyui_switch_set_on_toggled(0, 0, 0) == -1);
    assert(tinyui_switch_set_horizontal(0, 1) == -1);
    assert(tinyui_switch_get_horizontal(0, &horizontal) == -1);
    assert(tinyui_switch_set_direction(0, 0) == -1);
    assert(tinyui_switch_set_direction(sw, -1) == -1);
    assert(tinyui_switch_set_direction(sw, 3) == -1);
    assert(tinyui_switch_get_direction(0, &direction) == -1);
    assert(tinyui_switch_set_disabled(0, 1) == -1);
    assert(tinyui_switch_get_disabled(0, &disabled) == -1);
    assert(tinyui_switch_can_navigate(0, 1, &can_nav) == -1);
    assert(tinyui_switch_navigate(0, 1) == -1);
    assert(tinyui_switch_set_off_source(0, &empty_source) == -1);
    assert(tinyui_switch_set_off_source(sw, &empty_source) == -1);
    assert(tinyui_switch_set_on_source(sw, &empty_source) == -1);
    assert(tinyui_switch_set_knob_source(sw, &empty_source) == -1);
}

static void test_switch_no_dedicated_callback_fields(void)
{
    FILE *file = fopen("tinyui/src/core/internal.h", "rb");
    char content[65536];
    size_t n;

    if (file == 0) {
        file = fopen("/home/share/samba/embedded/LingDongGUI/tinyui/src/core/internal.h", "rb");
    }
    assert(file != 0);
    n = fread(content, 1, sizeof(content) - 1, file);
    content[n] = '\0';
    fclose(file);

    assert(strstr(content, "struct tinyui_switch") != 0);
    assert(strstr(content, "on_toggled_handle") != 0);
}

int main(void)
{
    tinyui_obj_t *root;

    tinyui_deinit();
    assert(tinyui_init() == TINYUI_OK);
    root = tinyui_screen_create();
    assert(root != 0);

    test_switch_create_and_ld_mapping(root);
    test_switch_create_with_props_pushes_all_fields(root);
    test_switch_set_checked_reads_ld_and_no_fake_event(root);
    test_switch_direction_navigation_image_skin_round_trip(root);
    test_switch_real_toggle_emits_value_changed_once(root);
    test_switch_set_on_toggled_forwards_unified_pool(root);
    test_switch_props_reject_legacy_on_toggled(root);
    test_switch_rejects_null_and_invalid(root);
    test_switch_no_dedicated_callback_fields();

    tinyui_deinit();
    return 0;
}
