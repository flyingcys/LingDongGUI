#include "app.h"
#include "keyboard.h"
#include "line_edit.h"
#include "widget.h"
#include "window.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldLineEdit.h"
#include "../../../src/misc/ldMsg.h"
#include "internal.h"

#include <assert.h>
#include <dlfcn.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

static int line_edit_finished_count = 0;
static struct tinyui_line_edit *line_edit_finished_widget = 0;
static void *line_edit_finished_user_data = 0;
static int line_edit_msg_queue_initialized = 0;
static const char *test_self_binary_path = 0;
static char test_line_edit_source_path[PATH_MAX];

/* Resolve tinyui/src/widgets/<name> relative to this test's own __FILE__ so
 * the source-contract checks work on any checkout, not a hardcoded path. */
static void init_line_edit_source_path(void)
{
    const char *source = __FILE__;
    const char *suffix = "tests/tinyui/unit/test_tinyui_line_edit.c";
    const char *match = strstr(source, suffix);
    size_t root_len;

    assert(match != 0);
    root_len = (size_t)(match - source);
    assert(root_len + strlen("tinyui/src/widgets/line_edit.c") < sizeof(test_line_edit_source_path));
    memcpy(test_line_edit_source_path, source, root_len);
    snprintf(test_line_edit_source_path + root_len,
             sizeof(test_line_edit_source_path) - root_len,
             "tinyui/src/widgets/line_edit.c");
}

static void assert_self_binary_lacks_symbol(const char *symbol)
{
    char command[1024];
    FILE *pipe;
    char line[512];

    assert(test_self_binary_path != 0);
    assert(symbol != 0);
    snprintf(command, sizeof(command), "nm %s 2>/dev/null", test_self_binary_path);
    pipe = popen(command, "r");
    assert(pipe != 0);
    while (fgets(line, sizeof(line), pipe) != 0) {
        size_t line_len = strlen(line);
        size_t symbol_len = strlen(symbol);

        while (line_len > 0 && (line[line_len - 1] == '\n' || line[line_len - 1] == '\r')) {
            line[--line_len] = '\0';
        }
        if (line_len >= symbol_len &&
            strcmp(line + line_len - symbol_len, symbol) == 0) {
            assert(!"unexpected symbol still present in test binary");
        }
    }
    assert(pclose(pipe) == 0);
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

static void on_line_edit_finished(struct tinyui_line_edit *line_edit, void *user_data)
{
    line_edit_finished_count++;
    line_edit_finished_widget = line_edit;
    line_edit_finished_user_data = user_data;
}

static void ensure_line_edit_msg_queue(struct tinyui_app *app_state)
{
    assert(app_state != 0);
    assert(app_state->ld_scene != 0);
    if (line_edit_msg_queue_initialized == 0) {
        assert(ldMsgInit(&app_state->ld_scene->ptMsgQueue, 8) == true);
        line_edit_msg_queue_initialized = 1;
    }
}

static void test_line_edit_create_with_props_sets_text_and_type(struct tinyui_window *win)
{
    struct tinyui_line_edit *line_edit;
    struct tinyui_widget *backend;
    ldLineEdit_t *ld_line_edit;
    enum tinyui_line_edit_type type = TINYUI_LINE_EDIT_TYPE_FLOAT;
    unsigned int keyboard_binding = 0;

    line_edit = tinyui_line_edit_create_with_props(
        win,
        &(struct tinyui_line_edit_props){
            .id = "line_edit_props",
            .text = "42",
            .type = TINYUI_LINE_EDIT_TYPE_INT,
            .keyboard_binding = 9U,
            .has_type = 1,
            .has_keyboard_binding = 1,
            .width = 180,
            .height = 32,
        });

    assert(line_edit != 0);
    backend = &line_edit->widget;
    assert(backend->ld_widget != 0);
    ld_line_edit = (ldLineEdit_t *)backend->ld_widget;
    assert(ld_line_edit != 0);
    assert(strcmp(tinyui_line_edit_get_text(line_edit), "42") == 0);
    assert(tinyui_line_edit_get_type(line_edit, &type) == 0);
    assert(type == TINYUI_LINE_EDIT_TYPE_INT);
    assert(tinyui_line_edit_get_keyboard_binding(line_edit, &keyboard_binding) == 0);
    assert(keyboard_binding == 9U);
    assert(ld_line_edit->editType == typeInt);
    assert(ld_line_edit->kbNameId == 9U);
}

static void test_line_edit_align_and_color_write_backend_state(struct tinyui_window *win)
{
    struct tinyui_line_edit *line_edit;
    struct tinyui_widget *backend;
    ldLineEdit_t *ld_line_edit;

    line_edit = tinyui_line_edit_create(win, "line_edit_style");
    assert(line_edit != 0);
    backend = &line_edit->widget;
    assert(backend->ld_widget != 0);
    ld_line_edit = (ldLineEdit_t *)backend->ld_widget;
    assert(ld_line_edit != 0);

    assert(tinyui_line_edit_set_align(line_edit, TINYUI_ALIGN_CENTER) == 0);
    assert(ld_line_edit->tAlign == ARM_2D_ALIGN_CENTRE);
    assert(line_edit->align == TINYUI_ALIGN_CENTER);
    assert(tinyui_line_edit_set_color(line_edit, 0x112233U, 0x445566U, 0x778899U) == 0);
    assert(ld_line_edit->textColor == __RGB(0x11, 0x22, 0x33));
    assert(ld_line_edit->backgroundColor == __RGB(0x44, 0x55, 0x66));
    assert(ld_line_edit->frameColor == __RGB(0x77, 0x88, 0x99));
    assert(line_edit->widget.text_color == 0x112233U);
    assert(line_edit->widget.bg_color == 0x445566U);
    assert(line_edit->widget.border_color == 0x778899U);
    assert(tinyui_line_edit_set_align(0, TINYUI_ALIGN_START) == -1);
    assert(tinyui_line_edit_set_color(0, 0, 0, 0) == -1);
}

static void test_line_edit_readback_matches_backend_after_finished_boundary(struct tinyui_window *win)
{
    struct tinyui_app *app;
    struct tinyui_line_edit *line_edit;
    struct tinyui_widget *backend;
    struct tinyui_app *app_state;
    ldLineEdit_t *ld_line_edit;
    int editing = -1;
    int finish_cookie = 17;

    app = win->widget.owner;
    line_edit = tinyui_line_edit_create(win, "line_edit_readback");
    assert(line_edit != 0);
    assert(tinyui_line_edit_set_text(line_edit, "before") == 0);
    assert(tinyui_line_edit_set_on_edit_finished(line_edit,
                                                 on_line_edit_finished,
                                                 &finish_cookie) == 0);

    backend = &line_edit->widget;
    assert(backend->ld_widget != 0);
    app_state = app;
    assert(app_state != 0);
    ensure_line_edit_msg_queue(app_state);
    ld_line_edit = (ldLineEdit_t *)backend->ld_widget;
    assert(ld_line_edit != 0);

    line_edit_finished_count = 0;
    line_edit_finished_widget = 0;
    line_edit_finished_user_data = 0;

    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue, backend->ld_widget, SIGNAL_PRESS, 0) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(tinyui_line_edit_get_editing(line_edit, &editing) == 0);
    assert(editing == 1);

    ldLineEditSetText(ld_line_edit, (uint8_t *)"after");
    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue, backend->ld_widget, SIGNAL_FINISHED, 0) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(strcmp(tinyui_line_edit_get_text(line_edit), "after") == 0);
    assert(tinyui_line_edit_get_editing(line_edit, &editing) == 0);
    assert(editing == 0);
    assert(line_edit_finished_count == 1);
    assert(line_edit_finished_widget == line_edit);
    assert(line_edit_finished_user_data == &finish_cookie);
}

static void test_line_edit_finished_boundary_clears_editing_state_without_reason(struct tinyui_window *win)
{
    struct tinyui_app *app;
    struct tinyui_line_edit *line_edit;
    struct tinyui_widget *backend;
    struct tinyui_app *app_state;
    int editing = -1;

    app = win->widget.owner;
    line_edit = tinyui_line_edit_create(win, "line_edit_finish_state");
    assert(line_edit != 0);

    backend = &line_edit->widget;
    assert(backend->ld_widget != 0);
    app_state = app;
    assert(app_state != 0);
    ensure_line_edit_msg_queue(app_state);

    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue, backend->ld_widget, SIGNAL_PRESS, 0) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(tinyui_line_edit_get_editing(line_edit, &editing) == 0);
    assert(editing == 1);

    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue, backend->ld_widget, SIGNAL_FINISHED, 0) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(tinyui_line_edit_get_editing(line_edit, &editing) == 0);
    assert(editing == 0);
}

static void test_line_edit_commit_and_cancel_paths_are_distinct(struct tinyui_window *win)
{
    struct tinyui_app *app;
    struct tinyui_line_edit *line_edit;
    struct tinyui_keyboard *keyboard;
    struct tinyui_widget *backend;
    struct tinyui_app *app_state;
    ldLineEdit_t *ld_line_edit;
    int editing = -1;
    int finish_cookie = 23;

    app = win->widget.owner;
    line_edit = tinyui_line_edit_create(win, "line_edit_commit_cancel");
    keyboard = tinyui_keyboard_create(win, "line_edit_commit_cancel_keyboard");
    assert(line_edit != 0);
    assert(keyboard != 0);
    assert(tinyui_line_edit_set_text(line_edit, "before") == 0);
    assert(tinyui_line_edit_set_on_edit_finished(line_edit,
                                                 on_line_edit_finished,
                                                 &finish_cookie) == 0);

    backend = &line_edit->widget;
    assert(backend->ld_widget != 0);
    app_state = app;
    assert(app_state != 0);
    ensure_line_edit_msg_queue(app_state);
    ld_line_edit = (ldLineEdit_t *)backend->ld_widget;
    assert(ld_line_edit != 0);

    line_edit_finished_count = 0;
    line_edit_finished_widget = 0;
    line_edit_finished_user_data = 0;

    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue, backend->ld_widget, SIGNAL_PRESS, 0) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(tinyui_line_edit_get_editing(line_edit, &editing) == 0);
    assert(editing == 1);
    assert(tinyui_widget_is_editing_owner(&line_edit->widget) == 1);

    ldLineEditSetText(ld_line_edit, (uint8_t *)"committed");
    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue, backend->ld_widget, SIGNAL_FINISHED, 0) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(tinyui_line_edit_get_editing(line_edit, &editing) == 0);
    assert(editing == 0);
    assert(strcmp(tinyui_line_edit_get_text(line_edit), "committed") == 0);
    assert(line_edit->widget.last_edit_result == TINYUI_EDIT_RESULT_COMMIT);
    assert(line_edit->widget.pending_edit_result == TINYUI_EDIT_RESULT_NONE);
    assert(line_edit_finished_count == 1);
    assert(line_edit_finished_widget == line_edit);
    assert(line_edit_finished_user_data == &finish_cookie);

    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue, backend->ld_widget, SIGNAL_PRESS, 0) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(tinyui_line_edit_get_editing(line_edit, &editing) == 0);
    assert(editing == 1);
    assert(tinyui_widget_claim_focus(&keyboard->widget) == 0);
    assert(tinyui_keyboard_exit(keyboard) == 0);
    assert(tinyui_line_edit_get_editing(line_edit, &editing) == 0);
    assert(editing == 0);
    assert(line_edit->widget.last_edit_result == TINYUI_EDIT_RESULT_CANCEL);
    assert(line_edit->widget.pending_edit_result == TINYUI_EDIT_RESULT_NONE);
    assert(line_edit_finished_count == 1);
}

static void test_line_edit_submit_cancel_reason_contract_is_release_ready(struct tinyui_window *win)
{
    struct tinyui_line_edit *line_edit;
    int editing = -1;

    line_edit = tinyui_line_edit_create(win, "line_edit_reason_contract");
    assert(line_edit != 0);
    assert(tinyui_line_edit_get_editing(line_edit, &editing) == 0);
    assert(editing == 0);
    assert(line_edit->widget.last_edit_result == TINYUI_EDIT_RESULT_NONE);
    assert(line_edit->widget.pending_edit_result == TINYUI_EDIT_RESULT_NONE);

    /* Final release contract is explicit: commit/cancel are distinguished, but no richer reason enum exists. */
    assert(line_edit->on_edit_finished == 0);
}

static void test_line_edit_rejects_invalid_keyboard_binding(struct tinyui_window *win)
{
    struct tinyui_line_edit *line_edit = tinyui_line_edit_create(win, "line_edit_invalid_binding");

    assert(line_edit != 0);
    assert(tinyui_line_edit_set_keyboard_binding(0, 1U) == -1);
    assert(tinyui_line_edit_set_keyboard_binding(line_edit, 0U) == -1);
    assert(tinyui_line_edit_set_keyboard_binding(line_edit, 0x10000U) == -1);
    assert(tinyui_line_edit_create_with_props(
               win,
               &(struct tinyui_line_edit_props){
                   .id = "line_edit_bad_props",
                   .keyboard_binding = 0U,
                   .has_keyboard_binding = 1,
               })
           == 0);
}

static void test_line_edit_set_keyboard_alias_matches_binding_contract(struct tinyui_window *win)
{
    struct tinyui_line_edit *line_edit = tinyui_line_edit_create(win, "line_edit_keyboard_alias");
    unsigned int keyboard_binding = 0;

    assert(line_edit != 0);
    assert(tinyui_line_edit_set_keyboard(line_edit, 15U) == 0);
    assert(tinyui_line_edit_get_keyboard_binding(line_edit, &keyboard_binding) == 0);
    assert(keyboard_binding == 15U);
    assert(tinyui_line_edit_set_keyboard(0, 15U) == -1);
}

static void test_line_edit_init_and_shared_base_aliases_round_trip(struct tinyui_window *win)
{
    struct tinyui_line_edit *line_edit = tinyui_line_edit_create(win, "line_edit_base_aliases");
    struct tinyui_widget *backend;
    /* volatile: this function clusters writes through ld* setters (separate TU)
     * with read-back assertions on adjacent ldBase_t bitfields. Under -Ofast
     * -flto the non-volatile reads get coalesced/hoisted and observe stale
     * bits even though the in-memory value is correct (verified via gdb).
     * volatile forces each read-back to reload from memory. */
    volatile ldBase_t *ld_base;

    assert(line_edit != 0);
    backend = &line_edit->widget;
    assert(backend->ld_widget != 0);
    ld_base = (volatile ldBase_t *)backend->ld_widget;
    assert(ld_base != 0);

    assert(tinyui_widget_set_pos(&line_edit->widget, 14, 28) == 0);
    assert(ld_base->use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 14);
    assert(ld_base->use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 28);

    assert(tinyui_widget_set_visible(&line_edit->widget, 0) == 0);
    assert(ld_base->isHidden == true);

    assert(tinyui_widget_set_opacity(&line_edit->widget, 66) == 0);
    assert(tinyui_widget_set_selectable(&line_edit->widget, 1) == 0);
    assert(tinyui_widget_set_selected(&line_edit->widget, 1) == 0);
    assert(tinyui_widget_set_corner(&line_edit->widget, 1) == 0);

    assert(ld_base->opacity == 66);
    assert(ld_base->isSelectable == true);
    assert(ld_base->isSelected == true);
    assert(ld_base->isCorner == true);
}

static void test_line_edit_error_paths_null_args(struct tinyui_window *win)
{
    assert(tinyui_line_edit_create(0, "id") == 0);
    assert(tinyui_line_edit_create(win, 0) == 0);
    assert(tinyui_line_edit_set_text(0, "text") == -1);
    assert(tinyui_line_edit_get_text(0) == 0);
    assert(tinyui_line_edit_set_type(0, TINYUI_LINE_EDIT_TYPE_STRING) == -1);
    assert(tinyui_line_edit_get_type(0, 0) == -1);
    assert(tinyui_line_edit_set_keyboard_binding(0, 1) == -1);
    assert(tinyui_line_edit_get_keyboard_binding(0, 0) == -1);
    assert(tinyui_line_edit_get_editing(0, 0) == -1);
    assert(tinyui_line_edit_set_align(0, TINYUI_ALIGN_START) == -1);
    assert(tinyui_line_edit_set_color(0, 0, 0, 0) == -1);
}

static void test_line_edit_error_paths_boundary_values(struct tinyui_window *win)
{
    struct tinyui_line_edit *le = tinyui_line_edit_create(win, "le_boundary");
    enum tinyui_line_edit_type type_out;
    unsigned int kb_out;
    int editing_out;

    assert(le != 0);

    assert(tinyui_line_edit_set_type(le, (enum tinyui_line_edit_type)999) == -1);
    assert(tinyui_line_edit_set_keyboard_binding(le, 0U) == -1);
    assert(tinyui_line_edit_set_keyboard_binding(le, 0x10000U) == -1);

    assert(tinyui_line_edit_set_type(le, TINYUI_LINE_EDIT_TYPE_INT) == 0);
    assert(tinyui_line_edit_get_type(le, &type_out) == 0);
    assert(type_out == TINYUI_LINE_EDIT_TYPE_INT);

    assert(tinyui_line_edit_set_keyboard_binding(le, 1U) == 0);
    assert(tinyui_line_edit_get_keyboard_binding(le, &kb_out) == 0);
    assert(kb_out == 1U);

    assert(tinyui_line_edit_get_editing(le, &editing_out) == 0);
    assert(editing_out == 0);

    assert(tinyui_line_edit_get_type(le, 0) == -1);
    assert(tinyui_line_edit_get_keyboard_binding(le, 0) == -1);
    assert(tinyui_line_edit_get_editing(le, 0) == -1);
}

static void test_line_edit_public_create_uses_widget_local_backend(struct tinyui_window *win)
{
    struct tinyui_line_edit *line_edit;
    struct tinyui_widget *backend;

    assert(win != 0);
    line_edit = tinyui_line_edit_create(win, "line_edit_widget_local");
    assert(line_edit != 0);

    backend = &line_edit->widget;
    assert(backend->kind == TINYUI_BACKEND_WIDGET_TEXT);
    assert(ldBaseGetParent((ldBase_t *)backend->ld_widget) == (ldBase_t *)win->widget.ld_widget);
    assert(backend->ld_widget != 0);
}

static void test_line_edit_internal_helpers_no_longer_use_tinyui_backend_prefix(void)
{
    assert_source_lacks_function_definition(test_line_edit_source_path, "tinyui_line_edit_type_is_valid");
    assert_source_lacks_function_definition(test_line_edit_source_path, "tinyui_line_edit_keyboard_binding_is_valid");
    assert_source_lacks_function_definition(test_line_edit_source_path, "tinyui_line_edit_props_are_valid");
    assert_source_lacks_function_definition(test_line_edit_source_path, "tinyui_line_edit_dispose_partial");
    assert_source_lacks_function_definition(test_line_edit_source_path, "tinyui_backend_line_edit_get_ld");
    assert_source_lacks_function_definition(test_line_edit_source_path, "tinyui_backend_line_edit_align_to_ld");
    assert_source_lacks_function_definition(test_line_edit_source_path, "tinyui_backend_line_edit_rgb_to_ld_color");
    assert_source_lacks_function_definition(test_line_edit_source_path, "tinyui_backend_line_edit_native_slot");
    assert_source_lacks_function_definition(test_line_edit_source_path, "tinyui_line_edit_create_backend_local");
    assert_source_lacks_function_definition(test_line_edit_source_path, "tinyui_backend_line_edit_set_text");
    assert_source_lacks_function_definition(test_line_edit_source_path, "tinyui_backend_line_edit_set_align");
    assert_source_lacks_function_definition(test_line_edit_source_path, "tinyui_backend_line_edit_set_color");
    assert_source_lacks_function_definition(test_line_edit_source_path, "tinyui_backend_line_edit_get_text");
    assert_source_lacks_function_definition(test_line_edit_source_path, "tinyui_backend_line_edit_set_type");
    assert_source_lacks_function_definition(test_line_edit_source_path, "tinyui_backend_line_edit_get_type");
    assert_source_lacks_function_definition(test_line_edit_source_path, "tinyui_backend_line_edit_set_keyboard_binding");
    assert_source_lacks_function_definition(test_line_edit_source_path, "tinyui_backend_line_edit_get_keyboard_binding");
    assert_source_lacks_function_definition(test_line_edit_source_path, "tinyui_backend_line_edit_bind_host");
    assert_source_lacks_function_definition(test_line_edit_source_path, "tinyui_backend_line_edit_get_editing");
    assert_self_binary_lacks_symbol("tinyui_backend_line_edit_set_text");
    assert_self_binary_lacks_symbol("tinyui_backend_line_edit_set_align");
    assert_self_binary_lacks_symbol("tinyui_backend_line_edit_set_color");
    assert_self_binary_lacks_symbol("tinyui_backend_line_edit_get_text");
    assert_self_binary_lacks_symbol("tinyui_backend_line_edit_set_type");
    assert_self_binary_lacks_symbol("tinyui_backend_line_edit_get_type");
    assert_self_binary_lacks_symbol("tinyui_backend_line_edit_set_keyboard_binding");
    assert_self_binary_lacks_symbol("tinyui_backend_line_edit_get_keyboard_binding");
    assert_self_binary_lacks_symbol("tinyui_backend_line_edit_bind_host");
    assert_self_binary_lacks_symbol("tinyui_backend_line_edit_get_editing");
}

int main(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *win;
    Dl_info self_info;

    assert(dladdr((void *)&main, &self_info) != 0);
    test_self_binary_path = self_info.dli_fname;
    init_line_edit_source_path();
    assert(app != 0);
    win = tinyui_window_create(app, "root");
    assert(win != 0);
    line_edit_msg_queue_initialized = 0;

    test_line_edit_create_with_props_sets_text_and_type(win);
    test_line_edit_align_and_color_write_backend_state(win);
    test_line_edit_readback_matches_backend_after_finished_boundary(win);
    test_line_edit_finished_boundary_clears_editing_state_without_reason(win);
    test_line_edit_commit_and_cancel_paths_are_distinct(win);
    test_line_edit_submit_cancel_reason_contract_is_release_ready(win);
    test_line_edit_rejects_invalid_keyboard_binding(win);
    test_line_edit_set_keyboard_alias_matches_binding_contract(win);
    test_line_edit_init_and_shared_base_aliases_round_trip(win);

    test_line_edit_error_paths_null_args(win);
    test_line_edit_error_paths_boundary_values(win);
    test_line_edit_public_create_uses_widget_local_backend(win);
    test_line_edit_internal_helpers_no_longer_use_tinyui_backend_prefix();

    tinyui_app_destroy(app);
    return 0;
}
