#include "tinyui.h"
#include "../../../src/gui/ldButton.h"
#include "../../../src/gui/ldGui.h"
#include "../../../src/misc/ldMsg.h"
#include "../../../examples/common/demo/widget/fonts/uiFonts.h"
#include "internal.h"

#include <assert.h>
#include <dlfcn.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

/* M2 local compatibility for pre-v2.3 unit harness signatures. */
static struct tinyui_button *test_button_create(void *parent, const char *id)
{
    (void)id;
    return (struct tinyui_button *)(void *)tinyui_button_create((tinyui_obj_t *)parent);
}
static struct tinyui_checkbox *test_checkbox_create(void *parent, const char *id)
{
    (void)id;
    return (struct tinyui_checkbox *)(void *)tinyui_checkbox_create((tinyui_obj_t *)parent);
}
static struct tinyui_switch *test_switch_create(void *parent, const char *id)
{
    (void)id;
    return (struct tinyui_switch *)(void *)tinyui_switch_create((tinyui_obj_t *)parent);
}
static struct tinyui_slider *test_slider_create(void *parent, const char *id)
{
    (void)id;
    return (struct tinyui_slider *)(void *)tinyui_slider_create((tinyui_obj_t *)parent);
}
static struct tinyui_window *test_window_create(struct tinyui_app *app, const char *id)
{
    tinyui_obj_t *screen;
    (void)app;
    (void)id;
    screen = tinyui_screen_create();
    return (struct tinyui_window *)(void *)screen;
}
#define tinyui_button_create(parent, id) test_button_create((parent), (id))
#define tinyui_checkbox_create(parent, id) test_checkbox_create((parent), (id))
#define tinyui_switch_create(parent, id) test_switch_create((parent), (id))
#define tinyui_slider_create(parent, id) test_slider_create((parent), (id))
#define tinyui_window_create(app, id) test_window_create((app), (id))
#define tinyui_widget_emit_event tinyui_runtime_internal_widget_emit_event
#define tinyui_widget_emit_clicked tinyui_runtime_internal_widget_emit_clicked
#define tinyui_widget_emit_value_changed tinyui_runtime_internal_widget_emit_value_changed
#define tinyui_widget_dispatch_event tinyui_runtime_internal_widget_dispatch_event
#define tinyui_widget_dispatch_signal tinyui_runtime_internal_widget_dispatch_signal
#define tinyui_widget_get_name_id tinyui_runtime_internal_widget_get_name_id
#define tinyui_widget_set_enabled tinyui_runtime_internal_widget_set_enabled
#define tinyui_widget_set_visible tinyui_runtime_internal_widget_set_visible
#define tinyui_widget_set_style_class tinyui_runtime_internal_widget_set_style_class
#define tinyui_widget_has_ld_binding tinyui_runtime_internal_widget_has_ld_binding

extern int tinyui_runtime_bridge_commit_pointer_event(struct tinyui_app *app,
                                                      int window_width,
                                                      int window_height,
                                                      int x,
                                                      int y,
                                                      int pressed);
void tinyui_button_test_fail_next_set_font(void);
void ldGuiClickedAction(ld_scene_t *ptScene, uint8_t touchSignal, arm_2d_location_t tLocation);

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
    char content[262144];
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
    char content[262144];
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

/* ── M2 Task 6 unified event pool fixture ─────────────────────────────────── */

static int g_press_count;
static int g_release_count;
static int g_click_count;
static int g_value_count;
static int g_event_order[8];
static int g_event_order_count;
static tinyui_obj_t *g_last_press_target;
static tinyui_obj_t *g_last_release_target;
static tinyui_obj_t *g_last_click_target;
static tinyui_obj_t *g_last_value_target;
static int g_last_press_cookie;
static int g_last_release_cookie;
static int g_last_click_cookie;
static int g_last_value_cookie;
static int g_last_value;

static void reset_button_event_fixture(void)
{
    g_press_count = 0;
    g_release_count = 0;
    g_click_count = 0;
    g_value_count = 0;
    g_event_order_count = 0;
    g_last_press_target = 0;
    g_last_release_target = 0;
    g_last_click_target = 0;
    g_last_value_target = 0;
    g_last_press_cookie = 0;
    g_last_release_cookie = 0;
    g_last_click_cookie = 0;
    g_last_value_cookie = 0;
    g_last_value = 0;
    memset(g_event_order, 0, sizeof(g_event_order));
}

static void on_pressed_event(const tinyui_event_t *event)
{
    assert(event != 0);
    assert(event->code == TINYUI_EVENT_PRESSED);
    g_press_count += 1;
    if (g_event_order_count < (int)(sizeof(g_event_order) / sizeof(g_event_order[0]))) {
        g_event_order[g_event_order_count++] = 1;
    }
    g_last_press_target = event->target;
    g_last_press_cookie = event->user_data != 0 ? *(const int *)event->user_data : -1;
}

static void on_released_event(const tinyui_event_t *event)
{
    assert(event != 0);
    assert(event->code == TINYUI_EVENT_RELEASED);
    g_release_count += 1;
    if (g_event_order_count < (int)(sizeof(g_event_order) / sizeof(g_event_order[0]))) {
        g_event_order[g_event_order_count++] = 2;
    }
    g_last_release_target = event->target;
    g_last_release_cookie = event->user_data != 0 ? *(const int *)event->user_data : -1;
}

static void on_clicked_event(const tinyui_event_t *event)
{
    assert(event != 0);
    assert(event->code == TINYUI_EVENT_CLICKED);
    g_click_count += 1;
    if (g_event_order_count < (int)(sizeof(g_event_order) / sizeof(g_event_order[0]))) {
        g_event_order[g_event_order_count++] = 3;
    }
    g_last_click_target = event->target;
    g_last_click_cookie = event->user_data != 0 ? *(const int *)event->user_data : -1;
}

static void on_value_changed_event(const tinyui_event_t *event)
{
    assert(event != 0);
    assert(event->code == TINYUI_EVENT_VALUE_CHANGED);
    g_value_count += 1;
    g_last_value_target = event->target;
    g_last_value = (int)event->data.value;
    g_last_value_cookie = event->user_data != 0 ? *(const int *)event->user_data : -1;
}

static void on_pressed_legacy(struct tinyui_widget *widget, void *user_data)
{
    (void)widget;
    (void)user_data;
}

static void on_clicked_legacy(struct tinyui_widget *widget, void *user_data)
{
    (void)widget;
    (void)user_data;
}

static void on_value_changed_legacy(struct tinyui_widget *widget, int value, void *user_data)
{
    (void)widget;
    (void)value;
    (void)user_data;
}

static void test_shared_emit_helpers_keep_callback_contract(struct tinyui_button *button,
                                                            int press_cookie,
                                                            int click_cookie,
                                                            int slider_cookie)
{
    int before_press = g_press_count;
    int before_click = g_click_count;
    int before_value = g_value_count;

    assert(button != 0);

    /* Shared emit helpers remain for internal dispatch; they take legacy cb form. */
    tinyui_widget_emit_event(on_pressed_legacy, &button->widget, &press_cookie);
    tinyui_widget_emit_clicked(on_clicked_legacy, &button->widget, &click_cookie);
    tinyui_widget_emit_value_changed(on_value_changed_legacy, &button->widget, 73, &slider_cookie);
    tinyui_widget_emit_event(0, &button->widget, &press_cookie);
    tinyui_widget_emit_clicked(0, &button->widget, &click_cookie);
    tinyui_widget_emit_value_changed(0, &button->widget, 91, &slider_cookie);

    /* Unified pool counters must stay unchanged — helpers do not bypass the pool. */
    assert(g_press_count == before_press);
    assert(g_click_count == before_click);
    assert(g_value_count == before_value);
    (void)slider_cookie;
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
        "tinyui_runtime_internal_widget_accepts_event",
        "tinyui_runtime_internal_widget_slider_value_to_percent",
        "tinyui_runtime_internal_widget_slider_percent_to_value",
        "tinyui_runtime_internal_widget_sync_ld_value",
        "tinyui_runtime_internal_widget_emit_ld_event_bridge",
        "tinyui_runtime_internal_widget_claim_focus_for_signal",
        "tinyui_runtime_internal_widget_restore_rejected_list_selection",
        "tinyui_runtime_internal_widget_get_owner_app",
        "tinyui_runtime_internal_widget_note_focus_event",
    };
    size_t i;

    for (i = 0; i < sizeof(old_event_helper_names) / sizeof(old_event_helper_names[0]); ++i) {
        assert_repo_file_lacks("tinyui/src/core/event.c", old_event_helper_names[i]);
    }
    for (i = 0; i < sizeof(new_event_helper_names) / sizeof(new_event_helper_names[0]); ++i) {
        assert_repo_file_contains("tinyui/src/core/event.c", new_event_helper_names[i]);
    }
    assert_repo_file_contains("tinyui/src/core/internal.h", "tinyui_runtime_internal_widget_sync_ld_value");
    assert_repo_file_contains("tinyui/src/core/internal.h", "tinyui_runtime_internal_widget_emit_ld_event_bridge");
    /* Old short names must not remain as external symbols. */
    assert_self_binary_lacks_symbol("tinyui_widget_sync_ld_value");
    assert_self_binary_lacks_symbol("tinyui_widget_emit_ld_event_bridge");
}

static void test_dispatch_entries_use_tinyui_prefix_in_core_seam(void)
{
    assert_repo_file_lacks("tinyui/src/core/event.c", "picoui_widget_dispatch_signal");
    assert_repo_file_lacks("tinyui/src/core/event.c", "picoui_widget_dispatch_event");
    assert_repo_file_contains("tinyui/src/core/event.c", "tinyui_runtime_internal_widget_dispatch_signal");
    assert_repo_file_contains("tinyui/src/core/event.c", "tinyui_runtime_internal_widget_dispatch_event");
    assert_repo_file_contains("tinyui/src/core/internal.h", "tinyui_runtime_internal_widget_dispatch_signal");
    assert_repo_file_contains("tinyui/src/core/internal.h", "tinyui_runtime_internal_widget_dispatch_event");
    assert_self_binary_lacks_symbol("tinyui_widget_dispatch_signal");
    assert_self_binary_lacks_symbol("tinyui_widget_dispatch_event");
}

static void test_button_shared_text_helper_uses_tinyui_prefix(void)
{
    assert_repo_file_lacks("tinyui/src/core/widget.c", "picoui_backend_set_text");
    assert_repo_file_contains("tinyui/src/core/widget.c", "tinyui_runtime_internal_widget_set_backend_text");
    assert_repo_file_contains("tinyui/src/widgets/button.c", "tinyui_runtime_internal_widget_set_text");
    assert_self_binary_lacks_symbol("tinyui_backend_set_text");
}

static void test_button_internal_seams_use_tinyui_prefix(void)
{
    assert_repo_file_lacks("tinyui/src/widgets/button.c", "picoui_button_alloc");
    assert_repo_file_lacks("tinyui/src/widgets/button.c", "picoui_button_set_event");
    assert_repo_file_lacks("tinyui/src/widgets/button.c", "picoui_button_get_ld");
    assert_repo_file_lacks("tinyui/src/widgets/button.c", "tinyui_button_get_ld");
    assert_repo_file_lacks("tinyui/src/widgets/button.c", "picoui_button_default_font");
    assert_repo_file_lacks("tinyui/src/widgets/button.c", "picoui_button_resolve_font");
    assert_repo_file_lacks("tinyui/src/widgets/button.c", "picoui_button_dispose_partial");
    assert_repo_file_lacks("tinyui/src/widgets/button.c", "tinyui_button_dispose_partial");
    assert_repo_file_lacks("tinyui/src/widgets/button.c", "picoui_button_props_are_valid");
    assert_repo_file_lacks("tinyui/src/widgets/button.c", "picoui_button_fail_next_set_font");
    assert_repo_file_lacks("tinyui/src/widgets/button.c", "picoui_backend_button_test_fail_next_set_font");
    /* Task 6: no dedicated callback fields or legacy set_event helper. */
    assert_repo_file_lacks("tinyui/src/widgets/button.c", "tinyui_button_set_event");
    assert_repo_file_lacks("tinyui/src/core/internal.h", "on_pressed_user_data");
    assert_repo_file_lacks("tinyui/src/core/internal.h", "on_released_user_data");
    assert_repo_file_contains("tinyui/src/widgets/button.c", "tinyui_obj_add_event_cb");
    assert_repo_file_contains("tinyui/src/widgets/button.c", "tinyui_runtime_internal_widget_create_leaf");
    assert_repo_file_contains("tinyui/src/widgets/button.c", "tinyui_button_alloc");
    assert_repo_file_contains("tinyui/src/widgets/button.c", "tinyui_button_default_font");
    assert_repo_file_contains("tinyui/src/widgets/button.c", "tinyui_button_resolve_font");
    assert_repo_file_contains("tinyui/src/widgets/button.c", "tinyui_button_rollback");
    assert_repo_file_contains("tinyui/src/widgets/button.c", "tinyui_button_props_are_valid");
    assert_repo_file_contains("tinyui/src/widgets/button.c", "tinyui_button_fail_next_set_font");
    assert_repo_file_contains("tinyui/src/widgets/button.c", "tinyui_button_test_fail_next_set_font");
    assert_self_binary_lacks_symbol("tinyui_button_get_ld");
    assert_self_binary_lacks_symbol("tinyui_button_default_font");
    assert_self_binary_lacks_symbol("tinyui_button_resolve_font");
    assert_self_binary_lacks_symbol("tinyui_button_alloc");
    assert_self_binary_lacks_symbol("tinyui_button_set_event");
    assert_self_binary_lacks_symbol("tinyui_button_rollback");
    assert_self_binary_lacks_symbol("tinyui_button_props_are_valid");
    assert_self_binary_lacks_symbol("tinyui_backend_button_test_fail_next_set_font");
}

static void test_button_create_with_props_pushes_all_fields(struct tinyui_window *win)
{
    struct tinyui_button *btn = tinyui_button_create_with_props(
        win,
        &(struct tinyui_button_props){
            .fields = TINYUI_BUTTON_FIELD_TEXT | TINYUI_BUTTON_FIELD_WIDTH |
                      TINYUI_BUTTON_FIELD_HEIGHT,
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
    struct tinyui_font failing_font;
    assert(tinyui_font_from_builtin(TINYUI_FONT_ARIAL_12, &failing_font) == TINYUI_OK);

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
            .fields = TINYUI_BUTTON_FIELD_TEXT | TINYUI_BUTTON_FIELD_FONT,
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
    ldButton_t *ld_button;
    const char *got = 0;

    assert(btn != 0);
    ld_button = (ldButton_t *)btn->widget.ld_widget;
    assert(ld_button != 0);
    assert(ldButtonGetFont(ld_button) == (arm_2d_font_t *)FONT_ARIAL_12);
    assert(tinyui_button_set_text(btn, "NewLabel") == 0);
    backend = &btn->widget;
    assert(backend->text != 0);
    assert(strcmp(backend->text, "NewLabel") == 0);
    assert(tinyui_button_get_text(btn, &got) == 0);
    assert(got != 0);
    assert(strcmp(got, "NewLabel") == 0);
    assert(ldButtonGetText(ld_button) != 0);
    assert(strcmp((const char *)ldButtonGetText(ld_button), "NewLabel") == 0);
}

static void test_button_set_font_maps_public_font_to_legacy_font(struct tinyui_window *win)
{
    struct tinyui_font arial16;
    struct tinyui_button *btn = tinyui_button_create(win, "btn_font");
    ldButton_t *ld_button;

    assert(tinyui_font_from_builtin(TINYUI_FONT_ARIAL_16_A8, &arial16) == TINYUI_OK);
    assert(btn != 0);
    ld_button = (ldButton_t *)btn->widget.ld_widget;
    assert(ld_button != 0);
    assert(tinyui_button_set_font(btn, &arial16) == 0);
    assert(btn->widget.font == &arial16);
    assert(ldButtonGetFont(ld_button) == (arm_2d_font_t *)FONT_ARIAL_16_A8);
}

static void test_button_set_style_class(struct tinyui_window *win)
{
    struct tinyui_button *btn = tinyui_button_create(win, "btn_style");
    assert(btn != 0);
    assert(tinyui_runtime_internal_widget_set_style_class(&btn->widget, "primary") == 0);
    assert(btn->widget.style_class != 0);
    assert(strcmp(btn->widget.style_class, "primary") == 0);
}

static void test_button_rejects_null_args(struct tinyui_window *win)
{
    (void)win;
    assert(tinyui_button_create(0, "id") == 0);
    /* id ignored in v2.3 create */ assert(1);
    assert(tinyui_button_set_text(0, "text") == -1);
    assert(tinyui_button_set_on_clicked(0, 0, 0) == -1);
}

static void test_button_constructor_binds_ld_without_backend_wrapper(struct tinyui_window *win)
{
    struct tinyui_button *btn = tinyui_button_create(win, "btn_direct_path");

    assert(btn != 0);
    assert(tinyui_widget_has_ld_binding(&btn->widget) == 1);
}

/* Task 6 L3/L4: each setter must update real ldButton_t fields. */
static void test_button_setters_update_real_ld_state(struct tinyui_window *win)
{
    struct tinyui_button *btn = tinyui_button_create(win, "btn_l4");
    ldButton_t *ld_button;
    unsigned int release_rgb = 0U;
    unsigned int press_rgb = 0U;
    unsigned int text_rgb = 0U;
    unsigned int key_value = 0U;
    int transparent = -1;
    int checkable = -1;
    int pressed = -1;
    const char *text = 0;

    assert(btn != 0);
    ld_button = (ldButton_t *)btn->widget.ld_widget;
    assert(ld_button != 0);

    assert(tinyui_button_set_text(btn, "L4Btn") == 0);
    assert(tinyui_button_get_text(btn, &text) == 0);
    assert(text != 0 && strcmp(text, "L4Btn") == 0);
    assert(ld_button->pStr != 0);
    assert(strcmp((const char *)ld_button->pStr, "L4Btn") == 0);

    assert(tinyui_button_set_color(btn, 0x112233U, 0x445566U) == 0);
    assert(tinyui_button_get_release_color(btn, &release_rgb) == 0);
    assert(tinyui_button_get_press_color(btn, &press_rgb) == 0);
    assert(ldButtonGetReleaseColor(ld_button) == (ldColor)tinyui_rgb_to_ld_color(0x112233U));
    assert(ldButtonGetPressColor(ld_button) == (ldColor)tinyui_rgb_to_ld_color(0x445566U));
    assert(release_rgb == tinyui_ld_color_to_rgb((unsigned int)ldButtonGetReleaseColor(ld_button)));
    assert(press_rgb == tinyui_ld_color_to_rgb((unsigned int)ldButtonGetPressColor(ld_button)));

    assert(tinyui_button_set_text_color(btn, 0xAABBCCU) == 0);
    assert(tinyui_button_get_text_color(btn, &text_rgb) == 0);
    assert(ldButtonGetTextColor(ld_button) == (ldColor)tinyui_rgb_to_ld_color(0xAABBCCU));
    assert(text_rgb == tinyui_ld_color_to_rgb((unsigned int)ldButtonGetTextColor(ld_button)));

    assert(tinyui_button_set_transparent(btn, 1) == 0);
    assert(tinyui_button_get_transparent(btn, &transparent) == 0);
    assert(transparent == 1);
    assert(ldButtonGetTransparent(ld_button) == true);

    assert(tinyui_button_set_checkable(btn, 1) == 0);
    assert(tinyui_button_get_checkable(btn, &checkable) == 0);
    assert(checkable == 1);
    assert(ldButtonGetCheckable(ld_button) == true);

    assert(tinyui_button_set_key_value(btn, 0x55AAu) == 0);
    assert(tinyui_button_get_key_value(btn, &key_value) == 0);
    assert(key_value == 0x55AAu);
    assert(ldButtonGetKeyValue(ld_button) == 0x55AAu);

    assert(tinyui_button_set_pressed(btn, 1) == 0);
    assert(tinyui_button_get_pressed(btn, &pressed) == 0);
    assert(pressed == 1);
    assert(ldButtonGetPress(ld_button) == true);
    assert(tinyui_button_set_pressed(btn, 0) == 0);
    assert(tinyui_button_get_pressed(btn, &pressed) == 0);
    assert(pressed == 0);
    assert(ldButtonGetPress(ld_button) == false);

    /* Canonical name is set_pressed; no public set_press. */
    assert_repo_file_lacks("tinyui/include/widgets/button.h", "tinyui_button_set_press(");
    assert_repo_file_contains("tinyui/include/widgets/button.h", "tinyui_button_set_pressed");
}

/* Task 6 L5-E: PRESSED/RELEASED/CLICKED via unified pool, order, target, user_data. */
static void test_button_unified_pool_press_release_click(struct tinyui_button *button)
{
    tinyui_event_handle_t h_press = 0U;
    tinyui_event_handle_t h_release = 0U;
    tinyui_event_handle_t h_click = 0U;
    int press_cookie = 11;
    int release_cookie = 22;
    int click_cookie = 33;
    struct tinyui_widget *backend;

    assert(button != 0);
    backend = &button->widget;
    reset_button_event_fixture();

    assert(tinyui_obj_add_event_cb((tinyui_obj_t *)button,
                                   TINYUI_EVENT_MASK(TINYUI_EVENT_PRESSED),
                                   on_pressed_event,
                                   &press_cookie,
                                   &h_press) == TINYUI_OK);
    assert(tinyui_obj_add_event_cb((tinyui_obj_t *)button,
                                   TINYUI_EVENT_MASK(TINYUI_EVENT_RELEASED),
                                   on_released_event,
                                   &release_cookie,
                                   &h_release) == TINYUI_OK);
    assert(tinyui_obj_add_event_cb((tinyui_obj_t *)button,
                                   TINYUI_EVENT_MASK(TINYUI_EVENT_CLICKED),
                                   on_clicked_event,
                                   &click_cookie,
                                   &h_click) == TINYUI_OK);
    assert(h_press != 0U);
    assert(h_release != 0U);
    assert(h_click != 0U);

    assert(tinyui_runtime_internal_widget_dispatch_native_signal(backend, SIGNAL_PRESS, 0) == 0);
    assert(g_press_count == 1);
    assert(g_release_count == 0);
    assert(g_click_count == 0);
    assert(g_event_order_count == 1);
    assert(g_event_order[0] == 1);
    assert(g_last_press_target == (tinyui_obj_t *)button);
    assert(g_last_press_cookie == press_cookie);

    assert(tinyui_runtime_internal_widget_dispatch_native_signal(backend, SIGNAL_RELEASE, 0) == 0);
    assert(g_press_count == 1);
    assert(g_release_count == 1);
    assert(g_click_count == 1);
    assert(g_event_order_count == 3);
    assert(g_event_order[1] == 2);
    assert(g_event_order[2] == 3);
    assert(g_last_release_target == (tinyui_obj_t *)button);
    assert(g_last_release_cookie == release_cookie);
    assert(g_last_click_target == (tinyui_obj_t *)button);
    assert(g_last_click_cookie == click_cookie);

    /* set_on_* is a narrow forward of the unified pool (replace semantics on its own slot). */
    {
        int alt_cookie = 99;
        int second_cookie = 100;

        assert(tinyui_obj_remove_event_cb((tinyui_obj_t *)button, h_click) == TINYUI_OK);
        assert(tinyui_button_set_on_clicked((tinyui_obj_t *)button,
                                            on_clicked_event,
                                            &alt_cookie) == 0);

        reset_button_event_fixture();
        assert(tinyui_runtime_internal_widget_dispatch_native_signal(backend, SIGNAL_PRESS, 0) == 0);
        assert(tinyui_runtime_internal_widget_dispatch_native_signal(backend, SIGNAL_RELEASE, 0) == 0);
        assert(g_click_count == 1);
        assert(g_last_click_cookie == alt_cookie);
        assert(g_last_click_target == (tinyui_obj_t *)button);

        /* Second set_on_clicked replaces previous set_on registration. */
        assert(tinyui_button_set_on_clicked((tinyui_obj_t *)button,
                                            on_clicked_event,
                                            &second_cookie) == 0);
        reset_button_event_fixture();
        assert(tinyui_runtime_internal_widget_dispatch_native_signal(backend, SIGNAL_PRESS, 0) == 0);
        assert(tinyui_runtime_internal_widget_dispatch_native_signal(backend, SIGNAL_RELEASE, 0) == 0);
        assert(g_click_count == 1);
        assert(g_last_click_cookie == second_cookie);
    }
}

static void test_button_disabled_or_hidden_suppresses_interaction(struct tinyui_button *button)
{
    struct tinyui_widget *backend;
    int press_cookie = 7;
    int release_cookie = 8;
    int click_cookie = 9;

    assert(button != 0);
    backend = &button->widget;
    reset_button_event_fixture();

    assert(tinyui_obj_add_event_cb((tinyui_obj_t *)button,
                                   TINYUI_EVENT_MASK(TINYUI_EVENT_PRESSED),
                                   on_pressed_event,
                                   &press_cookie,
                                   0) == TINYUI_OK);
    assert(tinyui_obj_add_event_cb((tinyui_obj_t *)button,
                                   TINYUI_EVENT_MASK(TINYUI_EVENT_RELEASED),
                                   on_released_event,
                                   &release_cookie,
                                   0) == TINYUI_OK);
    assert(tinyui_obj_add_event_cb((tinyui_obj_t *)button,
                                   TINYUI_EVENT_MASK(TINYUI_EVENT_CLICKED),
                                   on_clicked_event,
                                   &click_cookie,
                                   0) == TINYUI_OK);

    assert(tinyui_runtime_internal_widget_set_enabled(backend, 0) == 0);
    assert(tinyui_runtime_internal_widget_dispatch_native_signal(backend, SIGNAL_PRESS, 0) == 0);
    assert(tinyui_runtime_internal_widget_dispatch_native_signal(backend, SIGNAL_RELEASE, 0) == 0);
    assert(g_press_count == 0);
    assert(g_release_count == 0);
    assert(g_click_count == 0);
    assert(tinyui_runtime_internal_widget_set_enabled(backend, 1) == 0);

    assert(tinyui_runtime_internal_widget_set_visible(backend, 0) == 0);
    assert(tinyui_runtime_internal_widget_dispatch_native_signal(backend, SIGNAL_PRESS, 0) == 0);
    assert(tinyui_runtime_internal_widget_dispatch_native_signal(backend, SIGNAL_RELEASE, 0) == 0);
    assert(g_press_count == 0);
    assert(g_release_count == 0);
    assert(g_click_count == 0);
    assert(tinyui_runtime_internal_widget_set_visible(backend, 1) == 0);
}

static void test_button_old_handle_does_not_affect_new_callback(struct tinyui_button *button)
{
    tinyui_event_handle_t old_handle = 0U;
    tinyui_event_handle_t new_handle = 0U;
    int old_cookie = 1;
    int new_cookie = 2;
    struct tinyui_widget *backend;

    assert(button != 0);
    backend = &button->widget;
    reset_button_event_fixture();

    assert(tinyui_obj_add_event_cb((tinyui_obj_t *)button,
                                   TINYUI_EVENT_MASK(TINYUI_EVENT_CLICKED),
                                   on_clicked_event,
                                   &old_cookie,
                                   &old_handle) == TINYUI_OK);
    assert(tinyui_obj_remove_event_cb((tinyui_obj_t *)button, old_handle) == TINYUI_OK);
    assert(tinyui_obj_add_event_cb((tinyui_obj_t *)button,
                                   TINYUI_EVENT_MASK(TINYUI_EVENT_CLICKED),
                                   on_clicked_event,
                                   &new_cookie,
                                   &new_handle) == TINYUI_OK);
    assert(new_handle != 0U);
    assert(new_handle != old_handle);
    assert(tinyui_obj_remove_event_cb((tinyui_obj_t *)button, old_handle) == TINYUI_ERROR_INVALID_ARG);

    assert(tinyui_runtime_internal_widget_dispatch_native_signal(backend, SIGNAL_PRESS, 0) == 0);
    assert(tinyui_runtime_internal_widget_dispatch_native_signal(backend, SIGNAL_RELEASE, 0) == 0);
    assert(g_click_count == 1);
    assert(g_last_click_cookie == new_cookie);
}

static void test_blank_click_does_not_emit_null_sender_or_crash(struct tinyui_app *app,
                                                                struct tinyui_button *button)
{
    arm_2d_location_t blank = {0};
    arm_2d_location_t button_point = {0};
    ld_scene_t *scene;
    ldBase_t *button_ld;

    assert(app != 0);
    assert(button != 0);
    scene = app->ld_scene;
    assert(scene != 0);
    button_ld = (ldBase_t *)button->widget.ld_widget;
    assert(button_ld != 0);

    if (scene->ptMsgQueue != 0) {
        ldMsgDeinit(&scene->ptMsgQueue);
    }
    assert(ldMsgInit(&scene->ptMsgQueue, 8) == true);

    blank.iX = 1000;
    blank.iY = 590;
    ldGuiClickedAction(scene, SIGNAL_PRESS, blank);
    ldMsgProcess(scene);
    ldGuiClickedAction(scene, SIGNAL_RELEASE, blank);
    ldMsgProcess(scene);

    assert(ldMsgEmit(scene->ptMsgQueue, 0, SIGNAL_PRESS, 0) == true);
    ldMsgProcess(scene);

    assert(tinyui_runtime_bridge_commit_pointer_event(app, 1024, 600, 50000, 50000, 1) == 0);
    ldGuiTouchProcess(scene);
    ldMsgProcess(scene);
    assert(tinyui_runtime_bridge_commit_pointer_event(app, 1024, 600, 50000, 50000, 0) == 0);
    ldGuiTouchProcess(scene);
    ldMsgProcess(scene);

    button_point.iX = button_ld->use_as__arm_2d_control_node_t.tRegion.tLocation.iX;
    button_point.iY = button_ld->use_as__arm_2d_control_node_t.tRegion.tLocation.iY;
    ldGuiClickedAction(scene, SIGNAL_PRESS, button_point);
    ldMsgProcess(scene);
    ldGuiClickedAction(scene, SIGNAL_RELEASE, button_point);
    ldMsgProcess(scene);
}

int main(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win;
    tinyui_deinit();
    assert(tinyui_init() == TINYUI_OK);
    app = tinyui_runtime_internal_app_current();
    win = tinyui_window_create(app, "root");
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

    assert(tinyui_button_set_on_pressed(0, on_pressed_event, &press_cookie) == -1);
    assert(tinyui_button_set_on_released(0, on_released_event, &release_cookie) == -1);
    assert(tinyui_button_set_on_clicked(0, on_clicked_event, &click_cookie) == -1);

    /* Dedicated set_on_* only forward into the unified event pool. */
    assert(tinyui_button_set_on_pressed((tinyui_obj_t *)button, on_pressed_event, &press_cookie) == 0);
    assert(tinyui_button_set_on_released((tinyui_obj_t *)button, on_released_event, &release_cookie) == 0);
    assert(tinyui_button_set_on_clicked((tinyui_obj_t *)button, on_clicked_event, &click_cookie) == 0);

    /* Wrapper must not keep private on_* callback fields. */
    assert_repo_file_lacks("tinyui/src/core/internal.h", "tinyui_event_cb on_clicked");
    assert_repo_file_lacks("tinyui/src/core/internal.h", "tinyui_event_cb on_pressed");
    assert_repo_file_lacks("tinyui/src/core/internal.h", "tinyui_event_cb on_released");

    reset_button_event_fixture();
    test_shared_emit_helpers_keep_callback_contract(button, press_cookie, click_cookie, slider_cookie);
    test_event_shared_helpers_use_tinyui_prefix_in_core_seam();
    test_dispatch_entries_use_tinyui_prefix_in_core_seam();
    test_button_shared_text_helper_uses_tinyui_prefix();
    test_button_internal_seams_use_tinyui_prefix();
    test_blank_click_does_not_emit_null_sender_or_crash(app_state, button);

    button_name_id = tinyui_runtime_internal_widget_get_name_id((const struct tinyui_widget *)button);
    checkbox_name_id = tinyui_runtime_internal_widget_get_name_id((const struct tinyui_widget *)checkbox);
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

    /* Real LD signal path -> unified pool (set_on_* already registered above). */
    reset_button_event_fixture();
    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue,
                     backend->ld_widget,
                     SIGNAL_PRESS,
                     0) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(g_press_count == 1);
    assert(g_release_count == 0);
    assert(g_click_count == 0);
    assert(g_event_order_count == 1);
    assert(g_event_order[0] == 1);
    assert(g_last_press_target == (tinyui_obj_t *)button);
    assert(g_last_press_cookie == press_cookie);
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
    assert(g_press_count == 1);
    assert(g_release_count == 1);
    assert(g_click_count == 1);
    assert(g_event_order_count == 3);
    assert(g_event_order[1] == 2);
    assert(g_event_order[2] == 3);
    assert(g_last_release_target == (tinyui_obj_t *)button);
    assert(g_last_release_cookie == release_cookie);
    assert(g_last_click_target == (tinyui_obj_t *)button);
    assert(g_last_click_cookie == click_cookie);
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

    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue,
                     backend->ld_widget,
                     SIGNAL_HOLD_DOWN,
                     CONNECT32(3, 4, 10, 11)) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(g_press_count == 1);
    assert(g_release_count == 1);
    assert(g_click_count == 1);

    /* Fresh buttons for isolation of pool-order / disable / handle tests. */
    {
        struct tinyui_button *btn_pool = tinyui_button_create(win, "submit_pool");
        struct tinyui_button *btn_gate = tinyui_button_create(win, "submit_gate");
        struct tinyui_button *btn_handle = tinyui_button_create(win, "submit_handle");
        assert(btn_pool != 0);
        assert(btn_gate != 0);
        assert(btn_handle != 0);
        test_button_unified_pool_press_release_click(btn_pool);
        test_button_disabled_or_hidden_suppresses_interaction(btn_gate);
        test_button_old_handle_does_not_affect_new_callback(btn_handle);
    }

    test_button_setters_update_real_ld_state(win);

    /* Value-path widgets remain Task 7; only keep setters smoke-safe here. */
    (void)on_value_changed_event;
    (void)checkbox_cookie;
    (void)switch_cookie;
    (void)slider_cookie;
    assert(tinyui_checkbox_set_checked(checkbox, 1) == 0);
    assert(tinyui_switch_set_checked(sw, 1) == 0);
    assert(tinyui_slider_set_value(slider, 40) == 0);

    ldMsgDeinit(&app_state->ld_scene->ptMsgQueue);
    ldButtonSetFont((ldButton_t *)backend->ld_widget, (arm_2d_font_t *)FONT_ARIAL_12);

    test_shared_emit_helpers_no_longer_use_tinyui_backend_prefix();
    test_button_create_with_props_pushes_all_fields(win);
    test_button_set_text_round_trip(win);
    test_button_set_font_maps_public_font_to_legacy_font(win);
    test_button_set_style_class(win);
    test_button_rejects_null_args(win);

    tinyui_runtime_internal_app_destroy(app);
    return 0;
}
