#include "picoui/app.h"
#include "picoui/keyboard.h"
#include "picoui/line_edit.h"
#include "picoui/widget.h"
#include "picoui/window.h"
#include "../../../src/gui/ldLineEdit.h"
#include "../../../src/misc/ldMsg.h"
#include "internal.h"

#include <assert.h>
#include <string.h>

static int line_edit_finished_count = 0;
static struct picoui_line_edit *line_edit_finished_widget = 0;
static void *line_edit_finished_user_data = 0;
static int line_edit_msg_queue_initialized = 0;

static void on_line_edit_finished(struct picoui_line_edit *line_edit, void *user_data)
{
    line_edit_finished_count++;
    line_edit_finished_widget = line_edit;
    line_edit_finished_user_data = user_data;
}

static void ensure_line_edit_msg_queue(struct picoui_backend_app_state *app_state)
{
    assert(app_state != 0);
    assert(app_state->ld_scene != 0);
    if (line_edit_msg_queue_initialized == 0) {
        assert(ldMsgInit(&app_state->ld_scene->ptMsgQueue, 8) == true);
        line_edit_msg_queue_initialized = 1;
    }
}

static void test_line_edit_create_with_props_sets_text_and_type(struct picoui_window *win)
{
    struct picoui_line_edit *line_edit;
    struct picoui_backend_widget *backend;
    ldLineEdit_t *ld_line_edit;
    enum picoui_line_edit_type type = PICOUI_LINE_EDIT_TYPE_FLOAT;
    unsigned int keyboard_binding = 0;

    line_edit = picoui_line_edit_create_with_props(
        win,
        &(struct picoui_line_edit_props){
            .id = "line_edit_props",
            .text = "42",
            .type = PICOUI_LINE_EDIT_TYPE_INT,
            .keyboard_binding = 9U,
            .has_type = 1,
            .has_keyboard_binding = 1,
            .width = 180,
            .height = 32,
        });

    assert(line_edit != 0);
    backend = (struct picoui_backend_widget *)line_edit->widget.backend_widget;
    assert(backend != 0);
    ld_line_edit = (ldLineEdit_t *)backend->ld_widget;
    assert(ld_line_edit != 0);
    assert(strcmp(picoui_line_edit_get_text(line_edit), "42") == 0);
    assert(picoui_line_edit_get_type(line_edit, &type) == 0);
    assert(type == PICOUI_LINE_EDIT_TYPE_INT);
    assert(picoui_line_edit_get_keyboard_binding(line_edit, &keyboard_binding) == 0);
    assert(keyboard_binding == 9U);
    assert(ld_line_edit->editType == typeInt);
    assert(ld_line_edit->kbNameId == 9U);
}

static void test_line_edit_align_and_color_write_backend_state(struct picoui_window *win)
{
    struct picoui_line_edit *line_edit;
    struct picoui_backend_widget *backend;
    ldLineEdit_t *ld_line_edit;

    line_edit = picoui_line_edit_create(win, "line_edit_style");
    assert(line_edit != 0);
    backend = (struct picoui_backend_widget *)line_edit->widget.backend_widget;
    assert(backend != 0);
    ld_line_edit = (ldLineEdit_t *)backend->ld_widget;
    assert(ld_line_edit != 0);

    assert(picoui_line_edit_set_align(line_edit, PICOUI_ALIGN_CENTER) == 0);
    assert(ld_line_edit->tAlign == ARM_2D_ALIGN_CENTRE);
    assert(line_edit->align == PICOUI_ALIGN_CENTER);
    assert(picoui_line_edit_set_color(line_edit, 0x112233U, 0x445566U, 0x778899U) == 0);
    assert(ld_line_edit->textColor == __RGB(0x11, 0x22, 0x33));
    assert(ld_line_edit->backgroundColor == __RGB(0x44, 0x55, 0x66));
    assert(ld_line_edit->frameColor == __RGB(0x77, 0x88, 0x99));
    assert(line_edit->widget.text_color == 0x112233U);
    assert(line_edit->widget.bg_color == 0x445566U);
    assert(line_edit->widget.border_color == 0x778899U);
    assert(picoui_line_edit_set_align(0, PICOUI_ALIGN_START) == -1);
    assert(picoui_line_edit_set_color(0, 0, 0, 0) == -1);
}

static void test_line_edit_readback_matches_backend_after_finished_boundary(struct picoui_window *win)
{
    struct picoui_app *app;
    struct picoui_line_edit *line_edit;
    struct picoui_backend_widget *backend;
    struct picoui_backend_app_state *app_state;
    ldLineEdit_t *ld_line_edit;
    int editing = -1;
    int finish_cookie = 17;

    app = ((struct picoui_backend_widget *)win->widget.backend_widget)->owner;
    line_edit = picoui_line_edit_create(win, "line_edit_readback");
    assert(line_edit != 0);
    assert(picoui_line_edit_set_text(line_edit, "before") == 0);
    assert(picoui_line_edit_set_on_edit_finished(line_edit,
                                                 on_line_edit_finished,
                                                 &finish_cookie) == 0);

    backend = (struct picoui_backend_widget *)line_edit->widget.backend_widget;
    assert(backend != 0);
    app_state = (struct picoui_backend_app_state *)app->backend_app;
    assert(app_state != 0);
    ensure_line_edit_msg_queue(app_state);
    ld_line_edit = (ldLineEdit_t *)backend->ld_widget;
    assert(ld_line_edit != 0);

    line_edit_finished_count = 0;
    line_edit_finished_widget = 0;
    line_edit_finished_user_data = 0;

    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue, backend->ld_widget, SIGNAL_PRESS, 0) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(picoui_line_edit_get_editing(line_edit, &editing) == 0);
    assert(editing == 1);

    ldLineEditSetText(ld_line_edit, (uint8_t *)"after");
    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue, backend->ld_widget, SIGNAL_FINISHED, 0) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(strcmp(picoui_line_edit_get_text(line_edit), "after") == 0);
    assert(picoui_line_edit_get_editing(line_edit, &editing) == 0);
    assert(editing == 0);
    assert(line_edit_finished_count == 1);
    assert(line_edit_finished_widget == line_edit);
    assert(line_edit_finished_user_data == &finish_cookie);
}

static void test_line_edit_finished_boundary_clears_editing_state_without_reason(struct picoui_window *win)
{
    struct picoui_app *app;
    struct picoui_line_edit *line_edit;
    struct picoui_backend_widget *backend;
    struct picoui_backend_app_state *app_state;
    int editing = -1;

    app = ((struct picoui_backend_widget *)win->widget.backend_widget)->owner;
    line_edit = picoui_line_edit_create(win, "line_edit_finish_state");
    assert(line_edit != 0);

    backend = (struct picoui_backend_widget *)line_edit->widget.backend_widget;
    assert(backend != 0);
    app_state = (struct picoui_backend_app_state *)app->backend_app;
    assert(app_state != 0);
    ensure_line_edit_msg_queue(app_state);

    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue, backend->ld_widget, SIGNAL_PRESS, 0) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(picoui_line_edit_get_editing(line_edit, &editing) == 0);
    assert(editing == 1);

    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue, backend->ld_widget, SIGNAL_FINISHED, 0) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(picoui_line_edit_get_editing(line_edit, &editing) == 0);
    assert(editing == 0);
    assert(backend->last_native_signal == SIGNAL_FINISHED);
    assert(backend->last_native_value == 0);
}

static void test_line_edit_commit_and_cancel_paths_are_distinct(struct picoui_window *win)
{
    struct picoui_app *app;
    struct picoui_line_edit *line_edit;
    struct picoui_keyboard *keyboard;
    struct picoui_backend_widget *backend;
    struct picoui_backend_app_state *app_state;
    ldLineEdit_t *ld_line_edit;
    int editing = -1;
    int finish_cookie = 23;

    app = ((struct picoui_backend_widget *)win->widget.backend_widget)->owner;
    line_edit = picoui_line_edit_create(win, "line_edit_commit_cancel");
    keyboard = picoui_keyboard_create(win, "line_edit_commit_cancel_keyboard");
    assert(line_edit != 0);
    assert(keyboard != 0);
    assert(picoui_line_edit_set_text(line_edit, "before") == 0);
    assert(picoui_line_edit_set_on_edit_finished(line_edit,
                                                 on_line_edit_finished,
                                                 &finish_cookie) == 0);

    backend = (struct picoui_backend_widget *)line_edit->widget.backend_widget;
    assert(backend != 0);
    app_state = (struct picoui_backend_app_state *)app->backend_app;
    assert(app_state != 0);
    ensure_line_edit_msg_queue(app_state);
    ld_line_edit = (ldLineEdit_t *)backend->ld_widget;
    assert(ld_line_edit != 0);

    line_edit_finished_count = 0;
    line_edit_finished_widget = 0;
    line_edit_finished_user_data = 0;

    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue, backend->ld_widget, SIGNAL_PRESS, 0) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(picoui_line_edit_get_editing(line_edit, &editing) == 0);
    assert(editing == 1);
    assert(picoui_widget_is_editing_owner(&line_edit->widget) == 1);

    ldLineEditSetText(ld_line_edit, (uint8_t *)"committed");
    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue, backend->ld_widget, SIGNAL_FINISHED, 0) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(picoui_line_edit_get_editing(line_edit, &editing) == 0);
    assert(editing == 0);
    assert(strcmp(picoui_line_edit_get_text(line_edit), "committed") == 0);
    assert(line_edit->widget.last_edit_result == PICOUI_EDIT_RESULT_COMMIT);
    assert(line_edit->widget.pending_edit_result == PICOUI_EDIT_RESULT_NONE);
    assert(backend->last_native_signal == SIGNAL_FINISHED);
    assert(backend->last_native_value == 0);
    assert(line_edit_finished_count == 1);
    assert(line_edit_finished_widget == line_edit);
    assert(line_edit_finished_user_data == &finish_cookie);

    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue, backend->ld_widget, SIGNAL_PRESS, 0) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(picoui_line_edit_get_editing(line_edit, &editing) == 0);
    assert(editing == 1);
    assert(picoui_widget_claim_focus(&keyboard->widget) == 0);
    assert(picoui_keyboard_exit(keyboard) == 0);
    assert(picoui_line_edit_get_editing(line_edit, &editing) == 0);
    assert(editing == 0);
    assert(line_edit->widget.last_edit_result == PICOUI_EDIT_RESULT_CANCEL);
    assert(line_edit->widget.pending_edit_result == PICOUI_EDIT_RESULT_NONE);
    assert(backend->last_native_signal == SIGNAL_PRESS);
    assert(backend->last_native_value == 0);
    assert(line_edit_finished_count == 1);
}

static void test_line_edit_submit_cancel_reason_contract_is_release_ready(struct picoui_window *win)
{
    struct picoui_line_edit *line_edit;
    int editing = -1;

    line_edit = picoui_line_edit_create(win, "line_edit_reason_contract");
    assert(line_edit != 0);
    assert(picoui_line_edit_get_editing(line_edit, &editing) == 0);
    assert(editing == 0);
    assert(line_edit->widget.last_edit_result == PICOUI_EDIT_RESULT_NONE);
    assert(line_edit->widget.pending_edit_result == PICOUI_EDIT_RESULT_NONE);

    /* Final release contract is explicit: commit/cancel are distinguished, but no richer reason enum exists. */
    assert(line_edit->on_edit_finished == 0);
}

static void test_line_edit_rejects_invalid_keyboard_binding(struct picoui_window *win)
{
    struct picoui_line_edit *line_edit = picoui_line_edit_create(win, "line_edit_invalid_binding");

    assert(line_edit != 0);
    assert(picoui_line_edit_set_keyboard_binding(0, 1U) == -1);
    assert(picoui_line_edit_set_keyboard_binding(line_edit, 0U) == -1);
    assert(picoui_line_edit_set_keyboard_binding(line_edit, 0x10000U) == -1);
    assert(picoui_line_edit_create_with_props(
               win,
               &(struct picoui_line_edit_props){
                   .id = "line_edit_bad_props",
                   .keyboard_binding = 0U,
                   .has_keyboard_binding = 1,
               })
           == 0);
}

static void test_line_edit_set_keyboard_alias_matches_binding_contract(struct picoui_window *win)
{
    struct picoui_line_edit *line_edit = picoui_line_edit_create(win, "line_edit_keyboard_alias");
    unsigned int keyboard_binding = 0;

    assert(line_edit != 0);
    assert(picoui_line_edit_set_keyboard(line_edit, 15U) == 0);
    assert(picoui_line_edit_get_keyboard_binding(line_edit, &keyboard_binding) == 0);
    assert(keyboard_binding == 15U);
    assert(picoui_line_edit_set_keyboard(0, 15U) == -1);
}

static void test_line_edit_init_and_shared_base_aliases_round_trip(struct picoui_window *win)
{
    struct picoui_line_edit *line_edit = picoui_line_edit_create(win, "line_edit_base_aliases");
    struct picoui_backend_widget *backend;
    ldBase_t *ld_base;

    assert(line_edit != 0);
    backend = (struct picoui_backend_widget *)line_edit->widget.backend_widget;
    assert(backend != 0);
    ld_base = (ldBase_t *)backend->ld_widget;
    assert(ld_base != 0);

    assert(picoui_widget_set_pos(&line_edit->widget, 14, 28) == 0);
    assert(ld_base->use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 14);
    assert(ld_base->use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 28);

    assert(picoui_widget_set_visible(&line_edit->widget, 0) == 0);
    assert(ld_base->isHidden == true);

    assert(picoui_widget_set_opacity(&line_edit->widget, 66) == 0);
    assert(picoui_widget_set_selectable(&line_edit->widget, 1) == 0);
    assert(picoui_widget_set_selected(&line_edit->widget, 1) == 0);
    assert(picoui_widget_set_corner(&line_edit->widget, 1) == 0);

    assert(ld_base->opacity == 66);
    assert(ld_base->isSelectable == true);
    assert(ld_base->isSelected == true);
    assert(ld_base->isCorner == true);
}

int main(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_window *win;

    assert(app != 0);
    win = picoui_window_create(app, "root");
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

    picoui_app_destroy(app);
    return 0;
}
