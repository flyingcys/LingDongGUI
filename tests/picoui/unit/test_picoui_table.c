#include "picoui/app.h"
#include "picoui/keyboard.h"
#include "picoui/table.h"
#include "picoui/window.h"
#include "../../../src/gui/ldTable.h"
#include "../../../src/misc/ldMsg.h"
#include "backend.h"
#include "internal.h"

#include <assert.h>
#include <string.h>

static uint64_t make_signal_value_xy(uint16_t x, uint16_t y)
{
    return ((uint64_t)x << 16) | (uint64_t)y;
}

static void ensure_table_msg_queue(struct picoui_backend_app_state *app_state)
{
    assert(app_state != 0);
    assert(app_state->ld_scene != 0);
    assert(ldMsgInit(&app_state->ld_scene->ptMsgQueue, 8) == true);
}

static void test_table_current_cell_matches_backend_truth(void)
{
    struct picoui_app *app;
    struct picoui_window *win;
    struct picoui_table *table;
    struct picoui_backend_widget *backend;
    ldTable_t *ld_table;

    app = picoui_app_create();
    assert(app != 0);
    win = picoui_window_create(app, "root");
    assert(win != 0);
    table = picoui_table_create(win, "table_truth", 3, 3);
    assert(table != 0);

    backend = (struct picoui_backend_widget *)table->widget.backend_widget;
    assert(backend != 0);
    ld_table = (ldTable_t *)backend->ld_widget;
    assert(ld_table != 0);

    assert(picoui_table_set_current_cell(table, 1, 2) == 0);
    assert(picoui_table_get_current_row(table) == 1);
    assert(picoui_table_get_current_column(table) == 2);

    ldTableSetItemSelect(ld_table, 2, 1, true);
    assert(picoui_table_get_current_row(table) == 2);
    assert(picoui_table_get_current_column(table) == 1);
    picoui_app_destroy(app);
}

static void test_table_edit_commit_updates_model_and_visible_text(void)
{
    struct picoui_app *app;
    struct picoui_window *win;
    struct picoui_table *table;
    struct picoui_keyboard *keyboard;
    struct picoui_backend_widget *backend;
    struct picoui_backend_app_state *app_state;
    ldTable_t *ld_table;

    app = picoui_app_create();
    assert(app != 0);
    win = picoui_window_create(app, "root");
    assert(win != 0);
    keyboard = picoui_keyboard_create(win, "keyboard_commit");
    table = picoui_table_create(win, "table_commit", 3, 3);
    assert(keyboard != 0);
    assert(table != 0);
    assert(picoui_table_set_keyboard_binding(table, 2U) == 0);
    assert(picoui_table_set_cell_editable(table, 0, 0, 1, 16) == 0);
    assert(picoui_table_set_cell_text(table, 0, 0, "before") == 0);

    backend = (struct picoui_backend_widget *)table->widget.backend_widget;
    assert(backend != 0);
    app_state = (struct picoui_backend_app_state *)backend->owner->backend_app;
    assert(app_state != 0);
    ensure_table_msg_queue(app_state);
    ld_table = (ldTable_t *)backend->ld_widget;
    assert(ld_table != 0);

    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue,
                     backend->ld_widget,
                     SIGNAL_PRESS,
                     make_signal_value_xy(10, 10)) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue,
                     backend->ld_widget,
                     SIGNAL_PRESS,
                     make_signal_value_xy(10, 10)) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(picoui_widget_is_editing_owner(&table->widget) == 1);

    ldTableSetItemText(ld_table, 0, 0, (uint8_t *)"after");
    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue, backend->ld_widget, SIGNAL_FINISHED, 0) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(strcmp(picoui_table_get_cell_text(table, 0, 0), "after") == 0);
    assert(table->widget.last_edit_result == PICOUI_EDIT_RESULT_COMMIT);
    assert(table->widget.pending_edit_result == PICOUI_EDIT_RESULT_NONE);
    assert(picoui_widget_is_editing_owner(&table->widget) == 0);
    picoui_app_destroy(app);
}

static void test_table_reuses_editable_cell_contract(void)
{
    struct picoui_app *app;
    struct picoui_window *win;
    struct picoui_table *table;
    struct picoui_backend_widget *backend;
    struct picoui_backend_app_state *app_state;

    app = picoui_app_create();
    assert(app != 0);
    win = picoui_window_create(app, "root");
    assert(win != 0);
    table = picoui_table_create(win, "table_contract", 3, 3);
    assert(table != 0);
    assert(picoui_table_set_cell_editable(table, 0, 0, 1, 16) == 0);

    backend = (struct picoui_backend_widget *)table->widget.backend_widget;
    assert(backend != 0);
    app_state = (struct picoui_backend_app_state *)backend->owner->backend_app;
    assert(app_state != 0);
    ensure_table_msg_queue(app_state);

    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue,
                     backend->ld_widget,
                     SIGNAL_PRESS,
                     make_signal_value_xy(10, 10)) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(picoui_widget_is_focus_owner(&table->widget) == 1);
    assert(picoui_widget_is_editing_owner(&table->widget) == 0);

    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue,
                     backend->ld_widget,
                     SIGNAL_PRESS,
                     make_signal_value_xy(10, 10)) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(picoui_widget_is_editing_owner(&table->widget) == 1);

    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue, backend->ld_widget, SIGNAL_FINISHED, 0) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(picoui_widget_is_editing_owner(&table->widget) == 0);
    assert(table->widget.last_edit_result == PICOUI_EDIT_RESULT_COMMIT);
    picoui_app_destroy(app);
}

static void test_table_final_release_contract_covers_non_commit_exit_boundary(void)
{
    struct picoui_app *app;
    struct picoui_window *win;
    struct picoui_table *table;
    struct picoui_keyboard *keyboard;
    struct picoui_backend_widget *backend;
    struct picoui_backend_app_state *app_state;
    ldTable_t *ld_table;

    app = picoui_app_create();
    assert(app != 0);
    win = picoui_window_create(app, "table_release_root");
    assert(win != 0);
    keyboard = picoui_keyboard_create(win, "table_release_keyboard");
    table = picoui_table_create(win, "table_release_ready", 2, 2);
    assert(keyboard != 0);
    assert(table != 0);
    assert(picoui_table_set_keyboard_binding(table, 9U) == 0);
    assert(picoui_table_set_cell_editable(table, 0, 0, 1, 16) == 0);
    assert(picoui_table_set_cell_text(table, 0, 0, "before") == 0);

    backend = (struct picoui_backend_widget *)table->widget.backend_widget;
    assert(backend != 0);
    assert(backend->kind == PICOUI_BACKEND_WIDGET_TABLE);
    app_state = (struct picoui_backend_app_state *)backend->owner->backend_app;
    assert(app_state != 0);
    ensure_table_msg_queue(app_state);
    ld_table = (ldTable_t *)backend->ld_widget;
    assert(ld_table != 0);

    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue,
                     backend->ld_widget,
                     SIGNAL_PRESS,
                     make_signal_value_xy(10, 10)) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue,
                     backend->ld_widget,
                     SIGNAL_PRESS,
                     make_signal_value_xy(10, 10)) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(picoui_widget_is_editing_owner(&table->widget) == 1);
    assert(picoui_widget_claim_focus(&keyboard->widget) == 0);
    assert(picoui_keyboard_exit(keyboard) == 0);

    assert(picoui_widget_is_editing_owner(&table->widget) == 0);
    assert(table->widget.last_edit_result == PICOUI_EDIT_RESULT_CANCEL);
    assert(table->widget.pending_edit_result == PICOUI_EDIT_RESULT_NONE);
    assert(strcmp(picoui_table_get_cell_text(table, 0, 0), "before") == 0);
    assert(strcmp((const char *)ldTableGetItemText(ld_table, 0, 0), "before") == 0);

    picoui_app_destroy(app);
}

int main(void)
{
    test_table_current_cell_matches_backend_truth();
    test_table_edit_commit_updates_model_and_visible_text();
    test_table_reuses_editable_cell_contract();
    test_table_final_release_contract_covers_non_commit_exit_boundary();
    return 0;
}
