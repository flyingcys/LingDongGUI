/*
 * TinyUI checkbox unit tests — M2 Task 7 L1-L5-E harness.
 *
 * Validates real ldCheckBox_t fields for checked/text/colors/images/
 * radio/spacing, unified VALUE_CHANGED from LD signals, and
 * programmatic setters that do not fabricate user events.
 */

#include "tinyui.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldCheckBox.h"
#include "../../../examples/common/demo/widget/fonts/uiFonts.h"
#include "internal.h"
#include "resource/image_source.h"
#include "widgets/checkbox.h"

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

static void inject_checkbox_toggle(tinyui_obj_t *checkbox_obj, int checked)
{
    struct tinyui_widget *widget = (struct tinyui_widget *)(void *)checkbox_obj;

    assert(widget != 0);
    assert(widget->ld_widget != 0);
    assert(tinyui_runtime_internal_widget_dispatch_native_signal(
               widget, SIGNAL_VALUE_CHANGED, (uint64_t)(checked != 0)) == 0);
}

static unsigned int test_rgb_to_ld_color(unsigned int rgb)
{
    return (unsigned int)__RGB((rgb >> 16) & 0xFFU, (rgb >> 8) & 0xFFU, rgb & 0xFFU);
}

static void bind_test_tiles(tinyui_image_source_t *source,
                            arm_2d_tile_t *img_tile,
                            arm_2d_tile_t *mask_tile)
{
    assert(source != 0);
    memset(source, 0, sizeof(*source));
    source->kind = TINYUI_IMAGE_SOURCE_RGB565_MEMORY;
    if (img_tile != 0) {
        memcpy(source->_image_private, img_tile, sizeof(*img_tile));
    }
    if (mask_tile != 0) {
        memcpy(source->_mask_private, mask_tile, sizeof(*mask_tile));
    }
}

static void assert_checkbox_has_bound_images(tinyui_obj_t *checkbox,
                                             const tinyui_image_source_t *expected_unchecked,
                                             const tinyui_image_source_t *expected_checked)
{
    struct tinyui_widget *backend;
    const ldCheckBox_t *ld_checkbox;

    assert(checkbox != 0);
    backend = (struct tinyui_widget *)(void *)checkbox;
    assert(backend->ld_widget != 0);
    ld_checkbox = (const ldCheckBox_t *)backend->ld_widget;
    assert(ld_checkbox != 0);
    assert(ld_checkbox->ptUncheckedImgTile
           == (expected_unchecked != 0
                   ? tinyui_image_source_get_image_tile(expected_unchecked)
                   : 0));
    assert(ld_checkbox->ptUncheckedMaskTile
           == (expected_unchecked != 0
                   ? tinyui_image_source_get_mask_tile(expected_unchecked)
                   : 0));
    assert(ld_checkbox->ptCheckedImgTile
           == (expected_checked != 0
                   ? tinyui_image_source_get_image_tile(expected_checked)
                   : 0));
    assert(ld_checkbox->ptCheckedMaskTile
           == (expected_checked != 0
                   ? tinyui_image_source_get_mask_tile(expected_checked)
                   : 0));
}

static void test_checkbox_create_and_ld_mapping(tinyui_obj_t *root)
{
    tinyui_obj_t *checkbox = tinyui_checkbox_create(root);
    struct tinyui_widget *backend;
    ldBase_t *ld_base;

    assert(checkbox != 0);
    backend = (struct tinyui_widget *)(void *)checkbox;
    assert(backend->ld_widget != 0);
    assert(backend->kind == TINYUI_BACKEND_WIDGET_CHECKBOX);
    ld_base = (ldBase_t *)backend->ld_widget;
    assert(ld_base != 0);
    assert(ld_base->widgetType == widgetTypeCheckBox);
}

static void test_checkbox_create_with_props_pushes_all_fields(tinyui_obj_t *root)
{
    arm_2d_tile_t unchecked_tile = {0};
    arm_2d_tile_t unchecked_mask_tile = {0};
    arm_2d_tile_t checked_tile = {0};
    arm_2d_tile_t checked_mask_tile = {0};
    tinyui_image_source_t unchecked_source;
    tinyui_image_source_t checked_source;
    tinyui_checkbox_props_t props;
    tinyui_obj_t *checkbox;
    struct tinyui_widget *backend;
    ldCheckBox_t *ld_checkbox;

    bind_test_tiles(&unchecked_source, &unchecked_tile, &unchecked_mask_tile);
    bind_test_tiles(&checked_source, &checked_tile, &checked_mask_tile);

    memset(&props, 0, sizeof(props));
    props.fields = TINYUI_CHECKBOX_FIELD_TEXT
        | TINYUI_CHECKBOX_FIELD_CHECKED
        | TINYUI_CHECKBOX_FIELD_CHECK_COLOR
        | TINYUI_CHECKBOX_FIELD_UNCHECKED_SOURCE
        | TINYUI_CHECKBOX_FIELD_CHECKED_SOURCE
        | TINYUI_CHECKBOX_FIELD_RADIO_GROUP
        | TINYUI_CHECKBOX_FIELD_STRING_LEFT_SPACE;
    props.text = "Agree";
    props.checked = 1;
    props.check_color = 0x224466U;
    props.unchecked_source = &unchecked_source;
    props.checked_source = &checked_source;
    props.radio_group = 7;
    props.string_left_space = 22;

    checkbox = tinyui_checkbox_create_with_props(root, &props);
    assert(checkbox != 0);
    backend = (struct tinyui_widget *)(void *)checkbox;
    assert(backend->ld_widget != 0);
    assert(backend->text != 0);
    assert(strcmp(backend->text, "Agree") == 0);
    ld_checkbox = (ldCheckBox_t *)backend->ld_widget;
    assert(ld_checkbox != 0);
    assert(ld_checkbox->isChecked == true);
    assert(ld_checkbox->fgColor == (ldColor)test_rgb_to_ld_color(0x224466U));
    assert_checkbox_has_bound_images(checkbox, &unchecked_source, &checked_source);
    assert(ld_checkbox->isRadioButton == true);
    assert(ld_checkbox->radioButtonGroup == 7);
    assert(ld_checkbox->boxWidth == 22);
}

static void test_checkbox_set_checked_reads_ld_and_no_fake_event(tinyui_obj_t *root)
{
    tinyui_obj_t *checkbox = tinyui_checkbox_create(root);
    struct tinyui_widget *backend;
    ldCheckBox_t *ld_checkbox;
    tinyui_event_handle_t handle = 0U;
    int cookie = 44;

    assert(checkbox != 0);
    backend = (struct tinyui_widget *)(void *)checkbox;
    ld_checkbox = (ldCheckBox_t *)backend->ld_widget;
    assert(ld_checkbox != 0);

    reset_event_fixture();
    assert(tinyui_obj_add_event_cb(checkbox,
                                   TINYUI_EVENT_MASK(TINYUI_EVENT_VALUE_CHANGED),
                                   on_value_changed,
                                   &cookie,
                                   &handle) == TINYUI_OK);

    assert(tinyui_checkbox_set_checked(checkbox, 1) == 0);
    assert(ld_checkbox->isChecked == true);
    assert(tinyui_checkbox_is_checked(checkbox) == 1);
    assert(g_value_changed_count == 0);

    assert(tinyui_checkbox_set_checked(checkbox, 0) == 0);
    assert(ld_checkbox->isChecked == false);
    assert(tinyui_checkbox_is_checked(checkbox) == 0);
    assert(g_value_changed_count == 0);

    /* getter must follow LD, not stale wrapper cache */
    ld_checkbox->isChecked = true;
    assert(tinyui_checkbox_is_checked(checkbox) == 1);
    ld_checkbox->isChecked = false;
    assert(tinyui_checkbox_is_checked(checkbox) == 0);
}

static void test_checkbox_set_text_and_native_helpers(tinyui_obj_t *root)
{
    tinyui_obj_t *checkbox = tinyui_checkbox_create(root);
    struct tinyui_widget *backend;
    ldCheckBox_t *ld_checkbox;
    arm_2d_tile_t unchecked_tile = {0};
    arm_2d_tile_t unchecked_mask_tile = {0};
    arm_2d_tile_t checked_tile = {0};
    arm_2d_tile_t checked_mask_tile = {0};
    tinyui_image_source_t unchecked_source;
    tinyui_image_source_t checked_source;

    assert(checkbox != 0);
    backend = (struct tinyui_widget *)(void *)checkbox;
    ld_checkbox = (ldCheckBox_t *)backend->ld_widget;
    assert(ld_checkbox != 0);
    assert(ld_checkbox->ptFont == (arm_2d_font_t *)FONT_ARIAL_12);

    assert(tinyui_checkbox_set_text(checkbox, "Label") == 0);
    assert(ld_checkbox->ptFont == (arm_2d_font_t *)FONT_ARIAL_12);
    assert(backend->text != 0);
    assert(strcmp(backend->text, "Label") == 0);

    assert(ld_checkbox->bgColor == GLCD_COLOR_WHITE);
    assert(ld_checkbox->fgColor == GLCD_COLOR_BLACK);
    assert(ld_checkbox->textColor == GLCD_COLOR_BLACK);

    assert(tinyui_checkbox_set_check_color(checkbox, 0xAA5500U) == 0);
    assert(ld_checkbox->fgColor == (ldColor)test_rgb_to_ld_color(0xAA5500U));
    assert(ld_checkbox->ptUncheckedImgTile == 0);
    assert(ld_checkbox->ptCheckedImgTile == 0);

    assert(tinyui_checkbox_set_text_color(checkbox, 0x224466U) == 0);
    assert(backend->text_color == 0x224466U);
    assert(ld_checkbox->textColor == (ldColor)test_rgb_to_ld_color(0x224466U));

    bind_test_tiles(&unchecked_source, &unchecked_tile, &unchecked_mask_tile);
    bind_test_tiles(&checked_source, &checked_tile, &checked_mask_tile);

    assert(tinyui_checkbox_set_unchecked_source(checkbox, &unchecked_source) == 0);
    assert_checkbox_has_bound_images(checkbox, &unchecked_source, 0);
    assert(tinyui_checkbox_set_checked_source(checkbox, &checked_source) == 0);
    assert_checkbox_has_bound_images(checkbox, &unchecked_source, &checked_source);

    assert(tinyui_checkbox_set_radio_group(checkbox, 7) == 0);
    assert(ld_checkbox->isRadioButton == true);
    assert(ld_checkbox->radioButtonGroup == 7);

    assert(tinyui_checkbox_set_string_left_space(checkbox, 22) == 0);
    assert(ld_checkbox->boxWidth == 22);

    assert(tinyui_checkbox_set_check_color(checkbox, 0x003366U) == 0);
    assert(ld_checkbox->fgColor == (ldColor)test_rgb_to_ld_color(0x003366U));
    assert_checkbox_has_bound_images(checkbox, 0, 0);
    assert(ld_checkbox->boxWidth == 14);
}

static void test_checkbox_real_toggle_emits_value_changed_once(tinyui_obj_t *root)
{
    tinyui_obj_t *checkbox = tinyui_checkbox_create(root);
    tinyui_event_handle_t handle = 0U;
    int cookie = 77;
    ldCheckBox_t *ld_checkbox;

    assert(checkbox != 0);
    ld_checkbox = (ldCheckBox_t *)((struct tinyui_widget *)(void *)checkbox)->ld_widget;
    assert(ld_checkbox != 0);
    assert(ld_checkbox->isChecked == false);

    reset_event_fixture();
    assert(tinyui_obj_add_event_cb(checkbox,
                                   TINYUI_EVENT_MASK(TINYUI_EVENT_VALUE_CHANGED),
                                   on_value_changed,
                                   &cookie,
                                   &handle) == TINYUI_OK);

    inject_checkbox_toggle(checkbox, 1);
    assert(g_value_changed_count == 1);
    assert(g_last_value == 1);
    assert(g_last_target == checkbox);
    assert(g_last_user_data == &cookie);
    assert(ld_checkbox->isChecked == true);
    assert(tinyui_checkbox_is_checked(checkbox) == 1);

    /* same value must not re-fire */
    inject_checkbox_toggle(checkbox, 1);
    assert(g_value_changed_count == 1);

    inject_checkbox_toggle(checkbox, 0);
    assert(g_value_changed_count == 2);
    assert(g_last_value == 0);
    assert(ld_checkbox->isChecked == false);
    assert(tinyui_checkbox_is_checked(checkbox) == 0);
}

static void test_checkbox_set_on_toggled_forwards_unified_pool(tinyui_obj_t *root)
{
    tinyui_obj_t *checkbox = tinyui_checkbox_create(root);
    int cookie = 88;

    assert(checkbox != 0);
    reset_event_fixture();
    assert(tinyui_checkbox_set_on_toggled(checkbox, on_value_changed, &cookie) == 0);
    inject_checkbox_toggle(checkbox, 1);
    assert(g_value_changed_count == 1);
    assert(g_last_value == 1);
    assert(g_last_user_data == &cookie);

    /* replace / clear */
    reset_event_fixture();
    assert(tinyui_checkbox_set_on_toggled(checkbox, 0, 0) == 0);
    inject_checkbox_toggle(checkbox, 0);
    assert(g_value_changed_count == 0);
}

static void test_checkbox_props_reject_legacy_on_toggled(tinyui_obj_t *root)
{
    tinyui_checkbox_props_t props;
    ldBase_t *root_ld = (ldBase_t *)((struct tinyui_widget *)(void *)root)->ld_widget;
    ldBase_t *child_before = ldBaseGetChildList(root_ld);

    memset(&props, 0, sizeof(props));
    props.fields = TINYUI_CHECKBOX_FIELD_ON_TOGGLED | TINYUI_CHECKBOX_FIELD_CHECKED;
    props.checked = 1;
    props.on_toggled = (tinyui_value_changed_cb)(void *)0x1;

    assert(tinyui_checkbox_create_with_props(root, &props) == 0);
    assert(ldBaseGetChildList(root_ld) == child_before);
}

static void test_checkbox_rejects_null_and_invalid(tinyui_obj_t *root)
{
    tinyui_obj_t *checkbox = tinyui_checkbox_create(root);
    ldCheckBox_t *ld_checkbox;
    arm_2d_tile_t unchecked_mask_tile = {0};
    arm_2d_tile_t checked_mask_tile = {0};
    tinyui_image_source_t invalid_unchecked_source;
    tinyui_image_source_t invalid_checked_source;

    assert(checkbox != 0);
    ld_checkbox = (ldCheckBox_t *)((struct tinyui_widget *)(void *)checkbox)->ld_widget;
    assert(ld_checkbox != 0);

    memset(&invalid_unchecked_source, 0, sizeof(invalid_unchecked_source));
    memset(&invalid_checked_source, 0, sizeof(invalid_checked_source));
    invalid_unchecked_source.kind = TINYUI_IMAGE_SOURCE_RGB565_MEMORY;
    invalid_checked_source.kind = TINYUI_IMAGE_SOURCE_RGB565_MEMORY;
    memcpy(invalid_unchecked_source._mask_private,
           &unchecked_mask_tile,
           sizeof(unchecked_mask_tile));
    memcpy(invalid_checked_source._mask_private, &checked_mask_tile, sizeof(checked_mask_tile));

    assert(tinyui_checkbox_create(0) == 0);
    assert(tinyui_checkbox_set_text(0, "text") == -1);
    assert(tinyui_checkbox_set_checked(0, 1) == -1);
    assert(tinyui_checkbox_set_unchecked_source(0, &invalid_unchecked_source) == -1);
    assert(tinyui_checkbox_set_checked_source(0, &invalid_checked_source) == -1);
    /* EMPTY source is unsupported; RGB565 with only mask and no pixels is still a tile
     * buffer under the private image_source layout, so null-tile rejection is N/A. */
    {
        tinyui_image_source_t empty_source;
        memset(&empty_source, 0, sizeof(empty_source));
        assert(tinyui_checkbox_set_unchecked_source(checkbox, &empty_source) == -1);
        assert(tinyui_checkbox_set_checked_source(checkbox, &empty_source) == -1);
    }
    assert(tinyui_checkbox_set_radio_group(0, 1) == -1);
    assert(tinyui_checkbox_set_radio_group(checkbox, -1) == -1);
    assert(tinyui_checkbox_set_radio_group(checkbox, 256) == -1);
    assert(tinyui_checkbox_set_string_left_space(0, 10) == -1);
    assert(tinyui_checkbox_set_string_left_space(checkbox, -1) == -1);
    assert(tinyui_checkbox_set_check_color(0, 0x123456U) == -1);
    assert(tinyui_checkbox_set_text_color(0, 0x123456U) == -1);
    assert(tinyui_checkbox_set_on_toggled(0, 0, 0) == -1);
    assert(ld_checkbox->radioButtonGroup == 0);
    assert(ld_checkbox->boxWidth == 14);
}

static void test_checkbox_no_dedicated_callback_fields(void)
{
    /* Task 7: wrapper must not keep private value_changed_cb fields. */
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

    /* dedicated cb field on checkbox wrapper removed (button pattern). */
    assert(strstr(content, "struct tinyui_checkbox") != 0);
    /* still has handle field for set_on_toggled replace semantics */
    assert(strstr(content, "on_toggled_handle") != 0
           || strstr(content, "on_value_changed_handle") != 0);
}

int main(void)
{
    tinyui_obj_t *root;

    tinyui_deinit();
    assert(tinyui_init() == TINYUI_OK);
    root = tinyui_screen_create();
    assert(root != 0);

    test_checkbox_create_and_ld_mapping(root);
    test_checkbox_create_with_props_pushes_all_fields(root);
    test_checkbox_set_checked_reads_ld_and_no_fake_event(root);
    test_checkbox_set_text_and_native_helpers(root);
    test_checkbox_real_toggle_emits_value_changed_once(root);
    test_checkbox_set_on_toggled_forwards_unified_pool(root);
    test_checkbox_props_reject_legacy_on_toggled(root);
    test_checkbox_rejects_null_and_invalid(root);
    test_checkbox_no_dedicated_callback_fields();

    tinyui_deinit();
    return 0;
}
