#include "app.h"
#include "keyboard.h"
#include "line_edit.h"
#include "text.h"
#include "window.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldKeyboard.h"
#include "../../../src/gui/ldLineEdit.h"
#include "../../../src/gui/ldText.h"
#include "internal.h"

#include <assert.h>
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>

static const char *test_self_binary_path = 0;

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
    assert(pclose(pipe) != -1);
}

static int keyboard_event_count = 0;
static int keyboard_event_last_signal = -1;
static unsigned int keyboard_event_last_key_code = 0;
static int keyboard_event_cookie = 0;
static int keyboard_draw_count = 0;
static unsigned int keyboard_draw_last_key_code = 0;
static int keyboard_draw_cookie = 0;

static void on_keyboard_event(struct tinyui_keyboard *keyboard,
                              unsigned int key_code,
                              enum tinyui_native_signal signal,
                              void *user_data)
{
    assert(keyboard != 0);
    assert(user_data == &keyboard_event_cookie);
    keyboard_event_count++;
    keyboard_event_last_key_code = key_code;
    keyboard_event_last_signal = (int)signal;
}

static void on_keyboard_draw(struct tinyui_keyboard *keyboard,
                             const struct tinyui_keyboard_button *button,
                             void *user_data)
{
    assert(keyboard != 0);
    assert(button != 0);
    assert(user_data == &keyboard_draw_cookie);
    keyboard_draw_count++;
    keyboard_draw_last_key_code = button->key_code;
}

static struct tinyui_window *test_window_create(struct tinyui_app **app_out)
{
    struct tinyui_app *app;
    struct tinyui_window *win;

    app = tinyui_app_create();
    assert(app != 0);
    win = tinyui_window_create(app, "root");
    assert(win != 0);
    *app_out = app;
    return win;
}

static void test_keyboard_event_capture(struct tinyui_keyboard *keyboard,
                                        unsigned int key_code,
                                        unsigned int signal,
                                        void *user_data)
{
    unsigned int *capture = user_data;

    assert(keyboard != 0);
    assert(capture != 0);
    capture[0] = key_code;
    capture[1] = signal;
    capture[2] += 1U;
}

static void test_keyboard_create_builds_direct_backend_mapping(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win;
    struct tinyui_keyboard *keyboard;
    struct tinyui_backend_widget *backend;
    struct tinyui_backend_widget *parent_backend;
    ldKeyboard_t *ld_keyboard;

    win = test_window_create(&app);
    keyboard = tinyui_keyboard_create(win, "keyboard_direct_mapping");
    assert(keyboard != 0);

    backend = (struct tinyui_backend_widget *)keyboard->widget.backend_widget;
    parent_backend = (struct tinyui_backend_widget *)win->widget.backend_widget;
    assert(backend != 0);
    assert(parent_backend != 0);
    assert(backend->kind == TINYUI_BACKEND_WIDGET_KEYBOARD);
    assert(backend->owner == parent_backend->owner);
    assert(backend->root == parent_backend->root);
    assert(backend->parent == parent_backend);
    assert(backend->ld_name_id != 0);
    assert(backend->host_widget == &keyboard->widget);
    assert(backend->ld_event_bridge_scene != 0);
    assert(backend->ld_event_bridge_sender == backend->ld_widget);
    ld_keyboard = (ldKeyboard_t *)backend->ld_widget;
    assert(ld_keyboard != 0);
    assert(((ldBase_t *)ld_keyboard)->pInfo == backend);
    tinyui_app_destroy(app);
}

static void test_keyboard_draw_callback_round_trip(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win;
    struct tinyui_keyboard *keyboard;
    static const struct tinyui_keyboard_button custom_buttons[] = {
        {.x = 0, .y = 0, .width = 18, .height = 10, .text = "A", .key_code = 'a', .press_color = 0x102030, .release_color = 0x405060},
        {.x = 20, .y = 0, .width = 18, .height = 10, .text = "B", .key_code = 'b', .press_color = 0x708090, .release_color = 0xA0B0C0},
    };

    keyboard_draw_count = 0;
    keyboard_draw_last_key_code = 0;

    win = test_window_create(&app);
    keyboard = tinyui_keyboard_create(win, "keyboard_draw_callback");

    assert(keyboard != 0);
    assert(tinyui_keyboard_set_layout(keyboard, custom_buttons, 2) == 0);
    assert(tinyui_keyboard_set_draw_callback(0, on_keyboard_draw, &keyboard_draw_cookie) == -1);
    assert(tinyui_keyboard_set_draw_callback(keyboard, on_keyboard_draw, &keyboard_draw_cookie) == 0);
    assert(tinyui_keyboard_update(keyboard) == 0);
    assert(keyboard_draw_count == 2);
    assert(keyboard_draw_last_key_code == 'b');
    assert(tinyui_keyboard_set_draw_callback(keyboard, 0, 0) == 0);
    tinyui_app_destroy(app);
}

static void test_keyboard_dispatches_ascii_into_focused_line_edit(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win;
    struct tinyui_keyboard *keyboard;
    struct tinyui_line_edit *line_edit;

    win = test_window_create(&app);
    keyboard = tinyui_keyboard_create(win, "keyboard_ascii");
    line_edit = tinyui_line_edit_create(win, "line_edit_ascii");

    assert(keyboard != 0);
    assert(line_edit != 0);
    assert(tinyui_line_edit_set_text(line_edit, "A") == 0);
    assert(tinyui_widget_claim_focus(&line_edit->widget) == 0);
    assert(tinyui_keyboard_input_ascii(keyboard, 'b') == 0);
    assert(strcmp(tinyui_line_edit_get_text(line_edit), "Ab") == 0);
    tinyui_app_destroy(app);
}

static void test_keyboard_dispatches_ascii_into_editing_owner_before_focus_owner(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win;
    struct tinyui_keyboard *keyboard;
    struct tinyui_line_edit *line_edit;
    struct tinyui_text *text;

    win = test_window_create(&app);
    keyboard = tinyui_keyboard_create(win, "keyboard_ascii_editing_owner");
    line_edit = tinyui_line_edit_create(win, "line_edit_ascii_editing_owner");
    text = tinyui_text_create(win, "text_ascii_focus_owner");

    assert(keyboard != 0);
    assert(line_edit != 0);
    assert(text != 0);
    assert(tinyui_line_edit_set_text(line_edit, "Q") == 0);
    assert(tinyui_text_set_text(text, "focus-owner") == 0);
    line_edit->editing = 1;
    assert(tinyui_widget_claim_editing(&line_edit->widget) == 0);
    assert(tinyui_widget_claim_focus(&text->widget) == 0);
    assert(tinyui_keyboard_input_ascii(keyboard, 'w') == 0);
    assert(strcmp(tinyui_line_edit_get_text(line_edit), "Qw") == 0);
    assert(strcmp((const char *)((ldText_t *)((struct tinyui_backend_widget *)text->widget.backend_widget)->ld_widget)->pStr,
                  "focus-owner") == 0);
    tinyui_app_destroy(app);
}

static void test_keyboard_navigation_preserves_editing_owner_model_truth(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win;
    struct tinyui_keyboard *keyboard;
    struct tinyui_line_edit *line_edit;
    struct tinyui_text *text;
    struct tinyui_backend_widget *line_edit_backend;
    ldLineEdit_t *ld_line_edit;
    int editing = -1;

    win = test_window_create(&app);
    keyboard = tinyui_keyboard_create(win, "keyboard_navigation_editing_owner");
    line_edit = tinyui_line_edit_create(win, "line_edit_navigation_editing_owner");
    text = tinyui_text_create(win, "text_navigation_focus_owner");

    assert(keyboard != 0);
    assert(line_edit != 0);
    assert(text != 0);
    assert(tinyui_line_edit_set_text(line_edit, "Q") == 0);
    assert(tinyui_text_set_text(text, "focus-owner") == 0);

    line_edit_backend = (struct tinyui_backend_widget *)line_edit->widget.backend_widget;
    assert(line_edit_backend != 0);
    ld_line_edit = (ldLineEdit_t *)line_edit_backend->ld_widget;
    assert(ld_line_edit != 0);

    line_edit->editing = 1;
    ld_line_edit->isEditing = true;
    assert(tinyui_widget_claim_editing(&line_edit->widget) == 0);
    assert(tinyui_widget_claim_focus(&text->widget) == 0);
    assert(tinyui_keyboard_input_ascii(keyboard, 'w') == 0);
    assert(tinyui_line_edit_get_editing(line_edit, &editing) == 0);
    assert(editing == 1);
    assert(strcmp(tinyui_line_edit_get_text(line_edit), "Qw") == 0);
    assert(strcmp((const char *)ldLineEditGetText(ld_line_edit), "Qw") == 0);
    assert(line_edit->widget.last_edit_result == TINYUI_EDIT_RESULT_NONE);
    assert(tinyui_widget_is_editing_owner(&line_edit->widget) == 1);
    assert(tinyui_widget_is_focus_owner(&text->widget) == 1);
    tinyui_app_destroy(app);
}

static void test_keyboard_rejects_ascii_when_target_is_not_line_edit(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win;
    struct tinyui_keyboard *keyboard;
    struct tinyui_text *text;
    ldText_t *ld_text;

    win = test_window_create(&app);
    keyboard = tinyui_keyboard_create(win, "keyboard_ascii_reject_text");
    text = tinyui_text_create(win, "text_ascii_reject");

    assert(keyboard != 0);
    assert(text != 0);
    assert(tinyui_text_set_text(text, "plain-text") == 0);
    ld_text = (ldText_t *)((struct tinyui_backend_widget *)text->widget.backend_widget)->ld_widget;
    assert(ld_text != 0);
    assert(tinyui_widget_claim_focus(&text->widget) == 0);
    assert(tinyui_keyboard_input_ascii(keyboard, 'x') == -1);
    assert(strcmp((const char *)ld_text->pStr, "plain-text") == 0);
    tinyui_app_destroy(app);
}

static void test_keyboard_navigation_signal_respects_focus_owner(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win;
    struct tinyui_keyboard *keyboard;
    struct tinyui_backend_widget *backend;
    ldKeyboard_t *ld_keyboard;

    win = test_window_create(&app);
    keyboard = tinyui_keyboard_create(win, "keyboard_nav");
    assert(keyboard != 0);

    backend = (struct tinyui_backend_widget *)keyboard->widget.backend_widget;
    assert(backend != 0);
    ld_keyboard = (ldKeyboard_t *)backend->ld_widget;
    assert(ld_keyboard != 0);

    ld_keyboard->keyCode = 'a';
    assert(tinyui_keyboard_navigate(keyboard, NAV_RIGHT) == -1);
    assert(ld_keyboard->keyCode == 'a');
    assert(tinyui_widget_claim_focus(&keyboard->widget) == 0);
    assert(tinyui_keyboard_navigate(keyboard, NAV_RIGHT) == 0);
    assert(ld_keyboard->keyCode != 'a');
    tinyui_app_destroy(app);
}

static void test_keyboard_update_and_button_update_touch_native_state(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win;
    struct tinyui_keyboard *keyboard;
    struct tinyui_line_edit *line_edit;
    struct tinyui_backend_widget *backend;
    ldKeyboard_t *ld_keyboard;

    win = test_window_create(&app);
    keyboard = tinyui_keyboard_create(win, "keyboard_update");
    line_edit = tinyui_line_edit_create(win, "line_edit_update_target");

    assert(keyboard != 0);
    assert(line_edit != 0);
    backend = (struct tinyui_backend_widget *)keyboard->widget.backend_widget;
    assert(backend != 0);
    ld_keyboard = (ldKeyboard_t *)backend->ld_widget;
    assert(ld_keyboard != 0);

    assert(tinyui_widget_claim_focus(&line_edit->widget) == 0);
    assert(tinyui_keyboard_update(keyboard) == 0);
    assert(ld_keyboard->pBtnList != 0);
    assert(ld_keyboard->isWaitInit == false);
    assert(tinyui_keyboard_button_update(keyboard, '9') == 0);
    assert(ld_keyboard->keyCode == '9');
    assert(tinyui_keyboard_update(0) == -1);
    assert(tinyui_keyboard_button_update(0, '0') == -1);
    assert(tinyui_keyboard_button_update(keyboard, 0x1FFU) == -1);
    tinyui_app_destroy(app);
}

static void test_keyboard_set_layout_replaces_native_button_list(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win;
    struct tinyui_keyboard *keyboard;
    struct tinyui_line_edit *line_edit;
    struct tinyui_backend_widget *backend;
    ldKeyboard_t *ld_keyboard;
    static const struct tinyui_keyboard_button buttons[] = {
        {.x = 3, .y = 5, .width = 20, .height = 10, .text = "X", .key_code = 'x', .press_color = 0x11, .release_color = 0x22},
        {.x = 25, .y = 5, .width = 20, .height = 10, .text = "Y", .key_code = 'y', .press_color = 0x33, .release_color = 0x44},
    };

    win = test_window_create(&app);
    keyboard = tinyui_keyboard_create(win, "keyboard_custom_layout");
    line_edit = tinyui_line_edit_create(win, "line_edit_custom_layout");

    assert(keyboard != 0);
    assert(line_edit != 0);
    backend = (struct tinyui_backend_widget *)keyboard->widget.backend_widget;
    assert(backend != 0);
    ld_keyboard = (ldKeyboard_t *)backend->ld_widget;
    assert(ld_keyboard != 0);

    assert(tinyui_keyboard_set_buttons(keyboard, buttons, 2) == 0);
    assert(tinyui_widget_claim_focus(&line_edit->widget) == 0);
    assert(tinyui_keyboard_update(keyboard) == 0);
    assert(ld_keyboard->pBtnList != 0);
    assert(ld_keyboard->pBtnList[0].region.tLocation.iX == 3);
    assert(ld_keyboard->pBtnList[0].region.tLocation.iY == 5);
    assert(ld_keyboard->pBtnList[0].region.tSize.iWidth == 20);
    assert(ld_keyboard->pBtnList[0].region.tSize.iHeight == 10);
    assert(strcmp((const char *)ld_keyboard->pBtnList[0].pText, "X") == 0);
    assert(ld_keyboard->pBtnList[0].keyCode == 'x');
    assert(ld_keyboard->pBtnList[0].pressColor == __RGB(0x00, 0x00, 0x11));
    assert(ld_keyboard->pBtnList[0].releaseColor == __RGB(0x00, 0x00, 0x22));
    assert(strcmp((const char *)ld_keyboard->pBtnList[1].pText, "Y") == 0);
    assert(ld_keyboard->pBtnList[1].keyCode == 'y');
    assert(ld_keyboard->pBtnList[2].pText == 0);
    assert(ld_keyboard->pBtnList[2].keyCode == 0);
    assert(tinyui_keyboard_set_buttons(keyboard, 0, 0) == 0);
    assert(tinyui_keyboard_set_buttons(keyboard, buttons, 0) == 0);
    tinyui_app_destroy(app);
}

static void test_keyboard_callback_observes_button_update_and_click(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win;
    struct tinyui_keyboard *keyboard;
    struct tinyui_line_edit *line_edit;
    unsigned int capture[3] = {0, 0, 0};

    win = test_window_create(&app);
    keyboard = tinyui_keyboard_create(win, "keyboard_callback");
    line_edit = tinyui_line_edit_create(win, "line_edit_callback");

    assert(keyboard != 0);
    assert(line_edit != 0);
    assert(tinyui_keyboard_set_on_key_event(keyboard, test_keyboard_event_capture, capture) == 0);
    assert(tinyui_widget_claim_focus(&line_edit->widget) == 0);
    assert(tinyui_widget_claim_focus(&keyboard->widget) == 0);
    assert(tinyui_keyboard_button_update(keyboard, '8') == 0);
    assert(capture[0] == '8');
    assert(capture[1] == TINYUI_NATIVE_SIGNAL_VALUE_CHANGED);
    assert(capture[2] == 1U);
    assert(tinyui_keyboard_click(keyboard) == 0);
    assert(capture[0] == '8');
    assert(capture[1] == TINYUI_NATIVE_SIGNAL_PRESS);
    assert(capture[2] == 2U);
    assert(tinyui_keyboard_set_on_key_event(keyboard, 0, 0) == 0);
    assert(tinyui_keyboard_button_update(keyboard, '9') == 0);
    assert(capture[2] == 2U);
    tinyui_app_destroy(app);
}

static void test_keyboard_weak_hooks_remain_backend_private_not_public_api(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win;
    struct tinyui_keyboard *keyboard;
    struct tinyui_backend_widget *backend;
    ldKeyboard_t *ld_keyboard;

    win = test_window_create(&app);
    keyboard = tinyui_keyboard_create(win, "keyboard_backend_private_hooks");

    assert(keyboard != 0);
    backend = (struct tinyui_backend_widget *)keyboard->widget.backend_widget;
    assert(backend != 0);
    ld_keyboard = (ldKeyboard_t *)backend->ld_widget;
    assert(ld_keyboard != 0);
    assert(ldKeyboardGetTargetBtnList(ld_keyboard) != 0);
    ldKeyboardCallback(ld_keyboard, SIGNAL_PRESS);
    assert(tinyui_keyboard_set_on_key_event(keyboard, 0, 0) == 0);
    assert(tinyui_keyboard_set_buttons(keyboard, 0, 0) == 0);
    assert(keyboard->event_cb == 0);
    assert(keyboard->buttons == 0);
    assert(keyboard->layout_entries == 0);
    assert(keyboard->layout_count == 0);
    tinyui_app_destroy(app);
}

static void test_keyboard_init_and_shared_base_aliases_round_trip(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win;
    struct tinyui_keyboard *keyboard;
    struct tinyui_backend_widget *backend;
    ldBase_t *ld_base;
    arm_2d_region_t region;

    win = test_window_create(&app);
    keyboard = tinyui_keyboard_create(win, "keyboard_base_aliases");
    assert(keyboard != 0);
    backend = (struct tinyui_backend_widget *)keyboard->widget.backend_widget;
    assert(backend != 0);
    ld_base = (ldBase_t *)backend->ld_widget;
    assert(ld_base != 0);

    assert(tinyui_widget_set_pos(&keyboard->widget, 9, 19) == 0);
    assert(tinyui_widget_set_visible(&keyboard->widget, 0) == 0);
    assert(tinyui_widget_set_opacity(&keyboard->widget, 55) == 0);

    region = ldBaseGetRegion(ld_base);
    assert(region.tLocation.iX == 9);
    assert(region.tLocation.iY == 19);
    assert(ld_base->isHidden == true);
    assert(ld_base->opacity == 55);
    tinyui_app_destroy(app);
}

static void test_keyboard_exit_clears_focus_or_edit_session(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win;
    struct tinyui_keyboard *keyboard;
    struct tinyui_line_edit *line_edit;
    int editing = -1;

    win = test_window_create(&app);
    keyboard = tinyui_keyboard_create(win, "keyboard_exit");
    line_edit = tinyui_line_edit_create(win, "line_edit_exit");

    assert(keyboard != 0);
    assert(line_edit != 0);
    line_edit->editing = 1;
    assert(tinyui_widget_claim_editing(&line_edit->widget) == 0);
    assert(tinyui_widget_is_editing_owner(&line_edit->widget) == 1);
    assert(tinyui_widget_claim_focus(&line_edit->widget) == 0);
    assert(tinyui_widget_claim_focus(&keyboard->widget) == 0);
    assert(tinyui_keyboard_exit(keyboard) == 0);
    assert(tinyui_line_edit_get_editing(line_edit, &editing) == 0);
    assert(editing == 0);
    assert(tinyui_widget_is_editing_owner(&line_edit->widget) == 0);
    assert(tinyui_widget_is_focus_owner(&keyboard->widget) == 0);
    tinyui_app_destroy(app);
}

static void test_keyboard_click_respects_focus_owner(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win;
    struct tinyui_keyboard *keyboard;

    win = test_window_create(&app);
    keyboard = tinyui_keyboard_create(win, "keyboard_click_gate");
    assert(keyboard != 0);

    assert(tinyui_keyboard_click(keyboard) == -1);
    assert(tinyui_widget_claim_focus(&keyboard->widget) == 0);
    assert(tinyui_keyboard_click(keyboard) == 0);
    tinyui_app_destroy(app);
}

static void test_keyboard_click_and_exit_backend_symbols_are_no_longer_public(void)
{
    assert(dlsym(RTLD_DEFAULT, "tinyui_backend_keyboard_click") == 0);
    assert(dlsym(RTLD_DEFAULT, "tinyui_backend_keyboard_exit") == 0);
    assert_self_binary_lacks_symbol("tinyui_backend_keyboard_click");
    assert_self_binary_lacks_symbol("tinyui_backend_keyboard_exit");
}

static void test_keyboard_update_and_button_update_backend_symbols_are_no_longer_public(void)
{
    assert(dlsym(RTLD_DEFAULT, "tinyui_backend_keyboard_update") == 0);
    assert(dlsym(RTLD_DEFAULT, "tinyui_backend_keyboard_button_update") == 0);
    assert_self_binary_lacks_symbol("tinyui_backend_keyboard_update");
    assert_self_binary_lacks_symbol("tinyui_backend_keyboard_button_update");
}

static void test_keyboard_ascii_and_navigate_backend_symbols_are_no_longer_public(void)
{
    assert(dlsym(RTLD_DEFAULT, "tinyui_backend_keyboard_input_ascii") == 0);
    assert(dlsym(RTLD_DEFAULT, "tinyui_backend_keyboard_navigate") == 0);
    assert_self_binary_lacks_symbol("tinyui_backend_keyboard_input_ascii");
    assert_self_binary_lacks_symbol("tinyui_backend_keyboard_navigate");
}

static void test_keyboard_has_explicit_final_gate_coverage_contract(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win;
    struct tinyui_keyboard *keyboard;
    struct tinyui_backend_widget *backend;

    win = test_window_create(&app);
    keyboard = tinyui_keyboard_create(win, "keyboard_gate_contract");
    assert(keyboard != 0);
    backend = (struct tinyui_backend_widget *)keyboard->widget.backend_widget;
    assert(backend != 0);
    assert(backend->kind == TINYUI_BACKEND_WIDGET_KEYBOARD);
    assert(backend->ld_widget != 0);
    tinyui_app_destroy(app);
}

static void test_keyboard_button_update_rejects_corrupted_backend_binding_without_native_or_public_drift(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win;
    struct tinyui_keyboard *keyboard;
    struct tinyui_backend_widget *backend;
    enum tinyui_backend_widget_kind saved_kind;
    ldKeyboard_t *ld_keyboard;
    uint8_t saved_key_code;
    bool saved_is_key_select;
    unsigned int capture[3] = {0, 0, 0};

    win = test_window_create(&app);
    keyboard = tinyui_keyboard_create(win, "keyboard_corrupted_binding");

    assert(keyboard != 0);
    assert(tinyui_keyboard_set_on_key_event(keyboard, test_keyboard_event_capture, capture) == 0);

    backend = (struct tinyui_backend_widget *)keyboard->widget.backend_widget;
    assert(backend != 0);
    ld_keyboard = (ldKeyboard_t *)backend->ld_widget;
    assert(ld_keyboard != 0);
    saved_key_code = ld_keyboard->keyCode;
    saved_is_key_select = ld_keyboard->isKeySelect;

    saved_kind = backend->kind;
    backend->kind = TINYUI_BACKEND_WIDGET_LABEL;

    assert(tinyui_keyboard_button_update(keyboard, '7') == -1);
    assert(ld_keyboard->keyCode == saved_key_code);
    assert(ld_keyboard->isKeySelect == saved_is_key_select);
    assert(capture[0] == 0U);
    assert(capture[1] == 0U);
    assert(capture[2] == 0U);
    assert(backend->kind == TINYUI_BACKEND_WIDGET_LABEL);

    backend->kind = saved_kind;
    assert(tinyui_keyboard_button_update(keyboard, '7') == 0);
    assert(ld_keyboard->keyCode == '7');
    assert(ld_keyboard->isKeySelect == true);
    assert(capture[0] == '7');
    assert(capture[1] == TINYUI_NATIVE_SIGNAL_VALUE_CHANGED);
    assert(capture[2] == 1U);
    tinyui_app_destroy(app);
}

static void test_keyboard_custom_layout_round_trips_into_native_button_table(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win;
    struct tinyui_keyboard *keyboard;
    struct tinyui_backend_widget *backend;
    ldKeyboard_t *ld_keyboard;
    static const struct tinyui_keyboard_button custom_buttons[] = {
        {.text = "A", .key_code = 'A', .x = 10, .y = 120, .width = 50, .height = 24, .press_color = 0x010203U, .release_color = 0x040506U},
        {.text = "OK", .key_code = 0x0dU, .x = 70, .y = 120, .width = 60, .height = 24, .press_color = 0x111213U, .release_color = 0x141516U},
    };
    const struct tinyui_keyboard_button *round_trip = 0;
    int button_count = -1;

    win = test_window_create(&app);
    keyboard = tinyui_keyboard_create(win, "keyboard_custom_layout");
    assert(keyboard != 0);

    assert(tinyui_keyboard_set_buttons(keyboard, custom_buttons, 2) == 0);
    assert(tinyui_keyboard_get_buttons(keyboard, &round_trip, &button_count) == 0);
    assert(round_trip == custom_buttons);
    assert(button_count == 2);

    backend = (struct tinyui_backend_widget *)keyboard->widget.backend_widget;
    assert(backend != 0);
    ld_keyboard = (ldKeyboard_t *)backend->ld_widget;
    assert(ld_keyboard != 0);

    assert(tinyui_keyboard_update(keyboard) == 0);
    assert(ld_keyboard->pBtnList != 0);
    assert(strcmp((const char *)ld_keyboard->pBtnList[0].pText, "A") == 0);
    assert(ld_keyboard->pBtnList[0].keyCode == 'A');
    assert(ld_keyboard->pBtnList[0].region.tLocation.iX == 10);
    assert(ld_keyboard->pBtnList[0].region.tLocation.iY == 120);
    assert(ld_keyboard->pBtnList[0].region.tSize.iWidth == 50);
    assert(ld_keyboard->pBtnList[0].region.tSize.iHeight == 24);
    assert(ld_keyboard->pBtnList[0].pressColor == __RGB(0x01, 0x02, 0x03));
    assert(ld_keyboard->pBtnList[0].releaseColor == __RGB(0x04, 0x05, 0x06));
    assert(strcmp((const char *)ld_keyboard->pBtnList[1].pText, "OK") == 0);
    assert(ld_keyboard->pBtnList[1].keyCode == 0x0dU);
    assert(ld_keyboard->pBtnList[2].pText == 0);

    assert(tinyui_keyboard_set_buttons(keyboard, 0, 0) == 0);
    assert(tinyui_keyboard_get_buttons(keyboard, &round_trip, &button_count) == 0);
    assert(round_trip == 0);
    assert(button_count == 0);
    assert(tinyui_keyboard_update(keyboard) == 0);
    assert(ld_keyboard->pBtnList != 0);
    assert(ldKeyboardGetTargetBtnList(ld_keyboard) == ld_keyboard->pBtnList);
    tinyui_app_destroy(app);
}

static void test_keyboard_native_press_and_release_emit_tinyui_callback(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win;
    struct tinyui_keyboard *keyboard;
    struct tinyui_backend_widget *backend;
    ldKeyboard_t *ld_keyboard;

    keyboard_event_count = 0;
    keyboard_event_last_signal = -1;
    keyboard_event_last_key_code = 0;

    win = test_window_create(&app);
    keyboard = tinyui_keyboard_create(win, "keyboard_event_bridge");
    assert(keyboard != 0);
    assert(tinyui_keyboard_set_on_key_event(0, on_keyboard_event, &keyboard_event_cookie) == -1);
    assert(tinyui_keyboard_set_on_key_event(keyboard, on_keyboard_event, &keyboard_event_cookie) == 0);

    backend = (struct tinyui_backend_widget *)keyboard->widget.backend_widget;
    assert(backend != 0);
    ld_keyboard = (ldKeyboard_t *)backend->ld_widget;
    assert(ld_keyboard != 0);
    ld_keyboard->keyCode = 'Z';

    assert(tinyui_widget_dispatch_native_signal(backend, SIGNAL_PRESS, 0) == 0);
    assert(keyboard_event_count == 1);
    assert(keyboard_event_last_signal == TINYUI_NATIVE_SIGNAL_PRESS);
    assert(keyboard_event_last_key_code == 'Z');
    assert(tinyui_keyboard_get_selected_key_code(keyboard) == 'Z');

    assert(tinyui_widget_dispatch_native_signal(backend, SIGNAL_RELEASE, 0) == 0);
    assert(keyboard_event_count == 2);
    assert(keyboard_event_last_signal == TINYUI_NATIVE_SIGNAL_RELEASE);
    assert(keyboard_event_last_key_code == 'Z');
    tinyui_app_destroy(app);
}

int main(void)
{
    test_self_binary_path = "tests/tinyui/test_tinyui_keyboard";
    test_keyboard_create_builds_direct_backend_mapping();
    test_keyboard_dispatches_ascii_into_focused_line_edit();
    test_keyboard_dispatches_ascii_into_editing_owner_before_focus_owner();
    test_keyboard_navigation_preserves_editing_owner_model_truth();
    test_keyboard_rejects_ascii_when_target_is_not_line_edit();
    test_keyboard_navigation_signal_respects_focus_owner();
    test_keyboard_update_and_button_update_touch_native_state();
    test_keyboard_set_layout_replaces_native_button_list();
    test_keyboard_callback_observes_button_update_and_click();
    test_keyboard_weak_hooks_remain_backend_private_not_public_api();
    test_keyboard_init_and_shared_base_aliases_round_trip();
    test_keyboard_exit_clears_focus_or_edit_session();
    test_keyboard_click_respects_focus_owner();
    test_keyboard_click_and_exit_backend_symbols_are_no_longer_public();
    test_keyboard_update_and_button_update_backend_symbols_are_no_longer_public();
    test_keyboard_ascii_and_navigate_backend_symbols_are_no_longer_public();
    test_keyboard_has_explicit_final_gate_coverage_contract();
    test_keyboard_button_update_rejects_corrupted_backend_binding_without_native_or_public_drift();
    test_keyboard_custom_layout_round_trips_into_native_button_table();
    test_keyboard_native_press_and_release_emit_tinyui_callback();
    test_keyboard_draw_callback_round_trip();
    return 0;
}
