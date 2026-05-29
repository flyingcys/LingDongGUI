#include "picoui/picoui.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldCheckBox.h"
#include "../../../src/gui/ldImage.h"
#include "../../../src/gui/ldSlider.h"
#include "../../../src/gui/ldSwitch.h"
#include "../../../src/misc/ldMsg.h"
#include "internal.h"

#include <assert.h>

static int switch_toggled_count = 0;
static int switch_toggled_value = -1;
static int checkbox_toggled_count = 0;
static int checkbox_toggled_value = -1;
static int slider_value_count = 0;
static int slider_value = -1;
static int button_clicked = -1;

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

static void test_backend_value_changed_bridge_keeps_setter_sync_only(struct picoui_slider *slider,
                                                                     struct picoui_backend_widget *slider_backend)
{
    ld_scene_t scene = {0};
    ldBase_t sender = {0};
    ldMsg_t msg = {0};

    assert(ldMsgInit(&scene.ptMsgQueue, 4) == true);
    assert(picoui_backend_widget_bind_ld_event_bridge(slider_backend, &scene, &sender) == 0);
    assert(picoui_slider_set_value(slider, 12) == 0);
    assert(slider_value_count == 0);
    assert(slider_value == -1);
    assert(xQueueDequeue(scene.ptMsgQueue, &msg, sizeof(msg)) == false);
    ldMsgDeinit(&scene.ptMsgQueue);
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
    assert(picoui_slider_get_value(slider) == 35);
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

    assert(picoui_label_create_with_props(parent, &bad_label_padding) == 0);
    assert_backend_tree_unchanged(parent, child_count);
    assert(picoui_slider_create_with_props(parent, &bad_slider_range) == 0);
    assert_backend_tree_unchanged(parent, child_count);
    assert(picoui_slider_create_with_props(parent, &bad_slider_value) == 0);
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
        .id = "props_slider",
        .min_value = 10,
        .max_value = 60,
        .value = 45,
        .on_value_changed = on_slider,
        .user_data = common_cookie,
        .style_class = "slider-row",
        .width = 106,
        .height = 26,
        .bg_color = 0x717273,
        .text_color = 0x747576,
        .border_color = 0x777879,
        .radius = 15,
        .padding = 16,
    };
    struct picoui_button_props button_props = {
        .id = "props_button",
        .text = "Props button",
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
    };
    struct picoui_window *props_win = picoui_window_create_with_props(app, &win_props);
    struct picoui_label *props_label = picoui_label_create_with_props(props_win, &label_props);
    struct picoui_text *props_text = picoui_text_create_with_props(props_win, &text_props);
    struct picoui_image *props_image = picoui_image_create_with_props(props_win, &image_props);
    struct picoui_checkbox *props_cb = picoui_checkbox_create_with_props(props_win, &cb_props);
    struct picoui_switch *props_sw = picoui_switch_create_with_props(props_win, &sw_props);
    struct picoui_slider *props_slider = picoui_slider_create_with_props(props_win, &slider_props);
    struct picoui_button *props_button = picoui_button_create_with_props(props_win, &button_props);

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
    assert(picoui_slider_get_value(props_slider) == 45);
    assert(props_slider->cb == on_slider);
    assert(props_slider->user_data == common_cookie);
    assert(props_slider->widget.width == 106);
    assert(props_slider->widget.height == 26);

    assert(props_button->widget.text == (const char *)"Props button");
    assert(props_button->on_clicked == on_button_clicked);
    assert(props_button->user_data == button_cookie);
    assert(props_button->widget.width == 107);
    assert(props_button->widget.height == 27);

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
    struct picoui_app *app = picoui_app_create();
    struct picoui_theme *theme = picoui_theme_create();
    struct picoui_window *win = picoui_window_create(app, "root");
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
    struct picoui_slider_props slider_props = {
        .id = "volume",
        .min_value = 10,
        .max_value = 50,
        .value = 42,
        .on_value_changed = on_slider,
        .user_data = 0,
    };
    struct picoui_switch *sw = picoui_switch_create_with_props(win, &sw_props);
    struct picoui_checkbox *cb = picoui_checkbox_create_with_props(win, &cb_props);
    struct picoui_slider *slider = picoui_slider_create_with_props(win, &slider_props);
    struct picoui_button *button = picoui_button_create(win, "ok");
    struct picoui_label *label = picoui_label_create(win, "title");
    struct picoui_text *text = picoui_text_create(win, "body");
    struct picoui_image *image = picoui_image_create(win, "logo");
    struct picoui_image_source image_source = {
        .img_tile = &image_tile,
        .mask_tile = &image_mask_tile,
    };
    struct picoui_font font = {"Sans", 14};
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
    assert(picoui_slider_get_value(slider) == 42);
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

    test_props_initial_values(app, &font, &image_source, &button_cookie, &common_cookie);
    test_image_source_boundary(win, &image_source);
    test_image_theme_apply_is_rejected(theme, image, &image_source);

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
    test_backend_value_changed_bridge_keeps_setter_sync_only(slider, slider_backend);
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

    assert(picoui_label_set_text(label, "hello") == 0);
    assert(picoui_label_set_font(label, &font) == 0);
    assert(label->widget.text == (const char *)"hello");
    assert(label->widget.font == &font);
    assert(label_backend->font == &font);

    assert(picoui_text_set_text(text, "world") == 0);
    assert(picoui_text_set_font(text, &font) == 0);
    assert(text->widget.text == (const char *)"world");
    assert(text->widget.font == &font);
    assert(text_backend->font == &font);

    assert(picoui_image_set_source(image, &image_source) == 0);
    assert_image_has_bound_source(image, &image_source);

    ldMsgDeinit(&app_state->ld_scene->ptMsgQueue);
    picoui_theme_destroy(theme);
    picoui_app_destroy(app);
    return 0;
}
