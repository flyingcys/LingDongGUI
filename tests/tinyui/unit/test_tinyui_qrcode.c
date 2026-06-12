#include "app.h"
#include "picoui/qrcode.h"
#include "widget.h"
#include "window.h"
#include "../../../src/gui/ldQRCode.h"
#include "internal.h"
#include "picoui_test_support.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int tinyui_widget_has_ld_binding(const struct picoui_widget *widget);

static struct picoui_qrcode_test_dispose_snapshot g_tinyui_qrcode_snapshot;
static int g_tinyui_qrcode_snapshot_valid = 0;
static const char *test_source_file_path = __FILE__;

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

static void assert_source_lacks_function_definition(const char *source_path, const char *symbol)
{
    char command[1024];

    assert(source_path != 0);
    assert(symbol != 0);
    snprintf(command,
             sizeof(command),
             "python3 - '%s' '%s' <<'PY'\n"
             "from pathlib import Path\n"
             "import re\n"
             "import sys\n"
             "text = Path(sys.argv[1]).read_text()\n"
             "symbol = sys.argv[2]\n"
             "pattern = re.compile(r'(^|\\n)\\s*(?:static\\s+)?[A-Za-z_][A-Za-z0-9_\\s\\*]*\\b' + re.escape(symbol) + r'\\s*\\(', re.MULTILINE)\n"
             "raise SystemExit(1 if pattern.search(text) else 0)\n"
             "PY",
             source_path,
             symbol);
    assert(system(command) == 0);
}

static int tinyui_qrcode_finish_detach_after_backend_failure(struct picoui_backend_widget *backend)
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

static void tinyui_qrcode_fill_snapshot(struct picoui_backend_widget *backend,
                                        int detach_result,
                                        int unbind_result)
{
    ldBase_t *ld_base = backend != 0 ? (ldBase_t *)backend->ld_widget : 0;

    g_tinyui_qrcode_snapshot.kind = backend != 0 ? backend->kind : -1;
    g_tinyui_qrcode_snapshot.cleanup_complete = (detach_result == 0 && unbind_result == 0);
    g_tinyui_qrcode_snapshot.cleanup_incomplete = (detach_result != 0 || unbind_result != 0);
    g_tinyui_qrcode_snapshot.detach_result = detach_result;
    g_tinyui_qrcode_snapshot.unbind_result = unbind_result;
    g_tinyui_qrcode_snapshot.detached = (detach_result == 0 && backend != 0 && backend->parent == 0);
    g_tinyui_qrcode_snapshot.owner_cleared = (backend != 0 && backend->owner == 0);
    g_tinyui_qrcode_snapshot.root_cleared = (backend != 0 && backend->root == 0);
    g_tinyui_qrcode_snapshot.parent_cleared = (backend != 0 && backend->parent == 0);
    g_tinyui_qrcode_snapshot.next_sibling_cleared = (backend != 0 && backend->next_sibling == 0);
    g_tinyui_qrcode_snapshot.host_cleared = (backend != 0 && backend->host_widget == 0);
    g_tinyui_qrcode_snapshot.event_bridge_cleared = (backend != 0
        && backend->ld_event_bridge_scene == 0
        && backend->ld_event_bridge_sender == 0
        && backend->ld_event_bridge_next == 0);
    g_tinyui_qrcode_snapshot.ld_pinfo_cleared = (ld_base == 0 || ld_base->pInfo == 0);
    g_tinyui_qrcode_snapshot_valid = 1;
}

void tinyui_qrcode_test_reset_state(void)
{
    memset(&g_tinyui_qrcode_snapshot, 0, sizeof(g_tinyui_qrcode_snapshot));
    g_tinyui_qrcode_snapshot_valid = 0;
}

struct picoui_qrcode *tinyui_qrcode_test_create_with_props_fail_before_text(
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
        detach_result = tinyui_runtime_bridge_detach_from_parent(backend);
        if (detach_result != 0) {
            detach_result = tinyui_qrcode_finish_detach_after_backend_failure(backend);
        }
    }
    unbind_result = tinyui_runtime_bridge_unbind_host(backend);
    tinyui_qrcode_fill_snapshot(backend, detach_result, unbind_result);
    picoui_widget_destroy(&qrcode->widget);
    return 0;
}

int tinyui_qrcode_test_take_last_dispose_snapshot(
    struct picoui_qrcode_test_dispose_snapshot *snapshot)
{
    if (snapshot == 0 || g_tinyui_qrcode_snapshot_valid == 0) {
        return -1;
    }

    *snapshot = g_tinyui_qrcode_snapshot;
    memset(&g_tinyui_qrcode_snapshot, 0, sizeof(g_tinyui_qrcode_snapshot));
    g_tinyui_qrcode_snapshot_valid = 0;
    return 0;
}

static void test_qrcode_create_and_props(struct picoui_window *win)
{
    int user_cookie = 7;
    struct picoui_qrcode_props props = {
        .id = "qr_props",
        .style_class = "qr-code",
        .user_data = &user_cookie,
        .text = "https://example.local/props",
    };
    struct picoui_qrcode *qrcode = picoui_qrcode_create((struct picoui_widget *)win, "qr");
    struct picoui_qrcode *with_props = picoui_qrcode_create_with_props((struct picoui_widget *)win, &props);
    struct picoui_backend_widget *backend;
    struct picoui_backend_widget *parent_backend;
    ldQRCode_t *ld_qrcode;

    assert(qrcode != 0);
    assert(with_props != 0);
    backend = (struct picoui_backend_widget *)qrcode->widget.backend_widget;
    assert(backend != 0);
    parent_backend = (struct picoui_backend_widget *)win->widget.backend_widget;
    assert(parent_backend != 0);
    assert(backend->kind == PICOUI_BACKEND_WIDGET_QRCODE);
    assert(backend->owner == parent_backend->owner);
    assert(backend->root == parent_backend->root);
    assert(backend->parent == parent_backend);
    assert(backend->ld_name_id != 0);
    assert(backend->host_widget == &qrcode->widget);
    assert(backend->ld_event_bridge_scene != 0);
    assert(backend->ld_event_bridge_sender == backend->ld_widget);
    ld_qrcode = (ldQRCode_t *)backend->ld_widget;
    assert(ld_qrcode != 0);
    assert(((ldBase_t *)ld_qrcode)->pInfo == backend);
    assert(tinyui_widget_has_ld_binding(&qrcode->widget) == 1);
    assert(picoui_qrcode_get_text(qrcode) != 0);
    assert(strcmp(picoui_qrcode_get_text(qrcode), "") == 0);
    assert(picoui_qrcode_get_text(with_props) != 0);
    assert(strcmp(picoui_qrcode_get_text(with_props), props.text) == 0);
}

static void test_qrcode_set_get_text(struct picoui_window *win)
{
    struct picoui_qrcode *qrcode = picoui_qrcode_create((struct picoui_widget *)win, "qr_text");
    const char *value = "https://example.local/qrcode";

    assert(qrcode != 0);
    assert(picoui_qrcode_set_text(qrcode, value) == 0);
    assert(picoui_qrcode_get_text(qrcode) != 0);
    assert(strcmp(picoui_qrcode_get_text(qrcode), value) == 0);
}

static void test_qrcode_rejects_invalid_inputs(struct picoui_window *win)
{
    struct picoui_qrcode *qrcode = picoui_qrcode_create((struct picoui_widget *)win, "qr_invalid");

    assert(qrcode != 0);
    assert(picoui_qrcode_create(0, "qr") == 0);
    assert(picoui_qrcode_create((struct picoui_widget *)win, 0) == 0);
    assert(picoui_qrcode_create_with_props(0,
                                           &(struct picoui_qrcode_props){
                                               .id = "bad_parent",
                                               .text = "abc",
                                           }) == 0);
    assert(picoui_qrcode_create_with_props((struct picoui_widget *)win, 0) == 0);
    assert(picoui_qrcode_create_with_props((struct picoui_widget *)win,
                                           &(struct picoui_qrcode_props){
                                               .text = "abc",
                                           }) == 0);
    assert(picoui_qrcode_create_with_props((struct picoui_widget *)win,
                                           &(struct picoui_qrcode_props){
                                               .id = "bad_text",
                                           }) == 0);
    assert(picoui_qrcode_set_text(0, "abc") == -1);
    assert(picoui_qrcode_set_text(qrcode, 0) == -1);
    assert(picoui_qrcode_get_text(0) == 0);
}

static void test_qrcode_release_contract_covers_configuration_boundary(struct picoui_window *win)
{
    const char *value = "https://example.local/final-release";
    struct picoui_qrcode *qrcode = picoui_qrcode_create_with_props(
        (struct picoui_widget *)win,
        &(struct picoui_qrcode_props){
            .id = "qr_release_ready",
            .style_class = "qr-card",
            .text = value,
        });
    struct picoui_backend_widget *backend;
    ldQRCode_t *ld_qrcode;

    assert(qrcode != 0);
    backend = (struct picoui_backend_widget *)qrcode->widget.backend_widget;
    assert(backend != 0);
    assert(backend->kind == PICOUI_BACKEND_WIDGET_QRCODE);
    assert(backend->style_class == (const char *)"qr-card");
    ld_qrcode = (ldQRCode_t *)backend->ld_widget;
    assert(ld_qrcode != 0);

    assert(strcmp(picoui_qrcode_get_text(qrcode), value) == 0);
    assert(strcmp((const char *)ld_qrcode->pStr, value) == 0);
    assert(ld_qrcode->qrColor == GLCD_COLOR_BLACK);
    assert(ld_qrcode->bgColor == GLCD_COLOR_WHITE);
    assert(ld_qrcode->qrEcc == QR_ECC_7);
    assert(ld_qrcode->qrMaxVersion == 2);
    assert(ld_qrcode->qrZoom == 4);
}

static void test_qrcode_native_color_ecc_version_and_zoom_round_trip(struct picoui_window *win)
{
    struct picoui_qrcode *qrcode = picoui_qrcode_create((struct picoui_widget *)win, "qr_native_config");
    struct picoui_backend_widget *backend;
    ldQRCode_t *ld_qrcode;

    assert(qrcode != 0);
    backend = (struct picoui_backend_widget *)qrcode->widget.backend_widget;
    assert(backend != 0);
    ld_qrcode = (ldQRCode_t *)backend->ld_widget;
    assert(ld_qrcode != 0);

    assert(picoui_qrcode_set_qr_color(qrcode, 0x112233U) == 0);
    assert(picoui_qrcode_set_bg_color(qrcode, 0x445566U) == 0);
    assert(picoui_qrcode_set_ecc(qrcode, 2) == 0);
    assert(picoui_qrcode_set_max_version(qrcode, 5) == 0);
    assert(picoui_qrcode_set_zoom(qrcode, 7) == 0);

    assert(ld_qrcode->qrColor == (ldColor)0x112233U);
    assert(ld_qrcode->bgColor == (ldColor)0x445566U);
    assert(ld_qrcode->qrEcc == 2);
    assert(ld_qrcode->qrMaxVersion == 5);
    assert(ld_qrcode->qrZoom == 7);

    assert(picoui_qrcode_set_qr_color(0, 0x000000U) == -1);
    assert(picoui_qrcode_set_bg_color(0, 0x000000U) == -1);
    assert(picoui_qrcode_set_ecc(qrcode, -1) == -1);
    assert(picoui_qrcode_set_max_version(qrcode, 0) == -1);
    assert(picoui_qrcode_set_zoom(qrcode, 0) == -1);

    assert(ld_qrcode->qrColor == (ldColor)0x112233U);
    assert(ld_qrcode->bgColor == (ldColor)0x445566U);
    assert(ld_qrcode->qrEcc == 2);
    assert(ld_qrcode->qrMaxVersion == 5);
    assert(ld_qrcode->qrZoom == 7);
}

static void test_q_r_code_init_and_shared_base_aliases_round_trip(struct picoui_window *win)
{
    struct picoui_qrcode *qrcode = picoui_q_r_code_init((struct picoui_widget *)win, "qr_alias");
    struct picoui_backend_widget *backend;
    ldQRCode_t *ld_qrcode;

    assert(qrcode != 0);
    backend = (struct picoui_backend_widget *)qrcode->widget.backend_widget;
    assert(backend != 0);
    ld_qrcode = (ldQRCode_t *)backend->ld_widget;
    assert(ld_qrcode != 0);

    assert(picoui_q_r_code_set_text(qrcode, "alias://qrcode") == 0);
    assert(strcmp(picoui_qrcode_get_text(qrcode), "alias://qrcode") == 0);
    assert(strcmp((const char *)ld_qrcode->pStr, "alias://qrcode") == 0);

    assert(picoui_widget_set_pos(&qrcode->widget, 13, 17) == 0);
    assert(((ldBase_t *)ld_qrcode)->use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 13);
    assert(((ldBase_t *)ld_qrcode)->use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 17);

    assert(picoui_widget_set_visible(&qrcode->widget, 0) == 0);
    assert(((ldBase_t *)ld_qrcode)->isHidden == true);
    assert(picoui_widget_set_visible(&qrcode->widget, 1) == 0);
    assert(((ldBase_t *)ld_qrcode)->isHidden == false);

    assert(picoui_widget_set_opacity(&qrcode->widget, 77) == 0);
    assert(((ldBase_t *)ld_qrcode)->opacity == 77);

    assert(picoui_widget_set_selectable(&qrcode->widget, 0) == 0);
    assert(((ldBase_t *)ld_qrcode)->isSelectable == false);
    assert(picoui_widget_set_selectable(&qrcode->widget, 1) == 0);
    assert(((ldBase_t *)ld_qrcode)->isSelectable == true);

    assert(picoui_widget_set_selected(&qrcode->widget, 1) == 0);
    assert(((ldBase_t *)ld_qrcode)->isSelected == true);

    assert(picoui_widget_set_corner(&qrcode->widget, 9) == 0);
    assert(((ldBase_t *)ld_qrcode)->isCorner == true);
}

static void test_qrcode_rejects_null_args(struct picoui_window *win)
{
    assert(picoui_qrcode_create(0, "id") == 0);
    assert(picoui_qrcode_create((struct picoui_widget *)win, 0) == 0);
    assert(picoui_qrcode_set_text(0, "text") == -1);
    assert(picoui_qrcode_set_ecc(0, 1) == -1);
    assert(picoui_qrcode_set_zoom(0, 2) == -1);
    assert(picoui_qrcode_set_max_version(0, 10) == -1);
    assert(picoui_qrcode_set_qr_color(0, 0x000000U) == -1);
    assert(picoui_qrcode_set_bg_color(0, 0xFFFFFFU) == -1);
}

static void test_qrcode_create_with_props_failure_rolls_back_attached_child(struct picoui_window *win)
{
    struct picoui_backend_widget *parent_backend =
        (struct picoui_backend_widget *)win->widget.backend_widget;
    struct picoui_backend_widget *tail = parent_backend->first_child;
    struct picoui_backend_widget *next_before = 0;
    struct picoui_qrcode *probe;
    struct picoui_qrcode_test_dispose_snapshot snapshot = {0};

    while (tail != 0 && tail->next_sibling != 0) {
        tail = tail->next_sibling;
    }
    if (tail != 0) {
        next_before = tail->next_sibling;
    }

    tinyui_qrcode_test_reset_state();
    probe = picoui_qrcode_create((struct picoui_widget *)win, "qr_fail_text");
    assert(probe != 0);
    assert(picoui_widget_destroy(&probe->widget) == 0);

    assert(tinyui_qrcode_test_create_with_props_fail_before_text(
               (struct picoui_widget *)win,
               &(struct picoui_qrcode_props){
                   .id = "qr_fail_text",
                   .style_class = "qr-fail",
                   .text = "https://example.local/fail",
               })
           == 0);
    assert(tinyui_qrcode_test_take_last_dispose_snapshot(&snapshot) == 0);
    assert(snapshot.kind == PICOUI_BACKEND_WIDGET_QRCODE);
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
    assert(tinyui_qrcode_test_take_last_dispose_snapshot(&snapshot) == -1);
    if (tail != 0) {
        assert(tail->next_sibling == next_before);
    } else {
        assert(parent_backend->first_child == 0);
    }

    tinyui_qrcode_test_reset_state();
    assert(picoui_qrcode_create_with_props(
               (struct picoui_widget *)win,
               &(struct picoui_qrcode_props){
                   .id = "qr_fail_text",
                   .style_class = "qr-fail",
                   .text = "https://example.local/fail",
               })
           != 0);
}

static void test_qrcode_shared_widget_helpers_reject_null(void)
{
    const char *qrcode_source_path = resolve_repo_path("tinyui/src/widgets/qrcode.c");
    const char *test_source_path = resolve_repo_path("tests/tinyui/unit/test_tinyui_qrcode.c");

    assert(tinyui_runtime_bridge_unbind_host(0) == -1);
    assert(tinyui_runtime_bridge_detach_from_parent(0) == -1);
    assert(tinyui_widget_is_kind(0, PICOUI_BACKEND_WIDGET_QRCODE) == 0);
    assert_source_lacks_function_definition(qrcode_source_path,
                                            "picoui_qrcode_props_are_valid");
    assert_source_lacks_function_definition(qrcode_source_path,
                                            "picoui_qrcode_get_ld");
    assert_source_lacks_function_definition(qrcode_source_path,
                                            "picoui_qrcode_finish_detach_after_backend_failure");
    assert_source_lacks_function_definition(qrcode_source_path,
                                            "picoui_qrcode_dispose_partial_impl");
    assert_source_lacks_function_definition(qrcode_source_path,
                                            "picoui_qrcode_create_with_props_impl");
    assert_source_lacks_function_definition(test_source_path,
                                            "picoui_backend_qrcode_test_reset_state");
    assert_source_lacks_function_definition(test_source_path,
                                            "picoui_backend_qrcode_test_create_with_props_fail_before_text");
    assert_source_lacks_function_definition(test_source_path,
                                            "picoui_backend_qrcode_test_take_last_dispose_snapshot");
}

int main(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_window *win;

    assert(app != 0);
    win = picoui_window_create(app, "root");
    assert(win != 0);

    test_qrcode_create_and_props(win);
    test_qrcode_set_get_text(win);
    test_qrcode_rejects_invalid_inputs(win);
    test_qrcode_release_contract_covers_configuration_boundary(win);
    test_qrcode_native_color_ecc_version_and_zoom_round_trip(win);
    test_q_r_code_init_and_shared_base_aliases_round_trip(win);
    test_qrcode_rejects_null_args(win);
    test_qrcode_create_with_props_failure_rolls_back_attached_child(win);
    test_qrcode_shared_widget_helpers_reject_null();

    picoui_app_destroy(app);
    return 0;
}
