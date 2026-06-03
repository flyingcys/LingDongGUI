#include "picoui/picoui.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldWindow.h"
#include "internal.h"
#include <assert.h>

static void test_window_create_and_backend_mapping(struct picoui_window *win)
{
    struct picoui_backend_widget *backend;
    ldWindow_t *ld_win;

    assert(win != 0);
    backend = (struct picoui_backend_widget *)win->widget.backend_widget;
    assert(backend != 0);
    assert(backend->kind == PICOUI_BACKEND_WIDGET_WINDOW);
    ld_win = (ldWindow_t *)backend->ld_widget;
    assert(ld_win != 0);
    assert(ld_win->use_as__ldBase_t.widgetType == widgetTypeBackground);
}

static void test_window_padding_group_round_trip(struct picoui_window *win)
{
    assert(picoui_window_set_padding_group(win, 10, 20, 30, 40) == 0);
}

static void test_window_widget_base_api_round_trip(struct picoui_window *win)
{
    struct picoui_backend_widget *backend;
    ldBase_t *ld_base;

    backend = (struct picoui_backend_widget *)win->widget.backend_widget;
    ld_base = (ldBase_t *)backend->ld_widget;

    assert(picoui_widget_set_size(&win->widget, 480, 320) == 0);
    assert(ld_base->use_as__arm_2d_control_node_t.tRegion.tSize.iWidth == 480);
    assert(ld_base->use_as__arm_2d_control_node_t.tRegion.tSize.iHeight == 320);

    assert(picoui_widget_set_selectable(&win->widget, 1) == 0);
    assert(picoui_widget_set_selected(&win->widget, 1) == 0);
    assert(ld_base->isSelectable == true);
    assert(ld_base->isSelected == true);
}

int main(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_window *win;

    assert(app != 0);
    win = picoui_window_create(app, "root");
    assert(win != 0);

    test_window_create_and_backend_mapping(win);
    test_window_padding_group_round_trip(win);
    test_window_widget_base_api_round_trip(win);

    picoui_app_destroy(app);
    return 0;
}
