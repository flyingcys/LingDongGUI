#include "picoui/picoui.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldSlider.h"
#include "internal.h"

#include <assert.h>

extern int picoui_widget_has_ld_binding(const struct picoui_widget *widget);
void picoui_backend_slider_test_fail_indicator_width_for_id(const char *id);
const struct picoui_backend_widget *picoui_backend_slider_test_last_disposed_backend(void);

static void test_slider_create_and_backend_mapping(struct picoui_window *win)
{
    struct picoui_slider *slider = picoui_slider_create(win, "sl_test");
    struct picoui_backend_widget *backend;
    struct picoui_backend_widget *parent_backend;
    ldSlider_t *ld_slider;

    assert(slider != 0);
    backend = (struct picoui_backend_widget *)slider->widget.backend_widget;
    assert(backend != 0);
    parent_backend = (struct picoui_backend_widget *)win->widget.backend_widget;
    assert(parent_backend != 0);
    assert(backend->kind == PICOUI_BACKEND_WIDGET_SLIDER);
    assert(backend->owner == parent_backend->owner);
    assert(backend->root == parent_backend->root);
    assert(backend->parent == parent_backend);
    assert(backend->ld_name_id != 0);
    assert(backend->host_widget == &slider->widget);
    assert(backend->ld_event_bridge_scene != 0);
    assert(backend->ld_event_bridge_sender == backend->ld_widget);
    ld_slider = (ldSlider_t *)backend->ld_widget;
    assert(ld_slider != 0);
    assert(((ldBase_t *)ld_slider)->pInfo == backend);
    assert(slider->min_value == 0);
    assert(slider->max_value == 100);
    assert(slider->value == 0);
    assert(picoui_widget_has_ld_binding(&slider->widget) == 1);
}

static void test_slider_create_with_props_pushes_range(struct picoui_window *win)
{
    struct picoui_slider *slider = picoui_slider_create_with_props(
        win,
        &(struct picoui_slider_props){
            .id = "sl_props",
            .min_value = 10,
            .max_value = 100,
            .value = 50,
        });
    struct picoui_backend_widget *backend;
    ldSlider_t *ld_slider;

    assert(slider != 0);
    assert(slider->value == 50);
    assert(slider->min_value == 10);
    assert(slider->max_value == 100);

    backend = (struct picoui_backend_widget *)slider->widget.backend_widget;
    assert(backend != 0);
    assert(backend->kind == PICOUI_BACKEND_WIDGET_SLIDER);

    ld_slider = (ldSlider_t *)backend->ld_widget;
    assert(ld_slider != 0);
    /* value=50 in range [10,100] => percent=(50-10)*100/(100-10)=44 => permille=440 */
    assert(ld_slider->permille == 440U);
}

static void test_slider_set_range_and_value_round_trip(struct picoui_window *win)
{
    struct picoui_slider *slider = picoui_slider_create(win, "sl_rnd");
    int percent;
    int horizontal;
    arm_2d_tile_t bg_tile = {0};
    arm_2d_tile_t bg_mask = {0};
    arm_2d_tile_t indic_tile = {0};
    arm_2d_tile_t indic_mask = {0};
    struct picoui_image_source background_source = {
        .img_tile = &bg_tile,
        .mask_tile = &bg_mask,
    };
    struct picoui_image_source indicator_source = {
        .img_tile = &indic_tile,
        .mask_tile = &indic_mask,
    };
    struct picoui_backend_widget *backend;
    ldSlider_t *ld_slider;

    assert(slider != 0);
    backend = (struct picoui_backend_widget *)slider->widget.backend_widget;
    assert(backend != 0);
    ld_slider = (ldSlider_t *)backend->ld_widget;
    assert(ld_slider != 0);

    assert(picoui_slider_set_range(slider, 0, 200) == 0);
    assert(slider->min_value == 0);
    assert(slider->max_value == 200);

    assert(picoui_slider_set_value(slider, 75) == 0);
    assert(slider->value == 75);

    /* percent = (75-0)*100/(200-0) = 37 */
    assert(picoui_slider_get_percent(slider, &percent) == 0);
    assert(percent == 37);

    /* set_value rejects out-of-range */
    assert(picoui_slider_set_value(slider, -1) == -1);
    assert(picoui_slider_set_value(slider, 201) == -1);
    assert(slider->value == 75);

    /* set_percent updates value */
    assert(picoui_slider_set_percent(slider, 100) == 0);
    assert(slider->value == 200);
    assert(picoui_slider_get_percent(slider, &percent) == 0);
    assert(percent == 100);

    /* set_percent rejects bounds */
    assert(picoui_slider_set_percent(slider, -1) == -1);
    assert(picoui_slider_set_percent(slider, 101) == -1);
    assert(slider->value == 200);

    assert(picoui_slider_set_horizontal(slider, 0) == 0);
    assert(picoui_slider_get_horizontal(slider, &horizontal) == 0);
    assert(horizontal == 0);
    assert(ld_slider->isHorizontal == false);

    assert(picoui_slider_set_horizontal(slider, 1) == 0);
    assert(picoui_slider_get_horizontal(slider, &horizontal) == 0);
    assert(horizontal == 1);
    assert(ld_slider->isHorizontal == true);

    assert(picoui_slider_set_background_source(slider, &background_source) == 0);
    assert(picoui_slider_set_indicator_source(slider, &indicator_source) == 0);
    assert(ld_slider->ptBgImgTile == &bg_tile);
    assert(ld_slider->ptBgMaskTile == &bg_mask);
    assert(ld_slider->ptIndicImgTile == &indic_tile);
    assert(ld_slider->ptIndicMaskTile == &indic_mask);

    assert(picoui_slider_set_indicator_width(slider, 21) == 0);
    assert(ld_slider->indicWidth == 21U);

    assert(picoui_slider_set_slim_size(slider, 9) == 0);
    assert(ld_slider->slimSize == 9U);
}

static void test_slider_create_with_props_failure_rolls_back_attached_child(struct picoui_window *win)
{
    struct picoui_backend_widget *parent_backend =
        (struct picoui_backend_widget *)win->widget.backend_widget;
    struct picoui_backend_widget *tail = parent_backend->first_child;
    struct picoui_backend_widget *next_before = 0;
    struct picoui_slider *slider;
    const struct picoui_backend_widget *disposed_backend;

    while (tail != 0 && tail->next_sibling != 0) {
        tail = tail->next_sibling;
    }
    if (tail != 0) {
        next_before = tail->next_sibling;
    }

    picoui_backend_slider_test_fail_indicator_width_for_id("sl_fail_indicator_width");
    slider = picoui_slider_create_with_props(
        win,
        &(struct picoui_slider_props){
            .id = "sl_fail_indicator_width",
            .min_value = 0,
            .max_value = 100,
            .value = 10,
            .has_indicator_width = 1,
            .indicator_width = 12,
        });

    assert(slider == 0);
    disposed_backend = picoui_backend_slider_test_last_disposed_backend();
    assert(disposed_backend != 0);
    assert(disposed_backend->kind == PICOUI_BACKEND_WIDGET_SLIDER);
    assert(disposed_backend->parent == 0);
    assert(disposed_backend->owner == 0);
    assert(disposed_backend->root == 0);
    assert(disposed_backend->host_widget == 0);
    assert(disposed_backend->ld_event_bridge_scene == 0);
    assert(disposed_backend->ld_event_bridge_sender == 0);
    assert(disposed_backend->ld_event_bridge_next == 0);
    assert(disposed_backend->next_sibling == 0);
    assert(((const ldBase_t *)disposed_backend->ld_widget)->pInfo == 0);
    if (tail != 0) {
        assert(tail->next_sibling == next_before);
    } else {
        assert(parent_backend->first_child == 0);
    }

    assert(picoui_slider_create_with_props(
               win,
               &(struct picoui_slider_props){
                   .id = "sl_fail_indicator_width",
                   .min_value = 0,
                   .max_value = 100,
                   .value = 10,
                   .has_indicator_width = 1,
                   .indicator_width = 12,
               }) != 0);
}

static void test_slider_rejects_null_args(struct picoui_window *win)
{
    assert(picoui_slider_create(0, "id") == 0);
    assert(picoui_slider_create(win, 0) == 0);
    assert(picoui_slider_init(0, "id") == 0);
    assert(picoui_slider_init(win, 0) == 0);
    assert(picoui_slider_create_with_props(0, &(struct picoui_slider_props){.id = "p"}) == 0);
    assert(picoui_slider_create_with_props(win, 0) == 0);
    assert(picoui_slider_create_with_props(win, &(struct picoui_slider_props){.id = 0}) == 0);
    assert(picoui_slider_set_value(0, 10) == -1);
    assert(picoui_slider_set_range(0, 0, 100) == -1);
    assert(picoui_slider_set_percent(0, 50) == -1);
    assert(picoui_slider_set_horizontal(0, 1) == -1);
    assert(picoui_slider_get_percent(0, &(int){0}) == -1);
    assert(picoui_slider_set_on_value_changed(0, 0, 0) == -1);
}

int main(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_window *win;

    assert(app != 0);
    win = picoui_window_create(app, "root");
    assert(win != 0);

    test_slider_create_and_backend_mapping(win);
    test_slider_create_with_props_pushes_range(win);
    test_slider_create_with_props_failure_rolls_back_attached_child(win);
    test_slider_set_range_and_value_round_trip(win);
    test_slider_rejects_null_args(win);

    picoui_app_destroy(app);
    return 0;
}
