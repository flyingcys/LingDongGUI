#include "picoui/app.h"
#include "picoui/icon_slider.h"
#include "picoui/widget.h"
#include "picoui/window.h"
#include "../../../src/gui/ldIconSlider.h"
#include "backend.h"
#include "internal.h"

#include <assert.h>
#include <stdio.h>

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

static void test_icon_slider_rejects_items_beyond_native_capacity(void)
{
    struct picoui_app *app;
    struct picoui_window *win;
    struct picoui_icon_slider *icon_slider;
    int index;

    app = picoui_app_create();
    assert(app != 0);
    win = picoui_window_create(app, "root");
    assert(win != 0);

    icon_slider = picoui_icon_slider_create((struct picoui_widget *)win, "icon_slider");
    assert(icon_slider != 0);
    for (index = 0; index < 8; ++index) {
        char id[16];
        char text[16];

        snprintf(id, sizeof(id), "item_%d", index);
        snprintf(text, sizeof(text), "Item %d", index);
        assert(picoui_icon_slider_add_item(icon_slider, id, text) == 0);
    }

    assert(picoui_icon_slider_add_item(icon_slider, "overflow", "Overflow") == -1);
    assert(picoui_icon_slider_set_selected_index(icon_slider, 7) == 0);
    assert(picoui_icon_slider_set_selected_index(icon_slider, 8) == -1);

    picoui_app_destroy(app);
}

static void test_icon_slider_create_with_props_pushes_backend_dimensions(void)
{
    struct picoui_app *app;
    struct picoui_window *win;
    struct picoui_icon_slider *icon_slider;
    struct picoui_backend_widget *backend;
    ldIconSlider_t *ld_icon_slider;
    const struct picoui_icon_slider_props props = {
        .id = "icon_slider",
        .width = 180,
        .height = 120,
        .icon_width = 40,
        .icon_space = 9,
        .columns = 2,
        .rows = 3,
        .pages = 4,
        .horizontal = 0,
    };

    app = picoui_app_create();
    assert(app != 0);
    win = picoui_window_create(app, "root");
    assert(win != 0);

    icon_slider = picoui_icon_slider_create_with_props((struct picoui_widget *)win, &props);
    assert(icon_slider != 0);
    backend = (struct picoui_backend_widget *)icon_slider->widget.backend_widget;
    assert(backend != 0);
    ld_icon_slider = (ldIconSlider_t *)backend->ld_widget;
    assert(ld_icon_slider != 0);

    assert(ld_icon_slider->use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iWidth == props.width);
    assert(ld_icon_slider->use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iHeight == props.height);
    assert(ld_icon_slider->iconWidth == props.icon_width);
    assert(ld_icon_slider->iconSpace == props.icon_space);
    assert(ld_icon_slider->columnCount == props.columns);
    assert(ld_icon_slider->rowCount == props.rows);
    assert(ld_icon_slider->pageMax == props.pages);
    assert(ld_icon_slider->isHorizontalScroll == false);

    picoui_app_destroy(app);
}

static void test_icon_slider_native_icon_images_and_speed_round_trip(void)
{
    struct picoui_app *app;
    struct picoui_window *win;
    struct picoui_icon_slider *icon_slider;
    struct picoui_backend_widget *backend;
    ldIconSlider_t *ld_icon_slider;
    arm_2d_tile_t icon_img = {
        .tRegion = {
            .tSize = { .iWidth = 24, .iHeight = 24 },
        },
    };
    arm_2d_tile_t icon_mask = {
        .tRegion = {
            .tSize = { .iWidth = 24, .iHeight = 24 },
        },
    };
    struct picoui_image_source icon_source = {
        .img_tile = &icon_img,
        .mask_tile = &icon_mask,
    };

    app = picoui_app_create();
    assert(app != 0);
    win = picoui_window_create(app, "root");
    assert(win != 0);

    icon_slider = picoui_icon_slider_create((struct picoui_widget *)win, "icon_slider_native");
    assert(icon_slider != 0);
    backend = (struct picoui_backend_widget *)icon_slider->widget.backend_widget;
    assert(backend != 0);
    ld_icon_slider = (ldIconSlider_t *)backend->ld_widget;
    assert(ld_icon_slider != 0);

    assert(picoui_icon_slider_add_item_with_source(icon_slider, "mail", "Mail", &icon_source) == 0);
    assert(picoui_icon_slider_set_speed(icon_slider, 7) == 0);

    assert(ld_icon_slider->iconCount == 1);
    assert(ld_icon_slider->ptIconInfoList[0].ptImgTile == &icon_img);
    assert(ld_icon_slider->ptIconInfoList[0].ptMaskTile == &icon_mask);
    assert(strcmp((const char *)ld_icon_slider->ptIconInfoList[0].pName, "Mail") == 0);
    assert(ld_icon_slider->moveOffset == 7);

    assert(picoui_icon_slider_add_item_with_source(0, "mail", "Mail", &icon_source) == -1);
    assert(picoui_icon_slider_set_speed(0, 3) == -1);

    picoui_app_destroy(app);
}

static void test_icon_slider_init_aliases_and_shared_base_round_trip(void)
{
    struct picoui_app *app;
    struct picoui_window *win;
    struct picoui_icon_slider *icon_slider;
    struct picoui_backend_widget *backend;
    ldIconSlider_t *ld_icon_slider;
    arm_2d_tile_t icon_img = {
        .tRegion = {
            .tSize = { .iWidth = 22, .iHeight = 22 },
        },
    };
    arm_2d_tile_t icon_mask = {
        .tRegion = {
            .tSize = { .iWidth = 22, .iHeight = 22 },
        },
    };
    struct picoui_image_source icon_source = {
        .img_tile = &icon_img,
        .mask_tile = &icon_mask,
    };

    app = picoui_app_create();
    assert(app != 0);
    win = picoui_window_create(app, "root");
    assert(win != 0);

    icon_slider = picoui_icon_slider_init((struct picoui_widget *)win, "icon_slider_alias");
    assert(icon_slider != 0);
    backend = (struct picoui_backend_widget *)icon_slider->widget.backend_widget;
    assert(backend != 0);
    ld_icon_slider = (ldIconSlider_t *)backend->ld_widget;
    assert(ld_icon_slider != 0);

    assert(picoui_icon_slider_add_icon(icon_slider, "mail", "Mail", &icon_source) == 0);
    assert(picoui_icon_slider_set_horizontal_scroll(icon_slider, 0) == 0);

    assert(ld_icon_slider->iconCount == 1);
    assert(ld_icon_slider->ptIconInfoList[0].ptImgTile == &icon_img);
    assert(ld_icon_slider->ptIconInfoList[0].ptMaskTile == &icon_mask);
    assert(ld_icon_slider->isHorizontalScroll == false);

    assert(picoui_widget_set_pos(&icon_slider->widget, 8, 12) == 0);
    assert(((ldBase_t *)ld_icon_slider)->tRegion.tLocation.iX == 8);
    assert(((ldBase_t *)ld_icon_slider)->tRegion.tLocation.iY == 12);
    assert(picoui_widget_set_visible(&icon_slider->widget, 0) == 0);
    assert(((ldBase_t *)ld_icon_slider)->bIsVisible == false);
    assert(picoui_widget_set_opacity(&icon_slider->widget, 61) == 0);
    assert(((ldBase_t *)ld_icon_slider)->chOpacity == 61);
    assert(picoui_widget_set_selectable(&icon_slider->widget, 0) == 0);
    assert(((ldBase_t *)ld_icon_slider)->isSelectable == false);
    assert(picoui_widget_set_selected(&icon_slider->widget, 1) == 0);
    assert(((ldBase_t *)ld_icon_slider)->isSelect == true);
    assert(picoui_widget_set_corner(&icon_slider->widget, 4) == 0);
    assert(((ldBase_t *)ld_icon_slider)->chCorner == 4);

    picoui_app_destroy(app);
}

int main(void)
{
    test_icon_slider_selection_and_value_follow_backend_truth();
    test_icon_slider_rejects_items_beyond_native_capacity();
    test_icon_slider_create_with_props_pushes_backend_dimensions();
    test_icon_slider_native_icon_images_and_speed_round_trip();
    test_icon_slider_init_aliases_and_shared_base_round_trip();
    return 0;
}
