/*
 * TinyUI keyboard unit tests — M3 Task 4 L3/L4 harness.
 *
 * Validates layout/action, focus owner gates, real ldKeyboard_t state,
 * capacity rejection, and dedicated key-event callback observability.
 */

#include "tinyui.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldKeyboard.h"
#include "../../../src/gui/ldLineEdit.h"
#include "internal.h"
#include "widgets/keyboard.h"
#include "widgets/line_edit.h"
#include "widgets/text.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

enum {
    /* Soft upper bound for custom layout rows; public API returns -1 beyond. */
    TEST_KEYBOARD_LAYOUT_SOFT_MAX = 64
};

static int g_event_count;
static int g_event_last_signal = -1;
static unsigned int g_event_last_key_code;
static int g_event_cookie;
static int g_draw_count;
static unsigned int g_draw_last_key_code;
static int g_draw_cookie;

static void reset_event_fixture(void)
{
    g_event_count = 0;
    g_event_last_signal = -1;
    g_event_last_key_code = 0U;
}

static void on_keyboard_event(tinyui_obj_t *keyboard,
                              unsigned int key_code,
                              tinyui_signal_t signal,
                              void *user_data)
{
    assert(keyboard != 0);
    assert(user_data == &g_event_cookie);
    g_event_count += 1;
    g_event_last_key_code = key_code;
    g_event_last_signal = (int)signal;
}

static void on_keyboard_draw(tinyui_obj_t *keyboard,
                             const struct tinyui_keyboard_button *button,
                             void *user_data)
{
    assert(keyboard != 0);
    assert(button != 0);
    assert(user_data == &g_draw_cookie);
    g_draw_count += 1;
    g_draw_last_key_code = button->key_code;
}

static void test_keyboard_event_capture(tinyui_obj_t *keyboard,
                                        unsigned int key_code,
                                        tinyui_signal_t signal,
                                        void *user_data)
{
    unsigned int *capture = user_data;

    assert(keyboard != 0);
    assert(capture != 0);
    capture[0] = key_code;
    capture[1] = (unsigned int)signal;
    capture[2] += 1U;
}

static struct tinyui_widget *kb_widget(tinyui_obj_t *obj)
{
    return (struct tinyui_widget *)(void *)obj;
}

static ldKeyboard_t *kb_ld(tinyui_obj_t *obj)
{
    struct tinyui_widget *backend = kb_widget(obj);

    assert(backend != 0);
    assert(backend->ld_widget != 0);
    return (ldKeyboard_t *)backend->ld_widget;
}

static void test_keyboard_init_and_shared_base_aliases_round_trip(tinyui_obj_t *root)
{
    tinyui_obj_t *keyboard = tinyui_keyboard_create(root);
    struct tinyui_widget *backend;
    ldBase_t *ld_base;
    arm_2d_region_t region;

    assert(keyboard != 0);
    backend = kb_widget(keyboard);
    assert(backend->kind == TINYUI_BACKEND_WIDGET_KEYBOARD);
    assert(backend->ld_widget != 0);
    assert(backend->owner != 0);
    assert(backend->ld_name_id != 0);
    assert(tinyui_runtime_internal_app_lookup_host(backend->owner, backend->ld_name_id) == backend);

    ld_base = (ldBase_t *)backend->ld_widget;
    assert(ld_base != 0);
    assert(ld_base->widgetType == widgetTypeKeyboard);
    assert(ld_base->isHidden == true);

    assert(tinyui_obj_set_pos(keyboard, 9, 19) == TINYUI_OK);
    assert(tinyui_obj_set_visible(keyboard, 0) == TINYUI_OK);
    assert(tinyui_obj_set_opacity(keyboard, 55) == TINYUI_OK);

    region = ldBaseGetRegion(ld_base);
    assert(ld_base->isHidden == true);
    assert(region.tLocation.iX != 9);
    assert(region.tLocation.iY != 19);
    assert(ld_base->opacity == 55);

    assert(tinyui_obj_set_visible(keyboard, 1) == TINYUI_OK);
    region = ldBaseGetRegion(ld_base);
    assert(region.tLocation.iX == 9);
    assert(region.tLocation.iY == 19);
}

static void test_keyboard_update_and_button_update_touch_native_state(tinyui_obj_t *root)
{
    tinyui_obj_t *keyboard = tinyui_keyboard_create(root);
    tinyui_obj_t *line_edit = tinyui_line_edit_create(root);
    ldKeyboard_t *ld_keyboard;

    assert(keyboard != 0);
    assert(line_edit != 0);
    ld_keyboard = kb_ld(keyboard);

    assert(tinyui_focus_set(line_edit) == TINYUI_OK);
    assert(tinyui_keyboard_update(keyboard) == 0);
    assert(ld_keyboard->pBtnList != 0);
    assert(ld_keyboard->isWaitInit == false);

    assert(tinyui_keyboard_button_update(keyboard, '9') == 0);
    assert(ld_keyboard->keyCode == '9');
    assert(ld_keyboard->isKeySelect == true);
    assert(tinyui_keyboard_get_selected_key_code(keyboard) == '9');

    assert(tinyui_keyboard_update(0) == -1);
    assert(tinyui_keyboard_button_update(0, '0') == -1);
    assert(tinyui_keyboard_button_update(keyboard, 0x1FFU) == -1);
}

static void test_keyboard_custom_layout_button_table_round_trip(tinyui_obj_t *root)
{
    tinyui_obj_t *keyboard = tinyui_keyboard_create(root);
    ldKeyboard_t *ld_keyboard;
    static const struct tinyui_keyboard_button custom_buttons[] = {
        {.text = "A",
         .key_code = 'A',
         .x = 10,
         .y = 120,
         .width = 50,
         .height = 24,
         .press_color = 0x010203U,
         .release_color = 0x040506U},
        {.text = "OK",
         .key_code = 0x0dU,
         .x = 70,
         .y = 120,
         .width = 60,
         .height = 24,
         .press_color = 0x111213U,
         .release_color = 0x141516U},
    };
    static const struct tinyui_keyboard_button invalid_null_text[] = {
        {.text = 0, .key_code = 'x', .x = 0, .y = 0, .width = 10, .height = 10},
    };
    static const struct tinyui_keyboard_button invalid_key_code[] = {
        {.text = "X", .key_code = 0x100U, .x = 0, .y = 0, .width = 10, .height = 10},
    };
    static const struct tinyui_keyboard_button invalid_size[] = {
        {.text = "X", .key_code = 'x', .x = 0, .y = 0, .width = -1, .height = 10},
    };
    const struct tinyui_keyboard_button *round_trip = 0;
    int button_count = -1;
    struct tinyui_keyboard_button overflow[TEST_KEYBOARD_LAYOUT_SOFT_MAX + 1];
    int i;

    assert(keyboard != 0);
    ld_keyboard = kb_ld(keyboard);

    assert(tinyui_keyboard_set_buttons(keyboard, invalid_null_text, 1) == -1);
    assert(tinyui_keyboard_set_buttons(keyboard, invalid_key_code, 1) == -1);
    assert(tinyui_keyboard_set_layout(keyboard, invalid_size, 1) == -1);

    for (i = 0; i < TEST_KEYBOARD_LAYOUT_SOFT_MAX + 1; ++i) {
        overflow[i].x = 0;
        overflow[i].y = 0;
        overflow[i].width = 8;
        overflow[i].height = 8;
        overflow[i].text = "K";
        overflow[i].key_code = (unsigned int)('a' + (i % 26));
        overflow[i].press_color = 0U;
        overflow[i].release_color = 0U;
    }
    assert(tinyui_keyboard_set_buttons(keyboard, overflow, TEST_KEYBOARD_LAYOUT_SOFT_MAX + 1) == -1);
    assert(tinyui_last_result() == TINYUI_ERROR_CAPACITY);

    assert(tinyui_keyboard_set_buttons(keyboard, custom_buttons, 2) == 0);
    assert(tinyui_keyboard_get_buttons(keyboard, &round_trip, &button_count) == 0);
    assert(round_trip == custom_buttons);
    assert(button_count == 2);

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
}

static void test_keyboard_key_event_callback_receives_native_press_release(tinyui_obj_t *root)
{
    tinyui_obj_t *keyboard = tinyui_keyboard_create(root);
    struct tinyui_widget *backend;
    ldKeyboard_t *ld_keyboard;

    reset_event_fixture();
    assert(keyboard != 0);
    assert(tinyui_keyboard_set_on_key_event(0, on_keyboard_event, &g_event_cookie) == -1);
    assert(tinyui_keyboard_set_event_callback(keyboard, on_keyboard_event, &g_event_cookie) == 0);

    backend = kb_widget(keyboard);
    ld_keyboard = kb_ld(keyboard);
    ld_keyboard->keyCode = 'Z';

    assert(tinyui_runtime_internal_widget_dispatch_native_signal(backend, SIGNAL_PRESS, 0) == 0);
    assert(g_event_count == 1);
    assert(g_event_last_signal == (int)TINYUI_SIGNAL_PRESS);
    assert(g_event_last_key_code == 'Z');
    assert(tinyui_keyboard_get_selected_key_code(keyboard) == 'Z');

    assert(tinyui_runtime_internal_widget_dispatch_native_signal(backend, SIGNAL_RELEASE, 0) == 0);
    assert(g_event_count == 2);
    assert(g_event_last_signal == (int)TINYUI_SIGNAL_RELEASE);
    assert(g_event_last_key_code == 'Z');
}

static void test_keyboard_click_respects_focus_owner(tinyui_obj_t *root)
{
    tinyui_obj_t *keyboard = tinyui_keyboard_create(root);
    unsigned int capture[3] = {0, 0, 0};
    ldKeyboard_t *ld_keyboard;

    assert(keyboard != 0);
    ld_keyboard = kb_ld(keyboard);
    assert(tinyui_keyboard_set_on_key_event(keyboard, test_keyboard_event_capture, capture) == 0);

    assert(tinyui_keyboard_click(keyboard) == -1);
    assert(capture[2] == 0U);

    assert(tinyui_focus_set(keyboard) == TINYUI_OK);
    assert(tinyui_focus_current() == keyboard);
    assert(tinyui_keyboard_button_update(keyboard, '8') == 0);
    assert(ld_keyboard->keyCode == '8');
    assert(tinyui_keyboard_click(keyboard) == 0);
    assert(capture[0] == '8');
    assert(capture[1] == (unsigned int)TINYUI_SIGNAL_PRESS);
    assert(capture[2] == 2U);
}

static void test_keyboard_exit_clears_focus_or_edit_session(tinyui_obj_t *root)
{
    tinyui_obj_t *keyboard = tinyui_keyboard_create(root);
    tinyui_obj_t *line_edit = tinyui_line_edit_create(root);
    struct tinyui_line_edit *line_edit_host;
    int editing = -1;

    assert(keyboard != 0);
    assert(line_edit != 0);
    line_edit_host = (struct tinyui_line_edit *)(void *)line_edit;
    line_edit_host->editing = 1;
    assert(tinyui_runtime_internal_widget_claim_editing(kb_widget(line_edit)) == 0);
    assert(tinyui_runtime_internal_widget_is_editing_owner(kb_widget(line_edit)) == 1);
    assert(tinyui_focus_set(line_edit) == TINYUI_OK);
    assert(tinyui_focus_set(keyboard) == TINYUI_OK);
    assert(tinyui_focus_current() == keyboard);

    assert(tinyui_keyboard_exit(keyboard) == 0);
    assert(tinyui_line_edit_get_editing(line_edit, &editing) == 0);
    assert(editing == 0);
    assert(tinyui_runtime_internal_widget_is_editing_owner(kb_widget(line_edit)) == 0);
    assert(tinyui_focus_current() != keyboard);
    assert(tinyui_runtime_internal_widget_is_focus_owner(kb_widget(keyboard)) == 0);
}

static void test_keyboard_navigation_signal_respects_focus_owner(tinyui_obj_t *root)
{
    tinyui_obj_t *keyboard = tinyui_keyboard_create(root);
    ldKeyboard_t *ld_keyboard;

    assert(keyboard != 0);
    ld_keyboard = kb_ld(keyboard);

    assert(tinyui_keyboard_update(keyboard) == 0);
    ld_keyboard->keyCode = 'a';
    assert(tinyui_keyboard_navigate(keyboard, TINYUI_NAV_RIGHT) == -1);
    assert(ld_keyboard->keyCode == 'a');

    assert(tinyui_focus_set(keyboard) == TINYUI_OK);
    assert(tinyui_keyboard_navigate(keyboard, TINYUI_NAV_RIGHT) == 0);
    assert(ld_keyboard->keyCode != 'a');
    assert(ld_keyboard->isKeySelect == true);
}

static void test_keyboard_dispatches_ascii_into_focused_line_edit(tinyui_obj_t *root)
{
    tinyui_obj_t *keyboard = tinyui_keyboard_create(root);
    tinyui_obj_t *line_edit = tinyui_line_edit_create(root);

    assert(keyboard != 0);
    assert(line_edit != 0);
    assert(tinyui_line_edit_set_keyboard_widget(line_edit, keyboard) == 0);
    assert(tinyui_line_edit_set_text(line_edit, "A") == 0);
    assert(tinyui_focus_set(line_edit) == TINYUI_OK);
    assert(tinyui_keyboard_input_ascii(keyboard, 'b') == 0);
    assert(strcmp(tinyui_line_edit_get_text(line_edit), "Ab") == 0);
}

static void test_keyboard_dispatches_ascii_into_editing_owner_before_focus_owner(tinyui_obj_t *root)
{
    tinyui_obj_t *keyboard = tinyui_keyboard_create(root);
    tinyui_obj_t *line_edit = tinyui_line_edit_create(root);
    tinyui_obj_t *text = tinyui_text_create(root);
    struct tinyui_line_edit *line_edit_host;

    assert(keyboard != 0);
    assert(line_edit != 0);
    assert(text != 0);
    line_edit_host = (struct tinyui_line_edit *)(void *)line_edit;

    assert(tinyui_line_edit_set_text(line_edit, "Q") == 0);
    assert(tinyui_text_set_text(text, "focus-owner") == 0);
    line_edit_host->editing = 1;
    assert(tinyui_runtime_internal_widget_claim_editing(kb_widget(line_edit)) == 0);
    assert(tinyui_focus_set(text) == TINYUI_OK);
    assert(tinyui_keyboard_input_ascii(keyboard, 'w') == 0);
    assert(strcmp(tinyui_line_edit_get_text(line_edit), "Qw") == 0);

    /* Isolate later cases: release the claimed editing session. */
    line_edit_host->editing = 0;
    assert(tinyui_runtime_internal_widget_release_editing(kb_widget(line_edit)) == 0);
    assert(tinyui_runtime_internal_widget_is_editing_owner(kb_widget(line_edit)) == 0);
}

static void test_keyboard_rejects_ascii_when_target_is_not_line_edit(tinyui_obj_t *root)
{
    tinyui_obj_t *keyboard = tinyui_keyboard_create(root);
    tinyui_obj_t *text = tinyui_text_create(root);

    assert(keyboard != 0);
    assert(text != 0);
    assert(tinyui_text_set_text(text, "plain-text") == 0);
    assert(tinyui_focus_set(text) == TINYUI_OK);
    /* No line_edit focus/editing owner: ascii must be rejected. */
    assert(tinyui_runtime_internal_widget_is_editing_owner(kb_widget(text)) == 0);
    assert(tinyui_keyboard_input_ascii(keyboard, 'x') == -1);
}

static void test_keyboard_draw_callback_round_trip(tinyui_obj_t *root)
{
    tinyui_obj_t *keyboard = tinyui_keyboard_create(root);
    static const struct tinyui_keyboard_button custom_buttons[] = {
        {.x = 0, .y = 0, .width = 18, .height = 10, .text = "A", .key_code = 'a', .press_color = 0x102030, .release_color = 0x405060},
        {.x = 20, .y = 0, .width = 18, .height = 10, .text = "B", .key_code = 'b', .press_color = 0x708090, .release_color = 0xA0B0C0},
    };

    g_draw_count = 0;
    g_draw_last_key_code = 0;
    assert(keyboard != 0);
    assert(tinyui_keyboard_set_layout(keyboard, custom_buttons, 2) == 0);
    assert(tinyui_keyboard_set_draw_callback(0, on_keyboard_draw, &g_draw_cookie) == -1);
    assert(tinyui_keyboard_set_draw_callback(keyboard, on_keyboard_draw, &g_draw_cookie) == 0);
    assert(tinyui_keyboard_update(keyboard) == 0);
    assert(g_draw_count == 2);
    assert(g_draw_last_key_code == 'b');
    assert(tinyui_keyboard_set_draw_callback(keyboard, 0, 0) == 0);
}

static void test_keyboard_weak_hooks_remain_backend_private_not_public_api(tinyui_obj_t *root)
{
    tinyui_obj_t *keyboard = tinyui_keyboard_create(root);
    ldKeyboard_t *ld_keyboard;
    struct tinyui_keyboard *host;

    assert(keyboard != 0);
    host = (struct tinyui_keyboard *)(void *)keyboard;
    ld_keyboard = kb_ld(keyboard);
    assert(ldKeyboardGetTargetBtnList(ld_keyboard) != 0);
    ldKeyboardCallback(ld_keyboard, SIGNAL_PRESS);
    assert(tinyui_keyboard_set_on_key_event(keyboard, 0, 0) == 0);
    assert(tinyui_keyboard_set_buttons(keyboard, 0, 0) == 0);
    assert(host->event_cb == 0);
    assert(host->buttons == 0);
    assert(host->layout_entries == 0);
    assert(host->layout_count == 0);
}

int main(void)
{
    tinyui_obj_t *root;

    tinyui_deinit();
    assert(tinyui_init() == TINYUI_OK);
    root = tinyui_screen_create();
    assert(root != 0);

    test_keyboard_init_and_shared_base_aliases_round_trip(root);
    test_keyboard_update_and_button_update_touch_native_state(root);
    test_keyboard_custom_layout_button_table_round_trip(root);
    test_keyboard_key_event_callback_receives_native_press_release(root);
    test_keyboard_click_respects_focus_owner(root);
    test_keyboard_exit_clears_focus_or_edit_session(root);
    test_keyboard_navigation_signal_respects_focus_owner(root);
    test_keyboard_dispatches_ascii_into_focused_line_edit(root);
    test_keyboard_dispatches_ascii_into_editing_owner_before_focus_owner(root);
    test_keyboard_rejects_ascii_when_target_is_not_line_edit(root);
    test_keyboard_draw_callback_round_trip(root);
    test_keyboard_weak_hooks_remain_backend_private_not_public_api(root);

    tinyui_deinit();
    return 0;
}
