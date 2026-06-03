#include "picoui/picoui.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldImage.h"
#include "internal.h"
#include <assert.h>

static void test_image_create_and_ld_mapping(struct picoui_window *win)
{
    struct picoui_image *img = picoui_image_create(win, "img_test");
    struct picoui_backend_widget *backend;
    ldImage_t *ld_img;

    assert(img != 0);
    backend = (struct picoui_backend_widget *)img->widget.backend_widget;
    assert(backend != 0);
    assert(backend->kind == PICOUI_BACKEND_WIDGET_IMAGE);
    ld_img = (ldImage_t *)backend->ld_widget;
    assert(ld_img != 0);
}

static void test_image_create_with_props_sets_source(struct picoui_window *win)
{
    arm_2d_tile_t img_tile = {0};
    arm_2d_tile_t mask_tile = {0};
    struct picoui_image_source src = { .img_tile = &img_tile, .mask_tile = &mask_tile };
    struct picoui_image *img = picoui_image_create_with_props(
        win, &(struct picoui_image_props){ .id = "img_props", .source = &src, .width = 64, .height = 64 });
    struct picoui_backend_widget *backend;

    assert(img != 0);
    backend = (struct picoui_backend_widget *)img->widget.backend_widget;
    assert(backend->image_source != 0);
    assert(backend->image_source->img_tile == &img_tile);
}

static void test_image_rejects_null_source_boundary(struct picoui_window *win)
{
    struct picoui_image *img = picoui_image_create(win, "img_null_src");
    assert(img != 0);
    assert(picoui_image_set_source(img, 0) == 0);
}

static void test_image_create_with_props_rejects_null(struct picoui_window *win)
{
    assert(picoui_image_create_with_props(win, 0) == 0);
    assert(picoui_image_create_with_props(0, &(struct picoui_image_props){.id="x"}) == 0);
}

int main(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_window *win;
    assert(app != 0);
    win = picoui_window_create(app, "root");
    assert(win != 0);

    test_image_create_and_ld_mapping(win);
    test_image_create_with_props_sets_source(win);
    test_image_rejects_null_source_boundary(win);
    test_image_create_with_props_rejects_null(win);

    picoui_app_destroy(app);
    return 0;
}
