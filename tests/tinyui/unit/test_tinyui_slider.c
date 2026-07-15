/*
 * TinyUI slider unit tests — M2 Task 7 L1-L5-E harness.
 *
 * Validates range/value/percent mapping with real ldSlider_t, OUT_OF_RANGE
 * atomic failure, orientation/image/color/width/slim, and VALUE_CHANGED from
 * real LD permille signals via the unified event pool.
 */

#include "tinyui.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldSlider.h"
#include "internal.h"
#include "resource/image_source.h"
#include "widgets/slider.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void tinyui_slider_test_fail_indicator_width_for_id(const char *id);
static struct tinyui_widget g_disposed_backend_snapshot;
static int g_disposed_backend_valid = 0;

void tinyui_test_capture_destroyed_widget_snapshot(const struct tinyui_widget *widget)
{
    if (widget == 0) {
        memset(&g_disposed_backend_snapshot, 0, sizeof(g_disposed_backend_snapshot));
        g_disposed_backend_valid = 0;
        return;
    }

    g_disposed_backend_snapshot = *widget;
    g_disposed_backend_valid = 1;
}

const struct tinyui_widget *tinyui_slider_test_last_disposed_backend(void)
{
    if (g_disposed_backend_valid == 0) {
        return 0;
    }
    return &g_disposed_backend_snapshot;
}

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

static void inject_slider_permille(tinyui_obj_t *slider_obj, uint64_t native_permille)
{
    struct tinyui_widget *widget = (struct tinyui_widget *)(void *)slider_obj;

    assert(widget != 0);
    assert(tinyui_runtime_internal_widget_dispatch_native_signal(
               widget, SIGNAL_VALUE_CHANGED, native_permille) == 0);
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

static void test_slider_create_and_backend_mapping(tinyui_obj_t *root)
{
    tinyui_obj_t *slider = tinyui_slider_create(root);
    struct tinyui_widget *backend;
    struct tinyui_slider *wrapper;
    ldSlider_t *ld_slider;

    assert(slider != 0);
    backend = (struct tinyui_widget *)(void *)slider;
    wrapper = (struct tinyui_slider *)(void *)slider;
    assert(backend->ld_widget != 0);
    assert(backend->kind == TINYUI_BACKEND_WIDGET_SLIDER);
    assert(backend->ld_name_id != 0);
    assert(backend->ld_event_bridge_scene != 0);
    assert(backend->ld_event_bridge_sender == backend->ld_widget);
    ld_slider = (ldSlider_t *)backend->ld_widget;
    assert(ld_slider != 0);
    assert(wrapper->min_value == 0);
    assert(wrapper->max_value == 100);
    assert(wrapper->value == 0);
    assert(tinyui_runtime_internal_widget_has_ld_binding(backend) == 1);
}

static void test_slider_create_with_props_pushes_range(tinyui_obj_t *root)
{
    tinyui_slider_props_t props;
    tinyui_obj_t *slider;
    struct tinyui_slider *wrapper;
    ldSlider_t *ld_slider;

    memset(&props, 0, sizeof(props));
    props.fields = TINYUI_SLIDER_FIELD_MIN_VALUE
        | TINYUI_SLIDER_FIELD_MAX_VALUE
        | TINYUI_SLIDER_FIELD_VALUE;
    props.min_value = 10;
    props.max_value = 100;
    props.value = 50;

    slider = tinyui_slider_create_with_props(root, &props);
    assert(slider != 0);
    wrapper = (struct tinyui_slider *)(void *)slider;
    assert(wrapper->value == 50);
    assert(wrapper->min_value == 10);
    assert(wrapper->max_value == 100);

    ld_slider = (ldSlider_t *)wrapper->widget.ld_widget;
    assert(ld_slider != 0);
    /* value=50 in range [10,100] => percent=(50-10)*100/(100-10)=44 => permille=440 */
    assert(ld_slider->permille == 440U);
}

static void test_slider_range_mapping_negative_and_zero(tinyui_obj_t *root)
{
    tinyui_obj_t *slider = tinyui_slider_create(root);
    struct tinyui_slider *wrapper;
    ldSlider_t *ld_slider;
    int percent = -1;

    assert(slider != 0);
    wrapper = (struct tinyui_slider *)(void *)slider;
    ld_slider = (ldSlider_t *)wrapper->widget.ld_widget;
    assert(ld_slider != 0);

    /* range [-50, 150] */
    assert(tinyui_slider_set_range(slider, -50, 150) == 0);
    assert(wrapper->min_value == -50);
    assert(wrapper->max_value == 150);

    assert(tinyui_slider_set_value(slider, 0) == 0);
    assert(wrapper->value == 0);
    /* percent = (0-(-50))*100/200 = 25 => permille 250 */
    assert(ld_slider->permille == 250U);
    assert(tinyui_slider_get_percent(slider, &percent) == 0);
    assert(percent == 25);

    assert(tinyui_slider_set_value(slider, -50) == 0);
    assert(ld_slider->permille == 0U);
    assert(tinyui_slider_set_value(slider, 150) == 0);
    assert(ld_slider->permille == 1000U);

    /* OUT_OF_RANGE must not change value */
    assert(tinyui_slider_set_value(slider, 151) == -1);
    assert(tinyui_last_result() == TINYUI_ERROR_OUT_OF_RANGE);
    assert(wrapper->value == 150);
    assert(ld_slider->permille == 1000U);

    assert(tinyui_slider_set_value(slider, -51) == -1);
    assert(tinyui_last_result() == TINYUI_ERROR_OUT_OF_RANGE);
    assert(wrapper->value == 150);

    /* zero range [7, 7] */
    assert(tinyui_slider_set_range(slider, 7, 7) == 0);
    assert(wrapper->min_value == 7);
    assert(wrapper->max_value == 7);
    assert(wrapper->value == 7);
    assert(ld_slider->permille == 0U);
    assert(tinyui_slider_get_percent(slider, &percent) == 0);
    assert(percent == 0);
    assert(tinyui_slider_set_value(slider, 7) == 0);
    assert(tinyui_slider_set_value(slider, 8) == -1);
    assert(tinyui_last_result() == TINYUI_ERROR_OUT_OF_RANGE);
    assert(wrapper->value == 7);
}

static void test_slider_set_range_and_value_round_trip(tinyui_obj_t *root)
{
    tinyui_obj_t *slider = tinyui_slider_create(root);
    struct tinyui_slider *wrapper;
    int percent;
    int horizontal;
    arm_2d_tile_t bg_tile = {0};
    arm_2d_tile_t bg_mask = {0};
    arm_2d_tile_t indic_tile = {
        .tRegion = {
            .tSize = { .iWidth = 11, .iHeight = 26 },
        },
    };
    arm_2d_tile_t indic_mask = {
        .tRegion = {
            .tSize = { .iWidth = 11, .iHeight = 26 },
        },
    };
    tinyui_image_source_t background_source;
    tinyui_image_source_t indicator_source;
    ldSlider_t *ld_slider;

    assert(slider != 0);
    wrapper = (struct tinyui_slider *)(void *)slider;
    ld_slider = (ldSlider_t *)wrapper->widget.ld_widget;
    assert(ld_slider != 0);

    assert(tinyui_slider_set_range(slider, 0, 200) == 0);
    assert(wrapper->min_value == 0);
    assert(wrapper->max_value == 200);

    assert(tinyui_slider_set_value(slider, 75) == 0);
    assert(wrapper->value == 75);
    /* percent = (75-0)*100/(200-0) = 37 */
    assert(tinyui_slider_get_percent(slider, &percent) == 0);
    assert(percent == 37);
    assert(ld_slider->permille == 370U);

    assert(tinyui_slider_set_value(slider, -1) == -1);
    assert(tinyui_last_result() == TINYUI_ERROR_OUT_OF_RANGE);
    assert(tinyui_slider_set_value(slider, 201) == -1);
    assert(wrapper->value == 75);

    assert(tinyui_slider_set_percent(slider, 100) == 0);
    assert(wrapper->value == 200);
    assert(tinyui_slider_get_percent(slider, &percent) == 0);
    assert(percent == 100);

    assert(tinyui_slider_set_percent(slider, -1) == -1);
    assert(tinyui_slider_set_percent(slider, 101) == -1);
    assert(wrapper->value == 200);

    assert(tinyui_slider_set_horizontal(slider, 0) == 0);
    assert(tinyui_slider_get_horizontal(slider, &horizontal) == 0);
    assert(horizontal == 0);
    assert(ld_slider->isHorizontal == false);

    assert(tinyui_slider_set_horizontal(slider, 1) == 0);
    assert(tinyui_slider_get_horizontal(slider, &horizontal) == 0);
    assert(horizontal == 1);
    assert(ld_slider->isHorizontal == true);

    bind_test_tiles(&background_source, &bg_tile, &bg_mask);
    bind_test_tiles(&indicator_source, &indic_tile, &indic_mask);

    assert(tinyui_slider_set_background_source(slider, &background_source) == 0);
    assert(tinyui_slider_set_indicator_source(slider, &indicator_source) == 0);
    assert(ld_slider->ptBgImgTile == tinyui_image_source_get_image_tile(&background_source));
    assert(ld_slider->ptBgMaskTile == tinyui_image_source_get_mask_tile(&background_source));
    assert(ld_slider->ptIndicImgTile == tinyui_image_source_get_image_tile(&indicator_source));
    assert(ld_slider->ptIndicMaskTile == tinyui_image_source_get_mask_tile(&indicator_source));
    assert(ld_slider->indicWidth == 11U);

    assert(tinyui_slider_set_indicator_width(slider, 21) == 0);
    assert(ld_slider->indicWidth == 21U);

    assert(tinyui_slider_set_slim_size(slider, 9) == 0);
    assert(ld_slider->slimSize == 9U);

    assert(tinyui_slider_set_color(slider, 0x111111U, 0x222222U, 0x333333U) == 0);
    assert(ld_slider->bgColor == (ldColor)0x111111U);
    assert(ld_slider->frameColor == (ldColor)0x222222U);
    assert(ld_slider->indicColor == (ldColor)0x333333U);
}

static void test_slider_programmatic_set_value_no_fake_event(tinyui_obj_t *root)
{
    tinyui_obj_t *slider = tinyui_slider_create(root);
    tinyui_event_handle_t handle = 0U;
    int cookie = 11;

    assert(slider != 0);
    reset_event_fixture();
    assert(tinyui_obj_add_event_cb(slider,
                                   TINYUI_EVENT_MASK(TINYUI_EVENT_VALUE_CHANGED),
                                   on_value_changed,
                                   &cookie,
                                   &handle) == TINYUI_OK);
    assert(tinyui_slider_set_range(slider, 0, 100) == 0);
    assert(tinyui_slider_set_value(slider, 40) == 0);
    assert(g_value_changed_count == 0);
}

static void test_slider_real_permille_emits_canonical_value(tinyui_obj_t *root)
{
    tinyui_obj_t *slider = tinyui_slider_create(root);
    struct tinyui_slider *wrapper;
    tinyui_event_handle_t handle = 0U;
    int cookie = 66;

    assert(slider != 0);
    wrapper = (struct tinyui_slider *)(void *)slider;
    assert(tinyui_slider_set_range(slider, -50, 150) == 0);

    reset_event_fixture();
    assert(tinyui_obj_add_event_cb(slider,
                                   TINYUI_EVENT_MASK(TINYUI_EVENT_VALUE_CHANGED),
                                   on_value_changed,
                                   &cookie,
                                   &handle) == TINYUI_OK);

    /* permille 500 => value = -50 + (200 * 500) / 1000 = 50 */
    inject_slider_permille(slider, 500);
    assert(g_value_changed_count == 1);
    assert(g_last_value == 50);
    assert(wrapper->value == 50);
    assert(g_last_target == slider);
    assert(g_last_user_data == &cookie);

    /* same value no re-fire */
    inject_slider_permille(slider, 500);
    assert(g_value_changed_count == 1);

    /* permille 750 => -50 + 150 = 100 */
    inject_slider_permille(slider, 750);
    assert(g_value_changed_count == 2);
    assert(g_last_value == 100);
    assert(wrapper->value == 100);
}

static void test_slider_set_on_value_changed_forwards_unified_pool(tinyui_obj_t *root)
{
    tinyui_obj_t *slider = tinyui_slider_create(root);
    int cookie = 99;

    assert(slider != 0);
    assert(tinyui_slider_set_range(slider, 0, 100) == 0);
    reset_event_fixture();
    assert(tinyui_slider_set_on_value_changed(slider, on_value_changed, &cookie) == 0);
    inject_slider_permille(slider, 250); /* value 25 */
    assert(g_value_changed_count == 1);
    assert(g_last_value == 25);
    assert(g_last_user_data == &cookie);

    reset_event_fixture();
    assert(tinyui_slider_set_on_value_changed(slider, 0, 0) == 0);
    inject_slider_permille(slider, 500);
    assert(g_value_changed_count == 0);
}

static void test_slider_props_reject_legacy_on_value_changed(tinyui_obj_t *root)
{
    tinyui_slider_props_t props;
    ldBase_t *root_ld = (ldBase_t *)((struct tinyui_widget *)(void *)root)->ld_widget;
    ldBase_t *child_before = ldBaseGetChildList(root_ld);

    memset(&props, 0, sizeof(props));
    props.fields = TINYUI_SLIDER_FIELD_ON_VALUE_CHANGED | TINYUI_SLIDER_FIELD_VALUE;
    props.value = 10;
    props.on_value_changed = (tinyui_value_changed_cb)(void *)0x1;

    assert(tinyui_slider_create_with_props(root, &props) == 0);
    assert(ldBaseGetChildList(root_ld) == child_before);
}

static void test_slider_indicator_source_sets_native_width(tinyui_obj_t *root)
{
    tinyui_obj_t *slider = tinyui_slider_create(root);
    ldSlider_t *ld_slider;
    arm_2d_tile_t indic_tile = {
        .tRegion = {
            .tSize = { .iWidth = 24, .iHeight = 34 },
        },
    };
    arm_2d_tile_t indic_mask = {
        .tRegion = {
            .tSize = { .iWidth = 24, .iHeight = 34 },
        },
    };
    tinyui_image_source_t indicator_source;
    arm_2d_tile_t oversized_tile = {
        .tRegion = {
            .tSize = { .iWidth = 300, .iHeight = 34 },
        },
    };
    tinyui_image_source_t oversized_source;

    assert(slider != 0);
    ld_slider = (ldSlider_t *)((struct tinyui_widget *)(void *)slider)->ld_widget;
    assert(ld_slider != 0);

    bind_test_tiles(&indicator_source, &indic_tile, &indic_mask);
    assert(tinyui_slider_set_indicator_source(slider, &indicator_source) == 0);
    assert(ld_slider->ptIndicImgTile == tinyui_image_source_get_image_tile(&indicator_source));
    assert(ld_slider->ptIndicMaskTile == tinyui_image_source_get_mask_tile(&indicator_source));
    assert(ld_slider->indicWidth == 24U);

    bind_test_tiles(&oversized_source, &oversized_tile, &indic_mask);
    assert(tinyui_slider_set_indicator_source(slider, &oversized_source) == -1);
    assert(ld_slider->ptIndicImgTile == tinyui_image_source_get_image_tile(&indicator_source));
    assert(ld_slider->indicWidth == 24U);
}

static void test_slider_create_with_props_failure_rolls_back_attached_child(tinyui_obj_t *root)
{
    ldBase_t *win_ld = (ldBase_t *)((struct tinyui_widget *)(void *)root)->ld_widget;
    ldBase_t *tail_ld = ldBaseGetChildList(win_ld);
    ldBase_t *next_before_ld = 0;
    tinyui_obj_t *slider;
    const struct tinyui_widget *disposed_backend;
    tinyui_slider_props_t props;

    while (tail_ld != 0 && ldBaseGetNextSibling(tail_ld) != 0) {
        tail_ld = ldBaseGetNextSibling(tail_ld);
    }
    if (tail_ld != 0) {
        next_before_ld = ldBaseGetNextSibling(tail_ld);
    }

    tinyui_test_capture_destroyed_widget_snapshot(0);
    tinyui_slider_test_fail_indicator_width_for_id("slider");
    memset(&props, 0, sizeof(props));
    props.fields = TINYUI_SLIDER_FIELD_MIN_VALUE
        | TINYUI_SLIDER_FIELD_MAX_VALUE
        | TINYUI_SLIDER_FIELD_VALUE
        | TINYUI_SLIDER_FIELD_INDICATOR_WIDTH;
    props.min_value = 0;
    props.max_value = 100;
    props.value = 10;
    props.indicator_width = 12;

    slider = tinyui_slider_create_with_props(root, &props);
    assert(slider == 0);
    disposed_backend = tinyui_slider_test_last_disposed_backend();
    assert(disposed_backend != 0);
    assert(disposed_backend->kind == TINYUI_BACKEND_WIDGET_SLIDER);
    assert(disposed_backend->owner == 0);
    assert(disposed_backend->ld_event_bridge_scene == 0);
    assert(disposed_backend->ld_event_bridge_sender == 0);
    assert(disposed_backend->ld_widget == 0);
    if (tail_ld != 0) {
        assert(ldBaseGetNextSibling(tail_ld) == next_before_ld);
    } else {
        assert(ldBaseGetChildList(win_ld) == 0);
    }

    /* next create with same fields succeeds after one-shot fail seam cleared */
    assert(tinyui_slider_create_with_props(root, &props) != 0);
}

static void test_slider_rejects_null_args(void)
{
    assert(tinyui_slider_create(0) == 0);
    assert(tinyui_slider_create_with_props(0, &(tinyui_slider_props_t){.fields = 0}) == 0);
    assert(tinyui_slider_set_value(0, 10) == -1);
    assert(tinyui_slider_set_range(0, 0, 100) == -1);
    assert(tinyui_slider_set_percent(0, 50) == -1);
    assert(tinyui_slider_set_horizontal(0, 1) == -1);
    assert(tinyui_slider_get_percent(0, &(int){0}) == -1);
    assert(tinyui_slider_set_on_value_changed(0, 0, 0) == -1);
}

static void test_slider_no_dedicated_callback_fields(void)
{
    FILE *file = fopen("/home/share/samba/embedded/LingDongGUI/tinyui/src/core/internal.h", "rb");
    char content[65536];
    size_t n;

    assert(file != 0);
    n = fread(content, 1, sizeof(content) - 1, file);
    content[n] = '\0';
    fclose(file);
    assert(strstr(content, "struct tinyui_slider") != 0);
    assert(strstr(content, "on_value_changed_handle") != 0);
}

int main(void)
{
    tinyui_obj_t *root;

    tinyui_deinit();
    assert(tinyui_init() == TINYUI_OK);
    root = tinyui_screen_create();
    assert(root != 0);

    test_slider_create_and_backend_mapping(root);
    test_slider_create_with_props_pushes_range(root);
    test_slider_range_mapping_negative_and_zero(root);
    test_slider_set_range_and_value_round_trip(root);
    test_slider_programmatic_set_value_no_fake_event(root);
    test_slider_real_permille_emits_canonical_value(root);
    test_slider_set_on_value_changed_forwards_unified_pool(root);
    test_slider_props_reject_legacy_on_value_changed(root);
    test_slider_indicator_source_sets_native_width(root);
    test_slider_create_with_props_failure_rolls_back_attached_child(root);
    test_slider_rejects_null_args();
    test_slider_no_dedicated_callback_fields();

    tinyui_deinit();
    return 0;
}
