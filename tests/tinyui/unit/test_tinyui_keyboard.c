#include "core/app.h"
#include "display/display.h"
#include "widgets/keyboard.h"
#include "widgets/line_edit.h"
#include "widgets/text.h"
#include "widgets/window.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldGui.h"
#include "../../../src/gui/ldKeyboard.h"
#include "../../../src/gui/ldLineEdit.h"
#include "../../../src/gui/ldText.h"
#include "../../../src/misc/ldMsg.h"
#include "internal.h"

#include <assert.h>
#include <dlfcn.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

static const char *test_self_binary_path = 0;
static const char *test_source_file_path = __FILE__;

static const char *resolve_repo_path(const char *repo_relative_path)
{
    static char resolved_path[PATH_MAX];
    char base_path[PATH_MAX];
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

static void assert_source_contains(const char *path, const char *needle)
{
    FILE *fp;
    char line[1024];

    assert(path != 0);
    assert(needle != 0);
    fp = fopen(path, "r");
    assert(fp != 0);
    while (fgets(line, sizeof(line), fp) != 0) {
        if (strstr(line, needle) != 0) {
            assert(fclose(fp) == 0);
            return;
        }
    }
    assert(fclose(fp) == 0);
    assert(!"expected source marker not found");
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
    assert(pclose(pipe) != -1);
}

static int keyboard_event_count = 0;
static int keyboard_event_last_signal = -1;
static unsigned int keyboard_event_last_key_code = 0;
static int keyboard_event_cookie = 0;
static int keyboard_draw_count = 0;
static unsigned int keyboard_draw_last_key_code = 0;
static int keyboard_draw_cookie = 0;

static uint64_t make_signal_value_xy(uint16_t x, uint16_t y)
{
    return ((uint64_t)x << 16) | y;
}

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
    struct tinyui_widget *backend;
    struct tinyui_widget *parent_backend;
    ldKeyboard_t *ld_keyboard;

    win = test_window_create(&app);
    keyboard = tinyui_keyboard_create(win, "keyboard_direct_mapping");
    assert(keyboard != 0);

    backend = &keyboard->widget;
    parent_backend = &win->widget;
    assert(backend->ld_widget != 0);
    assert(parent_backend->ld_widget != 0);
    assert(backend->kind == TINYUI_BACKEND_WIDGET_KEYBOARD);
    assert(backend->owner == parent_backend->owner);
    assert((ldBase_t *)ldBaseGetRootNode((arm_2d_control_node_t *)backend->ld_widget) == (ldBase_t *)ldBaseGetRootNode((arm_2d_control_node_t *)parent_backend->ld_widget));
    assert(ldBaseGetParent((ldBase_t *)backend->ld_widget) == (ldBase_t *)parent_backend->ld_widget);
    assert(backend->ld_name_id != 0);
    assert(backend->ld_event_bridge_scene != 0);
    assert(backend->ld_event_bridge_sender == backend->ld_widget);
    ld_keyboard = (ldKeyboard_t *)backend->ld_widget;
    assert(ld_keyboard != 0);
    assert(tinyui_app_lookup_host(backend->owner, backend->ld_name_id) == backend);
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
    assert(strcmp((const char *)((ldText_t *)text->widget.ld_widget)->pStr,
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
    struct tinyui_widget *line_edit_backend;
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

    line_edit_backend = &line_edit->widget;
    assert(line_edit_backend->ld_widget != 0);
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
    ld_text = (ldText_t *)text->widget.ld_widget;
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
    struct tinyui_widget *backend;
    ldKeyboard_t *ld_keyboard;

    win = test_window_create(&app);
    keyboard = tinyui_keyboard_create(win, "keyboard_nav");
    assert(keyboard != 0);

    backend = &keyboard->widget;
    assert(backend->ld_widget != 0);
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
    struct tinyui_widget *backend;
    ldKeyboard_t *ld_keyboard;

    win = test_window_create(&app);
    keyboard = tinyui_keyboard_create(win, "keyboard_update");
    line_edit = tinyui_line_edit_create(win, "line_edit_update_target");

    assert(keyboard != 0);
    assert(line_edit != 0);
    backend = &keyboard->widget;
    assert(backend->ld_widget != 0);
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
    struct tinyui_widget *backend;
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
    backend = &keyboard->widget;
    assert(backend->ld_widget != 0);
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
    struct tinyui_widget *backend;
    ldKeyboard_t *ld_keyboard;

    win = test_window_create(&app);
    keyboard = tinyui_keyboard_create(win, "keyboard_backend_private_hooks");

    assert(keyboard != 0);
    backend = &keyboard->widget;
    assert(backend->ld_widget != 0);
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
    struct tinyui_widget *backend;
    ldBase_t *ld_base;
    arm_2d_region_t region;

    win = test_window_create(&app);
    keyboard = tinyui_keyboard_create(win, "keyboard_base_aliases");
    assert(keyboard != 0);
    backend = &keyboard->widget;
    assert(backend->ld_widget != 0);
    ld_base = (ldBase_t *)backend->ld_widget;
    assert(ld_base != 0);

    assert(tinyui_widget_set_pos(&keyboard->widget, 9, 19) == 0);
    assert(tinyui_widget_set_visible(&keyboard->widget, 0) == 0);
    assert(tinyui_widget_set_opacity(&keyboard->widget, 55) == 0);

    region = ldBaseGetRegion(ld_base);
    assert(ld_base->isHidden == true);
    assert(region.tLocation.iX != 9);
    assert(region.tLocation.iY != 19);
    assert(ld_base->opacity == 55);
    assert(tinyui_widget_set_visible(&keyboard->widget, 1) == 0);
    region = ldBaseGetRegion(ld_base);
    assert(region.tLocation.iX == 9);
    assert(region.tLocation.iY == 19);
    tinyui_app_destroy(app);
}

static void test_hidden_keyboard_set_pos_then_line_edit_press_keeps_native_state_bounded(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win;
    struct tinyui_keyboard *keyboard;
    struct tinyui_line_edit *line_edit;
    ldKeyboard_t *ld_keyboard;
    ldBase_t *ld_base;
    arm_2d_location_t press_point = {0};
    arm_2d_region_t region;
    arm_2d_region_t temp_region;
    struct tinyui_display_config display = {0};
    int16_t screen_width;
    int16_t screen_height;

    win = test_window_create(&app);
    assert(tinyui_display_get_config(app, &display) == 0);
    screen_width = (int16_t)display.width;
    screen_height = (int16_t)display.height;
    keyboard = tinyui_keyboard_create(win, "keyboard_line_edit_crash_path");
    line_edit = tinyui_line_edit_create(win, "line_edit_opens_keyboard");

    assert(keyboard != 0);
    assert(line_edit != 0);
    assert(tinyui_line_edit_set_keyboard_widget(line_edit, keyboard) == 0);
    assert(tinyui_widget_set_pos(&line_edit->widget, screen_width - 140, screen_height - 90) == 0);
    assert(tinyui_widget_set_size(&line_edit->widget, 100, 50) == 0);
    assert(tinyui_widget_set_pos(&keyboard->widget, screen_width - 160, screen_height - 80) == 0);

    ld_keyboard = (ldKeyboard_t *)keyboard->widget.ld_widget;
    assert(ld_keyboard != 0);
    ld_base = (ldBase_t *)ld_keyboard;
    assert(ld_base->isHidden == true);

    if (app->ld_scene->ptMsgQueue != 0) {
        ldMsgDeinit(&app->ld_scene->ptMsgQueue);
    }
    assert(ldMsgInit(&app->ld_scene->ptMsgQueue, 8) == true);

    press_point.iX = screen_width - 100;
    press_point.iY = screen_height - 70;
    ldGuiClickedAction(app->ld_scene, SIGNAL_PRESS, press_point);
    ldMsgProcess(app->ld_scene);

    region = ldBaseGetRegion(ld_base);
    temp_region = ld_base->tTempRegion;
    assert(ld_base->isHidden == false);
    assert(region.tLocation.iX == 0);
    assert(region.tLocation.iY == (screen_height >> 1));
    assert(temp_region.tSize.iWidth > 0);
    assert(temp_region.tSize.iHeight > 0);
    assert(temp_region.tSize.iWidth <= screen_width * 2);
    assert(temp_region.tSize.iHeight <= screen_height * 2);

    ldGuiFrameStart(app->ld_scene);

    ldGuiClickedAction(app->ld_scene, SIGNAL_RELEASE, press_point);
    ldMsgProcess(app->ld_scene);
    tinyui_app_destroy(app);
}

static void test_keyboard_uses_runtime_viewport_for_legacy_demo0_keyboard_model(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win;
    struct tinyui_keyboard *keyboard;
    struct tinyui_line_edit *line_edit;
    struct tinyui_display_config display = {
        .width = 1024,
        .height = 600,
        .color_format = TINYUI_COLOR_FORMAT_RGB565,
        .buffer_height = 0,
        .user_data = 0,
    };
    ldKeyboard_t *ld_keyboard;
    ldLineEdit_t *ld_line_edit;
    ldBase_t *ld_keyboard_base;
    ldBase_t *ld_root;
    ldBase_t *ld_window_base;
    arm_2d_region_t keyboard_region;
    arm_2d_region_t root_region;
    arm_2d_region_t window_region;
    arm_2d_location_t press_point = {900, 425};

    app = tinyui_app_create();
    assert(app != 0);
    assert(tinyui_display_set_config(app, &display) == 0);
    win = tinyui_window_create(app, "runtime_1024_root");
    assert(win != 0);
    keyboard = tinyui_keyboard_create(win, "runtime_1024_keyboard");
    line_edit = tinyui_line_edit_create(win, "runtime_1024_line_edit");
    assert(keyboard != 0);
    assert(line_edit != 0);

    assert(tinyui_line_edit_set_keyboard_widget(line_edit, keyboard) == 0);
    assert(tinyui_widget_set_pos(&line_edit->widget, 850, 400) == 0);
    assert(tinyui_widget_set_size(&line_edit->widget, 100, 50) == 0);
    assert(tinyui_widget_set_pos(&keyboard->widget, 780, 450) == 0);

    ld_keyboard = (ldKeyboard_t *)keyboard->widget.ld_widget;
    ld_line_edit = (ldLineEdit_t *)line_edit->widget.ld_widget;
    ld_keyboard_base = (ldBase_t *)ld_keyboard;
    ld_window_base = (ldBase_t *)win->widget.ld_widget;
    assert(ld_keyboard != 0);
    assert(ld_line_edit != 0);
    assert(ld_window_base != 0);

    keyboard_region = ldBaseGetRegion(ld_keyboard_base);
    assert(keyboard_region.tSize.iWidth == 1024);
    assert(keyboard_region.tSize.iHeight == 600);

    if (app->ld_scene->ptMsgQueue != 0) {
        ldMsgDeinit(&app->ld_scene->ptMsgQueue);
    }
    assert(ldMsgInit(&app->ld_scene->ptMsgQueue, 8) == true);

    ldGuiClickedAction(app->ld_scene, SIGNAL_PRESS, press_point);
    ldMsgProcess(app->ld_scene);

    keyboard_region = ldBaseGetRegion(ld_keyboard_base);
    assert(ld_keyboard_base->isHidden == false);
    assert(keyboard_region.tLocation.iX == 0);
    assert(keyboard_region.tLocation.iY == 300);
    assert(keyboard_region.tSize.iWidth == 1024);
    assert(keyboard_region.tSize.iHeight == 600);

    ld_root = (ldBase_t *)app->ld_scene->ptNodeRoot;
    root_region = ldBaseGetRegion(ld_root);
    assert(root_region.tLocation.iX == 0);
    assert(root_region.tLocation.iY == -300);
    assert(root_region.tSize.iWidth == 1024);
    assert(root_region.tSize.iHeight == 900);
    window_region = ldBaseGetRegion(ld_window_base);
    assert(window_region.tLocation.iX == 0);
    assert(window_region.tLocation.iY == -300);
    assert(window_region.tSize.iWidth == 1024);
    assert(window_region.tSize.iHeight == 900);

    ld_keyboard->pBtnList = ldKeyboardGetTargetBtnList(ld_keyboard);
    ld_keyboard->isWaitInit = false;
    ldMsgEmit(app->ld_scene->ptMsgQueue,
              ld_keyboard,
              SIGNAL_PRESS,
              make_signal_value_xy(20, 320));
    ldMsgProcess(app->ld_scene);
    assert(strcmp((const char *)ldLineEditGetText(ld_line_edit), "q") == 0);

    ld_keyboard->keyCode = 3;
    ld_keyboard->isClick = true;
    ldKeyboardCallback(ld_keyboard, SIGNAL_RELEASE);
    assert(ld_keyboard->pBtnList == numBtnInfo);
    ldMsgEmit(app->ld_scene->ptMsgQueue,
              ld_keyboard,
              SIGNAL_PRESS,
              make_signal_value_xy(390, 524));
    ldMsgProcess(app->ld_scene);
    assert(strcmp((const char *)ldLineEditGetText(ld_line_edit), "q0") == 0);

    tinyui_app_destroy(app);
}

static void test_keyboard_runtime_viewport_is_resolved_from_event_scene(void)
{
    struct tinyui_app *app_a;
    struct tinyui_app *app_b;
    struct tinyui_window *win_a;
    struct tinyui_window *win_b;
    struct tinyui_keyboard *keyboard_a;
    struct tinyui_keyboard *keyboard_b;
    struct tinyui_line_edit *line_edit_a;
    struct tinyui_display_config display_a = {
        .width = 1024,
        .height = 600,
        .color_format = TINYUI_COLOR_FORMAT_RGB565,
        .buffer_height = 0,
        .user_data = 0,
    };
    struct tinyui_display_config display_b = {
        .width = 480,
        .height = 320,
        .color_format = TINYUI_COLOR_FORMAT_RGB565,
        .buffer_height = 0,
        .user_data = 0,
    };
    ldBase_t *ld_keyboard_base_a;
    arm_2d_region_t keyboard_region;
    arm_2d_location_t press_point = {900, 425};

    app_a = tinyui_app_create();
    assert(app_a != 0);
    assert(tinyui_display_set_config(app_a, &display_a) == 0);
    win_a = tinyui_window_create(app_a, "runtime_scene_a_root");
    assert(win_a != 0);
    keyboard_a = tinyui_keyboard_create(win_a, "runtime_scene_a_keyboard");
    line_edit_a = tinyui_line_edit_create(win_a, "runtime_scene_a_line_edit");
    assert(keyboard_a != 0);
    assert(line_edit_a != 0);
    assert(tinyui_line_edit_set_keyboard_widget(line_edit_a, keyboard_a) == 0);
    assert(tinyui_widget_set_pos(&line_edit_a->widget, 850, 400) == 0);
    assert(tinyui_widget_set_size(&line_edit_a->widget, 100, 50) == 0);

    app_b = tinyui_app_create();
    assert(app_b != 0);
    assert(tinyui_display_set_config(app_b, &display_b) == 0);
    win_b = tinyui_window_create(app_b, "runtime_scene_b_root");
    assert(win_b != 0);
    keyboard_b = tinyui_keyboard_create(win_b, "runtime_scene_b_keyboard");
    assert(keyboard_b != 0);

    if (app_a->ld_scene->ptMsgQueue != 0) {
        ldMsgDeinit(&app_a->ld_scene->ptMsgQueue);
    }
    assert(ldMsgInit(&app_a->ld_scene->ptMsgQueue, 8) == true);

    ldGuiClickedAction(app_a->ld_scene, SIGNAL_PRESS, press_point);
    ldMsgProcess(app_a->ld_scene);

    ld_keyboard_base_a = (ldBase_t *)keyboard_a->widget.ld_widget;
    assert(ld_keyboard_base_a != 0);
    keyboard_region = ldBaseGetRegion(ld_keyboard_base_a);
    assert(keyboard_region.tLocation.iY == 300);
    assert(keyboard_region.tSize.iWidth == 1024);
    assert(keyboard_region.tSize.iHeight == 600);

    tinyui_app_destroy(app_b);
    tinyui_app_destroy(app_a);
}

static void test_keyboard_bg_move_syncs_only_related_root_window(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win_a;
    struct tinyui_window *win_b;
    struct tinyui_keyboard *keyboard;
    struct tinyui_line_edit *line_edit;
    struct tinyui_display_config display = {
        .width = 1024,
        .height = 600,
        .color_format = TINYUI_COLOR_FORMAT_RGB565,
        .buffer_height = 0,
        .user_data = 0,
    };
    ldBase_t *ld_window_a;
    ldBase_t *ld_window_b;
    arm_2d_region_t window_a_region;
    arm_2d_region_t window_b_region;
    arm_2d_location_t press_point = {900, 425};

    app = tinyui_app_create();
    assert(app != 0);
    assert(tinyui_display_set_config(app, &display) == 0);
    win_b = tinyui_window_create(app, "sibling_window");
    assert(win_b != 0);
    win_a = tinyui_window_create(app, "related_window");
    assert(win_a != 0);
    keyboard = tinyui_keyboard_create(win_a, "related_keyboard");
    line_edit = tinyui_line_edit_create(win_a, "related_line_edit");
    assert(keyboard != 0);
    assert(line_edit != 0);
    assert(tinyui_line_edit_set_keyboard_widget(line_edit, keyboard) == 0);
    assert(tinyui_widget_set_pos(&line_edit->widget, 850, 400) == 0);
    assert(tinyui_widget_set_size(&line_edit->widget, 100, 50) == 0);

    ld_window_a = (ldBase_t *)win_a->widget.ld_widget;
    ld_window_b = (ldBase_t *)win_b->widget.ld_widget;
    assert(ld_window_a != 0);
    assert(ld_window_b != 0);

    if (app->ld_scene->ptMsgQueue != 0) {
        ldMsgDeinit(&app->ld_scene->ptMsgQueue);
    }
    assert(ldMsgInit(&app->ld_scene->ptMsgQueue, 8) == true);

    ldGuiClickedAction(app->ld_scene, SIGNAL_PRESS, press_point);
    ldMsgProcess(app->ld_scene);

    window_a_region = ldBaseGetRegion(ld_window_a);
    window_b_region = ldBaseGetRegion(ld_window_b);
    assert(window_a_region.tLocation.iY == -300);
    assert(window_a_region.tSize.iHeight == 900);
    assert(window_b_region.tLocation.iY == 0);
    assert(window_b_region.tSize.iHeight == 600);

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
    struct tinyui_widget *backend;

    win = test_window_create(&app);
    keyboard = tinyui_keyboard_create(win, "keyboard_gate_contract");
    assert(keyboard != 0);
    backend = &keyboard->widget;
    assert(backend->kind == TINYUI_BACKEND_WIDGET_KEYBOARD);
    assert(backend->ld_widget != 0);
    tinyui_app_destroy(app);
}

static void test_keyboard_button_update_rejects_corrupted_backend_binding_without_native_or_public_drift(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win;
    struct tinyui_keyboard *keyboard;
    struct tinyui_widget *backend;
    enum tinyui_backend_widget_kind saved_kind;
    ldKeyboard_t *ld_keyboard;
    uint8_t saved_key_code;
    bool saved_is_key_select;
    unsigned int capture[3] = {0, 0, 0};

    win = test_window_create(&app);
    keyboard = tinyui_keyboard_create(win, "keyboard_corrupted_binding");

    assert(keyboard != 0);
    assert(tinyui_keyboard_set_on_key_event(keyboard, test_keyboard_event_capture, capture) == 0);

    backend = &keyboard->widget;
    assert(backend->ld_widget != 0);
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
    struct tinyui_widget *backend;
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

    backend = &keyboard->widget;
    assert(backend->ld_widget != 0);
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
    struct tinyui_widget *backend;
    ldKeyboard_t *ld_keyboard;

    keyboard_event_count = 0;
    keyboard_event_last_signal = -1;
    keyboard_event_last_key_code = 0;

    win = test_window_create(&app);
    keyboard = tinyui_keyboard_create(win, "keyboard_event_bridge");
    assert(keyboard != 0);
    assert(tinyui_keyboard_set_on_key_event(0, on_keyboard_event, &keyboard_event_cookie) == -1);
    assert(tinyui_keyboard_set_on_key_event(keyboard, on_keyboard_event, &keyboard_event_cookie) == 0);

    backend = &keyboard->widget;
    assert(backend->ld_widget != 0);
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

static void test_keyboard_create_path_uses_core_leaf_helper(void)
{
    assert_source_contains(resolve_repo_path("tinyui/src/widgets/keyboard.c"),
                           "tinyui_widget_create_leaf(");
}

int main(void)
{
    test_self_binary_path = "tests/tinyui/test_tinyui_keyboard";
    test_keyboard_create_path_uses_core_leaf_helper();
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
    test_hidden_keyboard_set_pos_then_line_edit_press_keeps_native_state_bounded();
    test_keyboard_uses_runtime_viewport_for_legacy_demo0_keyboard_model();
    test_keyboard_runtime_viewport_is_resolved_from_event_scene();
    test_keyboard_bg_move_syncs_only_related_root_window();
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
