#include "picoui/picoui.h"
#include "internal.h"

#include <assert.h>
#include <string.h>

int picoui_native_event_bind_root(struct picoui_screen *screen, struct picoui_window *root_window);
int picoui_native_line_edit_set_text(struct picoui_line_edit *line_edit, const char *text);
int picoui_native_line_edit_set_editing(struct picoui_line_edit *line_edit, int editing);

static int g_finished_count = 0;
static struct picoui_line_edit *g_finished_line_edit = 0;
static void *g_finished_user_data = 0;

static void on_line_edit_finished(struct picoui_line_edit *line_edit, void *user_data)
{
    g_finished_count++;
    g_finished_line_edit = line_edit;
    g_finished_user_data = user_data;
}

static void reset_finished_capture(void)
{
    g_finished_count = 0;
    g_finished_line_edit = 0;
    g_finished_user_data = 0;
}

static void test_edit_commit_round_trip(struct picoui_line_edit *line_edit,
                                        struct picoui_keyboard *keyboard,
                                        int *editing,
                                        int *callback_cookie,
                                        const char *expected_text)
{
    assert(picoui_widget_claim_focus(&line_edit->widget) == 0);
    assert(picoui_widget_claim_editing(&line_edit->widget) == 0);
    assert(picoui_native_line_edit_set_editing(line_edit, 1) == 0);
    assert(picoui_line_edit_get_editing(line_edit, editing) == 0);
    assert(*editing == 1);

    assert(picoui_keyboard_input_ascii(keyboard, 'A') == 0);
    assert(strcmp(picoui_line_edit_get_text(line_edit), expected_text) == 0);
    assert(picoui_keyboard_input_ascii(keyboard, '\b') == 0);
    assert(strcmp(picoui_line_edit_get_text(line_edit), "") == 0);

    assert(g_finished_count == 0);
    assert(picoui_keyboard_input_ascii(keyboard, '\r') == 0);
    assert(picoui_line_edit_get_editing(line_edit, editing) == 0);
    assert(*editing == 0);
    assert(g_finished_count == 1);
    assert(g_finished_line_edit == line_edit);
    assert(g_finished_user_data == callback_cookie);
}

int main(void)
{
    struct picoui_display *display;
    struct picoui_screen *screen;
    struct picoui_window *window;
    struct picoui_line_edit *line_edit;
    struct picoui_keyboard *keyboard;
    struct picoui_text *plain_text;
    struct picoui_app *app;
    int editing = -1;
    int rc;
    int callback_cookie = 37;

    assert(picoui_init() == 0);

    display = picoui_display_create(320, 480);
    assert(display != 0);
    assert(picoui_display_set_default(display) == 0);

    screen = picoui_screen_active();
    assert(screen != 0);

    window = picoui_window_create_root(screen, "root");
    assert(window != 0);

    line_edit = picoui_line_edit_create_with_props(
        window,
        &(struct picoui_line_edit_props){
            .id = "line_edit",
            .text = "",
            .keyboard_binding = 1U,
            .has_keyboard_binding = 1,
            .width = 220,
            .height = 32,
        });
    keyboard = picoui_keyboard_create_with_props(
        window,
        &(struct picoui_keyboard_props){
            .id = "keyboard",
            .width = 240,
            .height = 120,
        });
    plain_text = picoui_text_create_with_props(
        window,
        &(struct picoui_text_props){
            .id = "plain_text",
            .text = "static",
            .width = 120,
            .height = 24,
        });

    assert(line_edit != 0);
    assert(keyboard != 0);
    assert(plain_text != 0);
    assert(picoui_widget_set_pos((struct picoui_widget *)line_edit, 24, 24) == 0);
    assert(picoui_widget_set_size((struct picoui_widget *)line_edit, 220, 32) == 0);
    assert(picoui_widget_set_pos((struct picoui_widget *)keyboard, 24, 72) == 0);
    assert(picoui_widget_set_size((struct picoui_widget *)keyboard, 240, 120) == 0);
    assert(picoui_line_edit_set_on_edit_finished(line_edit,
                                                 on_line_edit_finished,
                                                 &callback_cookie) == 0);

    app = ((struct picoui_backend_widget *)line_edit->widget.backend_widget)->owner;
    assert(app != 0);

    assert(picoui_native_event_bind_root(screen, window) == 0);
    rc = picoui_timer_handler();
    assert(rc == 0 || rc == 1);

    reset_finished_capture();
    test_edit_commit_round_trip(line_edit, keyboard, &editing, &callback_cookie, "A");

    reset_finished_capture();
    test_edit_commit_round_trip(line_edit, keyboard, &editing, &callback_cookie, "A");

    assert(picoui_keyboard_input_ascii(keyboard, 'B') == 0);
    assert(strcmp(picoui_line_edit_get_text(line_edit), "B") == 0);
    assert(picoui_line_edit_get_editing(line_edit, &editing) == 0);
    assert(editing == 1);
    assert(picoui_widget_release_editing(&line_edit->widget) == 0);
    assert(picoui_native_line_edit_set_editing(line_edit, 0) == 0);

    assert(picoui_widget_claim_focus(&plain_text->widget) == 0);
    assert(picoui_widget_claim_editing(&plain_text->widget) == 0);
    assert(picoui_keyboard_input_ascii(keyboard, 'C') == -1);
    assert(picoui_widget_release_editing(&plain_text->widget) == 0);

    assert(picoui_widget_claim_focus(&line_edit->widget) == 0);
    assert(picoui_widget_claim_editing(&line_edit->widget) == 0);
    assert(picoui_native_line_edit_set_text(line_edit, "ZX") == 0);
    assert(picoui_native_line_edit_set_editing(line_edit, 1) == 0);
    assert(picoui_keyboard_exit(keyboard) == 0);
    assert(picoui_line_edit_get_editing(line_edit, &editing) == 0);
    assert(editing == 0);
    assert(line_edit->widget.last_edit_result == PICOUI_EDIT_RESULT_CANCEL);
    assert(picoui_widget_is_editing_owner(&line_edit->widget) == 0);
    assert(g_finished_count == 1);

    assert(picoui_widget_destroy(&line_edit->widget) == 0);
    line_edit = picoui_line_edit_create_with_props(
        window,
        &(struct picoui_line_edit_props){
            .id = "line_edit_recreated",
            .text = "",
            .keyboard_binding = 1U,
            .has_keyboard_binding = 1,
            .width = 220,
            .height = 32,
        });
    assert(line_edit != 0);
    assert(picoui_line_edit_set_on_edit_finished(line_edit,
                                                 on_line_edit_finished,
                                                 &callback_cookie) == 0);
    reset_finished_capture();
    test_edit_commit_round_trip(line_edit, keyboard, &editing, &callback_cookie, "A");

    picoui_deinit();
    assert(picoui_init() == 0);
    picoui_deinit();
    return 0;
}
