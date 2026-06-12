#include "picoui/picoui.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldImage.h"
#include "internal.h"
#include "picoui_test_support.h"
#include <assert.h>
#include <string.h>

extern int picoui_widget_has_ld_binding(const struct picoui_widget *widget);

static struct picoui_image_test_dispose_snapshot g_image_snapshot;
static int g_image_snapshot_valid = 0;

static int test_image_finish_detach_after_backend_failure(struct picoui_backend_widget *backend)
{
    struct picoui_backend_widget *parent;
    struct picoui_backend_widget *cursor;

    if (backend == 0 || backend->parent == 0) {
        return 0;
    }

    parent = backend->parent;
    if (parent->first_child == backend) {
        parent->first_child = backend->next_sibling;
    } else {
        cursor = parent->first_child;
        while (cursor != 0 && cursor->next_sibling != backend) {
            cursor = cursor->next_sibling;
        }
        if (cursor == 0) {
            return -1;
        }
        cursor->next_sibling = backend->next_sibling;
    }

    backend->parent = 0;
    backend->next_sibling = 0;
    backend->owner = 0;
    backend->root = 0;
    return 0;
}

static void test_image_fill_snapshot(struct picoui_backend_widget *backend,
                                     int detach_result,
                                     int unbind_result)
{
    ldBase_t *ld_base = backend != 0 ? (ldBase_t *)backend->ld_widget : 0;

    g_image_snapshot.kind = backend != 0 ? backend->kind : -1;
    g_image_snapshot.cleanup_complete = (detach_result == 0 && unbind_result == 0);
    g_image_snapshot.cleanup_incomplete = (detach_result != 0 || unbind_result != 0);
    g_image_snapshot.detach_result = detach_result;
    g_image_snapshot.unbind_result = unbind_result;
    g_image_snapshot.detached = (detach_result == 0 && backend != 0 && backend->parent == 0);
    g_image_snapshot.owner_cleared = (backend != 0 && backend->owner == 0);
    g_image_snapshot.root_cleared = (backend != 0 && backend->root == 0);
    g_image_snapshot.parent_cleared = (backend != 0 && backend->parent == 0);
    g_image_snapshot.next_sibling_cleared = (backend != 0 && backend->next_sibling == 0);
    g_image_snapshot.host_cleared = (backend != 0 && backend->host_widget == 0);
    g_image_snapshot.event_bridge_cleared = (backend != 0
        && backend->ld_event_bridge_scene == 0
        && backend->ld_event_bridge_sender == 0
        && backend->ld_event_bridge_next == 0);
    g_image_snapshot.ld_pinfo_cleared = (ld_base == 0 || ld_base->pInfo == 0);
    g_image_snapshot_valid = 1;
}

void picoui_backend_image_test_reset_state(void)
{
    memset(&g_image_snapshot, 0, sizeof(g_image_snapshot));
    g_image_snapshot_valid = 0;
}

struct picoui_image *picoui_backend_image_test_create_with_props_fail_before_size(
    struct picoui_window *parent,
    const struct picoui_image_props *props)
{
    struct picoui_image *image;
    struct picoui_backend_widget *backend;
    int detach_result = 0;
    int unbind_result;

    if (parent == 0 || props == 0) {
        return 0;
    }

    image = picoui_image_create(parent, props->id);
    if (image == 0) {
        return 0;
    }
    if (props->source != 0 && picoui_image_set_source(image, props->source) != 0) {
        picoui_widget_destroy(&image->widget);
        return 0;
    }
    if (props->style_class != 0
        && picoui_widget_set_style_class(&image->widget, props->style_class) != 0) {
        picoui_widget_destroy(&image->widget);
        return 0;
    }
    if (picoui_widget_set_user_data(&image->widget, props->user_data) != 0
        || picoui_widget_set_bg_color(&image->widget, props->bg_color) != 0
        || picoui_widget_set_text_color(&image->widget, props->text_color) != 0
        || picoui_widget_set_border_color(&image->widget, props->border_color) != 0
        || picoui_widget_set_radius(&image->widget, props->radius) != 0
        || picoui_widget_set_padding(&image->widget, props->padding) != 0) {
        picoui_widget_destroy(&image->widget);
        return 0;
    }

    backend = (struct picoui_backend_widget *)image->widget.backend_widget;
    if (backend == 0) {
        picoui_widget_destroy(&image->widget);
        return 0;
    }
    if (backend->parent != 0) {
        detach_result = tinyui_runtime_bridge_detach_from_parent(backend);
        if (detach_result != 0) {
            detach_result = test_image_finish_detach_after_backend_failure(backend);
        }
    }
    unbind_result = tinyui_runtime_bridge_unbind_host(backend);
    test_image_fill_snapshot(backend, detach_result, unbind_result);
    picoui_widget_destroy(&image->widget);
    return 0;
}

int picoui_backend_image_test_take_last_dispose_snapshot(
    struct picoui_image_test_dispose_snapshot *snapshot)
{
    if (snapshot == 0 || g_image_snapshot_valid == 0) {
        return -1;
    }

    *snapshot = g_image_snapshot;
    memset(&g_image_snapshot, 0, sizeof(g_image_snapshot));
    g_image_snapshot_valid = 0;
    return 0;
}

static void test_image_create_and_ld_mapping(struct picoui_window *win)
{
    struct picoui_image *img = picoui_image_create(win, "img_test");
    struct picoui_backend_widget *backend;
    struct picoui_backend_widget *parent_backend;
    ldImage_t *ld_img;

    assert(img != 0);
    backend = (struct picoui_backend_widget *)img->widget.backend_widget;
    assert(backend != 0);
    parent_backend = (struct picoui_backend_widget *)win->widget.backend_widget;
    assert(parent_backend != 0);
    assert(backend->kind == PICOUI_BACKEND_WIDGET_IMAGE);
    assert(backend->owner == parent_backend->owner);
    assert(backend->root == parent_backend->root);
    assert(backend->parent == parent_backend);
    assert(backend->ld_name_id != 0);
    assert(backend->host_widget == &img->widget);
    assert(backend->ld_event_bridge_scene != 0);
    assert(backend->ld_event_bridge_sender == backend->ld_widget);
    ld_img = (ldImage_t *)backend->ld_widget;
    assert(ld_img != 0);
    assert(((ldBase_t *)ld_img)->pInfo == backend);
    assert(picoui_widget_has_ld_binding(&img->widget) == 1);
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

static void test_image_create_with_props_failure_rolls_back_attached_child(struct picoui_window *win)
{
    arm_2d_tile_t img_tile = {0};
    struct picoui_backend_widget *parent_backend =
        (struct picoui_backend_widget *)win->widget.backend_widget;
    struct picoui_backend_widget *tail = parent_backend->first_child;
    struct picoui_backend_widget *next_before = 0;
    struct picoui_image *probe;
    struct picoui_image_test_dispose_snapshot snapshot = {0};

    while (tail != 0 && tail->next_sibling != 0) {
        tail = tail->next_sibling;
    }
    if (tail != 0) {
        next_before = tail->next_sibling;
    }

    picoui_backend_image_test_reset_state();
    probe = picoui_image_create(win, "image_fail_size");
    assert(probe != 0);
    assert(picoui_widget_destroy(&probe->widget) == 0);

    assert(picoui_backend_image_test_create_with_props_fail_before_size(
               win,
               &(struct picoui_image_props){
                   .id = "image_fail_size",
                   .source = &(struct picoui_image_source){
                       .img_tile = &img_tile,
                   },
                   .width = 48,
                   .height = 24,
               })
           == 0);
    assert(picoui_backend_image_test_take_last_dispose_snapshot(&snapshot) == 0);
    assert(snapshot.kind == PICOUI_BACKEND_WIDGET_IMAGE);
    assert(snapshot.cleanup_complete == 1);
    assert(snapshot.cleanup_incomplete == 0);
    assert(snapshot.detach_result == 0);
    assert(snapshot.unbind_result == 0);
    assert(snapshot.detached == 1);
    assert(snapshot.owner_cleared == 1);
    assert(snapshot.root_cleared == 1);
    assert(snapshot.parent_cleared == 1);
    assert(snapshot.next_sibling_cleared == 1);
    assert(snapshot.host_cleared == 1);
    assert(snapshot.event_bridge_cleared == 1);
    assert(snapshot.ld_pinfo_cleared == 1);
    assert(picoui_backend_image_test_take_last_dispose_snapshot(&snapshot) == -1);
    if (tail != 0) {
        assert(tail->next_sibling == next_before);
    } else {
        assert(parent_backend->first_child == 0);
    }

    picoui_backend_image_test_reset_state();
    assert(picoui_image_create_with_props(
               win,
               &(struct picoui_image_props){
                   .id = "image_fail_size",
                   .source = &(struct picoui_image_source){
                       .img_tile = &img_tile,
                   },
                   .width = 48,
                   .height = 24,
               })
           != 0);
}

static void test_image_shared_widget_helpers_reject_null(void)
{
    assert(tinyui_runtime_bridge_unbind_host(0) == -1);
    assert(tinyui_runtime_bridge_detach_from_parent(0) == -1);
    assert(tinyui_widget_is_kind(0, PICOUI_BACKEND_WIDGET_IMAGE) == 0);
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
    test_image_create_with_props_failure_rolls_back_attached_child(win);
    test_image_shared_widget_helpers_reject_null();

    picoui_app_destroy(app);
    return 0;
}
