#include "tinyui.h"
#include "../../../src/gui/ldButton.h"
#include "../../../src/gui/ldGui.h"
#include "../../../src/misc/ldMsg.h"
#include "internal.h"

#include <assert.h>
#include <dlfcn.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

extern const arm_2d_a1_font_t ARM_2D_FONT_6x8;
extern int tinyui_widget_has_ld_binding(const struct tinyui_widget *widget);
void tinyui_button_test_fail_next_set_font(void);

static const char *test_self_binary_path = 0;
static const char *test_source_file_path = __FILE__;

static FILE *open_repo_file_from_test_source(const char *relative_path)
{
    char base_path[PATH_MAX];
    char *tests_dir;
    size_t base_len;

    assert(test_source_file_path != 0);
    assert(relative_path != 0);
    assert(strlen(test_source_file_path) < sizeof(base_path));
    snprintf(base_path, sizeof(base_path), "%s", test_source_file_path);
    tests_dir = strstr(base_path, "tests/tinyui/unit/");
    assert(tests_dir != 0);
    *tests_dir = '\0';
    base_len = strlen(base_path);
    assert(base_len + strlen(relative_path) + 1 < sizeof(base_path));
    snprintf(base_path + base_len, sizeof(base_path) - base_len, "%s", relative_path);
    return fopen(base_path, "rb");
}

static void assert_repo_file_contains(const char *relative_path, const char *needle)
{
    FILE *file;
    char content[32768];
    size_t bytes_read;

    file = open_repo_file_from_test_source(relative_path);
    assert(file != 0);
    bytes_read = fread(content, 1, sizeof(content) - 1, file);
    assert(ferror(file) == 0);
    content[bytes_read] = '\0';
    assert(fclose(file) == 0);
    assert(strstr(content, needle) != 0);
}

static void assert_repo_file_lacks(const char *relative_path, const char *needle)
{
    FILE *file;
    char content[32768];
    size_t bytes_read;

    file = open_repo_file_from_test_source(relative_path);
    assert(file != 0);
    bytes_read = fread(content, 1, sizeof(content) - 1, file);
    assert(ferror(file) == 0);
    content[bytes_read] = '\0';
    assert(fclose(file) == 0);
    assert(strstr(content, needle) == 0);
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
        char *last_space;
        char *token;
        char type_char;

        while (line_len > 0 && (line[line_len - 1] == '\n' || line[line_len - 1] == '\r')) {
            line[--line_len] = '\0';
        }
        last_space = strrchr(line, ' ');
        token = last_space != 0 ? last_space + 1 : line;
        /* nm line: "<addr> <type> <name>"; the type char precedes the name.
         * Only externally-visible (global) symbols indicate a leaked public
         * symbol. Local symbols (lowercase type) and undefined references ('U')
         * are implementation details of an unstripped, LTO-internalized binary
         * and must not trip this contract. */
        type_char = (last_space != 0 && last_space != line) ? *(last_space - 1) : '\0';
        if (strcmp(token, symbol) == 0 && type_char >= 'A' && type_char <= 'Z' &&
            type_char != 'U') {
            assert(!"unexpected public symbol still present in test binary");
        }
    }
    assert(pclose(pipe) == 0);
}

static int press_count = 0;
static int release_count = 0;
static int click_count = 0;
static int value_count = 0;
static int event_order[4];
static int event_order_count = 0;
static struct tinyui_widget *last_press_widget = 0;
static struct tinyui_widget *last_release_widget = 0;
static struct tinyui_widget *last_click_widget = 0;
static struct tinyui_widget *last_value_widget = 0;
static int last_press_cookie = 0;
static int last_release_cookie = 0;
static int last_click_cookie = 0;
static int last_value_cookie = 0;
static int last_value = 0;
static uint64_t last_press_native_value = 0;
static uint64_t last_hold_native_value = 0;
static uint64_t last_release_native_value = 0;

static void on_pressed(struct tinyui_widget *widget, void *user_data)
{
    press_count++;
    event_order[event_order_count++] = 1;
    last_press_widget = widget;
    last_press_cookie = user_data != 0 ? *(const int *)user_data : -1;
}

static void on_released(struct tinyui_widget *widget, void *user_data)
{
    release_count++;
    event_order[event_order_count++] = 2;
    last_release_widget = widget;
    last_release_cookie = user_data != 0 ? *(const int *)user_data : -1;
}

static void on_clicked(struct tinyui_widget *widget, void *user_data)
{
    click_count++;
    event_order[event_order_count++] = 3;
    last_click_widget = widget;
    last_click_cookie = user_data != 0 ? *(const int *)user_data : -1;
}

static void on_value_changed(struct tinyui_widget *widget, int value, void *user_data)
{
    value_count++;
    last_value_widget = widget;
    last_value = value;
    last_value_cookie = user_data != 0 ? *(const int *)user_data : -1;
}

static void test_shared_emit_helpers_keep_callback_contract(struct tinyui_button *button,
                                                            int press_cookie,
                                                            int click_cookie,
                                                            int slider_cookie)
{
    assert(button != 0);

    tinyui_widget_emit_event(on_pressed, &button->widget, &press_cookie);
    assert(press_count == 1);
    assert(last_press_widget == &button->widget);
    assert(last_press_cookie == press_cookie);

    tinyui_widget_emit_clicked(on_clicked, &button->widget, &click_cookie);
    assert(click_count == 1);
    assert(last_click_widget == &button->widget);
    assert(last_click_cookie == click_cookie);

    tinyui_widget_emit_value_changed(on_value_changed, &button->widget, 73, &slider_cookie);
    assert(value_count == 1);
    assert(last_value_widget == &button->widget);
    assert(last_value == 73);
    assert(last_value_cookie == slider_cookie);

    tinyui_widget_emit_event(0, &button->widget, &press_cookie);
    tinyui_widget_emit_clicked(0, &button->widget, &click_cookie);
    tinyui_widget_emit_value_changed(0, &button->widget, 91, &slider_cookie);
    assert(press_count == 1);
    assert(click_count == 1);
    assert(value_count == 1);
    assert(last_value == 73);
}

static void test_shared_emit_helpers_no_longer_use_tinyui_backend_prefix(void)
{
    assert_self_binary_lacks_symbol("tinyui_backend_emit_event");
    assert_self_binary_lacks_symbol("tinyui_backend_emit_clicked");
    assert_self_binary_lacks_symbol("tinyui_backend_emit_value_changed");
}

static void test_event_shared_helpers_use_tinyui_prefix_in_core_seam(void)
{
    static const char *old_event_helper_names[] = {
        "picoui_widget_accepts_event",
        "picoui_widget_slider_value_to_percent",
        "picoui_widget_slider_percent_to_value",
        "picoui_widget_sync_ld_value",
        "picoui_widget_emit_ld_event_bridge",
        "picoui_widget_claim_focus_for_signal",
        "picoui_widget_restore_rejected_list_selection",
        "picoui_widget_get_owner_app",
        "picoui_widget_note_focus_event",
    };
    static const char *new_event_helper_names[] = {
        "tinyui_widget_accepts_event",
        "tinyui_widget_slider_value_to_percent",
        "tinyui_widget_slider_percent_to_value",
        "tinyui_widget_sync_ld_value",
        "tinyui_widget_emit_ld_event_bridge",
        "tinyui_widget_claim_focus_for_signal",
        "tinyui_widget_restore_rejected_list_selection",
        "tinyui_widget_get_owner_app",
        "tinyui_widget_note_focus_event",
    };
    size_t i;

    for (i = 0; i < sizeof(old_event_helper_names) / sizeof(old_event_helper_names[0]); ++i) {
        assert_repo_file_lacks("tinyui/src/core/event.c", old_event_helper_names[i]);
    }
    for (i = 0; i < sizeof(new_event_helper_names) / sizeof(new_event_helper_names[0]); ++i) {
        assert_repo_file_contains("tinyui/src/core/event.c", new_event_helper_names[i]);
    }
    assert_repo_file_contains("tinyui/src/core/internal.h", "tinyui_widget_sync_ld_value");
    assert_repo_file_contains("tinyui/src/core/internal.h", "tinyui_widget_emit_ld_event_bridge");
    assert_self_binary_lacks_symbol("tinyui_widget_sync_ld_value");
    assert_self_binary_lacks_symbol("tinyui_widget_emit_ld_event_bridge");
}

static void test_dispatch_entries_use_tinyui_prefix_in_core_seam(void)
{
    assert_repo_file_lacks("tinyui/src/core/event.c", "picoui_widget_dispatch_signal");
    assert_repo_file_lacks("tinyui/src/core/event.c", "picoui_widget_dispatch_event");
    assert_repo_file_contains("tinyui/src/core/event.c", "tinyui_widget_dispatch_signal");
    assert_repo_file_contains("tinyui/src/core/event.c", "tinyui_widget_dispatch_event");
    assert_repo_file_contains("tinyui/src/core/internal.h", "tinyui_widget_dispatch_signal");
    assert_repo_file_contains("tinyui/src/core/internal.h", "tinyui_widget_dispatch_event");
    assert_self_binary_lacks_symbol("tinyui_widget_dispatch_signal");
    assert_self_binary_lacks_symbol("tinyui_widget_dispatch_event");
}

static void test_button_shared_text_helper_uses_tinyui_prefix(void)
{
    assert_repo_file_lacks("tinyui/src/core/widget.c", "picoui_backend_set_text");
    assert_repo_file_contains("tinyui/src/core/widget.c", "tinyui_widget_set_backend_text");
    assert_repo_file_contains("tinyui/src/widgets/button.c", "tinyui_widget_set_backend_text");
    assert_self_binary_lacks_symbol("tinyui_backend_set_text");
}

static void test_button_internal_seams_use_tinyui_prefix(void)
{
    assert_repo_file_lacks("tinyui/src/widgets/button.c", "picoui_button_alloc");
    assert_repo_file_lacks("tinyui/src/widgets/button.c", "picoui_button_set_event");
    assert_repo_file_lacks("tinyui/src/widgets/button.c", "picoui_button_get_ld");
    assert_repo_file_lacks("tinyui/src/widgets/button.c", "picoui_button_default_font");
    assert_repo_file_lacks("tinyui/src/widgets/button.c", "picoui_button_resolve_font");
    assert_repo_file_lacks("tinyui/src/widgets/button.c", "picoui_button_dispose_partial");
    assert_repo_file_lacks("tinyui/src/widgets/button.c", "picoui_button_props_are_valid");
    assert_repo_file_lacks("tinyui/src/widgets/button.c", "picoui_button_fail_next_set_font");
    assert_repo_file_lacks("tinyui/src/widgets/button.c", "picoui_backend_button_test_fail_next_set_font");
    assert_repo_file_contains("tinyui/src/widgets/button.c", "tinyui_button_alloc");
    assert_repo_file_contains("tinyui/src/widgets/button.c", "tinyui_button_set_event");
    assert_repo_file_contains("tinyui/src/widgets/button.c", "tinyui_button_get_ld");
    assert_repo_file_contains("tinyui/src/widgets/button.c", "tinyui_button_default_font");
    assert_repo_file_contains("tinyui/src/widgets/button.c", "tinyui_button_resolve_font");
    assert_repo_file_contains("tinyui/src/widgets/button.c", "tinyui_button_dispose_partial");
    assert_repo_file_contains("tinyui/src/widgets/button.c", "tinyui_button_props_are_valid");
    assert_repo_file_contains("tinyui/src/widgets/button.c", "tinyui_button_fail_next_set_font");
    assert_repo_file_contains("tinyui/src/widgets/button.c", "tinyui_button_test_fail_next_set_font");
    assert_self_binary_lacks_symbol("tinyui_button_get_ld");
    assert_self_binary_lacks_symbol("tinyui_button_default_font");
    assert_self_binary_lacks_symbol("tinyui_button_resolve_font");
    assert_self_binary_lacks_symbol("tinyui_button_alloc");
    assert_self_binary_lacks_symbol("tinyui_button_set_event");
    assert_self_binary_lacks_symbol("tinyui_button_dispose_partial");
    assert_self_binary_lacks_symbol("tinyui_button_props_are_valid");
    assert_self_binary_lacks_symbol("tinyui_backend_button_test_fail_next_set_font");
}

static void test_button_create_with_props_pushes_all_fields(struct tinyui_window *win)
{
    struct tinyui_button *btn = tinyui_button_create_with_props(
        win,
        &(struct tinyui_button_props){
            .id = "btn_props",
            .text = "PropsBtn",
            .width = 120,
            .height = 36,
        });
    struct tinyui_widget *backend;
    ldBase_t *ld_base;

    assert(btn != 0);
    backend = &btn->widget;
    assert(backend->ld_widget != 0);
    assert(backend->kind == TINYUI_BACKEND_WIDGET_BUTTON);
    assert(backend->text != 0);
    assert(strcmp(backend->text, "PropsBtn") == 0);

    ld_base = (ldBase_t *)backend->ld_widget;
    assert(ld_base != 0);
    assert(ld_base->use_as__arm_2d_control_node_t.tRegion.tSize.iWidth == 120);
    assert(ld_base->use_as__arm_2d_control_node_t.tRegion.tSize.iHeight == 36);
}

static void test_button_create_with_props_failure_rolls_back_attached_child(struct tinyui_window *win)
{
    ldBase_t *win_ld = (ldBase_t *)win->widget.ld_widget;
    ldBase_t *tail_ld = ldBaseGetChildList(win_ld);
    ldBase_t *next_before_ld = 0;
    struct tinyui_button *button;
    struct tinyui_font failing_font = {
        .family = "Sans",
        .size = 12,
    };

    while (tail_ld != 0 && ldBaseGetNextSibling(tail_ld) != 0) {
        tail_ld = ldBaseGetNextSibling(tail_ld);
    }
    if (tail_ld != 0) {
        next_before_ld = ldBaseGetNextSibling(tail_ld);
    }

    tinyui_button_test_fail_next_set_font();
    button = tinyui_button_create_with_props(
        win,
        &(struct tinyui_button_props){
            .id = "btn_props_fail_font",
            .text = "PropsBtnFail",
            .font = &failing_font,
        });

    assert(button == 0);
    if (tail_ld != 0) {
        assert(ldBaseGetNextSibling(tail_ld) == next_before_ld);
    } else {
        assert(ldBaseGetChildList(win_ld) == 0);
    }
}

static void test_button_set_text_round_trip(struct tinyui_window *win)
{
    struct tinyui_button *btn = tinyui_button_create(win, "btn_text");
    struct tinyui_widget *backend;

    assert(btn != 0);
    assert(tinyui_button_set_text(btn, "NewLabel") == 0);
    backend = &btn->widget;
    assert(backend->text != 0);
    assert(strcmp(backend->text, "NewLabel") == 0);
}

static void test_button_set_style_class(struct tinyui_window *win)
{
    struct tinyui_button *btn = tinyui_button_create(win, "btn_style");
    assert(btn != 0);
    assert(tinyui_widget_set_style_class(&btn->widget, "primary") == 0);
    assert(btn->widget.style_class != 0);
    assert(strcmp(btn->widget.style_class, "primary") == 0);
}

static void test_button_rejects_null_args(struct tinyui_window *win)
{
    assert(tinyui_button_create(0, "id") == 0);
    assert(tinyui_button_create(win, 0) == 0);
    assert(tinyui_button_set_text(0, "text") == -1);
    assert(tinyui_button_set_on_clicked(0, 0, 0) == -1);
}

static void test_button_constructor_binds_ld_without_backend_wrapper(struct tinyui_window *win)
{
    struct tinyui_button *btn = tinyui_button_create(win, "btn_direct_path");

    assert(btn != 0);
    assert(tinyui_widget_has_ld_binding(&btn->widget) == 1);
}

int main(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *win = tinyui_window_create(app, "root");
    struct tinyui_button *button = tinyui_button_create(win, "submit");
    struct tinyui_checkbox *checkbox = tinyui_checkbox_create(win, "accept");
    struct tinyui_switch *sw = tinyui_switch_create(win, "power");
    struct tinyui_slider *slider = tinyui_slider_create(win, "level");
    struct tinyui_widget *backend;
    struct tinyui_widget *checkbox_backend;
    struct tinyui_widget *switch_backend;
    struct tinyui_widget *slider_backend;
    struct tinyui_app *app_state;
    int pressed_by_id = -1;
    int button_name_id = -1;
    int checkbox_name_id = -1;
    int click_cookie = 33;
    int press_cookie = 11;
    int release_cookie = 22;
    int checkbox_cookie = 44;
    int switch_cookie = 55;
    int slider_cookie = 66;
    Dl_info self_info;

    assert(app != 0);
    assert(win != 0);
    assert(dladdr((void *)&main, &self_info) != 0);
    test_self_binary_path = self_info.dli_fname;
    assert_self_binary_lacks_symbol("tinyui_backend_widget_dispatch_signal");
    assert_self_binary_lacks_symbol("tinyui_backend_widget_dispatch_event");
    assert_self_binary_lacks_symbol("tinyui_backend_sync_ld_value");
    assert_self_binary_lacks_symbol("tinyui_backend_emit_ld_event_bridge");
    assert_self_binary_lacks_symbol("tinyui_backend_sync_ld_value");
    assert_self_binary_lacks_symbol("tinyui_backend_emit_ld_event_bridge");
    test_button_constructor_binds_ld_without_backend_wrapper(win);
    test_button_create_with_props_failure_rolls_back_attached_child(win);
    assert(button != 0);
    assert(checkbox != 0);
    assert(sw != 0);
    assert(slider != 0);
    backend = &button->widget;
    checkbox_backend = &checkbox->widget;
    switch_backend = &sw->widget;
    slider_backend = &slider->widget;
    assert(backend->ld_widget != 0);
    assert(checkbox_backend->ld_widget != 0);
    assert(switch_backend->ld_widget != 0);
    assert(slider_backend->ld_widget != 0);
    app_state = app;
    assert(app_state != 0);
    assert(app_state->ld_scene != 0);
    assert(ldMsgInit(&app_state->ld_scene->ptMsgQueue, 8) == true);

    assert(tinyui_button_set_on_pressed(0, on_pressed, &press_cookie) == -1);
    assert(tinyui_button_set_on_released(0, on_released, &release_cookie) == -1);

    assert(tinyui_button_set_on_pressed(button, on_pressed, &press_cookie) == 0);
    assert(tinyui_button_set_on_released(button, on_released, &release_cookie) == 0);
    assert(tinyui_button_set_on_clicked(button, on_clicked, &click_cookie) == 0);
    assert(tinyui_checkbox_set_on_toggled(checkbox, on_value_changed, &checkbox_cookie) == 0);
    assert(tinyui_switch_set_on_toggled(sw, on_value_changed, &switch_cookie) == 0);
    assert(tinyui_slider_set_on_value_changed(slider, on_value_changed, &slider_cookie) == 0);
    assert(button->on_pressed == on_pressed);
    assert(button->on_pressed_user_data == &press_cookie);
    assert(button->on_released == on_released);
    assert(button->on_released_user_data == &release_cookie);
    assert(button->on_clicked == on_clicked);
    assert(button->user_data == &click_cookie);

    assert(press_count == 0);
    assert(release_count == 0);
    assert(click_count == 0);
    assert(event_order_count == 0);

    test_shared_emit_helpers_keep_callback_contract(button, press_cookie, click_cookie, slider_cookie);
    test_event_shared_helpers_use_tinyui_prefix_in_core_seam();
    test_dispatch_entries_use_tinyui_prefix_in_core_seam();
    test_button_shared_text_helper_uses_tinyui_prefix();
    test_button_internal_seams_use_tinyui_prefix();
    press_count = 0;
    release_count = 0;
    click_count = 0;
    value_count = 0;
    event_order_count = 0;
    last_press_widget = 0;
    last_release_widget = 0;
    last_click_widget = 0;
    last_value_widget = 0;
    last_press_cookie = 0;
    last_release_cookie = 0;
    last_click_cookie = 0;
    last_value_cookie = 0;
    last_value = 0;

    button_name_id = tinyui_widget_get_name_id((const struct tinyui_widget *)button);
    checkbox_name_id = tinyui_widget_get_name_id((const struct tinyui_widget *)checkbox);
    assert(button_name_id > 0);
    assert(checkbox_name_id > 0);

    assert(tinyui_button_set_pressed(button, 1) == 0);
    assert(tinyui_button_get_pressed_by_name_id((const struct tinyui_widget *)win,
                                                button_name_id,
                                                &pressed_by_id) == 0);
    assert(pressed_by_id == 1);
    assert(tinyui_button_set_pressed(button, 0) == 0);
    assert(tinyui_button_get_pressed_by_name_id((const struct tinyui_widget *)win,
                                                button_name_id,
                                                &pressed_by_id) == 0);
    assert(pressed_by_id == 0);
    assert(tinyui_button_get_pressed_by_name_id((const struct tinyui_widget *)win,
                                                checkbox_name_id,
                                                &pressed_by_id) == -1);
    assert(tinyui_button_get_pressed_by_name_id((const struct tinyui_widget *)win,
                                                65535,
                                                &pressed_by_id) == -1);
    assert(tinyui_button_get_pressed_by_name_id(0, button_name_id, &pressed_by_id) == -1);
    assert(tinyui_button_get_pressed_by_name_id((const struct tinyui_widget *)win,
                                                button_name_id,
                                                0) == -1);
    xBtnReset();
    xBtnTick(SYS_TICK_CYCLE_MS, app_state->ld_scene);
    assert(tinyui_button_get_action_state_by_name_id((const struct tinyui_widget *)win,
                                                     button_name_id,
                                                     TINYUI_BUTTON_ACTION_PRESS) == 0);
    assert(tinyui_button_get_action_state_by_name_id((const struct tinyui_widget *)win,
                                                     button_name_id,
                                                     TINYUI_BUTTON_ACTION_CLICK) == 0);
    assert(tinyui_button_get_action_state_by_name_id((const struct tinyui_widget *)win,
                                                     checkbox_name_id,
                                                     TINYUI_BUTTON_ACTION_PRESS) == -1);
    assert(tinyui_button_get_action_state_by_name_id(0,
                                                     button_name_id,
                                                     TINYUI_BUTTON_ACTION_PRESS) == -1);

    assert(tinyui_widget_dispatch_event(&button->widget,
                                        TINYUI_BACKEND_SIGNAL_PRESSED,
                                        button->on_pressed,
                                        &button->widget,
                                        button->on_pressed_user_data) == 0);
    assert(press_count == 1);
    assert(release_count == 0);
    assert(click_count == 0);
    assert(event_order_count == 1);
    assert(event_order[0] == 1);
    assert(last_press_widget == &button->widget);
    assert(last_press_cookie == press_cookie);

    assert(tinyui_widget_dispatch_event(&button->widget,
                                        TINYUI_BACKEND_SIGNAL_RELEASED,
                                        button->on_released,
                                        &button->widget,
                                        button->on_released_user_data) == 0);
    assert(press_count == 1);
    assert(release_count == 1);
    assert(click_count == 0);
    assert(event_order_count == 2);
    assert(event_order[1] == 2);
    assert(last_release_widget == &button->widget);
    assert(last_release_cookie == release_cookie);

    assert(tinyui_widget_dispatch_event(0,
                                        TINYUI_BACKEND_SIGNAL_PRESSED,
                                        on_pressed,
                                        &button->widget,
                                        &press_cookie) == -1);
    assert(tinyui_widget_dispatch_event(&button->widget,
                                        TINYUI_BACKEND_SIGNAL_VALUE_CHANGED,
                                        button->on_pressed,
                                        &button->widget,
                                        button->on_pressed_user_data) == -1);

    press_count = 0;
    release_count = 0;
    click_count = 0;
    event_order_count = 0;

    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue,
                     backend->ld_widget,
                     SIGNAL_PRESS,
                     0) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(press_count == 1);
    assert(release_count == 0);
    assert(click_count == 0);
    assert(event_order_count == 1);
    assert(event_order[0] == 1);
    assert(last_press_widget == &button->widget);
    assert(last_press_cookie == press_cookie);
    assert(ldButtonActionIsPressById((uint16_t)button_name_id, app_state->ld_scene) == true);
    xBtnTick(SYS_TICK_CYCLE_MS, app_state->ld_scene);
    assert(ldButtonActionIsPressById((uint16_t)button_name_id, app_state->ld_scene) == true);
    xBtnTick(SYS_TICK_CYCLE_MS, app_state->ld_scene);
    assert(tinyui_button_get_action_state_by_name_id((const struct tinyui_widget *)win,
                                                     button_name_id,
                                                     TINYUI_BUTTON_ACTION_HOLD_DOWN) == 1);
    assert(tinyui_button_get_action_state_by_name_id((const struct tinyui_widget *)win,
                                                     button_name_id,
                                                     TINYUI_BUTTON_ACTION_PRESS) == 1);

    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue,
                     backend->ld_widget,
                     SIGNAL_RELEASE,
                     0) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(press_count == 1);
    assert(release_count == 1);
    assert(click_count == 1);
    assert(event_order_count == 3);
    assert(event_order[1] == 2);
    assert(event_order[2] == 3);
    assert(last_release_widget == &button->widget);
    assert(last_release_cookie == release_cookie);
    assert(last_click_widget == &button->widget);
    assert(last_click_cookie == click_cookie);
    xBtnTick(SYS_TICK_CYCLE_MS, app_state->ld_scene);
    xBtnTick(SYS_TICK_CYCLE_MS, app_state->ld_scene);
    assert(tinyui_button_get_action_state_by_name_id((const struct tinyui_widget *)win,
                                                     button_name_id,
                                                     TINYUI_BUTTON_ACTION_RELEASE) == 1);
    assert(tinyui_button_get_action_state_by_name_id((const struct tinyui_widget *)win,
                                                     button_name_id,
                                                     TINYUI_BUTTON_ACTION_CLICK) == 1);
    assert(tinyui_button_get_action_state_by_name_id((const struct tinyui_widget *)win,
                                                     button_name_id,
                                                     TINYUI_BUTTON_ACTION_PRESS) == 0);

    last_hold_native_value = CONNECT32(3, 4, 10, 11);
    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue,
                     backend->ld_widget,
                     SIGNAL_HOLD_DOWN,
                     last_hold_native_value) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(press_count == 1);
    assert(release_count == 1);
    assert(click_count == 1);

    assert(tinyui_widget_set_enabled(&button->widget, 0) == 0);
    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue,
                     backend->ld_widget,
                     SIGNAL_PRESS,
                     0) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(press_count == 1);
    assert(release_count == 1);
    assert(click_count == 1);
    assert(event_order_count == 3);
    assert(tinyui_widget_set_enabled(&button->widget, 1) == 0);

    assert(tinyui_widget_set_visible(&button->widget, 0) == 0);
    assert(tinyui_widget_dispatch_event(&button->widget,
                                        TINYUI_BACKEND_SIGNAL_PRESSED,
                                        button->on_pressed,
                                        &button->widget,
                                        button->on_pressed_user_data) == 0);
    assert(tinyui_widget_dispatch_event(&button->widget,
                                        TINYUI_BACKEND_SIGNAL_RELEASED,
                                        button->on_released,
                                        &button->widget,
                                        button->on_released_user_data) == 0);
    assert(press_count == 1);
    assert(release_count == 1);
    assert(click_count == 1);
    assert(event_order_count == 3);

    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue,
                     backend->ld_widget,
                     SIGNAL_PRESS,
                     0) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue,
                     backend->ld_widget,
                     SIGNAL_RELEASE,
                     0) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(press_count == 1);
    assert(release_count == 1);
    assert(click_count == 1);
    assert(event_order_count == 3);
    assert(tinyui_widget_set_visible(&button->widget, 1) == 0);

    last_press_native_value = CONNECT32(0, 0, 7, 9);
    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue,
                     backend->ld_widget,
                     SIGNAL_PRESS,
                     last_press_native_value) == true);
    ldMsgProcess(app_state->ld_scene);

    last_release_native_value = CONNECT32(5, 6, 7, 9);
    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue,
                     backend->ld_widget,
                     SIGNAL_RELEASE,
                     last_release_native_value) == true);
    ldMsgProcess(app_state->ld_scene);

    assert(tinyui_checkbox_set_checked(checkbox, 1) == 0);
    assert(value_count == 0);
    assert(tinyui_switch_set_checked(sw, 1) == 0);
    assert(value_count == 0);
    assert(tinyui_slider_set_value(slider, 40) == 0);
    assert(value_count == 0);

    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue,
                     checkbox_backend->ld_widget,
                     SIGNAL_VALUE_CHANGED,
                     0) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(value_count == 1);
    assert(last_value_widget == &checkbox->widget);
    assert(last_value == 0);
    assert(last_value_cookie == checkbox_cookie);

    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue,
                     switch_backend->ld_widget,
                     SIGNAL_VALUE_CHANGED,
                     0) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(value_count == 2);
    assert(last_value_widget == &sw->widget);
    assert(last_value == 0);
    assert(last_value_cookie == switch_cookie);

    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue,
                     slider_backend->ld_widget,
                     SIGNAL_VALUE_CHANGED,
                     750) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(value_count == 3);
    assert(last_value_widget == &slider->widget);
    assert(last_value == 75);
    assert(last_value_cookie == slider_cookie);

    assert(tinyui_widget_set_visible(&checkbox->widget, 0) == 0);
    assert(tinyui_widget_dispatch_signal(&checkbox->widget,
                                         TINYUI_BACKEND_SIGNAL_VALUE_CHANGED,
                                         1,
                                         checkbox->cb,
                                         &checkbox->widget,
                                         checkbox->user_data) == 0);
    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue,
                     checkbox_backend->ld_widget,
                     SIGNAL_VALUE_CHANGED,
                     1) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(value_count == 3);
    assert(last_value_widget == &slider->widget);
    assert(last_value == 75);
    assert(last_value_cookie == slider_cookie);
    assert(tinyui_checkbox_is_checked(checkbox) == 0);
    assert(tinyui_widget_set_visible(&checkbox->widget, 1) == 0);

    assert(tinyui_widget_set_visible(&sw->widget, 0) == 0);
    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue,
                     switch_backend->ld_widget,
                     SIGNAL_VALUE_CHANGED,
                     1) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(value_count == 3);
    assert(last_value_widget == &slider->widget);
    assert(last_value == 75);
    assert(last_value_cookie == slider_cookie);
    assert(tinyui_switch_is_checked(sw) == 0);
    assert(tinyui_widget_set_visible(&sw->widget, 1) == 0);

    assert(tinyui_widget_set_visible(&slider->widget, 0) == 0);
    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue,
                     slider_backend->ld_widget,
                     SIGNAL_VALUE_CHANGED,
                     250) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(value_count == 3);
    assert(last_value_widget == &slider->widget);
    assert(last_value == 75);
    assert(last_value_cookie == slider_cookie);
    assert(slider_backend->value == 75);
    assert(tinyui_widget_set_visible(&slider->widget, 1) == 0);

    ldMsgDeinit(&app_state->ld_scene->ptMsgQueue);
    ldButtonSetFont((ldButton_t *)backend->ld_widget, (arm_2d_font_t *)&ARM_2D_FONT_6x8);

    test_shared_emit_helpers_no_longer_use_tinyui_backend_prefix();
    test_button_create_with_props_pushes_all_fields(win);
    test_button_set_text_round_trip(win);
    test_button_set_style_class(win);
    test_button_rejects_null_args(win);

    tinyui_app_destroy(app);
    return 0;
}
