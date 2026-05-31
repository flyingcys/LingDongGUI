#include "picoui/picoui.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldButton.h"
#include "../../../src/gui/ldCheckBox.h"
#include "../../../src/gui/ldImage.h"
#include "../../../src/gui/ldKeyboard.h"
#include "../../../src/gui/ldLabel.h"
#include "../../../src/gui/ldLineEdit.h"
#include "../../../src/gui/ldSlider.h"
#include "../../../src/gui/ldSwitch.h"
#include "../../../src/gui/ldText.h"
#include "../../../src/misc/ldMsg.h"
#include "internal.h"

#include <assert.h>
#include <string.h>

static int switch_toggled_count = 0;
static int switch_toggled_value = -1;
static int checkbox_toggled_count = 0;
static int checkbox_toggled_value = -1;
static int slider_value_count = 0;
static int slider_value = -1;
static int button_clicked = -1;
static int button_pressed_count = 0;
static int button_released_count = 0;

struct test_text_box_prefix_view {
    text_box_cfg_t tCFG;
};

void picoui_backend_text_test_fail_next_set_font(void);
extern const arm_2d_a1_font_t ARM_2D_FONT_6x8;

static arm_2d_font_t *test_text_consumed_font(const ldText_t *ld_text)
{
    const struct test_text_box_prefix_view *view;

    assert(ld_text != 0);
    view = (const struct test_text_box_prefix_view *)&ld_text->tTextPanel;
    return (arm_2d_font_t *)view->tCFG.ptFont;
}

static void assert_button_has_no_bound_images(const struct picoui_button *button)
{
    const struct picoui_backend_widget *backend = button->widget.backend_widget;
    const ldButton_t *ld_button = (const ldButton_t *)backend->ld_widget;

    assert(ld_button != 0);
    assert(ld_button->ptReleaseImgTile == 0);
    assert(ld_button->ptReleaseMaskTile == 0);
    assert(ld_button->ptPressImgTile == 0);
    assert(ld_button->ptPressMaskTile == 0);
}

static void assert_button_has_bound_images(const struct picoui_button *button,
                                           const struct picoui_image_source *release_source,
                                           const struct picoui_image_source *press_source)
{
    const struct picoui_backend_widget *backend = button->widget.backend_widget;
    const ldButton_t *ld_button = (const ldButton_t *)backend->ld_widget;

    assert(ld_button != 0);
    assert(ld_button->ptReleaseImgTile == (release_source != 0 ? release_source->img_tile : 0));
    assert(ld_button->ptReleaseMaskTile == (release_source != 0 ? release_source->mask_tile : 0));
    assert(ld_button->ptPressImgTile == (press_source != 0 ? press_source->img_tile : 0));
    assert(ld_button->ptPressMaskTile == (press_source != 0 ? press_source->mask_tile : 0));
}

static void assert_slider_has_bound_images(const struct picoui_slider *slider,
                                           const struct picoui_image_source *background_source,
                                           const struct picoui_image_source *indicator_source)
{
    const struct picoui_backend_widget *backend = slider->widget.backend_widget;
    const ldSlider_t *ld_slider = (const ldSlider_t *)backend->ld_widget;

    assert(ld_slider != 0);
    assert(ld_slider->ptBgImgTile == (background_source != 0 ? background_source->img_tile : 0));
    assert(ld_slider->ptBgMaskTile == (background_source != 0 ? background_source->mask_tile : 0));
    assert(ld_slider->ptIndicImgTile == (indicator_source != 0 ? indicator_source->img_tile : 0));
    assert(ld_slider->ptIndicMaskTile == (indicator_source != 0 ? indicator_source->mask_tile : 0));
}

static void on_switch_toggle(struct picoui_widget *widget, int value, void *user_data)
{
    (void)widget;
    (void)user_data;
    switch_toggled_count++;
    switch_toggled_value = value;
}

static void on_checkbox_toggle(struct picoui_widget *widget, int value, void *user_data)
{
    (void)widget;
    (void)user_data;
    checkbox_toggled_count++;
    checkbox_toggled_value = value;
}

static void on_slider(struct picoui_widget *widget, int value, void *user_data)
{
    (void)widget;
    (void)user_data;
    slider_value_count++;
    slider_value = value;
}

static void on_button_clicked(struct picoui_widget *widget, void *user_data)
{
    (void)widget;
    button_clicked = user_data != 0 ? *(const int *)user_data : 0;
}

static void on_button_pressed(struct picoui_widget *widget, void *user_data)
{
    (void)widget;
    (void)user_data;
    button_pressed_count++;
}

static void on_button_released(struct picoui_widget *widget, void *user_data)
{
    (void)widget;
    (void)user_data;
    button_released_count++;
}

static void test_focus_owner_switches_between_widgets(struct picoui_app *app,
                                                      struct picoui_button *button,
                                                      struct picoui_switch *sw,
                                                      struct picoui_checkbox *cb,
                                                      struct picoui_slider *slider,
                                                      struct ld_scene_t *scene)
{
    int button_enter_before;
    int button_leave_before;
    int button_change_before;
    int switch_enter_before;
    int switch_leave_before;
    int switch_change_before;
    int checkbox_enter_before;
    int checkbox_leave_before;
    int checkbox_change_before;
    int slider_enter_before;
    int slider_leave_before;
    int slider_change_before;
    struct picoui_backend_widget *sw_backend;
    struct picoui_backend_widget *cb_backend;
    struct picoui_backend_widget *slider_backend;

    assert(app != 0);
    assert(button != 0);
    assert(sw != 0);
    assert(cb != 0);
    assert(slider != 0);
    assert(scene != 0);
    assert(scene->ptMsgQueue != 0);
    assert(button->widget.backend_widget != 0);
    assert(sw->widget.backend_widget != 0);
    assert(cb->widget.backend_widget != 0);
    assert(slider->widget.backend_widget != 0);
    if (app->focus_owner != 0) {
        assert(picoui_widget_release_focus(app->focus_owner) == 0);
    }
    sw_backend = (struct picoui_backend_widget *)sw->widget.backend_widget;
    cb_backend = (struct picoui_backend_widget *)cb->widget.backend_widget;
    slider_backend = (struct picoui_backend_widget *)slider->widget.backend_widget;

    button_enter_before = button->widget.focus_enter_count;
    button_leave_before = button->widget.focus_leave_count;
    button_change_before = button->widget.focus_change_count;
    switch_enter_before = sw->widget.focus_enter_count;
    switch_leave_before = sw->widget.focus_leave_count;
    switch_change_before = sw->widget.focus_change_count;
    checkbox_enter_before = cb->widget.focus_enter_count;
    checkbox_leave_before = cb->widget.focus_leave_count;
    checkbox_change_before = cb->widget.focus_change_count;
    slider_enter_before = slider->widget.focus_enter_count;
    slider_leave_before = slider->widget.focus_leave_count;
    slider_change_before = slider->widget.focus_change_count;

    button_pressed_count = 0;
    button_released_count = 0;
    assert(picoui_button_set_on_pressed(button, on_button_pressed, 0) == 0);

    assert(app->focus_owner == 0);
    assert(picoui_backend_widget_dispatch_native_signal(button->widget.backend_widget, SIGNAL_PRESS, 0) == 0);
    assert(app->focus_owner == &button->widget);
    assert(button->widget.has_focus == 1);
    assert(button->widget.focus_enter_count == button_enter_before + 1);
    assert(button->widget.focus_leave_count == button_leave_before);
    assert(button->widget.focus_change_count == button_change_before + 1);
    assert(button->widget.last_focus_event == PICOUI_FOCUS_EVENT_ENTER);
    assert(button_pressed_count == 1);

    assert(ldMsgEmit(scene->ptMsgQueue, sw_backend->ld_widget, SIGNAL_VALUE_CHANGED, 1) == true);
    ldMsgProcess(scene);
    assert(app->focus_owner == &sw->widget);
    assert(button->widget.has_focus == 0);
    assert(button->widget.focus_enter_count == button_enter_before + 1);
    assert(button->widget.focus_leave_count == button_leave_before + 1);
    assert(button->widget.focus_change_count == button_change_before + 2);
    assert(sw->widget.has_focus == 1);
    assert(sw->widget.focus_enter_count == switch_enter_before + 1);
    assert(sw->widget.focus_leave_count == switch_leave_before);
    assert(sw->widget.focus_change_count == switch_change_before + 1);
    assert(sw->widget.last_focus_event == PICOUI_FOCUS_EVENT_ENTER);

    assert(ldMsgEmit(scene->ptMsgQueue, cb_backend->ld_widget, SIGNAL_VALUE_CHANGED, 0) == true);
    ldMsgProcess(scene);
    assert(app->focus_owner == &cb->widget);
    assert(sw->widget.has_focus == 0);
    assert(sw->widget.focus_enter_count == switch_enter_before + 1);
    assert(sw->widget.focus_leave_count == switch_leave_before + 1);
    assert(sw->widget.focus_change_count == switch_change_before + 2);
    assert(cb->widget.has_focus == 1);
    assert(cb->widget.focus_enter_count == checkbox_enter_before + 1);
    assert(cb->widget.focus_leave_count == checkbox_leave_before);
    assert(cb->widget.focus_change_count == checkbox_change_before + 1);
    assert(cb->widget.last_focus_event == PICOUI_FOCUS_EVENT_ENTER);

    assert(ldMsgEmit(scene->ptMsgQueue, slider_backend->ld_widget, SIGNAL_VALUE_CHANGED, 625) == true);
    ldMsgProcess(scene);
    assert(app->focus_owner == &slider->widget);
    assert(cb->widget.has_focus == 0);
    assert(cb->widget.focus_enter_count == checkbox_enter_before + 1);
    assert(cb->widget.focus_leave_count == checkbox_leave_before + 1);
    assert(cb->widget.focus_change_count == checkbox_change_before + 2);
    assert(cb->widget.last_focus_event == PICOUI_FOCUS_EVENT_LEAVE);
    assert(slider->widget.has_focus == 1);
    assert(slider->widget.focus_enter_count == slider_enter_before + 1);
    assert(slider->widget.focus_leave_count == slider_leave_before);
    assert(slider->widget.focus_change_count == slider_change_before + 1);
    assert(slider->widget.last_focus_event == PICOUI_FOCUS_EVENT_ENTER);
}

static void test_hidden_or_disabled_widget_cannot_keep_focus(struct picoui_app *app,
                                                             struct picoui_button *button)
{
    struct picoui_backend_widget *backend;
    int focus_enter_before;
    int focus_leave_before;

    assert(app != 0);
    assert(button != 0);

    backend = (struct picoui_backend_widget *)button->widget.backend_widget;
    assert(backend != 0);
    if (app->focus_owner != 0) {
        assert(picoui_widget_release_focus(app->focus_owner) == 0);
    }

    focus_enter_before = button->widget.focus_enter_count;
    focus_leave_before = button->widget.focus_leave_count;

    assert(picoui_backend_widget_dispatch_native_signal(backend, SIGNAL_PRESS, 0) == 0);
    assert(app->focus_owner == &button->widget);
    assert(button->widget.has_focus == 1);
    assert(button->widget.focus_enter_count == focus_enter_before + 1);
    assert(button->widget.focus_leave_count == focus_leave_before);

    assert(picoui_widget_set_visible(&button->widget, 0) == 0);
    assert(app->focus_owner == 0);
    assert(button->widget.has_focus == 0);
    assert(button->widget.focus_leave_count == focus_leave_before + 1);
    assert(button->widget.last_focus_event == PICOUI_FOCUS_EVENT_LEAVE);

    assert(picoui_widget_set_visible(&button->widget, 1) == 0);
    assert(picoui_backend_widget_dispatch_native_signal(backend, SIGNAL_PRESS, 0) == 0);
    assert(app->focus_owner == &button->widget);
    assert(button->widget.has_focus == 1);
    assert(button->widget.focus_enter_count == focus_enter_before + 2);
    assert(button->widget.focus_leave_count == focus_leave_before + 1);

    assert(picoui_widget_set_enabled(&button->widget, 0) == 0);
    assert(app->focus_owner == 0);
    assert(button->widget.has_focus == 0);
    assert(button->widget.focus_leave_count == focus_leave_before + 2);
    assert(button->widget.last_focus_event == PICOUI_FOCUS_EVENT_LEAVE);
    assert(picoui_backend_widget_dispatch_native_signal(backend, SIGNAL_PRESS, 0) == 0);
    assert(app->focus_owner == 0);
    assert(button->widget.has_focus == 0);
    assert(button->widget.focus_enter_count == focus_enter_before + 2);
}

static void test_model_readback_policy_is_explicit_for_data_widgets(struct picoui_switch *sw,
                                                                    struct picoui_checkbox *cb,
                                                                    struct picoui_slider *slider)
{
    struct picoui_backend_widget *sw_backend;
    struct picoui_backend_widget *cb_backend;
    struct picoui_backend_widget *slider_backend;

    assert(sw != 0);
    assert(cb != 0);
    assert(slider != 0);

    sw_backend = (struct picoui_backend_widget *)sw->widget.backend_widget;
    cb_backend = (struct picoui_backend_widget *)cb->widget.backend_widget;
    slider_backend = (struct picoui_backend_widget *)slider->widget.backend_widget;
    assert(sw_backend != 0);
    assert(cb_backend != 0);
    assert(slider_backend != 0);

    assert(sw_backend->data_truth_policy == PICOUI_BACKEND_DATA_TRUTH_BACKEND_VALUE);
    assert(cb_backend->data_truth_policy == PICOUI_BACKEND_DATA_TRUTH_BACKEND_VALUE);
    assert(slider_backend->data_truth_policy == PICOUI_BACKEND_DATA_TRUTH_BACKEND_VALUE);
}

static void test_item_model_identity_survives_frame_update(struct picoui_switch *sw,
                                                           struct picoui_checkbox *cb,
                                                           struct picoui_slider *slider,
                                                           struct ld_scene_t *scene)
{
    struct picoui_backend_widget *sw_backend;
    struct picoui_backend_widget *cb_backend;
    struct picoui_backend_widget *slider_backend;
    unsigned int sw_identity_before;
    unsigned int cb_identity_before;
    unsigned int slider_identity_before;

    assert(sw != 0);
    assert(cb != 0);
    assert(slider != 0);
    assert(scene != 0);
    assert(scene->ptMsgQueue != 0);

    sw_backend = (struct picoui_backend_widget *)sw->widget.backend_widget;
    cb_backend = (struct picoui_backend_widget *)cb->widget.backend_widget;
    slider_backend = (struct picoui_backend_widget *)slider->widget.backend_widget;
    assert(sw_backend != 0);
    assert(cb_backend != 0);
    assert(slider_backend != 0);

    assert(sw_backend->data_model_identity != 0);
    assert(cb_backend->data_model_identity != 0);
    assert(slider_backend->data_model_identity != 0);
    assert(sw_backend->data_model_identity != cb_backend->data_model_identity);
    assert(sw_backend->data_model_identity != slider_backend->data_model_identity);
    assert(cb_backend->data_model_identity != slider_backend->data_model_identity);

    sw_identity_before = sw_backend->data_model_identity;
    cb_identity_before = cb_backend->data_model_identity;
    slider_identity_before = slider_backend->data_model_identity;

    assert(picoui_switch_set_checked(sw, 0) == 0);
    assert(sw_backend->last_data_source == PICOUI_BACKEND_DATA_SOURCE_SETTER);
    assert(sw_backend->data_model_epoch > 0);

    assert(ldMsgEmit(scene->ptMsgQueue, sw_backend->ld_widget, SIGNAL_VALUE_CHANGED, 1) == true);
    ldMsgProcess(scene);
    assert(sw_backend->last_data_source == PICOUI_BACKEND_DATA_SOURCE_NATIVE_EVENT);
    assert(sw_backend->data_model_identity == sw_identity_before);

    assert(picoui_checkbox_set_checked(cb, 1) == 0);
    assert(cb_backend->last_data_source == PICOUI_BACKEND_DATA_SOURCE_SETTER);
    assert(cb_backend->data_model_epoch > 0);
    assert(cb_backend->data_model_identity == cb_identity_before);

    assert(picoui_slider_set_value(slider, 28) == 0);
    assert(slider_backend->last_data_source == PICOUI_BACKEND_DATA_SOURCE_SETTER);
    assert(slider_backend->data_model_epoch > 0);
    assert(slider_backend->data_model_identity == slider_identity_before);

    assert(ldMsgEmit(scene->ptMsgQueue, slider_backend->ld_widget, SIGNAL_VALUE_CHANGED, 350) == true);
    ldMsgProcess(scene);
    assert(slider_backend->last_data_source == PICOUI_BACKEND_DATA_SOURCE_NATIVE_EVENT);
    assert(slider_backend->data_model_identity == slider_identity_before);
}

static void test_native_duplicate_value_does_not_advance_data_model(struct picoui_switch *sw,
                                                                    struct picoui_checkbox *cb,
                                                                    struct picoui_slider *slider,
                                                                    struct ld_scene_t *scene)
{
    struct picoui_backend_widget *sw_backend;
    struct picoui_backend_widget *cb_backend;
    struct picoui_backend_widget *slider_backend;
    unsigned int sw_epoch_before;
    unsigned int cb_epoch_before;
    unsigned int slider_epoch_before;
    int sw_dispatch_before;
    int cb_dispatch_before;
    int slider_dispatch_before;

    assert(sw != 0);
    assert(cb != 0);
    assert(slider != 0);
    assert(scene != 0);
    assert(scene->ptMsgQueue != 0);

    sw_backend = (struct picoui_backend_widget *)sw->widget.backend_widget;
    cb_backend = (struct picoui_backend_widget *)cb->widget.backend_widget;
    slider_backend = (struct picoui_backend_widget *)slider->widget.backend_widget;
    assert(sw_backend != 0);
    assert(cb_backend != 0);
    assert(slider_backend != 0);

    switch_toggled_count = 0;
    checkbox_toggled_count = 0;
    slider_value_count = 0;

    assert(picoui_switch_set_checked(sw, 1) == 0);
    assert(picoui_checkbox_set_checked(cb, 0) == 0);
    assert(picoui_slider_set_value(slider, 35) == 0);

    sw_epoch_before = sw_backend->data_model_epoch;
    cb_epoch_before = cb_backend->data_model_epoch;
    slider_epoch_before = slider_backend->data_model_epoch;
    sw_dispatch_before = sw_backend->dispatch_count;
    cb_dispatch_before = cb_backend->dispatch_count;
    slider_dispatch_before = slider_backend->dispatch_count;

    assert(ldMsgEmit(scene->ptMsgQueue, sw_backend->ld_widget, SIGNAL_VALUE_CHANGED, 1) == true);
    assert(ldMsgEmit(scene->ptMsgQueue, cb_backend->ld_widget, SIGNAL_VALUE_CHANGED, 0) == true);
    assert(ldMsgEmit(scene->ptMsgQueue, slider_backend->ld_widget, SIGNAL_VALUE_CHANGED, 375) == true);
    ldMsgProcess(scene);

    assert(sw_backend->data_model_epoch == sw_epoch_before);
    assert(cb_backend->data_model_epoch == cb_epoch_before);
    assert(slider_backend->data_model_epoch == slider_epoch_before);
    assert(sw_backend->dispatch_count == sw_dispatch_before);
    assert(cb_backend->dispatch_count == cb_dispatch_before);
    assert(slider_backend->dispatch_count == slider_dispatch_before);
    assert(switch_toggled_count == 0);
    assert(checkbox_toggled_count == 0);
    assert(slider_value_count == 0);
}

static void test_slider_j5_contract(struct picoui_slider *slider,
                                    struct picoui_image_source *background_source,
                                    struct picoui_image_source *indicator_source)
{
    struct picoui_backend_widget *backend;
    ldSlider_t *ld_slider;
    int horizontal = -1;
    int percent = -1;
    struct picoui_image_source invalid_source = {
        .img_tile = 0,
        .mask_tile = background_source->mask_tile,
    };

    assert(slider != 0);
    backend = slider->widget.backend_widget;
    ld_slider = (ldSlider_t *)backend->ld_widget;
    assert(ld_slider != 0);

    assert(picoui_slider_set_range(slider, -20, 80) == 0);
    assert(picoui_slider_set_value(slider, 30) == 0);
    assert(picoui_slider_set_horizontal(slider, 0) == 0);
    assert(picoui_slider_set_background_source(slider, background_source) == 0);
    assert(picoui_slider_set_indicator_source(slider, indicator_source) == 0);
    assert(picoui_slider_set_indicator_width(slider, 18) == 0);
    assert(picoui_slider_set_slim_size(slider, 6) == 0);

    assert(picoui_slider_get_horizontal(slider, &horizontal) == 0);
    assert(horizontal == 0);
    assert(ld_slider->isHorizontal == false);
    assert_slider_has_bound_images(slider, background_source, indicator_source);
    assert(ld_slider->indicWidth == 18);
    assert(ld_slider->slimSize == 6);
    assert(picoui_slider_get_percent(slider, &percent) == 0);
    assert(percent == 50);
    assert(ld_slider->permille == 500);

    assert(picoui_slider_set_horizontal(slider, 1) == 0);
    assert(picoui_slider_get_horizontal(slider, &horizontal) == 0);
    assert(horizontal == 1);
    assert(ld_slider->isHorizontal == true);

    assert(picoui_slider_set_background_source(slider, 0) == 0);
    assert_slider_has_bound_images(slider, 0, indicator_source);
    assert(picoui_slider_set_indicator_source(slider, 0) == 0);
    assert_slider_has_bound_images(slider, 0, 0);
    assert(picoui_slider_set_background_source(slider, background_source) == 0);
    assert(picoui_slider_set_indicator_source(slider, indicator_source) == 0);
    assert_slider_has_bound_images(slider, background_source, indicator_source);

    assert(picoui_slider_set_indicator_width(slider, 22) == 0);
    assert(ld_slider->indicWidth == 22);
    assert(picoui_slider_set_slim_size(slider, 8) == 0);
    assert(ld_slider->slimSize == 8);
    assert(picoui_slider_set_indicator_width(slider, 256) == -1);
    assert(ld_slider->indicWidth == 22);
    assert(picoui_slider_set_slim_size(slider, 256) == -1);
    assert(ld_slider->slimSize == 8);

    assert(picoui_slider_set_value(slider, 80) == 0);
    assert(ld_slider->permille == 1000);
    assert(picoui_slider_get_percent(slider, &percent) == 0);
    assert(percent == 100);

    assert(picoui_slider_set_range(slider, 20, 60) == 0);
    assert(ld_slider->permille == 1000);
    assert(picoui_slider_get_percent(slider, &percent) == 0);
    assert(percent == 100);

    assert(picoui_slider_set_value(slider, 40) == 0);
    assert(ld_slider->permille == 500);
    assert(picoui_slider_get_percent(slider, &percent) == 0);
    assert(percent == 50);

    assert(picoui_slider_set_background_source(0, background_source) == -1);
    assert(picoui_slider_set_indicator_source(0, indicator_source) == -1);
    assert(picoui_slider_set_indicator_width(0, 10) == -1);
    assert(picoui_slider_set_slim_size(0, 4) == -1);
    assert(picoui_slider_set_horizontal(0, 1) == -1);
    assert(picoui_slider_get_horizontal(0, &horizontal) == -1);
    assert(picoui_slider_get_horizontal(slider, 0) == -1);
    assert(picoui_slider_get_percent(0, &percent) == -1);
    assert(picoui_slider_get_percent(slider, 0) == -1);
    assert(picoui_slider_set_background_source(slider, &invalid_source) == -1);
    assert(picoui_slider_set_indicator_source(slider, &invalid_source) == -1);
}

static void test_backend_value_changed_bridge_keeps_setter_sync_only(struct picoui_slider *slider,
                                                                     struct picoui_backend_widget *slider_backend,
                                                                     struct ld_scene_t *scene)
{
    ldBase_t sender = {0};
    ldMsg_t msg = {0};

    assert(scene != 0);
    assert(scene->ptMsgQueue != 0);
    assert(picoui_backend_widget_bind_ld_event_bridge(slider_backend, scene, &sender) == 0);
    assert(picoui_slider_set_value(slider, 12) == 0);
    assert(slider_value_count == 0);
    assert(slider_value == -1);
    assert(xQueueDequeue(scene->ptMsgQueue, &msg, sizeof(msg)) == false);
}

static void test_native_event_bridge_prefers_native_path(struct picoui_switch *sw,
                                                         struct picoui_checkbox *cb,
                                                         struct picoui_slider *slider,
                                                         struct ld_scene_t *scene)
{
    struct picoui_backend_widget *sw_backend = sw->widget.backend_widget;
    struct picoui_backend_widget *cb_backend = cb->widget.backend_widget;
    struct picoui_backend_widget *slider_backend = slider->widget.backend_widget;
    ldSlider_t *ld_slider = (ldSlider_t *)slider_backend->ld_widget;

    switch_toggled_count = 0;
    checkbox_toggled_count = 0;
    slider_value_count = 0;

    assert(picoui_switch_set_checked(sw, 0) == 0);
    assert(picoui_checkbox_set_checked(cb, 1) == 0);
    assert(picoui_slider_set_value(slider, 11) == 0);

    switch_toggled_count = 0;
    checkbox_toggled_count = 0;
    slider_value_count = 0;
    sw_backend->dispatch_count = 0;
    cb_backend->dispatch_count = 0;
    slider_backend->dispatch_count = 0;
    sw_backend->last_signal = PICOUI_BACKEND_SIGNAL_NONE;
    cb_backend->last_signal = PICOUI_BACKEND_SIGNAL_NONE;
    slider_backend->last_signal = PICOUI_BACKEND_SIGNAL_NONE;

    assert(ldMsgEmit(scene->ptMsgQueue, sw_backend->ld_widget, SIGNAL_VALUE_CHANGED, 1) == true);
    ldMsgProcess(scene);
    assert(picoui_switch_is_checked(sw) == 1);
    assert(switch_toggled_count == 1);
    assert(switch_toggled_value == 1);
    assert(sw_backend->value == 1);
    assert(sw_backend->last_signal == PICOUI_BACKEND_SIGNAL_VALUE_CHANGED);
    assert(sw_backend->dispatch_count == 1);

    assert(ldMsgEmit(scene->ptMsgQueue, cb_backend->ld_widget, SIGNAL_VALUE_CHANGED, 0) == true);
    ldMsgProcess(scene);
    assert(picoui_checkbox_is_checked(cb) == 0);
    assert(checkbox_toggled_count == 1);
    assert(checkbox_toggled_value == 0);
    assert(cb_backend->value == 0);
    assert(cb_backend->last_signal == PICOUI_BACKEND_SIGNAL_VALUE_CHANGED);
    assert(cb_backend->dispatch_count == 1);

    assert(ldMsgEmit(scene->ptMsgQueue, slider_backend->ld_widget, SIGNAL_VALUE_CHANGED, 625) == true);
    ldMsgProcess(scene);
    assert(slider_backend->value == 35);
    {
        int percent = -1;
        assert(picoui_slider_get_percent(slider, &percent) == 0);
        assert(percent == 62);
    }
    assert(slider_value_count == 1);
    assert(slider_value == 35);
    assert(ld_slider->permille == 620);
    assert(slider_backend->value == 35);
    assert(slider_backend->last_signal == PICOUI_BACKEND_SIGNAL_VALUE_CHANGED);
    assert(slider_backend->dispatch_count == 1);
}

static void test_backend_widget_tree_contract(struct picoui_app *app,
                                              struct picoui_window *win,
                                              struct picoui_switch *sw,
                                              struct picoui_checkbox *cb,
                                              struct picoui_slider *slider,
                                              struct picoui_label *label,
                                              struct picoui_button *button,
                                              struct picoui_text *text,
                                              struct picoui_image *image)
{
    struct picoui_backend_widget *win_backend = win->widget.backend_widget;
    struct picoui_backend_widget *sw_backend = sw->widget.backend_widget;
    struct picoui_backend_widget *cb_backend = cb->widget.backend_widget;
    struct picoui_backend_widget *slider_backend = slider->widget.backend_widget;
    struct picoui_backend_widget *label_backend = label->widget.backend_widget;
    struct picoui_backend_widget *button_backend = button->widget.backend_widget;
    struct picoui_backend_widget *text_backend = text->widget.backend_widget;
    struct picoui_backend_widget *image_backend = image->widget.backend_widget;
    struct picoui_backend_widget *dialog_backend;

    assert(picoui_backend_widget_is_kind(win_backend, PICOUI_BACKEND_WIDGET_WINDOW) == 1);
    assert(picoui_backend_widget_is_kind(sw_backend, PICOUI_BACKEND_WIDGET_SWITCH) == 1);
    assert(picoui_backend_widget_is_kind(cb_backend, PICOUI_BACKEND_WIDGET_CHECKBOX) == 1);
    assert(picoui_backend_widget_is_kind(slider_backend, PICOUI_BACKEND_WIDGET_SLIDER) == 1);
    assert(picoui_backend_widget_is_kind(label_backend, PICOUI_BACKEND_WIDGET_LABEL) == 1);
    assert(picoui_backend_widget_is_kind(button_backend, PICOUI_BACKEND_WIDGET_BUTTON) == 1);
    assert(picoui_backend_widget_is_kind(text_backend, PICOUI_BACKEND_WIDGET_TEXT) == 1);
    assert(picoui_backend_widget_is_kind(image_backend, PICOUI_BACKEND_WIDGET_IMAGE) == 1);
    assert(picoui_backend_widget_is_kind(win_backend, PICOUI_BACKEND_WIDGET_LABEL) == 0);

    assert(picoui_backend_widget_get_owner(win_backend) == app);
    assert(picoui_backend_widget_get_owner(sw_backend) == app);
    assert(picoui_backend_widget_get_owner(cb_backend) == app);
    assert(picoui_backend_widget_get_owner(slider_backend) == app);
    assert(picoui_backend_widget_get_owner(label_backend) == app);
    assert(picoui_backend_widget_get_owner(button_backend) == app);
    assert(picoui_backend_widget_get_owner(text_backend) == app);
    assert(picoui_backend_widget_get_owner(image_backend) == app);

    assert(picoui_backend_widget_get_root(win_backend) == win_backend);
    assert(picoui_backend_widget_get_root(sw_backend) == win_backend);
    assert(picoui_backend_widget_get_root(cb_backend) == win_backend);
    assert(picoui_backend_widget_get_root(slider_backend) == win_backend);
    assert(picoui_backend_widget_get_root(label_backend) == win_backend);
    assert(picoui_backend_widget_get_root(button_backend) == win_backend);
    assert(picoui_backend_widget_get_root(text_backend) == win_backend);
    assert(picoui_backend_widget_get_root(image_backend) == win_backend);

    assert(win_backend->first_child == sw_backend);
    assert(sw_backend->next_sibling == cb_backend);
    assert(cb_backend->next_sibling == slider_backend);
    assert(slider_backend->next_sibling == button_backend);
    assert(button_backend->next_sibling == label_backend);
    assert(label_backend->next_sibling == text_backend);
    assert(text_backend->next_sibling == image_backend);
    assert(image_backend->next_sibling == 0);

    assert(picoui_backend_create_label(label_backend, "bad-nested-label") == 0);
    dialog_backend = picoui_backend_create_window(app, "dialog");
    assert(dialog_backend != 0);
    assert(picoui_backend_widget_attach_child(win_backend, dialog_backend) == -1);
}

static void assert_widget_props(const struct picoui_widget *widget,
                                const struct picoui_backend_widget *backend,
                                const char *style_class,
                                void *user_data,
                                unsigned int bg_color,
                                unsigned int text_color,
                                unsigned int border_color,
                                int radius,
                                int padding)
{
    assert(widget->style_class == style_class);
    assert(widget->user_data == user_data);
    assert(widget->bg_color == bg_color);
    assert(widget->text_color == text_color);
    assert(widget->border_color == border_color);
    assert(widget->radius == radius);
    assert(widget->padding == padding);
    assert(backend->style_class == style_class);
    assert(backend->user_data == user_data);
}

static int backend_child_count(const struct picoui_backend_widget *parent)
{
    const struct picoui_backend_widget *child;
    int count = 0;

    for (child = parent->first_child; child != 0; child = child->next_sibling) {
        count++;
    }
    return count;
}

static void assert_backend_tree_unchanged(const struct picoui_window *parent, int expected_count)
{
    const struct picoui_backend_widget *backend = parent->widget.backend_widget;

    assert(backend_child_count(backend) == expected_count);
}

static void assert_image_has_no_bound_source(const struct picoui_image *image)
{
    const struct picoui_backend_widget *backend = image->widget.backend_widget;
    const ldImage_t *ld_image = (const ldImage_t *)backend->ld_widget;

    assert(image->source == 0);
    assert(backend->image_source == 0);
    assert(ld_image != 0);
    assert(ld_image->ptImgTile == 0);
    assert(ld_image->ptMaskTile == 0);
}

static void assert_image_has_bound_source(const struct picoui_image *image,
                                          const struct picoui_image_source *source)
{
    const struct picoui_backend_widget *backend = image->widget.backend_widget;
    const ldImage_t *ld_image = (const ldImage_t *)backend->ld_widget;

    assert(image->source == source);
    assert(backend->image_source == source);
    assert(ld_image->ptImgTile == source->img_tile);
    assert(ld_image->ptMaskTile == source->mask_tile);
}

static unsigned int quantize_rgb_to_ld_roundtrip(unsigned int rgb)
{
    unsigned int encoded = (unsigned int)__RGB((rgb >> 16) & 0xFFU,
                                               (rgb >> 8) & 0xFFU,
                                               rgb & 0xFFU);
    unsigned int red = (encoded >> 11) & 0x1FU;
    unsigned int green = (encoded >> 5) & 0x3FU;
    unsigned int blue = encoded & 0x1FU;

    red = (red << 3) | (red >> 2);
    green = (green << 2) | (green >> 4);
    blue = (blue << 3) | (blue >> 2);
    return (red << 16) | (green << 8) | blue;
}

static void assert_label_has_no_background_source(const struct picoui_label *label)
{
    const struct picoui_backend_widget *backend = label->widget.backend_widget;
    const ldLabel_t *ld_label = (const ldLabel_t *)backend->ld_widget;

    assert(ld_label != 0);
    assert(ld_label->ptImgTile == 0);
    assert(ld_label->ptMaskTile == 0);
}

static void assert_label_has_background_source(const struct picoui_label *label,
                                               const struct picoui_image_source *source)
{
    const struct picoui_backend_widget *backend = label->widget.backend_widget;
    const ldLabel_t *ld_label = (const ldLabel_t *)backend->ld_widget;

    assert(ld_label != 0);
    assert(ld_label->ptImgTile == source->img_tile);
    assert(ld_label->ptMaskTile == source->mask_tile);
}

static void test_label_parity_contract(struct picoui_label *label,
                                       struct picoui_image_source *image_source,
                                       const struct picoui_font *font)
{
    unsigned int rgb = 0;
    int transparent = -1;
    enum picoui_align align = (enum picoui_align)-1;
    struct picoui_image_source invalid_source = {
        .img_tile = 0,
        .mask_tile = image_source->mask_tile,
    };
    struct picoui_image_source unmasked_source = {
        .img_tile = image_source->img_tile,
        .mask_tile = 0,
    };
    struct picoui_backend_widget *backend = label->widget.backend_widget;
    ldLabel_t *ld_label = (ldLabel_t *)backend->ld_widget;

    assert(label != 0);
    assert(ld_label != 0);

    assert(picoui_label_set_text(label, "hello") == 0);
    assert(picoui_label_get_text(label) != 0);
    assert(strcmp(picoui_label_get_text(label), "hello") == 0);
    assert(label->widget.text == (const char *)"hello");
    assert(ldLabelGetText(ld_label) != 0);
    assert(strcmp((const char *)ldLabelGetText(ld_label), "hello") == 0);

    assert(picoui_label_set_font(label, font) == 0);
    assert(label->widget.font == font);
    assert(ldLabelGetFont(ld_label) != 0);

    assert(picoui_label_set_text_color(label, 0x445566U) == 0);
    assert(picoui_label_get_text_color(label, &rgb) == 0);
    assert(rgb == quantize_rgb_to_ld_roundtrip(0x445566U));
    assert(ldLabelGetTextColor(ld_label) == __RGB(0x44, 0x55, 0x66));
    assert(picoui_label_get_text_color(0, &rgb) == -1);
    assert(picoui_label_get_text_color(label, 0) == -1);

    assert(picoui_label_set_bg_color(label, 0x112233U) == 0);
    assert(picoui_label_get_bg_color(label, &rgb) == 0);
    assert(rgb == quantize_rgb_to_ld_roundtrip(0x112233U));
    assert(ldLabelGetBackgroundColor(ld_label) == __RGB(0x11, 0x22, 0x33));
    assert(picoui_label_get_bg_color(0, &rgb) == -1);
    assert(picoui_label_get_bg_color(label, 0) == -1);
    assert(picoui_label_get_transparent(label, &transparent) == 0);
    assert(transparent == 0);
    assert(ldLabelGetTransparent(ld_label) == false);
    assert_label_has_no_background_source(label);

    assert(picoui_label_get_transparent(label, &transparent) == 0);
    assert(transparent == 0);
    assert(ldLabelGetTransparent(ld_label) == false);
    assert(picoui_label_set_transparent(label, 1) == 0);
    assert(picoui_label_get_transparent(label, &transparent) == 0);
    assert(transparent == 1);
    assert(ldLabelGetTransparent(ld_label) == true);
    assert(picoui_label_set_transparent(label, 0) == 0);
    assert(picoui_label_get_transparent(label, &transparent) == 0);
    assert(transparent == 0);
    assert(ldLabelGetTransparent(ld_label) == false);
    assert(picoui_label_get_transparent(0, &transparent) == -1);
    assert(picoui_label_get_transparent(label, 0) == -1);

    assert(picoui_label_set_align(label, PICOUI_ALIGN_START) == 0);
    assert(picoui_label_get_align(label, &align) == 0);
    assert(align == PICOUI_ALIGN_START);
    assert(ldLabelGetAlign(ld_label) == ARM_2D_ALIGN_LEFT);
    assert(picoui_label_set_align(label, PICOUI_ALIGN_CENTER) == 0);
    assert(picoui_label_get_align(label, &align) == 0);
    assert(align == PICOUI_ALIGN_CENTER);
    assert(ldLabelGetAlign(ld_label) == ARM_2D_ALIGN_CENTRE);
    assert(picoui_label_set_align(label, PICOUI_ALIGN_END) == 0);
    assert(picoui_label_get_align(label, &align) == 0);
    assert(align == PICOUI_ALIGN_END);
    assert(ldLabelGetAlign(ld_label) == ARM_2D_ALIGN_RIGHT);
    assert(picoui_label_set_align(label, (enum picoui_align)99) == -1);
    assert(picoui_label_get_align(label, &align) == 0);
    assert(align == PICOUI_ALIGN_END);
    assert(ldLabelGetAlign(ld_label) == ARM_2D_ALIGN_RIGHT);
    assert(picoui_label_get_align(0, &align) == -1);
    assert(picoui_label_get_align(label, 0) == -1);

    assert_label_has_no_background_source(label);
    assert(picoui_label_set_background_source(label, 0) == 0);
    assert_label_has_no_background_source(label);
    assert(picoui_label_set_background_source(label, &invalid_source) == -1);
    assert_label_has_no_background_source(label);
    assert(picoui_label_set_transparent(label, 1) == 0);
    assert(picoui_label_get_transparent(label, &transparent) == 0);
    assert(transparent == 1);
    assert(ldLabelGetTransparent(ld_label) == true);
    assert(picoui_label_set_background_source(label, image_source) == 0);
    assert_label_has_background_source(label, image_source);
    assert(picoui_label_get_transparent(label, &transparent) == 0);
    assert(transparent == 0);
    assert(ldLabelGetTransparent(ld_label) == false);
    assert(picoui_label_set_background_source(label, &unmasked_source) == 0);
    assert_label_has_background_source(label, &unmasked_source);
    assert(picoui_label_get_transparent(label, &transparent) == 0);
    assert(transparent == 0);
    assert(ldLabelGetTransparent(ld_label) == false);
    assert(picoui_label_set_transparent(label, 1) == 0);
    assert(picoui_label_get_transparent(label, &transparent) == 0);
    assert(transparent == 1);
    assert(ldLabelGetTransparent(ld_label) == true);
    assert(picoui_label_set_bg_color(label, 0x334455U) == 0);
    assert(picoui_label_get_bg_color(label, &rgb) == 0);
    assert(rgb == quantize_rgb_to_ld_roundtrip(0x334455U));
    assert(ldLabelGetBackgroundColor(ld_label) == __RGB(0x33, 0x44, 0x55));
    assert(picoui_label_get_transparent(label, &transparent) == 0);
    assert(transparent == 0);
    assert(ldLabelGetTransparent(ld_label) == false);
    assert_label_has_no_background_source(label);
    assert(picoui_label_set_background_source(label, 0) == 0);
    assert_label_has_no_background_source(label);
}

static void test_label_props_j3_contract(struct picoui_window *parent,
                                         struct picoui_image_source *image_source,
                                         const struct picoui_font *font)
{
    struct picoui_label_props transparent_props = {
        .id = "label_j3_transparent_props",
        .transparent = 1,
        .align = PICOUI_ALIGN_START,
    };
    struct picoui_label_props background_props = {
        .id = "label_j3_background_props",
        .align = PICOUI_ALIGN_END,
        .background_source = image_source,
    };
    struct picoui_label *transparent_label = picoui_label_create_with_props(parent, &transparent_props);
    struct picoui_backend_widget *backend;
    ldLabel_t *ld_label;
    int transparent = -1;

    assert(transparent_label != 0);
    assert(background_props.align == PICOUI_ALIGN_END);
    assert(background_props.background_source == image_source);

    backend = transparent_label->widget.backend_widget;
    assert(backend != 0);
    ld_label = (ldLabel_t *)backend->ld_widget;
    assert(ld_label != 0);
    assert(picoui_label_get_transparent(transparent_label, &transparent) == 0);
    assert(transparent == 1);
    (void)font;
}

static void test_image_source_boundary(struct picoui_window *parent,
                                       struct picoui_image_source *image_source)
{
    struct picoui_backend_widget *parent_backend = parent->widget.backend_widget;
    int child_count = backend_child_count(parent_backend);
    struct picoui_image_props empty_props = {
        .id = "empty_image",
    };
    struct picoui_image *empty_image = picoui_image_create_with_props(parent, &empty_props);
    struct picoui_image *image = picoui_image_create(parent, "image_boundary");
    struct picoui_image_source invalid_source = {
        .img_tile = 0,
        .mask_tile = image_source->mask_tile,
    };
    struct picoui_image_source unmasked_source = {
        .img_tile = image_source->img_tile,
        .mask_tile = 0,
    };

    assert(empty_image != 0);
    assert_image_has_no_bound_source(empty_image);
    assert(picoui_image_set_source(empty_image, 0) == 0);
    assert_image_has_no_bound_source(empty_image);

    assert(image != 0);
    assert(picoui_image_set_source(image, &invalid_source) == -1);
    assert_image_has_no_bound_source(image);
    assert(picoui_image_set_source(image, &unmasked_source) == 0);
    assert_image_has_bound_source(image, &unmasked_source);
    assert(picoui_image_set_source(image, 0) == 0);
    assert_image_has_no_bound_source(image);
    assert(picoui_image_set_source(image, image_source) == 0);
    assert_image_has_bound_source(image, image_source);
    assert(backend_child_count(parent_backend) == child_count + 2);
}

static void test_image_theme_apply_is_rejected(struct picoui_theme *theme,
                                               struct picoui_image *image,
                                               struct picoui_image_source *image_source)
{
    const struct picoui_backend_widget *backend = image->widget.backend_widget;
    const ldImage_t *ld_image = (const ldImage_t *)backend->ld_widget;
    struct picoui_image_source *source = image_source;
    arm_2d_tile_t *img_tile = ld_image->ptImgTile;
    arm_2d_tile_t *mask_tile = ld_image->ptMaskTile;

    assert(picoui_image_set_source(image, image_source) == 0);
    img_tile = ld_image->ptImgTile;
    mask_tile = ld_image->ptMaskTile;
    assert(picoui_theme_apply_to_widget(theme, &image->widget, PICOUI_PART_MAIN, PICOUI_STATE_DEFAULT) == -1);
    assert(image->widget.bg_color == 0U);
    assert(image->widget.text_color == 0U);
    assert(image->widget.border_color == 0U);
    assert(image->source == source);
    assert(backend->image_source == source);
    assert(ld_image->ptImgTile == img_tile);
    assert(ld_image->ptMaskTile == mask_tile);
}

static void test_text_font_null_falls_back_to_default_contract(struct picoui_window *parent)
{
    struct picoui_text *text = picoui_text_create(parent, "text_font_null_fallback");
    struct picoui_backend_widget *backend;
    ldText_t *ld_text;
    arm_2d_font_t *initial_font;

    assert(text != 0);
    backend = text->widget.backend_widget;
    assert(backend != 0);
    ld_text = (ldText_t *)backend->ld_widget;
    assert(ld_text != 0);

    initial_font = test_text_consumed_font(ld_text);
    assert(initial_font != 0);
    assert(picoui_text_set_font(text, 0) == 0);
    assert(text->widget.font == 0);
    assert(backend->font == 0);
    assert(test_text_consumed_font(ld_text) != 0);
    assert(test_text_consumed_font(ld_text) == initial_font);
}

static void test_text_font_runtime_rebind_updates_real_ldtext(struct picoui_window *parent)
{
    struct picoui_font small_font = {"Sans", 8};
    struct picoui_font large_font = {"Sans", 24};
    struct picoui_text *text = picoui_text_create(parent, "text_font_rebind");
    struct picoui_backend_widget *backend;
    ldText_t *ld_text;
    arm_2d_font_t *small_real_font;
    arm_2d_font_t *large_real_font;

    assert(text != 0);
    backend = text->widget.backend_widget;
    assert(backend != 0);
    ld_text = (ldText_t *)backend->ld_widget;
    assert(ld_text != 0);

    assert(picoui_text_set_font(text, &small_font) == 0);
    small_real_font = test_text_consumed_font(ld_text);
    assert(small_real_font != 0);
    assert(small_real_font != (arm_2d_font_t *)&small_font);
    assert(text->widget.font == &small_font);
    assert(backend->font == &small_font);

    assert(picoui_text_set_font(text, &large_font) == 0);
    large_real_font = test_text_consumed_font(ld_text);
    assert(large_real_font != 0);
    assert(large_real_font != (arm_2d_font_t *)&large_font);
    assert(large_real_font != small_real_font);
    assert(ld_text->ptFont == large_real_font);
    assert(text->widget.font == &large_font);
    assert(backend->font == &large_font);
}

static void test_text_font_backend_failure_does_not_split_state(struct picoui_window *parent)
{
    struct picoui_font good_font = {"Sans", 24};
    struct picoui_font failed_font = {"Sans", 8};
    struct picoui_text *text = picoui_text_create(parent, "text_font_failure_atomicity");
    struct picoui_backend_widget *backend;
    ldText_t *ld_text;
    arm_2d_font_t *old_real_font;

    assert(text != 0);
    backend = text->widget.backend_widget;
    assert(backend != 0);
    ld_text = (ldText_t *)backend->ld_widget;
    assert(ld_text != 0);

    assert(picoui_text_set_font(text, &good_font) == 0);
    old_real_font = test_text_consumed_font(ld_text);
    assert(old_real_font != 0);

    picoui_backend_text_test_fail_next_set_font();
    assert(picoui_text_set_font(text, &failed_font) == -1);
    assert(text->widget.font == &good_font);
    assert(backend->font == &good_font);
    assert(test_text_consumed_font(ld_text) == old_real_font);
    assert(ld_text->ptFont == old_real_font);
}

static void test_image_style_class_and_user_data_are_metadata_only_contract(struct picoui_window *parent)
{
    struct picoui_image *image = picoui_image_create(parent, "image_metadata_only");
    struct picoui_backend_widget *backend;
    const char *style_class = "image-metadata-only";
    int cookie = 41;

    assert(image != 0);
    backend = image->widget.backend_widget;
    assert(backend != 0);

    assert(picoui_widget_set_style_class(&image->widget, style_class) == 0);
    assert(picoui_widget_set_user_data(&image->widget, &cookie) == 0);
    assert(image->widget.style_class != 0);
    assert(strcmp(image->widget.style_class, style_class) == 0);
    assert(image->widget.user_data == &cookie);
    assert(backend->style_class != 0);
    assert(strcmp(backend->style_class, style_class) == 0);
    assert(backend->user_data == &cookie);
}

static void test_image_theme_style_parts_remain_explicitly_rejected(struct picoui_theme *theme,
                                                                    struct picoui_window *parent)
{
    struct picoui_image *image = picoui_image_create(parent, "image_theme_text_reject");
    unsigned int bg_color_before;
    unsigned int text_color_before;
    unsigned int border_color_before;

    assert(image != 0);
    bg_color_before = image->widget.bg_color;
    text_color_before = image->widget.text_color;
    border_color_before = image->widget.border_color;
    assert(picoui_theme_apply_to_widget(theme,
                                        &image->widget,
                                        PICOUI_PART_TEXT,
                                        PICOUI_STATE_DEFAULT)
           == -1);
    assert(image->widget.bg_color == bg_color_before);
    assert(image->widget.text_color == text_color_before);
    assert(image->widget.border_color == border_color_before);
}

static void test_image_enabled_remains_rejected_contract(struct picoui_window *parent)
{
    struct picoui_image *image = picoui_image_create(parent, "image_enabled_reject");

    assert(image != 0);
    assert(image->widget.enabled == 1);
    assert(picoui_widget_set_enabled(&image->widget, 0) == -1);
    assert(image->widget.enabled == 1);
}

static void test_button_j4_contract(struct picoui_window *parent,
                                    struct picoui_backend_app_state *app_state,
                                    struct picoui_image_source *release_source,
                                    struct picoui_image_source *press_source,
                                    const struct picoui_font *font)
{
    struct picoui_button_props props = {
        .id = "button_j4_props",
        .text = "Button J4",
        .release_image = release_source,
        .press_image = press_source,
        .transparent = 1,
        .font = font,
        .checkable = 1,
        .key_value = 0x1234U,
        .pressed = 1,
    };
    struct picoui_button *button = picoui_button_create(parent, "button_j4");
    struct picoui_button *props_button = picoui_button_create_with_props(parent, &props);
    struct picoui_image_source invalid_source = {
        .img_tile = 0,
        .mask_tile = release_source->mask_tile,
    };
    struct picoui_backend_widget *backend;
    ldButton_t *ld_button;
    int transparent = -1;
    int checkable = -1;
    int pressed = -1;
    unsigned int key_value = 0;

    assert(button != 0);
    assert(props_button != 0);
    backend = button->widget.backend_widget;
    ld_button = (ldButton_t *)backend->ld_widget;
    assert(ld_button != 0);

    assert_button_has_no_bound_images(button);
    assert(picoui_button_set_release_image(button, 0) == 0);
    assert(picoui_button_set_press_image(button, 0) == 0);
    assert_button_has_no_bound_images(button);
    assert(picoui_button_set_release_image(button, &invalid_source) == -1);
    assert(picoui_button_set_press_image(button, &invalid_source) == -1);
    assert_button_has_no_bound_images(button);
    assert(picoui_button_set_release_image(button, release_source) == 0);
    assert_button_has_bound_images(button, release_source, 0);
    assert(picoui_button_set_press_image(button, press_source) == 0);
    assert_button_has_bound_images(button, release_source, press_source);

    assert(picoui_button_set_transparent(button, 1) == 0);
    assert(picoui_button_get_transparent(button, &transparent) == 0);
    assert(transparent == 1);
    assert(ldButtonGetTransparent(ld_button) == true);
    assert(picoui_button_set_transparent(button, 0) == 0);
    assert(picoui_button_get_transparent(button, &transparent) == 0);
    assert(transparent == 0);
    assert(ldButtonGetTransparent(ld_button) == false);
    assert(picoui_button_get_transparent(0, &transparent) == -1);
    assert(picoui_button_get_transparent(button, 0) == -1);

    assert(picoui_button_set_font(button, font) == 0);
    assert(ldButtonGetFont(ld_button) == (arm_2d_font_t *)&ARM_2D_FONT_6x8);

    assert(picoui_button_set_checkable(button, 1) == 0);
    assert(picoui_button_get_checkable(button, &checkable) == 0);
    assert(checkable == 1);
    assert(ldButtonGetCheckable(ld_button) == true);
    assert(picoui_button_set_checkable(button, 0) == 0);
    assert(picoui_button_get_checkable(button, &checkable) == 0);
    assert(checkable == 0);
    assert(ldButtonGetCheckable(ld_button) == false);
    assert(picoui_button_get_checkable(0, &checkable) == -1);
    assert(picoui_button_get_checkable(button, 0) == -1);

    assert(picoui_button_set_key_value(button, 0x55AAU) == 0);
    assert(picoui_button_get_key_value(button, &key_value) == 0);
    assert(key_value == 0x55AAU);
    assert(ldButtonGetKeyValue(ld_button) == 0x55AAU);
    assert(picoui_button_get_key_value(0, &key_value) == -1);
    assert(picoui_button_get_key_value(button, 0) == -1);

    assert(picoui_button_set_pressed(button, 1) == 0);
    assert(picoui_button_get_pressed(button, &pressed) == 0);
    assert(pressed == 1);
    assert(ldButtonGetPress(ld_button) == true);
    assert(picoui_button_set_pressed(button, 0) == 0);
    assert(picoui_button_get_pressed(button, &pressed) == 0);
    assert(pressed == 0);
    assert(ldButtonGetPress(ld_button) == false);
    assert(picoui_button_get_pressed(0, &pressed) == -1);
    assert(picoui_button_get_pressed(button, 0) == -1);

    assert(props_button->widget.text == (const char *)"Button J4");
    assert_button_has_bound_images(props_button, release_source, press_source);
    assert(picoui_button_get_transparent(props_button, &transparent) == 0);
    assert(transparent == 1);
    assert(picoui_button_get_checkable(props_button, &checkable) == 0);
    assert(checkable == 1);
    assert(picoui_button_get_key_value(props_button, &key_value) == 0);
    assert(key_value == 0x1234U);
    assert(picoui_button_get_pressed(props_button, &pressed) == 0);
    assert(pressed == 1);
    assert(ldButtonGetFont((ldButton_t *)((struct picoui_backend_widget *)props_button->widget.backend_widget)->ld_widget)
           == (arm_2d_font_t *)&ARM_2D_FONT_6x8);

    assert(picoui_button_set_checkable(button, 0) == 0);
    assert(picoui_button_set_pressed(button, 0) == 0);
    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue, backend->ld_widget, SIGNAL_PRESS, 0) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(picoui_button_get_pressed(button, &pressed) == 0);
    assert(pressed == 1);
    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue, backend->ld_widget, SIGNAL_RELEASE, 0) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(picoui_button_get_pressed(button, &pressed) == 0);
    assert(pressed == 0);

    assert(picoui_button_set_checkable(button, 1) == 0);
    assert(picoui_button_set_pressed(button, 0) == 0);
    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue, backend->ld_widget, SIGNAL_PRESS, 0) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(picoui_button_get_pressed(button, &pressed) == 0);
    assert(pressed == 1);
    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue, backend->ld_widget, SIGNAL_RELEASE, 0) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(picoui_button_get_pressed(button, &pressed) == 0);
    assert(pressed == 1);
    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue, backend->ld_widget, SIGNAL_PRESS, 0) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(picoui_button_get_pressed(button, &pressed) == 0);
    assert(pressed == 0);

    assert(picoui_button_set_release_image(0, release_source) == -1);
    assert(picoui_button_set_press_image(0, press_source) == -1);
    assert(picoui_button_set_font(0, font) == -1);
    assert(picoui_button_set_checkable(0, 1) == -1);
    assert(picoui_button_set_key_value(0, 1) == -1);
    assert(picoui_button_set_pressed(0, 1) == -1);
}

static void test_props_invalid_values_do_not_attach_backend_children(struct picoui_window *parent,
                                                                     struct picoui_image_source *image_source)
{
    struct picoui_backend_widget *parent_backend = parent->widget.backend_widget;
    int child_count = backend_child_count(parent_backend);
    struct picoui_label_props bad_label_padding = {
        .id = "bad_label_padding",
        .padding = -1,
    };
    struct picoui_slider_props bad_slider_range = {
        .id = "bad_slider_range",
        .min_value = 10,
        .max_value = 1,
        .value = 5,
    };
    struct picoui_slider_props bad_slider_value = {
        .id = "bad_slider_value",
        .min_value = 1,
        .max_value = 10,
        .value = 11,
    };
    struct picoui_button_props bad_button_size = {
        .id = "bad_button_size",
        .width = 20,
        .height = -1,
    };
    struct picoui_button_props bad_button_padding = {
        .id = "bad_button_padding",
        .padding = -1,
    };
    struct picoui_image_source bad_image_source = {
        .img_tile = 0,
        .mask_tile = image_source->mask_tile,
    };
    struct picoui_image_props bad_image_source_props = {
        .id = "bad_image_source",
        .source = &bad_image_source,
    };
    struct picoui_image_props bad_image_id = {
        .id = 0,
        .source = image_source,
    };
    struct picoui_slider_props bad_slider_indicator_width = {
        .id = "bad_slider_indicator_width",
        .has_indicator_width = 1,
        .indicator_width = -1,
    };
    struct picoui_slider_props bad_slider_indicator_width_oversize = {
        .id = "bad_slider_indicator_width_oversize",
        .has_indicator_width = 1,
        .indicator_width = 256,
    };
    struct picoui_slider_props bad_slider_slim_size = {
        .id = "bad_slider_slim_size",
        .has_slim_size = 1,
        .slim_size = -1,
    };
    struct picoui_slider_props bad_slider_slim_size_oversize = {
        .id = "bad_slider_slim_size_oversize",
        .has_slim_size = 1,
        .slim_size = 256,
    };
    struct picoui_slider_props bad_slider_background = {
        .id = "bad_slider_background",
        .has_background_source = 1,
        .background_source = &bad_image_source,
    };
    struct picoui_slider_props bad_slider_indicator = {
        .id = "bad_slider_indicator",
        .has_indicator_source = 1,
        .indicator_source = &bad_image_source,
    };

    assert(picoui_label_create_with_props(parent, &bad_label_padding) == 0);
    assert_backend_tree_unchanged(parent, child_count);
    assert(picoui_slider_create_with_props(parent, &bad_slider_range) == 0);
    assert_backend_tree_unchanged(parent, child_count);
    assert(picoui_slider_create_with_props(parent, &bad_slider_value) == 0);
    assert_backend_tree_unchanged(parent, child_count);
    assert(picoui_slider_create_with_props(parent, &bad_slider_indicator_width) == 0);
    assert_backend_tree_unchanged(parent, child_count);
    assert(picoui_slider_create_with_props(parent, &bad_slider_indicator_width_oversize) == 0);
    assert_backend_tree_unchanged(parent, child_count);
    assert(picoui_slider_create_with_props(parent, &bad_slider_slim_size) == 0);
    assert_backend_tree_unchanged(parent, child_count);
    assert(picoui_slider_create_with_props(parent, &bad_slider_slim_size_oversize) == 0);
    assert_backend_tree_unchanged(parent, child_count);
    assert(picoui_slider_create_with_props(parent, &bad_slider_background) == 0);
    assert_backend_tree_unchanged(parent, child_count);
    assert(picoui_slider_create_with_props(parent, &bad_slider_indicator) == 0);
    assert_backend_tree_unchanged(parent, child_count);
    assert(picoui_button_create_with_props(parent, &bad_button_size) == 0);
    assert_backend_tree_unchanged(parent, child_count);
    assert(picoui_button_create_with_props(parent, &bad_button_padding) == 0);
    assert_backend_tree_unchanged(parent, child_count);
    assert(picoui_image_create_with_props(parent, &bad_image_id) == 0);
    assert_backend_tree_unchanged(parent, child_count);
    assert(picoui_image_create_with_props(parent, &bad_image_source_props) == 0);
    assert_backend_tree_unchanged(parent, child_count);
}

static void test_props_initial_values(struct picoui_app *app,
                                      struct picoui_font *font,
                                      struct picoui_image_source *image_source,
                                      int *button_cookie,
                                      int *common_cookie)
{
    struct picoui_window_props win_props = {
        .id = "props_root",
        .style_class = "window-card",
        .user_data = common_cookie,
        .bg_color = 0x101112,
        .text_color = 0x131415,
        .border_color = 0x161718,
        .radius = 3,
        .padding = 4,
    };
    struct picoui_label_props label_props = {
        .id = "props_label",
        .text = "Props label",
        .font = font,
        .style_class = "label-title",
        .user_data = common_cookie,
        .width = 101,
        .height = 21,
        .bg_color = 0x212223,
        .text_color = 0x242526,
        .border_color = 0x272829,
        .radius = 5,
        .padding = 6,
    };
    struct picoui_text_props text_props = {
        .id = "props_text",
        .text = "Props body",
        .font = font,
        .style_class = "text-body",
        .user_data = common_cookie,
        .width = 102,
        .height = 22,
        .bg_color = 0x313233,
        .text_color = 0x343536,
        .border_color = 0x373839,
        .radius = 7,
        .padding = 8,
    };
    struct picoui_image_props image_props = {
        .id = "props_image",
        .source = image_source,
        .style_class = "image-frame",
        .user_data = common_cookie,
        .width = 103,
        .height = 23,
        .bg_color = 0x414243,
        .text_color = 0x444546,
        .border_color = 0x474849,
        .radius = 9,
        .padding = 10,
    };
    struct picoui_checkbox_props cb_props = {
        .id = "props_checkbox",
        .text = "Props checkbox",
        .checked = 1,
        .on_toggled = on_checkbox_toggle,
        .user_data = common_cookie,
        .style_class = "checkbox-row",
        .width = 104,
        .height = 24,
        .bg_color = 0x515253,
        .text_color = 0x545556,
        .border_color = 0x575859,
        .radius = 11,
        .padding = 12,
    };
    struct picoui_switch_props sw_props = {
        .id = "props_switch",
        .checked = 1,
        .on_toggled = on_switch_toggle,
        .user_data = common_cookie,
        .style_class = "switch-row",
        .width = 105,
        .height = 25,
        .bg_color = 0x616263,
        .text_color = 0x646566,
        .border_color = 0x676869,
        .radius = 13,
        .padding = 14,
    };
    struct picoui_slider_props slider_props = {
        "props_slider",
        10,
        60,
        45,
        on_slider,
        common_cookie,
        "slider-row",
        106,
        26,
        0x717273,
        0x747576,
        0x777879,
        15,
        16,
        0,
        image_source,
        image_source,
        12,
        5,
        1,
        1,
        1,
        1,
        1,
    };
    struct picoui_button_props button_props = {
        .id = "props_button",
        .text = "Props button",
        .font = font,
        .width = 107,
        .height = 27,
        .on_clicked = on_button_clicked,
        .user_data = button_cookie,
        .style_class = "button-primary",
        .bg_color = 0x818283,
        .text_color = 0x848586,
        .border_color = 0x878889,
        .radius = 17,
        .padding = 18,
        .release_image = image_source,
        .press_image = image_source,
        .transparent = 1,
        .checkable = 1,
        .key_value = 0x3344U,
        .pressed = 1,
    };
    struct picoui_window *props_win = picoui_window_create_with_props(app, &win_props);
    struct picoui_label *props_label = picoui_label_create_with_props(props_win, &label_props);
    struct picoui_text *props_text = picoui_text_create_with_props(props_win, &text_props);
    struct picoui_image *props_image = picoui_image_create_with_props(props_win, &image_props);
    struct picoui_checkbox *props_cb = picoui_checkbox_create_with_props(props_win, &cb_props);
    struct picoui_switch *props_sw = picoui_switch_create_with_props(props_win, &sw_props);
    struct picoui_slider *props_slider = picoui_slider_create_with_props(props_win, &slider_props);
    struct picoui_button *props_button = picoui_button_create_with_props(props_win, &button_props);
    unsigned int rgb = 0;
    int horizontal = -1;
    int percent = -1;

    assert(props_win && props_label && props_text && props_image);
    assert(props_cb && props_sw && props_slider && props_button);
    assert(props_win->id == (const char *)"props_root");
    assert(props_label->id == (const char *)"props_label");
    assert(props_text->id == (const char *)"props_text");
    assert(props_image->id == (const char *)"props_image");
    assert(props_cb->id == (const char *)"props_checkbox");
    assert(props_sw->id == (const char *)"props_switch");
    assert(props_slider->id == (const char *)"props_slider");
    assert(props_button->id == (const char *)"props_button");

    assert_widget_props(&props_win->widget,
                        props_win->widget.backend_widget,
                        "window-card",
                        common_cookie,
                        0x101112,
                        0x131415,
                        0x161718,
                        3,
                        4);
    assert_widget_props(&props_label->widget,
                        props_label->widget.backend_widget,
                        "label-title",
                        common_cookie,
                        0x212223,
                        0x242526,
                        0x272829,
                        5,
                        6);
    assert(picoui_label_get_text_color(props_label, &rgb) == 0);
    assert(rgb == quantize_rgb_to_ld_roundtrip(0x242526U));
    assert(picoui_label_get_bg_color(props_label, &rgb) == 0);
    assert(rgb == quantize_rgb_to_ld_roundtrip(0x212223U));
    assert_widget_props(&props_text->widget,
                        props_text->widget.backend_widget,
                        "text-body",
                        common_cookie,
                        0x313233,
                        0x343536,
                        0x373839,
                        7,
                        8);
    assert_widget_props(&props_image->widget,
                        props_image->widget.backend_widget,
                        "image-frame",
                        common_cookie,
                        0x414243,
                        0x444546,
                        0x474849,
                        9,
                        10);
    assert_widget_props(&props_cb->widget,
                        props_cb->widget.backend_widget,
                        "checkbox-row",
                        common_cookie,
                        0x515253,
                        0x545556,
                        0x575859,
                        11,
                        12);
    assert_widget_props(&props_sw->widget,
                        props_sw->widget.backend_widget,
                        "switch-row",
                        common_cookie,
                        0x616263,
                        0x646566,
                        0x676869,
                        13,
                        14);
    assert_widget_props(&props_slider->widget,
                        props_slider->widget.backend_widget,
                        "slider-row",
                        common_cookie,
                        0x717273,
                        0x747576,
                        0x777879,
                        15,
                        16);
    assert_widget_props(&props_button->widget,
                        props_button->widget.backend_widget,
                        "button-primary",
                        button_cookie,
                        0x818283,
                        0x848586,
                        0x878889,
                        17,
                        18);

    assert(props_label->widget.text == (const char *)"Props label");
    assert(props_label->widget.font == font);
    assert(((struct picoui_backend_widget *)props_label->widget.backend_widget)->font == font);
    assert(props_label->widget.width == 101);
    assert(props_label->widget.height == 21);

    assert(props_text->widget.text == (const char *)"Props body");
    assert(props_text->widget.font == font);
    assert(((struct picoui_backend_widget *)props_text->widget.backend_widget)->font == font);
    assert(props_text->widget.width == 102);
    assert(props_text->widget.height == 22);

    assert(props_image->source == image_source);
    assert(((struct picoui_backend_widget *)props_image->widget.backend_widget)->image_source == image_source);
    assert(props_image->widget.width == 103);
    assert(props_image->widget.height == 23);

    assert(props_cb->widget.text == (const char *)"Props checkbox");
    assert(picoui_checkbox_is_checked(props_cb) == 1);
    assert(props_cb->cb == on_checkbox_toggle);
    assert(props_cb->user_data == common_cookie);
    assert(props_cb->widget.width == 104);
    assert(props_cb->widget.height == 24);

    assert(picoui_switch_is_checked(props_sw) == 1);
    assert(props_sw->cb == on_switch_toggle);
    assert(props_sw->user_data == common_cookie);
    assert(props_sw->widget.width == 105);
    assert(props_sw->widget.height == 25);

    assert(props_slider->min_value == 10);
    assert(props_slider->max_value == 60);
    assert(picoui_slider_get_horizontal(props_slider, &horizontal) == 0);
    assert(horizontal == 0);
    assert(picoui_slider_get_percent(props_slider, &percent) == 0);
    assert(percent == 70);
    assert(props_slider->cb == on_slider);
    assert(props_slider->user_data == common_cookie);
    assert(props_slider->widget.width == 106);
    assert(props_slider->widget.height == 26);
    assert_slider_has_bound_images(props_slider, image_source, image_source);
    assert(((ldSlider_t *)((struct picoui_backend_widget *)props_slider->widget.backend_widget)->ld_widget)->isHorizontal == false);
    assert(((ldSlider_t *)((struct picoui_backend_widget *)props_slider->widget.backend_widget)->ld_widget)->indicWidth == 12);
    assert(((ldSlider_t *)((struct picoui_backend_widget *)props_slider->widget.backend_widget)->ld_widget)->slimSize == 5);
    assert(props_button->widget.text == (const char *)"Props button");
    assert(props_button->on_clicked == on_button_clicked);
    assert(props_button->user_data == button_cookie);
    assert(props_button->widget.font == font);
    assert(props_button->widget.width == 107);
    assert(props_button->widget.height == 27);
    assert_button_has_bound_images(props_button, image_source, image_source);
    assert(ldButtonGetTransparent((ldButton_t *)((struct picoui_backend_widget *)props_button->widget.backend_widget)->ld_widget) == true);
    assert(ldButtonGetCheckable((ldButton_t *)((struct picoui_backend_widget *)props_button->widget.backend_widget)->ld_widget) == true);
    assert(ldButtonGetKeyValue((ldButton_t *)((struct picoui_backend_widget *)props_button->widget.backend_widget)->ld_widget) == 0x3344U);
    assert(ldButtonGetPress((ldButton_t *)((struct picoui_backend_widget *)props_button->widget.backend_widget)->ld_widget) == true);

    test_props_invalid_values_do_not_attach_backend_children(props_win, image_source);

    assert(picoui_window_create_with_props(app, 0) == 0);
    assert(picoui_label_create_with_props(props_win, 0) == 0);
    assert(picoui_text_create_with_props(props_win, 0) == 0);
    assert(picoui_image_create_with_props(props_win, 0) == 0);
    assert(picoui_button_create_with_props(props_win, 0) == 0);
    assert(picoui_checkbox_create_with_props(props_win, 0) == 0);
    assert(picoui_switch_create_with_props(props_win, 0) == 0);
    assert(picoui_slider_create_with_props(props_win, 0) == 0);
}

int main(void)
{
    arm_2d_tile_t image_tile = {0};
    arm_2d_tile_t image_mask_tile = {0};
    struct picoui_app *app;
    struct picoui_theme *theme;
    struct picoui_window *win;
    struct picoui_switch_props sw_props = {
        .id = "wifi",
        .checked = 1,
        .on_toggled = on_switch_toggle,
        .user_data = 0,
    };
    struct picoui_checkbox_props cb_props = {
        .id = "agree",
        .text = "I agree",
        .checked = 0,
        .on_toggled = on_checkbox_toggle,
        .user_data = 0,
    };
    struct picoui_switch *sw;
    struct picoui_checkbox *cb;
    struct picoui_slider *slider;
    struct picoui_button *button;
    struct picoui_label *label;
    struct picoui_text *text;
    struct picoui_image *image;
    struct picoui_image_source image_source = {
        .img_tile = &image_tile,
        .mask_tile = &image_mask_tile,
    };
    struct picoui_slider_props slider_props = {
        "volume",
        10,
        50,
        42,
        on_slider,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        &image_source,
        &image_source,
        123,
        17,
        0,
        0,
        0,
        0,
        0,
    };
    struct picoui_font font = {"Sans", 14};
    arm_2d_tile_t button_release_tile = {0};
    arm_2d_tile_t button_release_mask_tile = {0};
    arm_2d_tile_t button_press_tile = {0};
    arm_2d_tile_t button_press_mask_tile = {0};
    struct picoui_image_source button_release_source = {
        .img_tile = &button_release_tile,
        .mask_tile = &button_release_mask_tile,
    };
    struct picoui_image_source button_press_source = {
        .img_tile = &button_press_tile,
        .mask_tile = &button_press_mask_tile,
    };
    struct picoui_backend_widget *sw_backend;
    struct picoui_backend_widget *cb_backend;
    struct picoui_backend_widget *slider_backend;
    struct picoui_backend_widget *button_backend;
    struct picoui_backend_widget *label_backend;
    struct picoui_backend_widget *text_backend;
    struct picoui_backend_widget *image_backend;
    struct picoui_backend_app_state *app_state;
    int button_cookie = 7;
    int common_cookie = 9;

    app = picoui_app_create();
    theme = picoui_theme_create();
    win = picoui_window_create(app, "root");

    sw = picoui_switch_create_with_props(win, &sw_props);
    cb = picoui_checkbox_create_with_props(win, &cb_props);
    slider = picoui_slider_create_with_props(win, &slider_props);
    button = picoui_button_create(win, "ok");
    label = picoui_label_create(win, "title");
    text = picoui_text_create(win, "body");
    image = picoui_image_create(win, "logo");

    assert(theme && sw && cb && slider && button && label && text && image);
    sw_backend = sw->widget.backend_widget;
    cb_backend = cb->widget.backend_widget;
    slider_backend = slider->widget.backend_widget;
    button_backend = button->widget.backend_widget;
    label_backend = label->widget.backend_widget;
    text_backend = text->widget.backend_widget;
    image_backend = image->widget.backend_widget;
    app_state = (struct picoui_backend_app_state *)app->backend_app;
    assert(app_state != 0);
    assert(app_state->ld_scene != 0);
    assert(ldMsgInit(&app_state->ld_scene->ptMsgQueue, 8) == true);
    test_backend_widget_tree_contract(app, win, sw, cb, slider, label, button, text, image);
    assert(switch_toggled_count == 0);
    assert(checkbox_toggled_count == 0);
    assert(slider_value_count == 0);
    assert(picoui_switch_is_checked(sw) == 1);
    assert(picoui_checkbox_is_checked(cb) == 0);
    assert(cb->widget.text == (const char *)"I agree");
    assert(picoui_slider_get_percent(slider, &button_cookie) == 0);
    assert(button_cookie == 80);
    assert(sw_backend->value == 1);
    assert(sw_backend->last_signal == PICOUI_BACKEND_SIGNAL_NONE);
    assert(sw_backend->dispatch_count == 0);
    assert(((ldSwitch_t *)sw_backend->ld_widget)->isChecked == true);
    assert(((ldSwitch_t *)sw_backend->ld_widget)->animProgress == 1000);
    assert(cb_backend->value == 0);
    assert(cb_backend->text == (const char *)"I agree");
    assert(cb_backend->last_signal == PICOUI_BACKEND_SIGNAL_NONE);
    assert(cb_backend->dispatch_count == 0);
    assert(((ldCheckBox_t *)cb_backend->ld_widget)->isChecked == false);
    assert(slider_backend->value == 42);
    assert(slider_backend->last_signal == PICOUI_BACKEND_SIGNAL_NONE);
    assert(slider_backend->dispatch_count == 0);
    assert(((ldSlider_t *)slider_backend->ld_widget)->permille == 800);
    assert(((ldSlider_t *)slider_backend->ld_widget)->isHorizontal == true);
    assert(((ldSlider_t *)slider_backend->ld_widget)->ptBgImgTile == 0);
    assert(((ldSlider_t *)slider_backend->ld_widget)->ptBgMaskTile == 0);
    assert(((ldSlider_t *)slider_backend->ld_widget)->ptIndicImgTile == 0);
    assert(((ldSlider_t *)slider_backend->ld_widget)->ptIndicMaskTile == 0);
    assert(((ldSlider_t *)slider_backend->ld_widget)->indicWidth == 10);
    assert(((ldSlider_t *)slider_backend->ld_widget)->slimSize == 4);

    test_props_initial_values(app, &font, &image_source, &button_cookie, &common_cookie);
    test_image_source_boundary(win, &image_source);
    test_image_theme_apply_is_rejected(theme, image, &image_source);
    test_image_style_class_and_user_data_are_metadata_only_contract(win);
    test_image_theme_style_parts_remain_explicitly_rejected(theme, win);
    test_image_enabled_remains_rejected_contract(win);
    test_text_font_null_falls_back_to_default_contract(win);
    test_text_font_runtime_rebind_updates_real_ldtext(win);
    test_text_font_backend_failure_does_not_split_state(win);
    test_button_j4_contract(win,
                            app_state,
                            &button_release_source,
                            &button_press_source,
                            &font);

    assert(picoui_switch_set_checked(sw, 1) == 0);
    assert(switch_toggled_count == 0);
    assert(sw_backend->value == 1);
    assert(sw_backend->dispatch_count == 0);
    assert(picoui_switch_set_checked(sw, 0) == 0);
    assert(switch_toggled_count == 0);
    assert(sw_backend->value == 0);
    assert(((ldSwitch_t *)sw_backend->ld_widget)->isChecked == false);
    assert(((ldSwitch_t *)sw_backend->ld_widget)->animProgress == 0);
    assert(sw_backend->last_signal == PICOUI_BACKEND_SIGNAL_NONE);
    assert(sw_backend->dispatch_count == 0);
    assert(picoui_switch_set_checked(sw, 0) == 0);
    assert(switch_toggled_count == 0);
    assert(sw_backend->value == 0);
    assert(sw_backend->dispatch_count == 0);

    assert(picoui_checkbox_set_checked(cb, 0) == 0);
    assert(checkbox_toggled_count == 0);
    assert(cb_backend->value == 0);
    assert(cb_backend->dispatch_count == 0);
    assert(picoui_checkbox_set_checked(cb, 1) == 0);
    assert(checkbox_toggled_count == 0);
    assert(cb_backend->value == 1);
    assert(((ldCheckBox_t *)cb_backend->ld_widget)->isChecked == true);
    assert(cb_backend->last_signal == PICOUI_BACKEND_SIGNAL_NONE);
    assert(cb_backend->dispatch_count == 0);
    assert(picoui_checkbox_set_checked(cb, 1) == 0);
    assert(checkbox_toggled_count == 0);
    assert(cb_backend->value == 1);
    assert(cb_backend->dispatch_count == 0);
    assert(picoui_checkbox_set_text(cb, "accept terms") == 0);
    assert(cb->widget.text == (const char *)"accept terms");
    assert(cb_backend->text == (const char *)"accept terms");

    assert(picoui_slider_set_value(slider, 42) == 0);
    assert(slider_value_count == 0);
    assert(slider_backend->value == 42);
    assert(slider_backend->dispatch_count == 0);
    assert(picoui_slider_set_value(slider, 11) == 0);
    assert(slider_value_count == 0);
    assert(slider_backend->value == 11);
    assert(((ldSlider_t *)slider_backend->ld_widget)->permille == 20);
    assert(slider_backend->last_signal == PICOUI_BACKEND_SIGNAL_NONE);
    assert(slider_backend->dispatch_count == 0);
    assert(picoui_slider_set_value(slider, 11) == 0);
    assert(slider_value_count == 0);
    assert(slider_backend->value == 11);
    assert(slider_backend->dispatch_count == 0);
    test_backend_value_changed_bridge_keeps_setter_sync_only(slider, slider_backend, app_state->ld_scene);
    test_native_event_bridge_prefers_native_path(sw, cb, slider, app_state->ld_scene);

    assert(picoui_switch_set_on_toggled(sw, on_switch_toggle, 0) == 0);
    assert(picoui_checkbox_set_on_toggled(cb, on_checkbox_toggle, 0) == 0);
    assert(picoui_slider_set_on_value_changed(slider, on_slider, 0) == 0);
    assert(picoui_button_set_on_clicked(button, on_button_clicked, &button_cookie) == 0);
    assert(button->on_clicked == on_button_clicked);
    assert(button->user_data == &button_cookie);
    button->on_clicked(&button->widget, button->user_data);
    assert(button_clicked == button_cookie);

    assert(picoui_button_set_text(button, "launch") == 0);
    assert(picoui_widget_set_style_class(&button->widget, "primary") == 0);
    assert(picoui_widget_set_user_data(&button->widget, &button_cookie) == 0);
    assert(picoui_widget_set_bg_color(&button->widget, 0x112233) == 0);
    assert(picoui_widget_set_text_color(&button->widget, 0x445566) == 0);
    assert(picoui_widget_set_border_color(&button->widget, 0x778899) == 0);
    assert(picoui_widget_set_radius(&button->widget, 8) == 0);
    assert(picoui_widget_set_padding(&button->widget, 12) == 0);
    assert(button->widget.text == (const char *)"launch");
    assert(button->widget.style_class == (const char *)"primary");
    assert(button->widget.user_data == &button_cookie);
    assert(button_backend->style_class == (const char *)"primary");
    assert(button_backend->user_data == &button_cookie);
    assert(button->widget.bg_color == 0x112233);
    assert(button->widget.text_color == 0x445566);
    assert(button->widget.border_color == 0x778899);
    assert(button->widget.radius == 8);
    assert(button->widget.padding == 12);
    assert(picoui_widget_set_radius(&button->widget, -1) == -1);
    assert(picoui_widget_set_padding(&button->widget, -1) == -1);
    assert(picoui_widget_set_text(&button->widget, 0) == -1);
    assert(picoui_checkbox_set_text(0, "x") == -1);
    assert(picoui_checkbox_set_text(cb, 0) == -1);
    assert(picoui_checkbox_create_with_props(win, 0) == 0);

    test_label_parity_contract(label, &image_source, &font);
    test_label_props_j3_contract(win, &image_source, &font);
    assert(label_backend->font == &font);

    assert(picoui_text_set_text(text, "world") == 0);
    assert(picoui_text_set_font(text, &font) == 0);
    assert(text->widget.text == (const char *)"world");
    assert(text->widget.font == &font);
    assert(text_backend->font == &font);
    assert(picoui_image_set_source(image, &image_source) == 0);
    assert_image_has_bound_source(image, &image_source);
    test_slider_j5_contract(slider, &button_release_source, &button_press_source);
    test_focus_owner_switches_between_widgets(app, button, sw, cb, slider, app_state->ld_scene);
    test_hidden_or_disabled_widget_cannot_keep_focus(app, button);
    test_model_readback_policy_is_explicit_for_data_widgets(sw, cb, slider);
    test_item_model_identity_survives_frame_update(sw, cb, slider, app_state->ld_scene);
    test_native_duplicate_value_does_not_advance_data_model(sw, cb, slider, app_state->ld_scene);

    ldMsgDeinit(&app_state->ld_scene->ptMsgQueue);
    picoui_theme_destroy(theme);
    picoui_app_destroy(app);
    return 0;
}
