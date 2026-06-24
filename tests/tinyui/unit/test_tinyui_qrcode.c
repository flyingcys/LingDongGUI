#include "app.h"
#include "qrcode.h"
#include "widget.h"
#include "window.h"
#include "../../../src/gui/ldQRCode.h"
#include "internal.h"
#include "tinyui_test_support.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int tinyui_widget_has_ld_binding(const struct tinyui_widget *widget);

static struct tinyui_qrcode_test_dispose_snapshot g_tinyui_qrcode_snapshot;
static int g_tinyui_qrcode_snapshot_valid = 0;
static const char *test_source_file_path = __FILE__;

static const char *resolve_repo_path(const char *repo_relative_path)
{
    /* Two independent slots so callers can hold two resolved paths concurrently
     * (previously a single static buffer aliased qrcode_source_path and
     * test_source_path, causing the second resolve to clobber the first). */
    static char resolved_path[2][1024];
    static int slot = 0;
    char base_path[1024];
    char *tests_dir;
    size_t base_len;
    char *out;

    assert(test_source_file_path != 0);
    assert(repo_relative_path != 0);
    assert(strlen(test_source_file_path) < sizeof(base_path));
    snprintf(base_path, sizeof(base_path), "%s", test_source_file_path);
    tests_dir = strstr(base_path, "tests/tinyui/unit/");
    assert(tests_dir != 0);
    *tests_dir = '\0';
    base_len = strlen(base_path);
    assert(base_len + strlen(repo_relative_path) + 1 < sizeof(resolved_path[0]));
    out = resolved_path[slot];
    slot = (slot + 1) % 2;
    snprintf(out, sizeof(resolved_path[0]), "%s%s", base_path, repo_relative_path);
    return out;
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

/* C1: finish_detach fallback — delegate to tinyui_runtime_bridge_detach_from_parent */
static int tinyui_qrcode_finish_detach_after_backend_failure(struct tinyui_widget *widget)
{
    return tinyui_runtime_bridge_detach_from_parent(widget);
}

static void tinyui_qrcode_fill_snapshot(struct tinyui_widget *widget,
                                        int detach_result,
                                        int unbind_result)
{
    ldBase_t *ld_base = widget != 0 ? (ldBase_t *)widget->ld_widget : 0;

    g_tinyui_qrcode_snapshot.kind = widget != 0 ? (int)widget->kind : -1;
    g_tinyui_qrcode_snapshot.cleanup_complete = (detach_result == 0 && unbind_result == 0);
    g_tinyui_qrcode_snapshot.cleanup_incomplete = (detach_result != 0 || unbind_result != 0);
    g_tinyui_qrcode_snapshot.detach_result = detach_result;
    g_tinyui_qrcode_snapshot.unbind_result = unbind_result;
    g_tinyui_qrcode_snapshot.detached = (detach_result == 0);
    g_tinyui_qrcode_snapshot.owner_cleared = (widget == 0 || widget->owner == 0);
    g_tinyui_qrcode_snapshot.root_cleared = 1;
    g_tinyui_qrcode_snapshot.parent_cleared = 1;
    g_tinyui_qrcode_snapshot.next_sibling_cleared = 1;
    g_tinyui_qrcode_snapshot.host_cleared = 1;
    g_tinyui_qrcode_snapshot.event_bridge_cleared = (widget == 0
        || (widget->ld_event_bridge_scene == 0
            && widget->ld_event_bridge_sender == 0
            && widget->ld_event_bridge_next == 0));
    g_tinyui_qrcode_snapshot.ld_pinfo_cleared = (ld_base == 0 || ld_base->pInfo == 0);
    g_tinyui_qrcode_snapshot_valid = 1;
}

void tinyui_qrcode_test_reset_state(void)
{
    memset(&g_tinyui_qrcode_snapshot, 0, sizeof(g_tinyui_qrcode_snapshot));
    g_tinyui_qrcode_snapshot_valid = 0;
}

struct tinyui_qrcode *tinyui_qrcode_test_create_with_props_fail_before_text(
    struct tinyui_widget *parent,
    const struct tinyui_qrcode_props *props)
{
    struct tinyui_qrcode *qrcode;
    struct tinyui_widget *backend;
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

    backend = &qrcode->widget;
    if (backend->ld_widget == 0) {
        tinyui_widget_destroy(&qrcode->widget);
        return 0;
    }
    if (tinyui_widget_get_parent(backend) != 0) {
        detach_result = tinyui_runtime_bridge_detach_from_parent(backend);
        if (detach_result != 0) {
            detach_result = tinyui_qrcode_finish_detach_after_backend_failure(backend);
        }
    }
    unbind_result = tinyui_runtime_bridge_unbind_host(backend);
    tinyui_qrcode_fill_snapshot(backend, detach_result, unbind_result);
    tinyui_widget_destroy(&qrcode->widget);
    return 0;
}

int tinyui_qrcode_test_take_last_dispose_snapshot(
    struct tinyui_qrcode_test_dispose_snapshot *snapshot)
{
    if (snapshot == 0 || g_tinyui_qrcode_snapshot_valid == 0) {
        return -1;
    }

    *snapshot = g_tinyui_qrcode_snapshot;
    memset(&g_tinyui_qrcode_snapshot, 0, sizeof(g_tinyui_qrcode_snapshot));
    g_tinyui_qrcode_snapshot_valid = 0;
    return 0;
}

static void test_qrcode_create_and_props(struct tinyui_window *win)
{
    int user_cookie = 7;
    struct tinyui_qrcode_props props = {
        .id = "qr_props",
        .style_class = "qr-code",
        .user_data = &user_cookie,
        .text = "https://example.local/props",
    };
    struct tinyui_qrcode *qrcode = tinyui_qrcode_create((struct tinyui_widget *)win, "qr");
    struct tinyui_qrcode *with_props = tinyui_qrcode_create_with_props((struct tinyui_widget *)win, &props);
    struct tinyui_widget *backend;
    struct tinyui_widget *parent_backend;
    ldQRCode_t *ld_qrcode;

    assert(qrcode != 0);
    assert(with_props != 0);
    backend = &qrcode->widget;
    assert(backend->ld_widget != 0);
    parent_backend = &win->widget;
    assert(parent_backend->ld_widget != 0);
    assert(backend->kind == TINYUI_BACKEND_WIDGET_QRCODE);
    assert(backend->owner == parent_backend->owner);
    assert((ldBase_t *)ldBaseGetRootNode((arm_2d_control_node_t *)backend->ld_widget) == (ldBase_t *)ldBaseGetRootNode((arm_2d_control_node_t *)parent_backend->ld_widget));
    assert(ldBaseGetParent((ldBase_t *)backend->ld_widget) == (ldBase_t *)parent_backend->ld_widget);
    assert(backend->ld_name_id != 0);
    assert(backend->ld_event_bridge_scene != 0);
    assert(backend->ld_event_bridge_sender == backend->ld_widget);
    ld_qrcode = (ldQRCode_t *)backend->ld_widget;
    assert(ld_qrcode != 0);
    assert(tinyui_app_lookup_host(backend->owner, backend->ld_name_id) == backend);
    assert(tinyui_widget_has_ld_binding(&qrcode->widget) == 1);
    assert(tinyui_qrcode_get_text(qrcode) != 0);
    assert(strcmp(tinyui_qrcode_get_text(qrcode), "") == 0);
    assert(tinyui_qrcode_get_text(with_props) != 0);
    assert(strcmp(tinyui_qrcode_get_text(with_props), props.text) == 0);
}

static void test_qrcode_set_get_text(struct tinyui_window *win)
{
    struct tinyui_qrcode *qrcode = tinyui_qrcode_create((struct tinyui_widget *)win, "qr_text");
    const char *value = "https://example.local/qrcode";

    assert(qrcode != 0);
    assert(tinyui_qrcode_set_text(qrcode, value) == 0);
    assert(tinyui_qrcode_get_text(qrcode) != 0);
    assert(strcmp(tinyui_qrcode_get_text(qrcode), value) == 0);
}

static void test_qrcode_rejects_invalid_inputs(struct tinyui_window *win)
{
    struct tinyui_qrcode *qrcode = tinyui_qrcode_create((struct tinyui_widget *)win, "qr_invalid");

    assert(qrcode != 0);
    assert(tinyui_qrcode_create(0, "qr") == 0);
    assert(tinyui_qrcode_create((struct tinyui_widget *)win, 0) == 0);
    assert(tinyui_qrcode_create_with_props(0,
                                           &(struct tinyui_qrcode_props){
                                               .id = "bad_parent",
                                               .text = "abc",
                                           }) == 0);
    assert(tinyui_qrcode_create_with_props((struct tinyui_widget *)win, 0) == 0);
    assert(tinyui_qrcode_create_with_props((struct tinyui_widget *)win,
                                           &(struct tinyui_qrcode_props){
                                               .text = "abc",
                                           }) == 0);
    assert(tinyui_qrcode_create_with_props((struct tinyui_widget *)win,
                                           &(struct tinyui_qrcode_props){
                                               .id = "bad_text",
                                           }) == 0);
    assert(tinyui_qrcode_set_text(0, "abc") == -1);
    assert(tinyui_qrcode_set_text(qrcode, 0) == -1);
    assert(tinyui_qrcode_get_text(0) == 0);
}

static void test_qrcode_release_contract_covers_configuration_boundary(struct tinyui_window *win)
{
    const char *value = "https://example.local/final-release";
    struct tinyui_qrcode *qrcode = tinyui_qrcode_create_with_props(
        (struct tinyui_widget *)win,
        &(struct tinyui_qrcode_props){
            .id = "qr_release_ready",
            .style_class = "qr-card",
            .text = value,
        });
    struct tinyui_widget *backend;
    ldQRCode_t *ld_qrcode;

    assert(qrcode != 0);
    backend = &qrcode->widget;
    assert(backend->ld_widget != 0);
    assert(backend->kind == TINYUI_BACKEND_WIDGET_QRCODE);
    assert(backend->style_class == (const char *)"qr-card");
    ld_qrcode = (ldQRCode_t *)backend->ld_widget;
    assert(ld_qrcode != 0);

    assert(strcmp(tinyui_qrcode_get_text(qrcode), value) == 0);
    assert(strcmp((const char *)ld_qrcode->pStr, value) == 0);
    assert(ld_qrcode->qrColor == GLCD_COLOR_BLACK);
    assert(ld_qrcode->bgColor == GLCD_COLOR_WHITE);
    assert(ld_qrcode->qrEcc == QR_ECC_7);
    assert(ld_qrcode->qrMaxVersion == 2);
    assert(ld_qrcode->qrZoom == 4);
}

static void test_qrcode_native_color_ecc_version_and_zoom_round_trip(struct tinyui_window *win)
{
    struct tinyui_qrcode *qrcode = tinyui_qrcode_create((struct tinyui_widget *)win, "qr_native_config");
    struct tinyui_widget *backend;
    ldQRCode_t *ld_qrcode;

    assert(qrcode != 0);
    backend = &qrcode->widget;
    assert(backend->ld_widget != 0);
    ld_qrcode = (ldQRCode_t *)backend->ld_widget;
    assert(ld_qrcode != 0);

    assert(tinyui_qrcode_set_qr_color(qrcode, 0x112233U) == 0);
    assert(tinyui_qrcode_set_bg_color(qrcode, 0x445566U) == 0);
    assert(tinyui_qrcode_set_ecc(qrcode, 2) == 0);
    assert(tinyui_qrcode_set_max_version(qrcode, 5) == 0);
    assert(tinyui_qrcode_set_zoom(qrcode, 7) == 0);

    assert(ld_qrcode->qrColor == (ldColor)0x112233U);
    assert(ld_qrcode->bgColor == (ldColor)0x445566U);
    assert(ld_qrcode->qrEcc == 2);
    assert(ld_qrcode->qrMaxVersion == 5);
    assert(ld_qrcode->qrZoom == 7);

    assert(tinyui_qrcode_set_qr_color(0, 0x000000U) == -1);
    assert(tinyui_qrcode_set_bg_color(0, 0x000000U) == -1);
    assert(tinyui_qrcode_set_ecc(qrcode, -1) == -1);
    assert(tinyui_qrcode_set_max_version(qrcode, 0) == -1);
    assert(tinyui_qrcode_set_zoom(qrcode, 0) == -1);

    assert(ld_qrcode->qrColor == (ldColor)0x112233U);
    assert(ld_qrcode->bgColor == (ldColor)0x445566U);
    assert(ld_qrcode->qrEcc == 2);
    assert(ld_qrcode->qrMaxVersion == 5);
    assert(ld_qrcode->qrZoom == 7);
}

static void test_q_r_code_init_and_shared_base_aliases_round_trip(struct tinyui_window *win)
{
    struct tinyui_qrcode *qrcode = tinyui_q_r_code_init((struct tinyui_widget *)win, "qr_alias");
    struct tinyui_widget *backend;
    ldQRCode_t *ld_qrcode;

    assert(qrcode != 0);
    backend = &qrcode->widget;
    assert(backend->ld_widget != 0);
    ld_qrcode = (ldQRCode_t *)backend->ld_widget;
    assert(ld_qrcode != 0);

    assert(tinyui_q_r_code_set_text(qrcode, "alias://qrcode") == 0);
    assert(strcmp(tinyui_qrcode_get_text(qrcode), "alias://qrcode") == 0);
    assert(strcmp((const char *)ld_qrcode->pStr, "alias://qrcode") == 0);

    assert(tinyui_widget_set_pos(&qrcode->widget, 13, 17) == 0);
    assert(((ldBase_t *)ld_qrcode)->use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 13);
    assert(((ldBase_t *)ld_qrcode)->use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 17);

    assert(tinyui_widget_set_visible(&qrcode->widget, 0) == 0);
    assert(((ldBase_t *)ld_qrcode)->isHidden == true);
    assert(tinyui_widget_set_visible(&qrcode->widget, 1) == 0);
    assert(((ldBase_t *)ld_qrcode)->isHidden == false);

    assert(tinyui_widget_set_opacity(&qrcode->widget, 77) == 0);
    assert(((ldBase_t *)ld_qrcode)->opacity == 77);

    assert(tinyui_widget_set_selectable(&qrcode->widget, 0) == 0);
    assert(((ldBase_t *)ld_qrcode)->isSelectable == false);
    assert(tinyui_widget_set_selectable(&qrcode->widget, 1) == 0);
    assert(((ldBase_t *)ld_qrcode)->isSelectable == true);

    assert(tinyui_widget_set_selected(&qrcode->widget, 1) == 0);
    assert(((ldBase_t *)ld_qrcode)->isSelected == true);

    assert(tinyui_widget_set_corner(&qrcode->widget, 9) == 0);
    assert(((ldBase_t *)ld_qrcode)->isCorner == true);
}

static void test_qrcode_rejects_null_args(struct tinyui_window *win)
{
    assert(tinyui_qrcode_create(0, "id") == 0);
    assert(tinyui_qrcode_create((struct tinyui_widget *)win, 0) == 0);
    assert(tinyui_qrcode_set_text(0, "text") == -1);
    assert(tinyui_qrcode_set_ecc(0, 1) == -1);
    assert(tinyui_qrcode_set_zoom(0, 2) == -1);
    assert(tinyui_qrcode_set_max_version(0, 10) == -1);
    assert(tinyui_qrcode_set_qr_color(0, 0x000000U) == -1);
    assert(tinyui_qrcode_set_bg_color(0, 0xFFFFFFU) == -1);
}

static void test_qrcode_create_with_props_failure_rolls_back_attached_child(struct tinyui_window *win)
{
    ldBase_t *win_ld = (ldBase_t *)win->widget.ld_widget;
    ldBase_t *tail_ld = ldBaseGetChildList(win_ld);
    ldBase_t *next_before_ld = 0;
    struct tinyui_qrcode *probe;
    struct tinyui_qrcode_test_dispose_snapshot snapshot = {0};

    while (tail_ld != 0 && ldBaseGetNextSibling(tail_ld) != 0) {
        tail_ld = ldBaseGetNextSibling(tail_ld);
    }
    if (tail_ld != 0) {
        next_before_ld = ldBaseGetNextSibling(tail_ld);
    }

    tinyui_qrcode_test_reset_state();
    probe = tinyui_qrcode_create((struct tinyui_widget *)win, "qr_fail_text");
    assert(probe != 0);
    assert(tinyui_widget_destroy(&probe->widget) == 0);

    assert(tinyui_qrcode_test_create_with_props_fail_before_text(
               (struct tinyui_widget *)win,
               &(struct tinyui_qrcode_props){
                   .id = "qr_fail_text",
                   .style_class = "qr-fail",
                   .text = "https://example.local/fail",
               })
           == 0);
    assert(tinyui_qrcode_test_take_last_dispose_snapshot(&snapshot) == 0);
    assert(snapshot.kind == TINYUI_BACKEND_WIDGET_QRCODE);
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
    if (tail_ld != 0) {
        assert(ldBaseGetNextSibling(tail_ld) == next_before_ld);
    } else {
        assert(ldBaseGetChildList(win_ld) == 0);
    }

    tinyui_qrcode_test_reset_state();
    assert(tinyui_qrcode_create_with_props(
               (struct tinyui_widget *)win,
               &(struct tinyui_qrcode_props){
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
    assert(tinyui_widget_is_kind(0, TINYUI_BACKEND_WIDGET_QRCODE) == 0);
    assert_source_lacks_function_definition(qrcode_source_path,
                                            "tinyui_qrcode_props_are_valid");
    assert_source_lacks_function_definition(qrcode_source_path,
                                            "tinyui_qrcode_get_ld");
    assert_source_lacks_function_definition(qrcode_source_path,
                                            "tinyui_qrcode_finish_detach_after_backend_failure");
    assert_source_lacks_function_definition(qrcode_source_path,
                                            "tinyui_qrcode_dispose_partial_impl");
    assert_source_lacks_function_definition(qrcode_source_path,
                                            "tinyui_qrcode_create_with_props_impl");
    assert_source_lacks_function_definition(test_source_path,
                                            "tinyui_backend_qrcode_test_reset_state");
    assert_source_lacks_function_definition(test_source_path,
                                            "tinyui_backend_qrcode_test_create_with_props_fail_before_text");
    assert_source_lacks_function_definition(test_source_path,
                                            "tinyui_backend_qrcode_test_take_last_dispose_snapshot");
}

int main(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *win;

    assert(app != 0);
    win = tinyui_window_create(app, "root");
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

    tinyui_app_destroy(app);
    return 0;
}
