#include "picoui/app.h"
#include "picoui/icon_slider.h"
#include "picoui/widget.h"
#include "picoui/window.h"

#include <assert.h>

static void icon_slider_on_selected(struct picoui_icon_slider *icon_slider, int index, void *user_data)
{
    (void)icon_slider;
    (void)index;
    (void)user_data;
}

static void test_icon_slider_selection_and_value_follow_backend_truth(void)
{
    struct picoui_app *app;
    struct picoui_window *win;
    struct picoui_icon_slider *icon_slider;
    int horizontal = 0;

    app = picoui_app_create();
    assert(app != 0);
    win = picoui_window_create(app, "root");
    assert(win != 0);

    icon_slider = picoui_icon_slider_create((struct picoui_widget *)win, "icon_slider");
    assert(icon_slider != 0);
    assert(picoui_icon_slider_add_item(icon_slider, "weather", "Weather") == 0);
    assert(picoui_icon_slider_add_item(icon_slider, "note", "Note") == 0);
    assert(picoui_icon_slider_add_item(icon_slider, "book", "Book") == 0);
    assert(picoui_icon_slider_set_selected_index(icon_slider, 2) == 0);
    assert(picoui_icon_slider_get_selected_index(icon_slider) == 2);
    assert(picoui_icon_slider_set_horizontal(icon_slider, 0) == 0);
    assert(picoui_icon_slider_get_horizontal(icon_slider, &horizontal) == 0);
    assert(horizontal == 0);

    picoui_icon_slider_set_on_selected(icon_slider, icon_slider_on_selected, icon_slider);
    picoui_app_destroy(app);
}

int main(void)
{
    test_icon_slider_selection_and_value_follow_backend_truth();
    return 0;
}
