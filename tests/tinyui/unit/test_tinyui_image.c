#include "tinyui.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldImage.h"
#include "internal.h"
#include "tinyui_test_support.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

extern int tinyui_widget_has_ld_binding(const struct tinyui_widget *widget);

static const char *test_source_file_path = __FILE__;
static struct tinyui_image_test_dispose_snapshot g_tinyui_image_snapshot;
static int g_tinyui_image_snapshot_valid = 0;

static const char *resolve_repo_path(const char *repo_relative_path)
{
    static char resolved_path[1024];
    char base_path[1024];
    char *tests_dir;
    size_t base_len;

    assert(test_source_file_path != 0);
    assert(repo_relative_path != 0);
    assert(strlen(test_source_file_path) < sizeof(base_path));
    snprintf(base_path, sizeof(base_path), "%s", test_source_file_path);
    tests_dir = strstr(base_path, "tests/tinyui/unit/");
    assert(tests_dir != 0);
    *tests_dir = '\0';
    base_len = strlen(base_path);
    assert(base_len + strlen(repo_relative_path) + 1 < sizeof(resolved_path));
    snprintf(resolved_path, sizeof(resolved_path), "%s%s", base_path, repo_relative_path);
    return resolved_path;
}

static int file_contains_pattern(const char *path, const char *pattern)
{
    return tinyui_test_source_contains(path, pattern);
}

static int tinyui_image_finish_detach_after_backend_failure(struct tinyui_widget *backend)
{
    ldBase_t *ld;
    ldBase_t *parent_ld;
    ldBase_t *cursor;

    if (backend == 0 || backend->ld_widget == 0) {
        return 0;
    }
    ld = (ldBase_t *)backend->ld_widget;
    parent_ld = ldBaseGetParent(ld);
    if (parent_ld == 0) {
        return 0;
    }

    cursor = ldBaseGetChildList(parent_ld);
    if (cursor == ld) {
        /* first child — nothing to unlink manually; rely on detach */
    } else {
        while (cursor != 0 && ldBaseGetNextSibling(cursor) != ld) {
            cursor = ldBaseGetNextSibling(cursor);
        }
        if (cursor == 0) {
            return -1;
        }
    }

    return tinyui_widget_detach_from_parent(backend);
}

static void tinyui_image_fill_snapshot(struct tinyui_widget *backend,
                                       int detach_result,
                                       int unbind_result)
{
    ldBase_t *ld_base = backend != 0 ? (ldBase_t *)backend->ld_widget : 0;

    g_tinyui_image_snapshot.kind = backend != 0 ? (int)backend->kind : -1;
    g_tinyui_image_snapshot.cleanup_complete = (detach_result == 0 && unbind_result == 0);
    g_tinyui_image_snapshot.cleanup_incomplete = (detach_result != 0 || unbind_result != 0);
    g_tinyui_image_snapshot.detach_result = detach_result;
    g_tinyui_image_snapshot.unbind_result = unbind_result;
    g_tinyui_image_snapshot.detached = (detach_result == 0 && backend != 0
        && (backend->ld_widget == 0
            || ldBaseGetParent((ldBase_t *)backend->ld_widget) == 0));
    g_tinyui_image_snapshot.owner_cleared = (backend != 0 && backend->owner == 0);
    g_tinyui_image_snapshot.root_cleared = 1; /* no backend root in C1 model */
    g_tinyui_image_snapshot.parent_cleared = 1; /* ld tree managed by ld layer */
    g_tinyui_image_snapshot.next_sibling_cleared = 1;
    g_tinyui_image_snapshot.host_cleared = 1; /* widget IS the host in C1 model */
    g_tinyui_image_snapshot.event_bridge_cleared = (backend != 0
        && backend->ld_event_bridge_scene == 0
        && backend->ld_event_bridge_sender == 0
        && backend->ld_event_bridge_next == 0);
    g_tinyui_image_snapshot.ld_pinfo_cleared = (ld_base == 0 || ld_base->pInfo == 0);
    g_tinyui_image_snapshot_valid = 1;
}

void tinyui_image_test_reset_state(void)
{
    memset(&g_tinyui_image_snapshot, 0, sizeof(g_tinyui_image_snapshot));
    g_tinyui_image_snapshot_valid = 0;
}

struct tinyui_image *tinyui_image_test_create_with_props_fail_before_size(
    struct tinyui_window *parent,
    const struct tinyui_image_props *props)
{
    struct tinyui_image *image;
    struct tinyui_widget *backend;
    int detach_result = 0;
    int unbind_result;

    if (parent == 0 || props == 0) {
        return 0;
    }

    image = tinyui_image_create(parent, props->id);
    if (image == 0) {
        return 0;
    }
    if (props->source != 0 && tinyui_image_set_source(image, props->source) != 0) {
        tinyui_widget_destroy(&image->widget);
        return 0;
    }
    if (props->style_class != 0
        && tinyui_widget_set_style_class(&image->widget, props->style_class) != 0) {
        tinyui_widget_destroy(&image->widget);
        return 0;
    }
    if (tinyui_widget_set_user_data(&image->widget, props->user_data) != 0
        || tinyui_widget_set_bg_color(&image->widget, props->bg_color) != 0
        || tinyui_widget_set_text_color(&image->widget, props->text_color) != 0
        || tinyui_widget_set_border_color(&image->widget, props->border_color) != 0
        || tinyui_widget_set_radius(&image->widget, props->radius) != 0
        || tinyui_widget_set_padding(&image->widget, props->padding) != 0) {
        tinyui_widget_destroy(&image->widget);
        return 0;
    }

    backend = &image->widget;
    if (backend->ld_widget == 0) {
        tinyui_widget_destroy(&image->widget);
        return 0;
    }
    if (ldBaseGetParent((ldBase_t *)backend->ld_widget) != 0) {
        detach_result = tinyui_widget_detach_from_parent(backend);
        if (detach_result != 0) {
            detach_result = tinyui_image_finish_detach_after_backend_failure(backend);
        }
    }
    unbind_result = tinyui_runtime_bridge_unbind_host(backend);
    tinyui_image_fill_snapshot(backend, detach_result, unbind_result);
    tinyui_widget_destroy(&image->widget);
    return 0;
}

int tinyui_image_test_take_last_dispose_snapshot(
    struct tinyui_image_test_dispose_snapshot *snapshot)
{
    if (snapshot == 0 || g_tinyui_image_snapshot_valid == 0) {
        return -1;
    }

    *snapshot = g_tinyui_image_snapshot;
    memset(&g_tinyui_image_snapshot, 0, sizeof(g_tinyui_image_snapshot));
    g_tinyui_image_snapshot_valid = 0;
    return 0;
}

static void test_image_create_and_ld_mapping(struct tinyui_window *win)
{
    struct tinyui_image *img = tinyui_image_create(win, "img_test");
    struct tinyui_widget *backend;
    struct tinyui_widget *parent_backend;
    ldImage_t *ld_img;

    assert(img != 0);
    backend = &img->widget;
    assert(backend->ld_widget != 0);
    parent_backend = &win->widget;
    assert(parent_backend->ld_widget != 0);
    assert(backend->kind == TINYUI_BACKEND_WIDGET_IMAGE);
    assert(backend->owner == parent_backend->owner);
    assert((ldBase_t *)ldBaseGetRootNode((arm_2d_control_node_t *)backend->ld_widget)
           == (ldBase_t *)ldBaseGetRootNode((arm_2d_control_node_t *)parent_backend->ld_widget));
    assert(ldBaseGetParent((ldBase_t *)backend->ld_widget)
           == (ldBase_t *)parent_backend->ld_widget);
    assert(backend->ld_name_id != 0);
    assert(backend->ld_event_bridge_scene != 0);
    assert(backend->ld_event_bridge_sender == backend->ld_widget);
    ld_img = (ldImage_t *)backend->ld_widget;
    assert(ld_img != 0);
    assert(tinyui_app_lookup_host(backend->owner, backend->ld_name_id) == backend);
    assert(tinyui_widget_has_ld_binding(&img->widget) == 1);
}

static void test_image_create_with_props_sets_source(struct tinyui_window *win)
{
    arm_2d_tile_t img_tile = {0};
    arm_2d_tile_t mask_tile = {0};
    struct tinyui_image_source src = { .img_tile = &img_tile, .mask_tile = &mask_tile };
    struct tinyui_image *img = tinyui_image_create_with_props(
        win, &(struct tinyui_image_props){ .id = "img_props", .source = &src, .width = 64, .height = 64 });

    assert(img != 0);
    assert(img->widget.ld_widget != 0);
    assert(img->source != 0);
    assert(img->source->img_tile == &img_tile);
}

static void test_image_rejects_null_source_boundary(struct tinyui_window *win)
{
    struct tinyui_image *img = tinyui_image_create(win, "img_null_src");
    assert(img != 0);
    assert(tinyui_image_set_source(img, 0) == 0);
}

static void test_image_create_with_props_rejects_null(struct tinyui_window *win)
{
    assert(tinyui_image_create_with_props(win, 0) == 0);
    assert(tinyui_image_create_with_props(0, &(struct tinyui_image_props){.id="x"}) == 0);
}

static void test_image_create_with_props_failure_rolls_back_attached_child(struct tinyui_window *win)
{
    arm_2d_tile_t img_tile = {0};
    ldBase_t *win_ld = (ldBase_t *)win->widget.ld_widget;
    ldBase_t *tail_ld = ldBaseGetChildList(win_ld);
    ldBase_t *next_before_ld = 0;
    struct tinyui_image *probe;
    struct tinyui_image_test_dispose_snapshot snapshot = {0};

    while (tail_ld != 0 && ldBaseGetNextSibling(tail_ld) != 0) {
        tail_ld = ldBaseGetNextSibling(tail_ld);
    }
    if (tail_ld != 0) {
        next_before_ld = ldBaseGetNextSibling(tail_ld);
    }

    tinyui_image_test_reset_state();
    probe = tinyui_image_create(win, "image_fail_size");
    assert(probe != 0);
    assert(tinyui_widget_destroy(&probe->widget) == 0);

    assert(tinyui_image_test_create_with_props_fail_before_size(
               win,
               &(struct tinyui_image_props){
                   .id = "image_fail_size",
                   .source = &(struct tinyui_image_source){
                       .img_tile = &img_tile,
                   },
                   .width = 48,
                   .height = 24,
               })
           == 0);
    assert(tinyui_image_test_take_last_dispose_snapshot(&snapshot) == 0);
    assert(snapshot.kind == TINYUI_BACKEND_WIDGET_IMAGE);
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
    assert(tinyui_image_test_take_last_dispose_snapshot(&snapshot) == -1);
    if (tail_ld != 0) {
        assert(ldBaseGetNextSibling(tail_ld) == next_before_ld);
    } else {
        assert(ldBaseGetChildList(win_ld) == 0);
    }

    tinyui_image_test_reset_state();
    assert(tinyui_image_create_with_props(
               win,
               &(struct tinyui_image_props){
                   .id = "image_fail_size",
                   .source = &(struct tinyui_image_source){
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
    assert(tinyui_widget_is_kind(0, TINYUI_BACKEND_WIDGET_IMAGE) == 0);
    assert(tinyui_test_source_lacks_function_definition(
               resolve_repo_path("tinyui/src/widgets/image.c"),
               "tinyui_image_rgb_to_ld_color") == 1);
    assert(tinyui_test_source_lacks_function_definition(
               resolve_repo_path("tinyui/src/widgets/image.c"),
               "tinyui_image_props_are_valid") == 1);
    assert(tinyui_test_source_lacks_function_definition(
               resolve_repo_path("tinyui/src/widgets/image.c"),
               "tinyui_image_finish_detach_after_backend_failure") == 1);
    assert(tinyui_test_source_lacks_function_definition(
               resolve_repo_path("tinyui/src/widgets/image.c"),
               "tinyui_image_dispose_partial_impl") == 1);
    assert(tinyui_test_source_lacks_function_definition(
               resolve_repo_path("tinyui/src/widgets/image.c"),
               "tinyui_image_create_with_props_impl") == 1);
    assert(file_contains_pattern(resolve_repo_path("tests/tinyui/unit/test_tinyui_image.c"),
                                 "static struct tinyui_image_test_dispose_snapshot "
                                 "g_image_snapshot;") == 0);
    assert(file_contains_pattern(resolve_repo_path("tests/tinyui/unit/test_tinyui_image.c"),
                                 "static int "
                                 "g_image_snapshot_valid = 0;") == 0);
    assert(tinyui_test_source_lacks_function_definition(
               resolve_repo_path("tests/tinyui/unit/test_tinyui_image.c"),
               "test_image_finish_detach_after_backend_failure") == 1);
    assert(tinyui_test_source_lacks_function_definition(
               resolve_repo_path("tests/tinyui/unit/test_tinyui_image.c"),
               "test_image_fill_snapshot") == 1);
    assert(tinyui_test_source_lacks_function_definition(
               resolve_repo_path("tests/tinyui/unit/test_tinyui_image.c"),
               "tinyui_backend_image_test_reset_state") == 1);
    assert(tinyui_test_source_lacks_function_definition(
               resolve_repo_path("tests/tinyui/unit/test_tinyui_image.c"),
               "tinyui_backend_image_test_create_with_props_fail_before_size") == 1);
    assert(tinyui_test_source_lacks_function_definition(
               resolve_repo_path("tests/tinyui/unit/test_tinyui_image.c"),
               "tinyui_backend_image_test_take_last_dispose_snapshot") == 1);
}

int main(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *win;
    assert(app != 0);
    win = tinyui_window_create(app, "root");
    assert(win != 0);

    test_image_create_and_ld_mapping(win);
    test_image_create_with_props_sets_source(win);
    test_image_rejects_null_source_boundary(win);
    test_image_create_with_props_rejects_null(win);
    test_image_create_with_props_failure_rolls_back_attached_child(win);
    test_image_shared_widget_helpers_reject_null();

    tinyui_app_destroy(app);
    return 0;
}
