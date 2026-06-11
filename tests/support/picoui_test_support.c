#include "picoui_test_support.h"

#include "internal.h"
#include "../../../src/gui/ldBase.h"

#include <string.h>

int picoui_backend_widget_unbind_host(void *backend_widget);
int picoui_backend_widget_detach_from_parent(void *backend_widget);

static struct picoui_image_test_dispose_snapshot g_image_snapshot;
static int g_image_snapshot_valid = 0;
static struct picoui_qrcode_test_dispose_snapshot g_qrcode_snapshot;
static int g_qrcode_snapshot_valid = 0;

int picoui_test_support_stub(void)
{
    return 0;
}

static int picoui_test_support_finish_detach_after_backend_failure(
    struct picoui_backend_widget *backend)
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

static void picoui_test_support_fill_common_snapshot(
    struct picoui_backend_widget *backend,
    int detach_result,
    int unbind_result,
    int kind,
    int *valid,
    int *kind_out,
    int *cleanup_complete,
    int *cleanup_incomplete,
    int *detach_out,
    int *unbind_out,
    int *detached,
    int *owner_cleared,
    int *root_cleared,
    int *parent_cleared,
    int *next_sibling_cleared,
    int *host_cleared,
    int *event_bridge_cleared,
    int *ld_pinfo_cleared)
{
    ldBase_t *ld_base = backend != 0 ? (ldBase_t *)backend->ld_widget : 0;

    *kind_out = kind;
    *cleanup_complete = (detach_result == 0 && unbind_result == 0);
    *cleanup_incomplete = (detach_result != 0 || unbind_result != 0);
    *detach_out = detach_result;
    *unbind_out = unbind_result;
    *detached = (detach_result == 0 && backend != 0 && backend->parent == 0);
    *owner_cleared = (backend != 0 && backend->owner == 0);
    *root_cleared = (backend != 0 && backend->root == 0);
    *parent_cleared = (backend != 0 && backend->parent == 0);
    *next_sibling_cleared = (backend != 0 && backend->next_sibling == 0);
    *host_cleared = (backend != 0 && backend->host_widget == 0);
    *event_bridge_cleared = (backend != 0
        && backend->ld_event_bridge_scene == 0
        && backend->ld_event_bridge_sender == 0
        && backend->ld_event_bridge_next == 0);
    *ld_pinfo_cleared = (ld_base == 0 || ld_base->pInfo == 0);
    *valid = 1;
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
        detach_result = picoui_backend_widget_detach_from_parent(backend);
        if (detach_result != 0) {
            detach_result = picoui_test_support_finish_detach_after_backend_failure(backend);
        }
    }
    unbind_result = picoui_backend_widget_unbind_host(backend);
    picoui_test_support_fill_common_snapshot(
        backend,
        detach_result,
        unbind_result,
        backend->kind,
        &g_image_snapshot_valid,
        &g_image_snapshot.kind,
        &g_image_snapshot.cleanup_complete,
        &g_image_snapshot.cleanup_incomplete,
        &g_image_snapshot.detach_result,
        &g_image_snapshot.unbind_result,
        &g_image_snapshot.detached,
        &g_image_snapshot.owner_cleared,
        &g_image_snapshot.root_cleared,
        &g_image_snapshot.parent_cleared,
        &g_image_snapshot.next_sibling_cleared,
        &g_image_snapshot.host_cleared,
        &g_image_snapshot.event_bridge_cleared,
        &g_image_snapshot.ld_pinfo_cleared);
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

void picoui_backend_qrcode_test_reset_state(void)
{
    memset(&g_qrcode_snapshot, 0, sizeof(g_qrcode_snapshot));
    g_qrcode_snapshot_valid = 0;
}

struct picoui_qrcode *picoui_backend_qrcode_test_create_with_props_fail_before_text(
    struct picoui_widget *parent,
    const struct picoui_qrcode_props *props)
{
    struct picoui_qrcode *qrcode;
    struct picoui_backend_widget *backend;
    int detach_result = 0;
    int unbind_result;

    if (parent == 0 || props == 0) {
        return 0;
    }

    qrcode = picoui_qrcode_create(parent, props->id);
    if (qrcode == 0) {
        return 0;
    }
    if ((props->style_class != 0
         && picoui_widget_set_style_class(&qrcode->widget, props->style_class) != 0)
        || picoui_widget_set_user_data(&qrcode->widget, props->user_data) != 0) {
        picoui_widget_destroy(&qrcode->widget);
        return 0;
    }

    backend = (struct picoui_backend_widget *)qrcode->widget.backend_widget;
    if (backend == 0) {
        picoui_widget_destroy(&qrcode->widget);
        return 0;
    }
    if (backend->parent != 0) {
        detach_result = picoui_backend_widget_detach_from_parent(backend);
        if (detach_result != 0) {
            detach_result = picoui_test_support_finish_detach_after_backend_failure(backend);
        }
    }
    unbind_result = picoui_backend_widget_unbind_host(backend);
    picoui_test_support_fill_common_snapshot(
        backend,
        detach_result,
        unbind_result,
        backend->kind,
        &g_qrcode_snapshot_valid,
        &g_qrcode_snapshot.kind,
        &g_qrcode_snapshot.cleanup_complete,
        &g_qrcode_snapshot.cleanup_incomplete,
        &g_qrcode_snapshot.detach_result,
        &g_qrcode_snapshot.unbind_result,
        &g_qrcode_snapshot.detached,
        &g_qrcode_snapshot.owner_cleared,
        &g_qrcode_snapshot.root_cleared,
        &g_qrcode_snapshot.parent_cleared,
        &g_qrcode_snapshot.next_sibling_cleared,
        &g_qrcode_snapshot.host_cleared,
        &g_qrcode_snapshot.event_bridge_cleared,
        &g_qrcode_snapshot.ld_pinfo_cleared);
    picoui_widget_destroy(&qrcode->widget);
    return 0;
}

int picoui_backend_qrcode_test_take_last_dispose_snapshot(
    struct picoui_qrcode_test_dispose_snapshot *snapshot)
{
    if (snapshot == 0 || g_qrcode_snapshot_valid == 0) {
        return -1;
    }

    *snapshot = g_qrcode_snapshot;
    memset(&g_qrcode_snapshot, 0, sizeof(g_qrcode_snapshot));
    g_qrcode_snapshot_valid = 0;
    return 0;
}
