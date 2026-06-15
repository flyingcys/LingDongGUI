#include "tinyui_test_support.h"

#include "internal.h"
#include "../../../src/gui/ldBase.h"

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int tinyui_backend_widget_unbind_host(void *backend_widget);
int tinyui_backend_widget_detach_from_parent(void *backend_widget);

uint32_t arm_2d_helper_get_reference_clock_frequency(void)
{
    return 1000000u;
}

static struct tinyui_image_test_dispose_snapshot g_image_snapshot;
static int g_image_snapshot_valid = 0;
static struct tinyui_qrcode_test_dispose_snapshot g_qrcode_snapshot;
static int g_qrcode_snapshot_valid = 0;

static const char *const k_repo_markers[] = {
    "/tests/support/tinyui_test_support.c",
    "/tests/tinyui/unit/",
    "/tests/tinyui/unit/",
};

int tinyui_test_support_stub(void)
{
    return 0;
}

const char *tinyui_test_repo_path_from_file(const char *file, const char *relative_path)
{
    static char path[2048];
    size_t i;

    assert(file != 0);
    assert(relative_path != 0);

    for (i = 0; i < sizeof(k_repo_markers) / sizeof(k_repo_markers[0]); ++i) {
        const char *hit = strstr(file, k_repo_markers[i]);
        if (hit != 0) {
            size_t root_len = (size_t)(hit - file);
            int written;

            written = snprintf(path,
                               sizeof(path),
                               "%.*s/%s",
                               (int)root_len,
                               file,
                               relative_path);
            assert(written > 0 && (size_t)written < sizeof(path));
            return path;
        }
    }

    assert(!"could not infer repository root from file path");
    return 0;
}

static char *tinyui_test_read_file_text(const char *path)
{
    const char *resolved_path = path;
    FILE *fp;
    long size;
    char *buf;

    assert(path != 0);
    fp = fopen(resolved_path, "rb");
    if (fp == 0 && path[0] != '/') {
        resolved_path = tinyui_test_repo_path_from_file(__FILE__, path);
        fp = fopen(resolved_path, "rb");
    }
    assert(fp != 0);
    assert(fseek(fp, 0, SEEK_END) == 0);
    size = ftell(fp);
    assert(size >= 0);
    assert(fseek(fp, 0, SEEK_SET) == 0);

    buf = (char *)malloc((size_t)size + 1U);
    assert(buf != 0);
    assert(fread(buf, 1U, (size_t)size, fp) == (size_t)size);
    buf[size] = '\0';
    assert(fclose(fp) == 0);
    return buf;
}

int tinyui_test_source_contains(const char *path, const char *needle)
{
    char *text;
    int found;

    assert(path != 0);
    assert(needle != 0);
    text = tinyui_test_read_file_text(path);
    found = strstr(text, needle) != 0 ? 1 : 0;
    free(text);
    return found;
}

static int tinyui_test_source_match_function_definition(const char *cursor,
                                                        const char *prefix,
                                                        const char *symbol)
{
    while (*prefix != '\0') {
        if (isspace((unsigned char)*prefix)) {
            if (!isspace((unsigned char)*cursor)) {
                return 0;
            }
            while (isspace((unsigned char)*prefix)) {
                prefix++;
            }
            while (isspace((unsigned char)*cursor)) {
                cursor++;
            }
        } else {
            if (*cursor != *prefix) {
                return 0;
            }
            prefix++;
            cursor++;
        }
    }

    while (isspace((unsigned char)*cursor)) {
        cursor++;
    }
    return strncmp(cursor, symbol, strlen(symbol)) == 0
        && cursor[strlen(symbol)] == '(';
}

int tinyui_test_source_has_function_definition(const char *path, const char *symbol)
{
    static const char *const prefixes[] = {
        "static int ",
        "static void ",
        "static unsigned int ",
        "static ldColor ",
        "static ldArc_t *",
        "static const ldArc_t *",
        "static struct tinyui_backend_widget *",
        "static struct tinyui_arc *",
        "static struct tinyui_gauge *",
        "static struct tinyui_image *",
        "static struct tinyui_qrcode *",
        "int ",
        "void ",
        "unsigned int ",
        "ldColor ",
        "ldArc_t *",
        "const ldArc_t *",
        "struct tinyui_backend_widget *",
        "struct tinyui_arc *",
        "struct tinyui_gauge *",
        "struct tinyui_image *",
        "struct tinyui_qrcode *",
    };
    char needle[256];
    char *text;
    size_t i;

    assert(path != 0);
    assert(symbol != 0);

    text = tinyui_test_read_file_text(path);
    for (i = 0; i < sizeof(prefixes) / sizeof(prefixes[0]); ++i) {
        const char *cursor;
        int written = snprintf(needle, sizeof(needle), "%s%s(", prefixes[i], symbol);
        assert(written > 0 && (size_t)written < sizeof(needle));
        if (strstr(text, needle) != 0) {
            free(text);
            return 1;
        }
        for (cursor = text; *cursor != '\0'; ++cursor) {
            if (tinyui_test_source_match_function_definition(cursor, prefixes[i], symbol) == 1) {
                free(text);
                return 1;
            }
        }
    }
    free(text);
    return 0;
}

int tinyui_test_source_lacks_function_definition(const char *path, const char *symbol)
{
    return tinyui_test_source_has_function_definition(path, symbol) == 1 ? 0 : 1;
}

static int tinyui_test_support_finish_detach_after_backend_failure(
    struct tinyui_backend_widget *backend)
{
    struct tinyui_backend_widget *parent;
    struct tinyui_backend_widget *cursor;

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

static void tinyui_test_support_fill_common_snapshot(
    struct tinyui_backend_widget *backend,
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

void tinyui_backend_image_test_reset_state(void)
{
    memset(&g_image_snapshot, 0, sizeof(g_image_snapshot));
    g_image_snapshot_valid = 0;
}

struct tinyui_image *tinyui_backend_image_test_create_with_props_fail_before_size(
    struct tinyui_window *parent,
    const struct tinyui_image_props *props)
{
    struct tinyui_image *image;
    struct tinyui_backend_widget *backend;
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

    backend = (struct tinyui_backend_widget *)image->widget.backend_widget;
    if (backend == 0) {
        tinyui_widget_destroy(&image->widget);
        return 0;
    }
    if (backend->parent != 0) {
        detach_result = tinyui_backend_widget_detach_from_parent(backend);
        if (detach_result != 0) {
            detach_result = tinyui_test_support_finish_detach_after_backend_failure(backend);
        }
    }
    unbind_result = tinyui_backend_widget_unbind_host(backend);
    tinyui_test_support_fill_common_snapshot(
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
    tinyui_widget_destroy(&image->widget);
    return 0;
}

int tinyui_backend_image_test_take_last_dispose_snapshot(
    struct tinyui_image_test_dispose_snapshot *snapshot)
{
    if (snapshot == 0 || g_image_snapshot_valid == 0) {
        return -1;
    }

    *snapshot = g_image_snapshot;
    memset(&g_image_snapshot, 0, sizeof(g_image_snapshot));
    g_image_snapshot_valid = 0;
    return 0;
}

void tinyui_backend_qrcode_test_reset_state(void)
{
    memset(&g_qrcode_snapshot, 0, sizeof(g_qrcode_snapshot));
    g_qrcode_snapshot_valid = 0;
}

struct tinyui_qrcode *tinyui_backend_qrcode_test_create_with_props_fail_before_text(
    struct tinyui_widget *parent,
    const struct tinyui_qrcode_props *props)
{
    struct tinyui_qrcode *qrcode;
    struct tinyui_backend_widget *backend;
    int detach_result = 0;
    int unbind_result;

    if (parent == 0 || props == 0) {
        return 0;
    }

    qrcode = tinyui_qrcode_create(parent, props->id);
    if (qrcode == 0) {
        return 0;
    }
    if ((props->style_class != 0
         && tinyui_widget_set_style_class(&qrcode->widget, props->style_class) != 0)
        || tinyui_widget_set_user_data(&qrcode->widget, props->user_data) != 0) {
        tinyui_widget_destroy(&qrcode->widget);
        return 0;
    }

    backend = (struct tinyui_backend_widget *)qrcode->widget.backend_widget;
    if (backend == 0) {
        tinyui_widget_destroy(&qrcode->widget);
        return 0;
    }
    if (backend->parent != 0) {
        detach_result = tinyui_backend_widget_detach_from_parent(backend);
        if (detach_result != 0) {
            detach_result = tinyui_test_support_finish_detach_after_backend_failure(backend);
        }
    }
    unbind_result = tinyui_backend_widget_unbind_host(backend);
    tinyui_test_support_fill_common_snapshot(
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
    tinyui_widget_destroy(&qrcode->widget);
    return 0;
}

int tinyui_backend_qrcode_test_take_last_dispose_snapshot(
    struct tinyui_qrcode_test_dispose_snapshot *snapshot)
{
    if (snapshot == 0 || g_qrcode_snapshot_valid == 0) {
        return -1;
    }

    *snapshot = g_qrcode_snapshot;
    memset(&g_qrcode_snapshot, 0, sizeof(g_qrcode_snapshot));
    g_qrcode_snapshot_valid = 0;
    return 0;
}
