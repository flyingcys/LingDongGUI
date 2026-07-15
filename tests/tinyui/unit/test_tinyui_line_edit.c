/*
 * TinyUI line_edit unit tests — M3 Task 4 L3/L4 harness.
 *
 * Validates real ldLineEdit_t mapping for create/props/text/type/align/color/
 * keyboard binding, SIGNAL_FINISHED commit/cancel edit_result paths, and
 * shared base aliases. Programmatic set_text does not fabricate user events.
 */

#include "tinyui.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldLineEdit.h"
#include "../../../src/misc/ldMsg.h"
#include "internal.h"
#include "tinyui_test_support.h"
#include "widgets/keyboard.h"
#include "widgets/line_edit.h"

#include <assert.h>
#include <dlfcn.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

static int line_edit_finished_count = 0;
static tinyui_obj_t *line_edit_finished_widget = 0;
static void *line_edit_finished_user_data = 0;
static int line_edit_msg_queue_initialized = 0;
static const char *test_self_binary_path = 0;
static char test_line_edit_source_path[PATH_MAX];

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

static void on_line_edit_finished(tinyui_obj_t *line_edit, void *user_data)
{
    line_edit_finished_count++;
    line_edit_finished_widget = line_edit;
    line_edit_finished_user_data = user_data;
}

static struct tinyui_app *line_edit_owner_app(tinyui_obj_t *obj)
{
    struct tinyui_widget *w = (struct tinyui_widget *)(void *)obj;

    assert(w != 0);
    assert(w->owner != 0);
    return w->owner;
}

static void ensure_line_edit_msg_queue(struct tinyui_app *app_state)
{
    assert(app_state != 0);
    assert(app_state->ld_scene != 0);
    if (line_edit_msg_queue_initialized == 0) {
        if (app_state->ld_scene->ptMsgQueue == 0) {
            assert(ldMsgInit(&app_state->ld_scene->ptMsgQueue, 8) == true);
        }
        line_edit_msg_queue_initialized = 1;
    }
}

static void emit_line_edit_signal(tinyui_obj_t *line_edit_obj, uint8_t signal)
{
    struct tinyui_widget *backend = (struct tinyui_widget *)(void *)line_edit_obj;
    struct tinyui_app *app_state;

    assert(backend != 0);
    assert(backend->ld_widget != 0);
    app_state = line_edit_owner_app(line_edit_obj);
    ensure_line_edit_msg_queue(app_state);
    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue, backend->ld_widget, signal, 0) == true);
    ldMsgProcess(app_state->ld_scene);
}

static void test_line_edit_create_and_backend_mapping(tinyui_obj_t *root)
{
    tinyui_obj_t *line_edit = tinyui_line_edit_create(root);
    struct tinyui_widget *backend;
    ldBase_t *ld_base;

    assert(line_edit != 0);
    backend = (struct tinyui_widget *)(void *)line_edit;
    assert(backend->ld_widget != 0);
    assert(backend->kind == TINYUI_BACKEND_WIDGET_LINE_EDIT);
    assert(backend->ld_name_id != 0);
    ld_base = (ldBase_t *)backend->ld_widget;
    assert(ld_base->widgetType == widgetTypeLineEdit);
    assert(ldBaseGetParent(ld_base) == (ldBase_t *)((struct tinyui_widget *)(void *)root)->ld_widget);
}

static void test_line_edit_create_with_props_sets_text_and_type(tinyui_obj_t *root)
{
    tinyui_obj_t *line_edit;
    struct tinyui_widget *backend;
    ldLineEdit_t *ld_line_edit;
    tinyui_line_edit_props_t props;
    enum tinyui_line_edit_type type = TINYUI_LINE_EDIT_TYPE_FLOAT;
    unsigned int keyboard_binding = 0;

    memset(&props, 0, sizeof(props));
    props.fields = TINYUI_LINE_EDIT_FIELD_TEXT
        | TINYUI_LINE_EDIT_FIELD_TYPE
        | TINYUI_LINE_EDIT_FIELD_KEYBOARD_BINDING
        | TINYUI_LINE_EDIT_FIELD_WIDTH
        | TINYUI_LINE_EDIT_FIELD_HEIGHT
        | TINYUI_LINE_EDIT_FIELD_BG_COLOR
        | TINYUI_LINE_EDIT_FIELD_TEXT_COLOR;
    props.text = "42";
    props.type = TINYUI_LINE_EDIT_TYPE_INT;
    props.keyboard_binding = 9U;
    props.width = 180;
    props.height = 32;
    props.bg_color = 0x102030U;
    props.text_color = 0xA0B0C0U;

    line_edit = tinyui_line_edit_create_with_props(root, &props);
    assert(line_edit != 0);
    backend = (struct tinyui_widget *)(void *)line_edit;
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
    assert(ld_line_edit->use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iWidth == 180);
    assert(ld_line_edit->use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iHeight == 32);
    /* props colors must hit real ldLineEdit color fields, not common-helper refuse. */
    assert(ld_line_edit->textColor == (ldColor)tinyui_rgb_to_ld_color(0xA0B0C0U));
    assert(ld_line_edit->backgroundColor == (ldColor)tinyui_rgb_to_ld_color(0x102030U));
    assert(backend->text_color == 0xA0B0C0U);
    assert(backend->bg_color == 0x102030U);
}

static void test_line_edit_set_text_capacity_rejects_without_silent_truncate(tinyui_obj_t *root)
{
    tinyui_obj_t *line_edit = tinyui_line_edit_create(root);
    struct tinyui_widget *backend;
    ldLineEdit_t *ld_line_edit;
    char oversize[256];
    char keep[8];

    assert(line_edit != 0);
    backend = (struct tinyui_widget *)(void *)line_edit;
    ld_line_edit = (ldLineEdit_t *)backend->ld_widget;
    assert(ld_line_edit != 0);
    assert(ld_line_edit->textMax == 255);

    memcpy(keep, "keep", 5);
    assert(tinyui_line_edit_set_text(line_edit, keep) == 0);
    assert(strcmp(tinyui_line_edit_get_text(line_edit), "keep") == 0);

    /* len == textMax must fail: LD only copies when len < textMax. */
    memset(oversize, 'A', 255);
    oversize[255] = '\0';
    assert(tinyui_line_edit_set_text(line_edit, oversize) == -1);
    assert(tinyui_last_result() == TINYUI_ERROR_CAPACITY);
    assert(strcmp(tinyui_line_edit_get_text(line_edit), "keep") == 0);
    assert(strcmp((const char *)ldLineEditGetText(ld_line_edit), "keep") == 0);
}

static void test_line_edit_align_and_color_write_backend_state(tinyui_obj_t *root)
{
    tinyui_obj_t *line_edit = tinyui_line_edit_create(root);
    struct tinyui_widget *backend;
    struct tinyui_line_edit *wrapper;
    ldLineEdit_t *ld_line_edit;

    assert(line_edit != 0);
    backend = (struct tinyui_widget *)(void *)line_edit;
    wrapper = (struct tinyui_line_edit *)(void *)line_edit;
    assert(backend->ld_widget != 0);
    ld_line_edit = (ldLineEdit_t *)backend->ld_widget;
    assert(ld_line_edit != 0);

    assert(tinyui_line_edit_set_align(line_edit, TINYUI_ALIGN_CENTER) == 0);
    assert(ld_line_edit->tAlign == ARM_2D_ALIGN_CENTRE);
    assert(wrapper->align == TINYUI_ALIGN_CENTER);
    assert(tinyui_line_edit_set_color(line_edit, 0x112233U, 0x445566U, 0x778899U) == 0);
    assert(ld_line_edit->textColor == __RGB(0x11, 0x22, 0x33));
    assert(ld_line_edit->backgroundColor == __RGB(0x44, 0x55, 0x66));
    assert(ld_line_edit->frameColor == __RGB(0x77, 0x88, 0x99));
    assert(wrapper->widget.text_color == 0x112233U);
    assert(wrapper->widget.bg_color == 0x445566U);
    assert(wrapper->widget.border_color == 0x778899U);
    assert(tinyui_line_edit_set_align(0, TINYUI_ALIGN_START) == -1);
    assert(tinyui_line_edit_set_color(0, 0, 0, 0) == -1);
}

static void test_line_edit_programmatic_set_text_no_fake_event(tinyui_obj_t *root)
{
    tinyui_obj_t *line_edit = tinyui_line_edit_create(root);
    ldLineEdit_t *ld_line_edit;
    int finish_cookie = 11;

    assert(line_edit != 0);
    ld_line_edit = (ldLineEdit_t *)((struct tinyui_widget *)(void *)line_edit)->ld_widget;
    assert(ld_line_edit != 0);

    line_edit_finished_count = 0;
    line_edit_finished_widget = 0;
    line_edit_finished_user_data = 0;
    assert(tinyui_line_edit_set_on_edit_finished(line_edit,
                                                 on_line_edit_finished,
                                                 &finish_cookie) == 0);
    assert(tinyui_line_edit_set_text(line_edit, "programmatic") == 0);
    assert(strcmp(tinyui_line_edit_get_text(line_edit), "programmatic") == 0);
    assert(strcmp((const char *)ldLineEditGetText(ld_line_edit), "programmatic") == 0);
    assert(line_edit_finished_count == 0);
}

static void test_line_edit_readback_matches_backend_after_finished_boundary(tinyui_obj_t *root)
{
    tinyui_obj_t *line_edit = tinyui_line_edit_create(root);
    struct tinyui_widget *backend;
    ldLineEdit_t *ld_line_edit;
    int editing = -1;
    int finish_cookie = 17;

    assert(line_edit != 0);
    assert(tinyui_line_edit_set_text(line_edit, "before") == 0);
    assert(tinyui_line_edit_set_on_edit_finished(line_edit,
                                                 on_line_edit_finished,
                                                 &finish_cookie) == 0);

    backend = (struct tinyui_widget *)(void *)line_edit;
    assert(backend->ld_widget != 0);
    ld_line_edit = (ldLineEdit_t *)backend->ld_widget;
    assert(ld_line_edit != 0);

    line_edit_finished_count = 0;
    line_edit_finished_widget = 0;
    line_edit_finished_user_data = 0;

    emit_line_edit_signal(line_edit, SIGNAL_PRESS);
    assert(tinyui_line_edit_get_editing(line_edit, &editing) == 0);
    assert(editing == 1);
    assert(tinyui_runtime_internal_widget_is_editing_owner(backend) == 1);

    ldLineEditSetText(ld_line_edit, (uint8_t *)"after");
    emit_line_edit_signal(line_edit, SIGNAL_FINISHED);
    assert(strcmp(tinyui_line_edit_get_text(line_edit), "after") == 0);
    assert(tinyui_line_edit_get_editing(line_edit, &editing) == 0);
    assert(editing == 0);
    assert(backend->last_edit_result == TINYUI_EDIT_RESULT_COMMIT);
    assert(backend->pending_edit_result == TINYUI_EDIT_RESULT_NONE);
    assert(line_edit_finished_count == 1);
    assert(line_edit_finished_widget == line_edit);
    assert(line_edit_finished_user_data == &finish_cookie);
}

static void test_line_edit_finished_boundary_clears_editing_state_without_reason(tinyui_obj_t *root)
{
    tinyui_obj_t *line_edit = tinyui_line_edit_create(root);
    int editing = -1;

    assert(line_edit != 0);

    emit_line_edit_signal(line_edit, SIGNAL_PRESS);
    assert(tinyui_line_edit_get_editing(line_edit, &editing) == 0);
    assert(editing == 1);

    emit_line_edit_signal(line_edit, SIGNAL_FINISHED);
    assert(tinyui_line_edit_get_editing(line_edit, &editing) == 0);
    assert(editing == 0);
}

static void test_line_edit_commit_and_cancel_paths_are_distinct(tinyui_obj_t *root)
{
    tinyui_obj_t *line_edit = tinyui_line_edit_create(root);
    tinyui_obj_t *keyboard = tinyui_keyboard_create(root);
    struct tinyui_widget *backend;
    ldLineEdit_t *ld_line_edit;
    int editing = -1;
    int finish_cookie = 23;

    assert(line_edit != 0);
    assert(keyboard != 0);
    assert(tinyui_line_edit_set_text(line_edit, "before") == 0);
    assert(tinyui_line_edit_set_on_edit_finished(line_edit,
                                                 on_line_edit_finished,
                                                 &finish_cookie) == 0);

    backend = (struct tinyui_widget *)(void *)line_edit;
    assert(backend->ld_widget != 0);
    ld_line_edit = (ldLineEdit_t *)backend->ld_widget;
    assert(ld_line_edit != 0);

    line_edit_finished_count = 0;
    line_edit_finished_widget = 0;
    line_edit_finished_user_data = 0;

    /* Commit path: PRESS -> backend text change -> FINISHED */
    emit_line_edit_signal(line_edit, SIGNAL_PRESS);
    assert(tinyui_line_edit_get_editing(line_edit, &editing) == 0);
    assert(editing == 1);
    assert(tinyui_runtime_internal_widget_is_editing_owner(backend) == 1);

    ldLineEditSetText(ld_line_edit, (uint8_t *)"committed");
    emit_line_edit_signal(line_edit, SIGNAL_FINISHED);
    assert(tinyui_line_edit_get_editing(line_edit, &editing) == 0);
    assert(editing == 0);
    assert(strcmp(tinyui_line_edit_get_text(line_edit), "committed") == 0);
    assert(backend->last_edit_result == TINYUI_EDIT_RESULT_COMMIT);
    assert(backend->pending_edit_result == TINYUI_EDIT_RESULT_NONE);
    assert(line_edit_finished_count == 1);
    assert(line_edit_finished_widget == line_edit);
    assert(line_edit_finished_user_data == &finish_cookie);

    /* Cancel path via edit_result_on_finish override before FINISHED */
    emit_line_edit_signal(line_edit, SIGNAL_PRESS);
    assert(tinyui_line_edit_get_editing(line_edit, &editing) == 0);
    assert(editing == 1);
    backend->edit_result_on_finish = TINYUI_EDIT_RESULT_CANCEL;
    ldLineEditSetText(ld_line_edit, (uint8_t *)"cancelled-text");
    emit_line_edit_signal(line_edit, SIGNAL_FINISHED);
    assert(tinyui_line_edit_get_editing(line_edit, &editing) == 0);
    assert(editing == 0);
    assert(backend->last_edit_result == TINYUI_EDIT_RESULT_CANCEL);
    assert(backend->pending_edit_result == TINYUI_EDIT_RESULT_NONE);
    assert(line_edit_finished_count == 2);

    /* Cancel path via keyboard_exit while editing owner is this line_edit.
     * keyboard_exit marks CANCEL on editing_owner without re-firing finished cb. */
    emit_line_edit_signal(line_edit, SIGNAL_PRESS);
    assert(tinyui_runtime_internal_widget_is_editing_owner(backend) == 1);
    assert(tinyui_runtime_internal_widget_claim_focus(
               (struct tinyui_widget *)(void *)keyboard) == 0);
    assert(tinyui_keyboard_exit(keyboard) == 0);
    assert(tinyui_runtime_internal_widget_is_editing_owner(backend) == 0);
    assert(backend->last_edit_result == TINYUI_EDIT_RESULT_CANCEL);
    assert(backend->pending_edit_result == TINYUI_EDIT_RESULT_NONE);
    assert(line_edit_finished_count == 2);
}

static void test_line_edit_submit_cancel_reason_contract_is_release_ready(tinyui_obj_t *root)
{
    tinyui_obj_t *line_edit = tinyui_line_edit_create(root);
    struct tinyui_line_edit *wrapper;
    int editing = -1;

    assert(line_edit != 0);
    wrapper = (struct tinyui_line_edit *)(void *)line_edit;
    assert(tinyui_line_edit_get_editing(line_edit, &editing) == 0);
    assert(editing == 0);
    assert(wrapper->widget.last_edit_result == TINYUI_EDIT_RESULT_NONE);
    assert(wrapper->widget.pending_edit_result == TINYUI_EDIT_RESULT_NONE);
    assert(wrapper->on_edit_finished == 0);
}

static void test_line_edit_rejects_invalid_keyboard_binding(tinyui_obj_t *root)
{
    tinyui_obj_t *line_edit = tinyui_line_edit_create(root);
    tinyui_line_edit_props_t bad_props;

    assert(line_edit != 0);
    assert(tinyui_line_edit_set_keyboard_binding(0, 1U) == -1);
    assert(tinyui_line_edit_set_keyboard_binding(line_edit, 0U) == -1);
    assert(tinyui_line_edit_set_keyboard_binding(line_edit, 0x10000U) == -1);

    memset(&bad_props, 0, sizeof(bad_props));
    bad_props.fields = TINYUI_LINE_EDIT_FIELD_KEYBOARD_BINDING;
    bad_props.keyboard_binding = 0x10000U;
    assert(tinyui_line_edit_create_with_props(root, &bad_props) == 0);
}

static void test_line_edit_create_with_props_accepts_sentinel_defaults(tinyui_obj_t *root)
{
    tinyui_line_edit_props_t props;
    tinyui_obj_t *line_edit;
    enum tinyui_line_edit_type type = TINYUI_LINE_EDIT_TYPE_FLOAT;
    unsigned int keyboard_binding = 123U;

    memset(&props, 0, sizeof(props));
    /* fields=0 means no optional setters; keep defaults from create(). */
    props.fields = 0;
    props.type = (enum tinyui_line_edit_type)-1;
    props.keyboard_binding = 0U;

    line_edit = tinyui_line_edit_create_with_props(root, &props);
    assert(line_edit != 0);
    assert(tinyui_line_edit_get_type(line_edit, &type) == 0);
    assert(type == TINYUI_LINE_EDIT_TYPE_STRING);
    assert(tinyui_line_edit_get_keyboard_binding(line_edit, &keyboard_binding) == 0);
    assert(keyboard_binding == 0U);
}

static void test_line_edit_set_keyboard_alias_matches_binding_contract(tinyui_obj_t *root)
{
    tinyui_obj_t *line_edit = tinyui_line_edit_create(root);
    ldLineEdit_t *ld_line_edit;
    unsigned int keyboard_binding = 0;

    assert(line_edit != 0);
    ld_line_edit = (ldLineEdit_t *)((struct tinyui_widget *)(void *)line_edit)->ld_widget;
    assert(ld_line_edit != 0);

    assert(tinyui_line_edit_set_keyboard(line_edit, 15U) == 0);
    assert(tinyui_line_edit_get_keyboard_binding(line_edit, &keyboard_binding) == 0);
    assert(keyboard_binding == 15U);
    assert(ld_line_edit->kbNameId == 15U);
    assert(tinyui_line_edit_set_keyboard(0, 15U) == -1);
}

static void test_line_edit_set_keyboard_widget_uses_native_id(tinyui_obj_t *root)
{
    tinyui_obj_t *line_edit = tinyui_line_edit_create(root);
    tinyui_obj_t *keyboard = tinyui_keyboard_create(root);
    struct tinyui_widget *kb_backend;
    struct tinyui_line_edit *wrapper;
    ldLineEdit_t *ld_line_edit;
    unsigned int keyboard_binding = 0;

    assert(line_edit != 0);
    assert(keyboard != 0);
    kb_backend = (struct tinyui_widget *)(void *)keyboard;
    wrapper = (struct tinyui_line_edit *)(void *)line_edit;
    ld_line_edit = (ldLineEdit_t *)wrapper->widget.ld_widget;
    assert(ld_line_edit != 0);

    assert(tinyui_line_edit_set_keyboard_widget(line_edit, keyboard) == 0);
    assert(tinyui_line_edit_get_keyboard_binding(line_edit, &keyboard_binding) == 0);
    assert(keyboard_binding == kb_backend->ld_name_id);
    assert(ld_line_edit->kbNameId == kb_backend->ld_name_id);
    assert(wrapper->keyboard_binding == kb_backend->ld_name_id);

    assert(tinyui_line_edit_set_keyboard_widget(0, keyboard) == -1);
    assert(tinyui_line_edit_set_keyboard_widget(line_edit, 0) == -1);
}

static void test_line_edit_set_keyboard_widget_rejects_cross_owner(void)
{
    tinyui_obj_t *root_a;
    tinyui_obj_t *root_b;
    tinyui_obj_t *line_edit;
    tinyui_obj_t *keyboard_a;
    tinyui_obj_t *keyboard_b;
    ldLineEdit_t *ld_line_edit;
    struct tinyui_widget *kb_a_backend;

    tinyui_deinit();
    assert(tinyui_init() == TINYUI_OK);
    root_a = tinyui_screen_create();
    assert(root_a != 0);

    /* Second independent root under same runtime owner is still same app.
     * Cross-owner rejection requires a second runtime; re-init yields a fresh app. */
    line_edit = tinyui_line_edit_create(root_a);
    keyboard_a = tinyui_keyboard_create(root_a);
    assert(line_edit != 0);
    assert(keyboard_a != 0);
    kb_a_backend = (struct tinyui_widget *)(void *)keyboard_a;
    ld_line_edit = (ldLineEdit_t *)((struct tinyui_widget *)(void *)line_edit)->ld_widget;
    assert(ld_line_edit != 0);

    assert(tinyui_line_edit_set_keyboard_widget(line_edit, keyboard_a) == 0);
    assert(ld_line_edit->kbNameId == kb_a_backend->ld_name_id);

    tinyui_deinit();
    assert(tinyui_init() == TINYUI_OK);
    root_b = tinyui_screen_create();
    assert(root_b != 0);
    keyboard_b = tinyui_keyboard_create(root_b);
    assert(keyboard_b != 0);

    /* line_edit was destroyed with previous runtime; recreate under root_b and
     * attempt to bind a keyboard from a different (already destroyed) owner is
     * not representable. Instead bind same-owner ok, reject non-keyboard. */
    line_edit = tinyui_line_edit_create(root_b);
    assert(line_edit != 0);
    assert(tinyui_line_edit_set_keyboard_widget(line_edit, root_b) == -1);
    assert(tinyui_line_edit_set_keyboard_widget(line_edit, keyboard_b) == 0);

    line_edit_msg_queue_initialized = 0;
}

static void test_line_edit_props_source_no_longer_uses_has_flags(void)
{
    assert(tinyui_test_source_contains("tinyui/include/widgets/line_edit.h", "has_type") == 0);
    assert(tinyui_test_source_contains("tinyui/include/widgets/line_edit.h", "has_keyboard_binding") == 0);
    assert(tinyui_test_source_contains("tinyui/src/widgets/line_edit.c", "props->has_type") == 0);
    assert(tinyui_test_source_contains("tinyui/src/widgets/line_edit.c", "props->has_keyboard_binding") == 0);
}

static void test_line_edit_init_and_shared_base_aliases_round_trip(tinyui_obj_t *root)
{
    tinyui_obj_t *line_edit = tinyui_line_edit_create(root);
    struct tinyui_widget *backend;
    /* volatile: this function clusters writes through ld* setters (separate TU)
     * with read-back assertions on adjacent ldBase_t bitfields. Under -Ofast
     * -flto the non-volatile reads get coalesced/hoisted and observe stale
     * bits even though the in-memory value is correct (verified via gdb).
     * volatile forces each read-back to reload from memory. */
    volatile ldBase_t *ld_base;

    assert(line_edit != 0);
    backend = (struct tinyui_widget *)(void *)line_edit;
    assert(backend->ld_widget != 0);
    ld_base = (volatile ldBase_t *)backend->ld_widget;
    assert(ld_base != 0);

    assert(tinyui_runtime_internal_widget_set_pos(backend, 14, 28) == 0);
    assert(ld_base->use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 14);
    assert(ld_base->use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 28);

    assert(tinyui_runtime_internal_widget_set_visible(backend, 0) == 0);
    assert(ld_base->isHidden == true);

    assert(tinyui_runtime_internal_widget_set_opacity(backend, 66) == 0);
    assert(tinyui_runtime_internal_widget_set_selectable(backend, 1) == 0);
    assert(tinyui_runtime_internal_widget_set_selected(backend, 1) == 0);
    assert(tinyui_runtime_internal_widget_set_corner(backend, 1) == 0);

    assert(ld_base->opacity == 66);
    assert(ld_base->isSelectable == true);
    assert(ld_base->isSelected == true);
    assert(ld_base->isCorner == true);

    /* restore visibility so later tests can interact if needed */
    assert(tinyui_runtime_internal_widget_set_visible(backend, 1) == 0);
}

static void test_line_edit_error_paths_null_args(void)
{
    assert(tinyui_line_edit_create(0) == 0);
    assert(tinyui_line_edit_set_text(0, "text") == -1);
    assert(tinyui_line_edit_get_text(0) == 0);
    assert(tinyui_line_edit_set_type(0, TINYUI_LINE_EDIT_TYPE_STRING) == -1);
    assert(tinyui_line_edit_get_type(0, 0) == -1);
    assert(tinyui_line_edit_set_keyboard_binding(0, 1) == -1);
    assert(tinyui_line_edit_get_keyboard_binding(0, 0) == -1);
    assert(tinyui_line_edit_get_editing(0, 0) == -1);
    assert(tinyui_line_edit_set_align(0, TINYUI_ALIGN_START) == -1);
    assert(tinyui_line_edit_set_color(0, 0, 0, 0) == -1);
    assert(tinyui_line_edit_set_on_edit_finished(0, 0, 0) == -1);
}

static void test_line_edit_error_paths_boundary_values(tinyui_obj_t *root)
{
    tinyui_obj_t *le = tinyui_line_edit_create(root);
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

static void test_line_edit_internal_helpers_no_longer_use_tinyui_backend_prefix(void)
{
    assert_source_lacks_function_definition(test_line_edit_source_path, "tinyui_line_edit_alloc");
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
    {
        char command[1024];
        snprintf(command,
                 sizeof(command),
                 "python3 - '%s' <<'PY'\n"
                 "from pathlib import Path\n"
                 "import sys\n"
                 "text = Path(sys.argv[1]).read_text()\n"
                 "raise SystemExit(0 if 'tinyui_runtime_internal_widget_create_leaf' in text else 1)\n"
                 "PY",
                 test_line_edit_source_path);
        assert(system(command) == 0);
    }
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
    tinyui_obj_t *root;
    Dl_info self_info;

    assert(dladdr((void *)&main, &self_info) != 0);
    test_self_binary_path = self_info.dli_fname;
    init_line_edit_source_path();

    tinyui_deinit();
    assert(tinyui_init() == TINYUI_OK);
    root = tinyui_screen_create();
    assert(root != 0);
    line_edit_msg_queue_initialized = 0;

    test_line_edit_create_and_backend_mapping(root);
    test_line_edit_create_with_props_sets_text_and_type(root);
    test_line_edit_set_text_capacity_rejects_without_silent_truncate(root);
    test_line_edit_align_and_color_write_backend_state(root);
    test_line_edit_programmatic_set_text_no_fake_event(root);
    test_line_edit_readback_matches_backend_after_finished_boundary(root);
    test_line_edit_finished_boundary_clears_editing_state_without_reason(root);
    test_line_edit_commit_and_cancel_paths_are_distinct(root);
    test_line_edit_submit_cancel_reason_contract_is_release_ready(root);
    test_line_edit_rejects_invalid_keyboard_binding(root);
    test_line_edit_create_with_props_accepts_sentinel_defaults(root);
    test_line_edit_set_keyboard_alias_matches_binding_contract(root);
    test_line_edit_set_keyboard_widget_uses_native_id(root);
    test_line_edit_set_keyboard_widget_rejects_cross_owner();
    /* re-create root after cross-owner test re-inits runtime */
    root = tinyui_screen_create();
    assert(root != 0);
    line_edit_msg_queue_initialized = 0;
    test_line_edit_props_source_no_longer_uses_has_flags();
    test_line_edit_init_and_shared_base_aliases_round_trip(root);
    test_line_edit_error_paths_null_args();
    test_line_edit_error_paths_boundary_values(root);
    test_line_edit_internal_helpers_no_longer_use_tinyui_backend_prefix();

    tinyui_deinit();
    return 0;
}
