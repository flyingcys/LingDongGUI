/*
 * Copyright (c) 2023-2026 flyingcys (flyingcys@gmail.com). All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "tinyui.h"
#include "internal.h"

#include <assert.h>

static void test_focus_claim_release_round_trip(struct tinyui_window *win)
{
    struct tinyui_button *btn = tinyui_button_create(win, "btn_focus");
    int was_focus;

    assert(btn != 0);
    assert(tinyui_widget_is_focus_owner(&btn->widget) == 0);

    assert(tinyui_widget_claim_focus(&btn->widget) == 0);
    assert(tinyui_widget_is_focus_owner(&btn->widget) == 1);
    assert(btn->widget.has_focus == 1);
    assert(btn->widget.focus_enter_count == 1);

    /* re-claim same widget = no-op */
    was_focus = btn->widget.focus_enter_count;
    assert(tinyui_widget_claim_focus(&btn->widget) == 0);
    assert(btn->widget.focus_enter_count == was_focus);

    assert(tinyui_widget_release_focus(&btn->widget) == 0);
    assert(tinyui_widget_is_focus_owner(&btn->widget) == 0);
    assert(btn->widget.has_focus == 0);
    assert(btn->widget.focus_leave_count == 1);
}

static void test_focus_switches_between_two_widgets(struct tinyui_window *win)
{
    struct tinyui_button *b1 = tinyui_button_create(win, "btn_f1");
    struct tinyui_button *b2 = tinyui_button_create(win, "btn_f2");

    assert(b1 != 0 && b2 != 0);

    assert(tinyui_widget_claim_focus(&b1->widget) == 0);
    assert(tinyui_widget_is_focus_owner(&b1->widget) == 1);
    assert(b1->widget.focus_enter_count == 1);

    assert(tinyui_widget_claim_focus(&b2->widget) == 0);
    assert(tinyui_widget_is_focus_owner(&b1->widget) == 0);
    assert(tinyui_widget_is_focus_owner(&b2->widget) == 1);
    assert(b1->widget.focus_leave_count == 1);
    assert(b2->widget.focus_enter_count == 1);
}

static void test_focus_null_and_hidden_guards(struct tinyui_window *win)
{
    struct tinyui_button *btn = tinyui_button_create(win, "btn_focus_guard");

    assert(btn != 0);
    assert(tinyui_widget_claim_focus(0) == -1);
    assert(tinyui_widget_release_focus(0) == -1);
    assert(tinyui_widget_is_focus_owner(0) == 0);

    /* hidden widget cannot claim focus */
    assert(tinyui_widget_set_visible(&btn->widget, 0) == 0);
    assert(tinyui_widget_claim_focus(&btn->widget) == -1);
    assert(tinyui_widget_release_focus(&btn->widget) == 0);
}

static void test_edit_result_marking(struct tinyui_window *win)
{
    struct tinyui_button *btn = tinyui_button_create(win, "btn_edit");

    assert(btn != 0);
    assert(tinyui_widget_mark_edit_result(0, TINYUI_EDIT_RESULT_COMMIT) == -1);
    assert(tinyui_widget_mark_edit_result(&btn->widget, TINYUI_EDIT_RESULT_NONE) == -1);
    assert(tinyui_widget_mark_edit_result(&btn->widget, (enum tinyui_edit_result)99) == -1);

    assert(tinyui_widget_mark_edit_result(&btn->widget, TINYUI_EDIT_RESULT_COMMIT) == 0);
    assert(btn->widget.pending_edit_result == TINYUI_EDIT_RESULT_COMMIT);

    assert(tinyui_widget_mark_edit_result(&btn->widget, TINYUI_EDIT_RESULT_CANCEL) == 0);
    assert(btn->widget.pending_edit_result == TINYUI_EDIT_RESULT_CANCEL);
}

static void test_editing_claim_release_round_trip(struct tinyui_window *win)
{
    struct tinyui_button *btn = tinyui_button_create(win, "btn_edit_own");

    assert(btn != 0);
    assert(tinyui_widget_is_editing_owner(&btn->widget) == 0);

    assert(tinyui_widget_claim_editing(&btn->widget) == 0);
    assert(tinyui_widget_is_editing_owner(&btn->widget) == 1);
    assert(btn->widget.pending_edit_result == TINYUI_EDIT_RESULT_NONE);

    assert(tinyui_widget_mark_edit_result(&btn->widget, TINYUI_EDIT_RESULT_COMMIT) == 0);
    assert(tinyui_widget_release_editing(&btn->widget) == 0);
    assert(tinyui_widget_is_editing_owner(&btn->widget) == 0);
    assert(btn->widget.last_edit_result == TINYUI_EDIT_RESULT_COMMIT);
    assert(btn->widget.pending_edit_result == TINYUI_EDIT_RESULT_NONE);
}

static void test_editing_guards(struct tinyui_window *win)
{
    struct tinyui_button *btn = tinyui_button_create(win, "btn_edit_guard");

    assert(btn != 0);
    assert(tinyui_widget_claim_editing(0) == -1);
    assert(tinyui_widget_release_editing(0) == -1);
    assert(tinyui_widget_is_editing_owner(0) == 0);

    /* hidden widget cannot claim editing */
    assert(tinyui_widget_set_visible(&btn->widget, 0) == 0);
    assert(tinyui_widget_claim_editing(&btn->widget) == -1);
}

int main(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *win;

    assert(app != 0);
    win = tinyui_window_create(app, "root");
    assert(win != 0);

    test_focus_claim_release_round_trip(win);
    test_focus_switches_between_two_widgets(win);
    test_focus_null_and_hidden_guards(win);
    test_edit_result_marking(win);
    test_editing_claim_release_round_trip(win);
    test_editing_guards(win);

    tinyui_app_destroy(app);
    return 0;
}
