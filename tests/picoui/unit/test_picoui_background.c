#include "picoui/picoui.h"
#include "../../../src/gui/ldBase.h"
#include "internal.h"
#include <assert.h>

static void test_background_create_and_backend_mapping(struct picoui_background *bg)
{
    struct picoui_backend_widget *backend;
    ldBase_t *ld_base;

    assert(bg != 0);
    backend = (struct picoui_backend_widget *)bg->window.widget.backend_widget;
    assert(backend != 0);
    assert(backend->kind == PICOUI_BACKEND_WIDGET_BACKGROUND);
    ld_base = (ldBase_t *)backend->ld_widget;
    assert(ld_base != 0);
    assert(ld_base->widgetType == widgetTypeBackground);
}

static void test_background_window_accepts_widget_base_api(struct picoui_background *bg)
{
    struct picoui_backend_widget *backend;
    ldBase_t *ld_base;

    backend = (struct picoui_backend_widget *)bg->window.widget.backend_widget;
    ld_base = (ldBase_t *)backend->ld_widget;

    assert(picoui_widget_set_pos(&bg->window.widget, 10, 20) == 0);
    assert(ld_base->use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 10);
    assert(ld_base->use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 20);

    assert(picoui_widget_set_size(&bg->window.widget, 400, 300) == 0);
    assert(ld_base->use_as__arm_2d_control_node_t.tRegion.tSize.iWidth == 400);
    assert(ld_base->use_as__arm_2d_control_node_t.tRegion.tSize.iHeight == 300);
}

static void test_background_offset_round_trip(struct picoui_background *bg)
{
    assert(picoui_window_set_background_offset((struct picoui_window *)bg, 5, 10) == 0);
    assert(bg->window.background_offset_x == 5);
    assert(bg->window.background_offset_y == 10);
}

int main(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_background *bg;

    assert(app != 0);
    bg = picoui_background_create(app, "bg_root");
    assert(bg != 0);

    test_background_create_and_backend_mapping(bg);
    test_background_window_accepts_widget_base_api(bg);
    test_background_offset_round_trip(bg);

    picoui_app_destroy(app);
    return 0;
}
