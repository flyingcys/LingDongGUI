#include "picoui/picoui.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldWindow.h"
#include "../../../src/porting/ldConfig.h"
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
    struct picoui_backend_widget *backend;
    ldWindow_t *ld_win;

    assert(picoui_window_set_padding_group(win, 10, 20, 30, 40) == 0);

    backend = (struct picoui_backend_widget *)win->widget.backend_widget;
    ld_win = (ldWindow_t *)backend->ld_widget;
    assert(ld_win->flexPadding.left == 10);
    assert(ld_win->flexPadding.top == 20);
    assert(ld_win->flexPadding.right == 30);
    assert(ld_win->flexPadding.bottom == 40);
    assert(ld_win->gridPadding.left == 10);
    assert(ld_win->gridPadding.top == 20);
    assert(ld_win->gridPadding.right == 30);
    assert(ld_win->gridPadding.bottom == 40);
}

static void test_window_widget_base_api_round_trip(struct picoui_window *win)
{
    struct picoui_backend_widget *backend;
    ldBase_t *ld_base;

    backend = (struct picoui_backend_widget *)win->widget.backend_widget;
    ld_base = (ldBase_t *)backend->ld_widget;

    assert(ld_base->use_as__arm_2d_control_node_t.tRegion.tSize.iWidth == LD_CFG_SCREEN_WIDTH);
    assert(ld_base->use_as__arm_2d_control_node_t.tRegion.tSize.iHeight == LD_CFG_SCREEN_HEIGHT);

    assert(picoui_widget_set_size(&win->widget, 200, 120) == 0);
    assert(ld_base->use_as__arm_2d_control_node_t.tRegion.tSize.iWidth == 200);
    assert(ld_base->use_as__arm_2d_control_node_t.tRegion.tSize.iHeight == 120);

    assert(picoui_widget_set_selectable(&win->widget, 1) == 0);
    assert(picoui_widget_set_selected(&win->widget, 1) == 0);
    assert(ld_base->isSelectable == true);
    assert(ld_base->isSelected == true);
}

static void test_window_grid_padding_positions_switch(struct picoui_window *win)
{
    const int cols[] = {220, 0};
    const int rows[] = {24, 0};
    struct picoui_switch *sw;
    struct picoui_backend_widget *window_backend;
    struct picoui_backend_widget *switch_backend;
    ldWindow_t *ld_win;
    ldBase_t *ld_switch;

    assert(picoui_grid_set_columns(win, cols, 2) == 0);
    assert(picoui_grid_set_rows(win, rows, 2) == 0);
    assert(picoui_grid_set_gap(win, 12, 12) == 0);
    assert(picoui_window_set_padding_group(win, 16, 24, 16, 16) == 0);

    sw = picoui_switch_create(win, "window_grid_padding_switch");
    assert(sw != 0);
    assert(picoui_widget_set_size((struct picoui_widget *)sw, 48, 24) == 0);
    assert(picoui_widget_set_grid_cell((struct picoui_widget *)sw,
                                       0,
                                       0,
                                       1,
                                       1,
                                       PICOUI_ALIGN_START,
                                       PICOUI_ALIGN_START) == 0);

    window_backend = (struct picoui_backend_widget *)win->widget.backend_widget;
    switch_backend = (struct picoui_backend_widget *)sw->widget.backend_widget;
    ld_win = (ldWindow_t *)window_backend->ld_widget;
    ld_switch = (ldBase_t *)switch_backend->ld_widget;

    ldWindow_on_frame_start(0, ld_win);
    assert(ld_switch->use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 16);
    assert(ld_switch->use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 24);
    assert(ld_switch->use_as__arm_2d_control_node_t.tRegion.tSize.iWidth == 48);
    assert(ld_switch->use_as__arm_2d_control_node_t.tRegion.tSize.iHeight == 24);
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
    test_window_grid_padding_positions_switch(win);
    test_window_widget_base_api_round_trip(win);

    picoui_app_destroy(app);
    return 0;
}
