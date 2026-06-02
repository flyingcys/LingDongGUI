#include "picoui/app.h"
#include "picoui/keyboard.h"
#include "picoui/line_edit.h"
#include "picoui/text.h"
#include "picoui/window.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldKeyboard.h"
#include "../../../src/gui/ldLineEdit.h"
#include "../../../src/gui/ldText.h"
#include "internal.h"

#include <assert.h>
#include <string.h>

static struct picoui_window *test_window_create(struct picoui_app **app_out)
{
    struct picoui_app *app;
    struct picoui_window *win;

    app = picoui_app_create();
    assert(app != 0);
    win = picoui_window_create(app, "root");
    assert(win != 0);
    *app_out = app;
    return win;
}

static void test_keyboard_dispatches_ascii_into_focused_line_edit(void)
{
    struct picoui_app *app;
    struct picoui_window *win;
    struct picoui_keyboard *keyboard;
    struct picoui_line_edit *line_edit;

    win = test_window_create(&app);
    keyboard = picoui_keyboard_create(win, "keyboard_ascii");
    line_edit = picoui_line_edit_create(win, "line_edit_ascii");

    assert(keyboard != 0);
    assert(line_edit != 0);
    assert(picoui_line_edit_set_text(line_edit, "A") == 0);
    assert(picoui_widget_claim_focus(&line_edit->widget) == 0);
    assert(picoui_keyboard_input_ascii(keyboard, 'b') == 0);
    assert(strcmp(picoui_line_edit_get_text(line_edit), "Ab") == 0);
    picoui_app_destroy(app);
}

static void test_keyboard_dispatches_ascii_into_editing_owner_before_focus_owner(void)
{
    struct picoui_app *app;
    struct picoui_window *win;
    struct picoui_keyboard *keyboard;
    struct picoui_line_edit *line_edit;
    struct picoui_text *text;

    win = test_window_create(&app);
    keyboard = picoui_keyboard_create(win, "keyboard_ascii_editing_owner");
    line_edit = picoui_line_edit_create(win, "line_edit_ascii_editing_owner");
    text = picoui_text_create(win, "text_ascii_focus_owner");

    assert(keyboard != 0);
    assert(line_edit != 0);
    assert(text != 0);
    assert(picoui_line_edit_set_text(line_edit, "Q") == 0);
    assert(picoui_text_set_text(text, "focus-owner") == 0);
    line_edit->editing = 1;
    assert(picoui_widget_claim_editing(&line_edit->widget) == 0);
    assert(picoui_widget_claim_focus(&text->widget) == 0);
    assert(picoui_keyboard_input_ascii(keyboard, 'w') == 0);
    assert(strcmp(picoui_line_edit_get_text(line_edit), "Qw") == 0);
    assert(strcmp((const char *)((ldText_t *)((struct picoui_backend_widget *)text->widget.backend_widget)->ld_widget)->pStr,
                  "focus-owner") == 0);
    picoui_app_destroy(app);
}

static void test_keyboard_navigation_preserves_editing_owner_model_truth(void)
{
    struct picoui_app *app;
    struct picoui_window *win;
    struct picoui_keyboard *keyboard;
    struct picoui_line_edit *line_edit;
    struct picoui_text *text;
    struct picoui_backend_widget *line_edit_backend;
    ldLineEdit_t *ld_line_edit;
    int editing = -1;

    win = test_window_create(&app);
    keyboard = picoui_keyboard_create(win, "keyboard_navigation_editing_owner");
    line_edit = picoui_line_edit_create(win, "line_edit_navigation_editing_owner");
    text = picoui_text_create(win, "text_navigation_focus_owner");

    assert(keyboard != 0);
    assert(line_edit != 0);
    assert(text != 0);
    assert(picoui_line_edit_set_text(line_edit, "Q") == 0);
    assert(picoui_text_set_text(text, "focus-owner") == 0);

    line_edit_backend = (struct picoui_backend_widget *)line_edit->widget.backend_widget;
    assert(line_edit_backend != 0);
    ld_line_edit = (ldLineEdit_t *)line_edit_backend->ld_widget;
    assert(ld_line_edit != 0);

    line_edit->editing = 1;
    ld_line_edit->isEditing = true;
    assert(picoui_widget_claim_editing(&line_edit->widget) == 0);
    assert(picoui_widget_claim_focus(&text->widget) == 0);
    assert(picoui_keyboard_input_ascii(keyboard, 'w') == 0);
    assert(picoui_line_edit_get_editing(line_edit, &editing) == 0);
    assert(editing == 1);
    assert(strcmp(picoui_line_edit_get_text(line_edit), "Qw") == 0);
    assert(strcmp((const char *)ldLineEditGetText(ld_line_edit), "Qw") == 0);
    assert(line_edit->widget.last_edit_result == PICOUI_EDIT_RESULT_NONE);
    assert(picoui_widget_is_editing_owner(&line_edit->widget) == 1);
    assert(picoui_widget_is_focus_owner(&text->widget) == 1);
    picoui_app_destroy(app);
}

static void test_keyboard_rejects_ascii_when_target_is_not_line_edit(void)
{
    struct picoui_app *app;
    struct picoui_window *win;
    struct picoui_keyboard *keyboard;
    struct picoui_text *text;
    ldText_t *ld_text;

    win = test_window_create(&app);
    keyboard = picoui_keyboard_create(win, "keyboard_ascii_reject_text");
    text = picoui_text_create(win, "text_ascii_reject");

    assert(keyboard != 0);
    assert(text != 0);
    assert(picoui_text_set_text(text, "plain-text") == 0);
    ld_text = (ldText_t *)((struct picoui_backend_widget *)text->widget.backend_widget)->ld_widget;
    assert(ld_text != 0);
    assert(picoui_widget_claim_focus(&text->widget) == 0);
    assert(picoui_keyboard_input_ascii(keyboard, 'x') == -1);
    assert(strcmp((const char *)ld_text->pStr, "plain-text") == 0);
    picoui_app_destroy(app);
}

static void test_keyboard_navigation_signal_respects_focus_owner(void)
{
    struct picoui_app *app;
    struct picoui_window *win;
    struct picoui_keyboard *keyboard;
    struct picoui_backend_widget *backend;
    ldKeyboard_t *ld_keyboard;

    win = test_window_create(&app);
    keyboard = picoui_keyboard_create(win, "keyboard_nav");
    assert(keyboard != 0);

    backend = (struct picoui_backend_widget *)keyboard->widget.backend_widget;
    assert(backend != 0);
    ld_keyboard = (ldKeyboard_t *)backend->ld_widget;
    assert(ld_keyboard != 0);

    ld_keyboard->keyCode = 'a';
    assert(picoui_keyboard_navigate(keyboard, NAV_RIGHT) == -1);
    assert(ld_keyboard->keyCode == 'a');
    assert(picoui_widget_claim_focus(&keyboard->widget) == 0);
    assert(picoui_keyboard_navigate(keyboard, NAV_RIGHT) == 0);
    assert(ld_keyboard->keyCode != 'a');
    picoui_app_destroy(app);
}

static void test_keyboard_update_and_button_update_touch_native_state(void)
{
    struct picoui_app *app;
    struct picoui_window *win;
    struct picoui_keyboard *keyboard;
    struct picoui_line_edit *line_edit;
    struct picoui_backend_widget *backend;
    ldKeyboard_t *ld_keyboard;

    win = test_window_create(&app);
    keyboard = picoui_keyboard_create(win, "keyboard_update");
    line_edit = picoui_line_edit_create(win, "line_edit_update_target");

    assert(keyboard != 0);
    assert(line_edit != 0);
    backend = (struct picoui_backend_widget *)keyboard->widget.backend_widget;
    assert(backend != 0);
    ld_keyboard = (ldKeyboard_t *)backend->ld_widget;
    assert(ld_keyboard != 0);

    assert(picoui_widget_claim_focus(&line_edit->widget) == 0);
    assert(picoui_keyboard_update(keyboard) == 0);
    assert(ld_keyboard->pBtnList != 0);
    assert(ld_keyboard->isWaitInit == false);
    assert(picoui_keyboard_button_update(keyboard, '9') == 0);
    assert(ld_keyboard->keyCode == '9');
    assert(picoui_keyboard_update(0) == -1);
    assert(picoui_keyboard_button_update(0, '0') == -1);
    assert(picoui_keyboard_button_update(keyboard, 0x1FFU) == -1);
    picoui_app_destroy(app);
}

static void test_keyboard_weak_hooks_remain_backend_private_not_public_api(void)
{
    struct picoui_app *app;
    struct picoui_window *win;
    struct picoui_keyboard *keyboard;
    struct picoui_backend_widget *backend;
    ldKeyboard_t *ld_keyboard;

    win = test_window_create(&app);
    keyboard = picoui_keyboard_create(win, "keyboard_backend_private_hooks");

    assert(keyboard != 0);
    backend = (struct picoui_backend_widget *)keyboard->widget.backend_widget;
    assert(backend != 0);
    ld_keyboard = (ldKeyboard_t *)backend->ld_widget;
    assert(ld_keyboard != 0);
    assert(ldKeyboardGetTargetBtnList(ld_keyboard) != 0);
    ldKeyboardCallback(ld_keyboard, SIGNAL_PRESS);
    assert(ldKeyboardBtnUserDraw(0, ld_keyboard, (kbBtnInfo_t *)ld_keyboard->pBtnList) == false);
    picoui_app_destroy(app);
}

static void test_keyboard_init_and_shared_base_aliases_round_trip(void)
{
    struct picoui_app *app;
    struct picoui_window *win;
    struct picoui_keyboard *keyboard;
    struct picoui_backend_widget *backend;
    ldBase_t *ld_base;

    win = test_window_create(&app);
    keyboard = picoui_keyboard_create(win, "keyboard_base_aliases");
    assert(keyboard != 0);
    backend = (struct picoui_backend_widget *)keyboard->widget.backend_widget;
    assert(backend != 0);
    ld_base = (ldBase_t *)backend->ld_widget;
    assert(ld_base != 0);

    assert(picoui_widget_set_pos(&keyboard->widget, 9, 19) == 0);
    assert(picoui_widget_set_visible(&keyboard->widget, 0) == 0);
    assert(picoui_widget_set_opacity(&keyboard->widget, 55) == 0);

    assert(ld_base->tRegion.tLocation.iX == 9);
    assert(ld_base->tRegion.tLocation.iY == 19);
    assert(ld_base->isHidden == false);
    assert(ld_base->opa == 55);
    picoui_app_destroy(app);
}

static void test_keyboard_exit_clears_focus_or_edit_session(void)
{
    struct picoui_app *app;
    struct picoui_window *win;
    struct picoui_keyboard *keyboard;
    struct picoui_line_edit *line_edit;
    int editing = -1;

    win = test_window_create(&app);
    keyboard = picoui_keyboard_create(win, "keyboard_exit");
    line_edit = picoui_line_edit_create(win, "line_edit_exit");

    assert(keyboard != 0);
    assert(line_edit != 0);
    line_edit->editing = 1;
    assert(picoui_widget_claim_editing(&line_edit->widget) == 0);
    assert(picoui_widget_is_editing_owner(&line_edit->widget) == 1);
    assert(picoui_widget_claim_focus(&line_edit->widget) == 0);
    assert(picoui_widget_claim_focus(&keyboard->widget) == 0);
    assert(picoui_keyboard_exit(keyboard) == 0);
    assert(picoui_line_edit_get_editing(line_edit, &editing) == 0);
    assert(editing == 0);
    assert(picoui_widget_is_editing_owner(&line_edit->widget) == 0);
    assert(picoui_widget_is_focus_owner(&keyboard->widget) == 0);
    picoui_app_destroy(app);
}

static void test_keyboard_click_respects_focus_owner(void)
{
    struct picoui_app *app;
    struct picoui_window *win;
    struct picoui_keyboard *keyboard;

    win = test_window_create(&app);
    keyboard = picoui_keyboard_create(win, "keyboard_click_gate");
    assert(keyboard != 0);

    assert(picoui_keyboard_click(keyboard) == -1);
    assert(picoui_widget_claim_focus(&keyboard->widget) == 0);
    assert(picoui_keyboard_click(keyboard) == 0);
    picoui_app_destroy(app);
}

static void test_keyboard_has_explicit_final_gate_coverage_contract(void)
{
    struct picoui_app *app;
    struct picoui_window *win;
    struct picoui_keyboard *keyboard;
    struct picoui_backend_widget *backend;

    win = test_window_create(&app);
    keyboard = picoui_keyboard_create(win, "keyboard_gate_contract");
    assert(keyboard != 0);
    backend = (struct picoui_backend_widget *)keyboard->widget.backend_widget;
    assert(backend != 0);
    assert(backend->kind == PICOUI_BACKEND_WIDGET_KEYBOARD);
    assert(backend->ld_widget != 0);
    picoui_app_destroy(app);
}

int main(void)
{
    test_keyboard_dispatches_ascii_into_focused_line_edit();
    test_keyboard_dispatches_ascii_into_editing_owner_before_focus_owner();
    test_keyboard_navigation_preserves_editing_owner_model_truth();
    test_keyboard_rejects_ascii_when_target_is_not_line_edit();
    test_keyboard_navigation_signal_respects_focus_owner();
    test_keyboard_update_and_button_update_touch_native_state();
    test_keyboard_weak_hooks_remain_backend_private_not_public_api();
    test_keyboard_init_and_shared_base_aliases_round_trip();
    test_keyboard_exit_clears_focus_or_edit_session();
    test_keyboard_click_respects_focus_owner();
    test_keyboard_has_explicit_final_gate_coverage_contract();
    return 0;
}
