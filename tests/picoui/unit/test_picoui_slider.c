#include "picoui/picoui.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldSlider.h"
#include "internal.h"

#include <assert.h>

static void test_slider_create_and_backend_mapping(struct picoui_window *win)
{
    struct picoui_slider *slider = picoui_slider_create(win, "sl_test");
    struct picoui_backend_widget *backend;
    ldSlider_t *ld_slider;

    assert(slider != 0);
    backend = (struct picoui_backend_widget *)slider->widget.backend_widget;
    assert(backend != 0);
    assert(backend->kind == PICOUI_BACKEND_WIDGET_SLIDER);
    ld_slider = (ldSlider_t *)backend->ld_widget;
    assert(ld_slider != 0);
    assert(slider->min_value == 0);
    assert(slider->max_value == 100);
    assert(slider->value == 0);
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

    assert(slider != 0);

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
    test_slider_set_range_and_value_round_trip(win);
    test_slider_rejects_null_args(win);

    picoui_app_destroy(app);
    return 0;
}
