#include "picoui/picoui.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldCheckBox.h"
#include "internal.h"
#include <assert.h>
#include <string.h>

static unsigned int test_rgb_to_ld_color(unsigned int rgb)
{
    return (unsigned int)__RGB((rgb >> 16) & 0xFFU, (rgb >> 8) & 0xFFU, rgb & 0xFFU);
}

static void assert_checkbox_has_bound_images(const struct picoui_checkbox *checkbox,
                                             const struct picoui_image_source *expected_unchecked,
                                             const struct picoui_image_source *expected_checked)
{
    const struct picoui_backend_widget *backend;
    const ldCheckBox_t *ld_checkbox;

    assert(checkbox != 0);
    backend = (const struct picoui_backend_widget *)checkbox->widget.backend_widget;
    assert(backend != 0);
    ld_checkbox = (const ldCheckBox_t *)backend->ld_widget;
    assert(ld_checkbox != 0);
    assert(ld_checkbox->ptUncheckedImgTile
           == (expected_unchecked != 0 ? expected_unchecked->img_tile : 0));
    assert(ld_checkbox->ptUncheckedMaskTile
           == (expected_unchecked != 0 ? expected_unchecked->mask_tile : 0));
    assert(ld_checkbox->ptCheckedImgTile
           == (expected_checked != 0 ? expected_checked->img_tile : 0));
    assert(ld_checkbox->ptCheckedMaskTile
           == (expected_checked != 0 ? expected_checked->mask_tile : 0));
}

static void test_checkbox_create_and_ld_mapping(struct picoui_window *win)
{
    struct picoui_checkbox *checkbox = picoui_checkbox_create(win, "cb_test");
    struct picoui_backend_widget *backend;
    ldBase_t *ld_base;

    assert(checkbox != 0);
    backend = (struct picoui_backend_widget *)checkbox->widget.backend_widget;
    assert(backend != 0);
    assert(backend->kind == PICOUI_BACKEND_WIDGET_CHECKBOX);
    ld_base = (ldBase_t *)backend->ld_widget;
    assert(ld_base != 0);
    assert(ld_base->widgetType == widgetTypeCheckBox);
}

static void test_checkbox_create_with_props_pushes_all_fields(struct picoui_window *win)
{
    struct picoui_checkbox *checkbox = picoui_checkbox_create_with_props(
        win,
        &(struct picoui_checkbox_props){
            .id = "cb_props",
            .text = "Agree",
            .checked = 1,
        });
    struct picoui_backend_widget *backend;
    ldCheckBox_t *ld_checkbox;

    assert(checkbox != 0);
    backend = (struct picoui_backend_widget *)checkbox->widget.backend_widget;
    assert(backend != 0);
    assert(backend->text != 0);
    assert(strcmp(backend->text, "Agree") == 0);
    ld_checkbox = (ldCheckBox_t *)backend->ld_widget;
    assert(ld_checkbox != 0);
    assert(ld_checkbox->isChecked == true);
}

static void test_checkbox_set_checked_round_trip(struct picoui_window *win)
{
    struct picoui_checkbox *checkbox = picoui_checkbox_create(win, "cb_checked");
    struct picoui_backend_widget *backend;
    ldCheckBox_t *ld_checkbox;

    assert(checkbox != 0);
    backend = (struct picoui_backend_widget *)checkbox->widget.backend_widget;
    assert(backend != 0);
    ld_checkbox = (ldCheckBox_t *)backend->ld_widget;
    assert(ld_checkbox != 0);

    assert(picoui_checkbox_set_checked(checkbox, 1) == 0);
    assert(ld_checkbox->isChecked == true);
    assert(checkbox->checked == 1);
    assert(picoui_checkbox_is_checked(checkbox) == 1);

    assert(picoui_checkbox_set_checked(checkbox, 0) == 0);
    assert(ld_checkbox->isChecked == false);
    assert(checkbox->checked == 0);
    assert(picoui_checkbox_is_checked(checkbox) == 0);
}

static void test_checkbox_set_text_round_trip(struct picoui_window *win)
{
    struct picoui_checkbox *checkbox = picoui_checkbox_create(win, "cb_text");
    struct picoui_backend_widget *backend;

    assert(checkbox != 0);
    assert(picoui_checkbox_set_text(checkbox, "Label") == 0);
    backend = (struct picoui_backend_widget *)checkbox->widget.backend_widget;
    assert(backend->text != 0);
    assert(strcmp(backend->text, "Label") == 0);
}

static void test_checkbox_native_helper_behaviors(struct picoui_window *win)
{
    struct picoui_checkbox *checkbox = picoui_checkbox_create(win, "cb_native");
    struct picoui_backend_widget *backend;
    ldCheckBox_t *ld_checkbox;
    arm_2d_tile_t unchecked_tile = {0};
    arm_2d_tile_t unchecked_mask_tile = {0};
    arm_2d_tile_t checked_tile = {0};
    arm_2d_tile_t checked_mask_tile = {0};
    struct picoui_image_source unchecked_source = {
        .img_tile = &unchecked_tile,
        .mask_tile = &unchecked_mask_tile,
    };
    struct picoui_image_source checked_source = {
        .img_tile = &checked_tile,
        .mask_tile = &checked_mask_tile,
    };

    assert(checkbox != 0);
    backend = (struct picoui_backend_widget *)checkbox->widget.backend_widget;
    assert(backend != 0);
    ld_checkbox = (ldCheckBox_t *)backend->ld_widget;
    assert(ld_checkbox != 0);

    assert(picoui_checkbox_set_check_color(checkbox, 0xAA5500U) == 0);
    assert(ld_checkbox->fgColor == (ldColor)test_rgb_to_ld_color(0xAA5500U));
    assert(ld_checkbox->ptUncheckedImgTile == 0);
    assert(ld_checkbox->ptCheckedImgTile == 0);

    assert(picoui_checkbox_set_text_color(checkbox, 0x224466U) == 0);
    assert(checkbox->widget.text_color == 0x224466U);
    assert(ld_checkbox->textColor == (ldColor)test_rgb_to_ld_color(0x224466U));

    assert(picoui_checkbox_set_unchecked_source(checkbox, &unchecked_source) == 0);
    assert_checkbox_has_bound_images(checkbox, &unchecked_source, 0);
    assert(picoui_checkbox_set_checked_source(checkbox, &checked_source) == 0);
    assert_checkbox_has_bound_images(checkbox, &unchecked_source, &checked_source);

    assert(picoui_checkbox_set_radio_group(checkbox, 7) == 0);
    assert(ld_checkbox->isRadioButton == true);
    assert(ld_checkbox->radioButtonGroup == 7);

    assert(picoui_checkbox_set_string_left_space(checkbox, 22) == 0);
    assert(ld_checkbox->boxWidth == 22);

    assert(picoui_checkbox_set_check_color(checkbox, 0x003366U) == 0);
    assert(ld_checkbox->fgColor == (ldColor)test_rgb_to_ld_color(0x003366U));
    assert_checkbox_has_bound_images(checkbox, 0, 0);
    assert(ld_checkbox->boxWidth == 14);
}

static void test_checkbox_rejects_null_args(struct picoui_window *win)
{
    struct picoui_checkbox *checkbox = picoui_checkbox_create(win, "cb_invalid");
    struct picoui_backend_widget *backend;
    ldCheckBox_t *ld_checkbox;
    arm_2d_tile_t unchecked_mask_tile = {0};
    arm_2d_tile_t checked_mask_tile = {0};
    struct picoui_image_source invalid_unchecked_source = {
        .img_tile = 0,
        .mask_tile = &unchecked_mask_tile,
    };
    struct picoui_image_source invalid_checked_source = {
        .img_tile = 0,
        .mask_tile = &checked_mask_tile,
    };

    assert(checkbox != 0);
    backend = (struct picoui_backend_widget *)checkbox->widget.backend_widget;
    assert(backend != 0);
    ld_checkbox = (ldCheckBox_t *)backend->ld_widget;
    assert(ld_checkbox != 0);

    assert(picoui_checkbox_create(0, "id") == 0);
    assert(picoui_checkbox_create(win, 0) == 0);
    assert(picoui_checkbox_set_text(0, "text") == -1);
    assert(picoui_checkbox_set_checked(0, 1) == -1);
    assert(picoui_checkbox_set_unchecked_source(0, &invalid_unchecked_source) == -1);
    assert(picoui_checkbox_set_checked_source(0, &invalid_checked_source) == -1);
    assert(picoui_checkbox_set_unchecked_source(checkbox, &invalid_unchecked_source) == -1);
    assert(picoui_checkbox_set_checked_source(checkbox, &invalid_checked_source) == -1);
    assert(picoui_checkbox_set_radio_group(0, 1) == -1);
    assert(picoui_checkbox_set_radio_group(checkbox, -1) == -1);
    assert(picoui_checkbox_set_radio_group(checkbox, 256) == -1);
    assert(picoui_checkbox_set_string_left_space(0, 10) == -1);
    assert(picoui_checkbox_set_string_left_space(checkbox, -1) == -1);
    assert(picoui_checkbox_set_check_color(0, 0x123456U) == -1);
    assert(picoui_checkbox_set_text_color(0, 0x123456U) == -1);
    assert(ld_checkbox->radioButtonGroup == 0);
    assert(ld_checkbox->boxWidth == 14);
}

int main(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_window *win;
    assert(app != 0);
    win = picoui_window_create(app, "root");
    assert(win != 0);

    test_checkbox_create_and_ld_mapping(win);
    test_checkbox_create_with_props_pushes_all_fields(win);
    test_checkbox_set_checked_round_trip(win);
    test_checkbox_set_text_round_trip(win);
    test_checkbox_native_helper_behaviors(win);
    test_checkbox_rejects_null_args(win);

    picoui_app_destroy(app);
    return 0;
}
