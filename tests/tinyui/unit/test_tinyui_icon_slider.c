#include "app.h"
#include "icon_slider.h"
#include "widget.h"
#include "window.h"
#include "../../../src/gui/ldIconSlider.h"
#include "internal.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static bool test_source_contains_symbol_definition(const char *path, const char *symbol)
{
    FILE *fp;
    char line[1024];
    char pattern[256];

    snprintf(pattern, sizeof(pattern), "static ");
    fp = fopen(path, "r");
    if (fp == 0) {
        return false;
    }

    while (fgets(line, sizeof(line), fp) != 0) {
        if (strstr(line, pattern) != 0 && strstr(line, symbol) != 0) {
            fclose(fp);
            return true;
        }
    }

    fclose(fp);
    return false;
}

static void test_icon_slider_internal_seams_renamed(void)
{
    const char *source_path = __FILE__;
    const char *widget_path;
    char widget_path_buf[1024];
    const char *marker = "/tests/tinyui/unit/test_tinyui_icon_slider.c";
    const char *marker_pos = strstr(source_path, marker);
    size_t prefix_len;

    assert(marker_pos != 0);
    prefix_len = (size_t)(marker_pos - source_path);
    assert(prefix_len + strlen("/tinyui/src/widgets/icon_slider.c") < sizeof(widget_path_buf));
    memcpy(widget_path_buf, source_path, prefix_len);
    widget_path_buf[prefix_len] = '\0';
    strcat(widget_path_buf, "/tinyui/src/widgets/icon_slider.c");
    widget_path = widget_path_buf;

    assert(test_source_contains_symbol_definition(widget_path, "tinyui_icon_slider_rollback"));
    assert(test_source_contains_symbol_definition(widget_path, "tinyui_icon_slider_native_slot"));
    assert(test_source_contains_symbol_definition(widget_path, "tinyui_icon_slider_props_are_valid"));
    assert(test_source_contains_symbol_definition(widget_path, "tinyui_icon_slider_create_with_backend_config"));
    assert(test_source_contains_symbol_definition(widget_path, "tinyui_icon_slider_ld_init"));
}

static bool test_source_contains_text(const char *path, const char *needle)
{
    FILE *fp;
    char line[1024];

    fp = fopen(path, "r");
    if (fp == 0) {
        return false;
    }

    while (fgets(line, sizeof(line), fp) != 0) {
        if (strstr(line, needle) != 0) {
            fclose(fp);
            return true;
        }
    }

    fclose(fp);
    return false;
}

static void test_icon_slider_create_uses_create_leaf(void)
{
    const char *source_path = __FILE__;
    const char *widget_path;
    char widget_path_buf[1024];
    const char *marker = "/tests/tinyui/unit/test_tinyui_icon_slider.c";
    const char *marker_pos = strstr(source_path, marker);
    size_t prefix_len;

    assert(marker_pos != 0);
    prefix_len = (size_t)(marker_pos - source_path);
    assert(prefix_len + strlen("/tinyui/src/widgets/icon_slider.c") < sizeof(widget_path_buf));
    memcpy(widget_path_buf, source_path, prefix_len);
    widget_path_buf[prefix_len] = '\0';
    strcat(widget_path_buf, "/tinyui/src/widgets/icon_slider.c");
    widget_path = widget_path_buf;

    assert(test_source_contains_text(widget_path, "tinyui_widget_create_leaf("));
}

static void icon_slider_on_selected(struct tinyui_icon_slider *icon_slider, int index, void *user_data)
{
    (void)icon_slider;
    (void)index;
    (void)user_data;
}

static void test_icon_slider_selection_and_value_follow_backend_truth(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win;
    struct tinyui_icon_slider *icon_slider;
    int horizontal = 0;

    app = tinyui_app_create();
    assert(app != 0);
    win = tinyui_window_create(app, "root");
    assert(win != 0);

    icon_slider = tinyui_icon_slider_create((struct tinyui_widget *)win, "icon_slider");
    assert(icon_slider != 0);
    assert(tinyui_icon_slider_add_item(icon_slider, "weather", "Weather") == 0);
    assert(tinyui_icon_slider_add_item(icon_slider, "note", "Note") == 0);
    assert(tinyui_icon_slider_add_item(icon_slider, "book", "Book") == 0);
    assert(tinyui_icon_slider_set_selected_index(icon_slider, 2) == 0);
    assert(tinyui_icon_slider_get_selected_index(icon_slider) == 2);
    assert(tinyui_icon_slider_set_horizontal(icon_slider, 0) == 0);
    assert(tinyui_icon_slider_get_horizontal(icon_slider, &horizontal) == 0);
    assert(horizontal == 0);

    tinyui_icon_slider_set_on_selected(icon_slider, icon_slider_on_selected, icon_slider);
    tinyui_app_destroy(app);
}

static void test_icon_slider_rejects_items_beyond_native_capacity(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win;
    struct tinyui_icon_slider *icon_slider;
    int index;

    app = tinyui_app_create();
    assert(app != 0);
    win = tinyui_window_create(app, "root");
    assert(win != 0);

    icon_slider = tinyui_icon_slider_create((struct tinyui_widget *)win, "icon_slider");
    assert(icon_slider != 0);
    for (index = 0; index < 8; ++index) {
        char id[16];
        char text[16];

        snprintf(id, sizeof(id), "item_%d", index);
        snprintf(text, sizeof(text), "Item %d", index);
        assert(tinyui_icon_slider_add_item(icon_slider, id, text) == 0);
    }

    assert(tinyui_icon_slider_add_item(icon_slider, "overflow", "Overflow") == -1);
    assert(tinyui_icon_slider_set_selected_index(icon_slider, 7) == 0);
    assert(tinyui_icon_slider_set_selected_index(icon_slider, 8) == -1);

    tinyui_app_destroy(app);
}

static void test_icon_slider_create_builds_direct_backend_mapping(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win;
    struct tinyui_icon_slider *icon_slider;
    struct tinyui_widget *backend;
    struct tinyui_widget *parent_backend;
    ldIconSlider_t *ld_icon_slider;

    app = tinyui_app_create();
    assert(app != 0);
    win = tinyui_window_create(app, "icon_slider_direct_root");
    assert(win != 0);

    icon_slider = tinyui_icon_slider_create((struct tinyui_widget *)win, "icon_slider_direct");
    assert(icon_slider != 0);

    backend = &icon_slider->widget;
    parent_backend = &win->widget;
    assert(backend != 0);
    assert(parent_backend != 0);
    assert(backend->kind == TINYUI_BACKEND_WIDGET_ICON_SLIDER);
    assert(backend->owner == parent_backend->owner);
    assert((ldBase_t *)ldBaseGetRootNode((arm_2d_control_node_t *)backend->ld_widget) == (ldBase_t *)ldBaseGetRootNode((arm_2d_control_node_t *)parent_backend->ld_widget));
    assert(ldBaseGetParent((ldBase_t *)backend->ld_widget) == (ldBase_t *)parent_backend->ld_widget);
    assert(backend->ld_name_id != 0);
    assert(backend->ld_event_bridge_scene != 0);
    assert(backend->ld_event_bridge_sender == backend->ld_widget);
    ld_icon_slider = (ldIconSlider_t *)backend->ld_widget;
    assert(ld_icon_slider != 0);
    assert(tinyui_app_lookup_host(backend->owner, backend->ld_name_id) == backend);

    tinyui_app_destroy(app);
}

static void test_icon_slider_create_with_props_pushes_backend_dimensions(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win;
    struct tinyui_icon_slider *icon_slider;
    struct tinyui_widget *backend;
    ldIconSlider_t *ld_icon_slider;
    const struct tinyui_icon_slider_props props = {
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

    app = tinyui_app_create();
    assert(app != 0);
    win = tinyui_window_create(app, "root");
    assert(win != 0);

    icon_slider = tinyui_icon_slider_create_with_props((struct tinyui_widget *)win, &props);
    assert(icon_slider != 0);
    backend = &icon_slider->widget;
    assert(backend->ld_widget != 0);
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

    tinyui_app_destroy(app);
}

static void test_icon_slider_native_icon_images_and_speed_round_trip(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win;
    struct tinyui_icon_slider *icon_slider;
    struct tinyui_widget *backend;
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
    struct tinyui_image_source icon_source = {
        .img_tile = &icon_img,
        .mask_tile = &icon_mask,
    };

    app = tinyui_app_create();
    assert(app != 0);
    win = tinyui_window_create(app, "root");
    assert(win != 0);

    icon_slider = tinyui_icon_slider_create((struct tinyui_widget *)win, "icon_slider_native");
    assert(icon_slider != 0);
    backend = &icon_slider->widget;
    assert(backend->ld_widget != 0);
    ld_icon_slider = (ldIconSlider_t *)backend->ld_widget;
    assert(ld_icon_slider != 0);

    assert(tinyui_icon_slider_add_item_with_source(icon_slider, "mail", "Mail", &icon_source) == 0);
    assert(tinyui_icon_slider_set_speed(icon_slider, 7) == 0);

    assert(ld_icon_slider->iconCount == 1);
    assert(ld_icon_slider->ptIconInfoList[0].ptImgTile == &icon_img);
    assert(ld_icon_slider->ptIconInfoList[0].ptMaskTile == &icon_mask);
    assert(strcmp((const char *)ld_icon_slider->ptIconInfoList[0].pName, "Mail") == 0);
    assert(ld_icon_slider->moveOffset == 7);

    assert(tinyui_icon_slider_add_item_with_source(0, "mail", "Mail", &icon_source) == -1);
    assert(tinyui_icon_slider_set_speed(0, 3) == -1);

    tinyui_app_destroy(app);
}

static void test_icon_slider_init_aliases_and_shared_base_round_trip(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win;
    struct tinyui_icon_slider *icon_slider;
    struct tinyui_widget *backend;
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
    struct tinyui_image_source icon_source = {
        .img_tile = &icon_img,
        .mask_tile = &icon_mask,
    };

    app = tinyui_app_create();
    assert(app != 0);
    win = tinyui_window_create(app, "root");
    assert(win != 0);

    icon_slider = tinyui_icon_slider_init((struct tinyui_widget *)win, "icon_slider_alias");
    assert(icon_slider != 0);
    backend = &icon_slider->widget;
    assert(backend->ld_widget != 0);
    ld_icon_slider = (ldIconSlider_t *)backend->ld_widget;
    assert(ld_icon_slider != 0);

    assert(tinyui_icon_slider_add_icon(icon_slider, "mail", "Mail", &icon_source) == 0);
    assert(tinyui_icon_slider_set_horizontal_scroll(icon_slider, 0) == 0);

    assert(ld_icon_slider->iconCount == 1);
    assert(ld_icon_slider->ptIconInfoList[0].ptImgTile == &icon_img);
    assert(ld_icon_slider->ptIconInfoList[0].ptMaskTile == &icon_mask);
    assert(ld_icon_slider->isHorizontalScroll == false);

    assert(tinyui_widget_set_pos(&icon_slider->widget, 8, 12) == 0);
    assert(((ldBase_t *)ld_icon_slider)->use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 8);
    assert(((ldBase_t *)ld_icon_slider)->use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 12);
    assert(tinyui_widget_set_visible(&icon_slider->widget, 0) == 0);
    assert(((ldBase_t *)ld_icon_slider)->isHidden == true);
    assert(tinyui_widget_set_opacity(&icon_slider->widget, 61) == 0);
    assert(((ldBase_t *)ld_icon_slider)->opacity == 61);
    assert(tinyui_widget_set_selectable(&icon_slider->widget, 1) == 0);
    assert(((ldBase_t *)ld_icon_slider)->isSelectable == true);
    assert(tinyui_widget_set_selected(&icon_slider->widget, 1) == 0);
    assert(((ldBase_t *)ld_icon_slider)->isSelected == true);
    assert(tinyui_widget_set_selectable(&icon_slider->widget, 0) == 0);
    assert(((ldBase_t *)ld_icon_slider)->isSelectable == false);
    assert(tinyui_widget_set_corner(&icon_slider->widget, 4) == 0);
    assert(((ldBase_t *)ld_icon_slider)->isCorner == true);

    tinyui_app_destroy(app);
}

static void test_icon_slider_rejects_null_args(struct tinyui_window *win)
{
    assert(tinyui_icon_slider_create(0, "id") == 0);
    assert(tinyui_icon_slider_create((struct tinyui_widget *)win, 0) == 0);
    assert(tinyui_icon_slider_add_icon(0, "icon", "Icon", 0) == -1);
    assert(tinyui_icon_slider_set_selected_index(0, 0) == -1);
    assert(tinyui_icon_slider_get_selected_index(0) == -1);
    assert(tinyui_icon_slider_set_horizontal_scroll(0, 1) == -1);
}

int main(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *win;

    assert(app != 0);
    win = tinyui_window_create(app, "root");
    assert(win != 0);

    test_icon_slider_selection_and_value_follow_backend_truth();
    test_icon_slider_rejects_items_beyond_native_capacity();
    test_icon_slider_create_builds_direct_backend_mapping();
    test_icon_slider_create_with_props_pushes_backend_dimensions();
    test_icon_slider_native_icon_images_and_speed_round_trip();
    test_icon_slider_init_aliases_and_shared_base_round_trip();
    test_icon_slider_rejects_null_args(win);
    test_icon_slider_internal_seams_renamed();

    tinyui_app_destroy(app);
    return 0;
}
