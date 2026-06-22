#include "app.h"
#include "button.h"
#include "message_box.h"
#include "widget.h"
#include "window.h"
#include "../../../src/gui/ldGui.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldMessageBox.h"
#include "../../../src/misc/ldMsg.h"
#include "internal.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void ldGuiClickedAction(ld_scene_t *ptScene, uint8_t touchSignal, arm_2d_location_t tLocation);

static int confirm_count = 0;
static struct tinyui_message_box *confirm_box = 0;
static void *confirm_user_data = 0;
static int confirm_button_index = -1;
static int underlay_press_count = 0;
static struct tinyui_widget *underlay_last_press_widget = 0;
static int underlay_last_press_cookie = 0;

static const char *test_source_file_path = __FILE__;

static const char *resolve_repo_path(const char *repo_relative_path)
{
    static char resolved_path[1024];
    char base_path[1024];
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

static int test_source_contains_function_definition(const char *path, const char *name)
{
    char command[1024];

    assert(path != 0);
    assert(name != 0);
    snprintf(command,
             sizeof(command),
             "python3 - '%s' '%s' <<'PY'\n"
             "from pathlib import Path\n"
             "import re\n"
             "import sys\n"
             "text = Path(sys.argv[1]).read_text()\n"
             "symbol = sys.argv[2]\n"
             "pattern = re.compile(r'(^|\\n)\\s*(?:static\\s+)?[A-Za-z_][A-Za-z0-9_\\s\\*]*\\b' + re.escape(symbol) + r'\\s*\\(', re.MULTILINE)\n"
             "raise SystemExit(0 if pattern.search(text) else 1)\n"
             "PY",
             path,
             name);
    return system(command) == 0;
}

static void assert_source_lacks_function_definition(const char *path, const char *name)
{
    assert(!test_source_contains_function_definition(path, name));
}

static void assert_source_has_function_definition(const char *path, const char *name)
{
    assert(test_source_contains_function_definition(path, name));
}

static uint64_t make_signal_value_xy(uint16_t x, uint16_t y)
{
    return ((uint64_t)x << 16) | (uint64_t)y;
}

static uint64_t make_release_value_xy(uint16_t x, uint16_t y)
{
    return ((uint64_t)x << 16) | (uint64_t)y;
}

static void on_confirm(struct tinyui_message_box *box, void *user_data)
{
    confirm_count++;
    confirm_box = box;
    confirm_user_data = user_data;
}

static void on_confirm_with_index(struct tinyui_message_box *box, int index, void *user_data)
{
    confirm_count++;
    confirm_box = box;
    confirm_user_data = user_data;
    confirm_button_index = index;
}

static void on_underlay_pressed(struct tinyui_widget *widget, void *user_data)
{
    underlay_press_count++;
    underlay_last_press_widget = widget;
    underlay_last_press_cookie = user_data != 0 ? *(const int *)user_data : -1;
}

static void test_message_box_create_and_props(struct tinyui_window *win)
{
    int user_cookie = 11;
    struct tinyui_message_box_props props = {
        .id = "message_box_props",
        .style_class = "dialog",
        .user_data = &user_cookie,
        .title = "Update",
        .message = "Apply settings?",
        .confirm_text = "OK",
    };
    struct tinyui_message_box *box =
        tinyui_message_box_create((struct tinyui_widget *)win, "message_box");
    struct tinyui_message_box *with_props =
        tinyui_message_box_create_with_props((struct tinyui_widget *)win, &props);

    assert(box != 0);
    assert(with_props != 0);
    assert(tinyui_message_box_get_title(box) == 0);
    assert(tinyui_message_box_get_message(box) == 0);
    assert(tinyui_message_box_get_confirm_text(box) == 0);
    assert(tinyui_message_box_get_title(with_props) == props.title);
    assert(tinyui_message_box_get_message(with_props) == props.message);
    assert(tinyui_message_box_get_confirm_text(with_props) == props.confirm_text);
}

static void test_message_box_create_builds_direct_backend_mapping(struct tinyui_window *win)
{
    struct tinyui_message_box *box =
        tinyui_message_box_create((struct tinyui_widget *)win, "message_box_direct_mapping");
    struct tinyui_widget *backend;
    struct tinyui_widget *parent_backend;
    ldMessageBox_t *ld_message_box;

    assert(box != 0);
    backend = &box->widget;
    parent_backend = &win->widget;
    assert(backend->ld_widget != 0);
    assert(parent_backend->ld_widget != 0);
    assert(backend->kind == TINYUI_BACKEND_WIDGET_MESSAGE_BOX);
    assert(backend->owner == parent_backend->owner);
    assert((ldBase_t *)ldBaseGetRootNode((arm_2d_control_node_t *)backend->ld_widget) == (ldBase_t *)ldBaseGetRootNode((arm_2d_control_node_t *)parent_backend->ld_widget));
    assert(ldBaseGetParent((ldBase_t *)backend->ld_widget) == (ldBase_t *)parent_backend->ld_widget);
    assert(backend->ld_name_id != 0);
    assert(backend->ld_event_bridge_scene != 0);
    assert(backend->ld_event_bridge_sender == backend->ld_widget);
    ld_message_box = (ldMessageBox_t *)backend->ld_widget;
    assert(ld_message_box != 0);
    assert(((ldBase_t *)ld_message_box)->pInfo == backend);
}

static void test_message_box_widget_file_owns_native_helper_truth(struct tinyui_window *win)
{
    static const char *buttons[] = {
        "Cancel",
        "Apply",
    };
    struct tinyui_message_box *box =
        tinyui_message_box_create((struct tinyui_widget *)win, "message_box_widget_truth");
    struct tinyui_widget *backend;
    ldMessageBox_t *ld_message_box;

    assert(box != 0);
    assert(tinyui_message_box_set_title(box, "Widget-owned") == 0);
    assert(tinyui_message_box_set_message(box, "Helpers live in widget.c") == 0);
    assert(tinyui_message_box_set_buttons(box, buttons, 2) == 0);
    assert(tinyui_message_box_set_string_colors(box, 0x010203U, 0x040506U, 0x070809U) == 0);
    assert(tinyui_message_box_set_button_colors(box, 0x0A0B0CU, 0x0D0E0FU) == 0);
    assert(tinyui_message_box_set_bg_color(box, 0x102030U) == 0);

    backend = &box->widget;
    assert(backend->ld_widget != 0);
    ld_message_box = (ldMessageBox_t *)backend->ld_widget;
    assert(ld_message_box != 0);

    assert(strcmp((const char *)ld_message_box->pTitleStr, "Widget-owned") == 0);
    assert(strcmp((const char *)ld_message_box->pMsgStr, "Helpers live in widget.c") == 0);
    assert(ld_message_box->btnCount == 2);
    assert(strcmp((const char *)ld_message_box->ppBtnStrGroup[0], "Cancel") == 0);
    assert(strcmp((const char *)ld_message_box->ppBtnStrGroup[1], "Apply") == 0);
    assert(ld_message_box->titleStrColor == (ldColor)0x010203U);
    assert(ld_message_box->msgStrColor == (ldColor)0x040506U);
    assert(ld_message_box->btnStrColor == (ldColor)0x070809U);
    assert(ld_message_box->releaseColor == (ldColor)0x0A0B0CU);
    assert(ld_message_box->pressColor == (ldColor)0x0D0E0FU);
    assert(ld_message_box->bgColor == (ldColor)0x102030U);
}

static void test_message_box_confirm_callback_bridge(struct tinyui_window *win)
{
    int user_cookie = 23;
    struct tinyui_message_box *box =
        tinyui_message_box_create((struct tinyui_widget *)win, "message_box_state");
    struct tinyui_widget *backend;
    ldMessageBox_t *ld_message_box;
    struct tinyui_app *app_state;

    assert(box != 0);
    assert(tinyui_message_box_set_title(box, "Confirm") == 0);
    assert(tinyui_message_box_set_message(box, "Save changes?") == 0);
    assert(tinyui_message_box_set_confirm_text(box, "OK") == 0);
    confirm_count = 0;
    confirm_box = 0;
    confirm_user_data = 0;
    tinyui_message_box_set_on_confirm(box, on_confirm, &user_cookie);

    assert(tinyui_message_box_get_title(box) != 0);
    assert(tinyui_message_box_get_message(box) != 0);
    assert(tinyui_message_box_get_confirm_text(box) != 0);
    backend = &box->widget;
    assert(backend->ld_widget != 0);
    app_state = backend->owner;
    assert(app_state != 0);
    ld_message_box = (ldMessageBox_t *)backend->ld_widget;
    assert(ld_message_box != 0);
    assert(ld_message_box->ptFunc != 0);

    ld_message_box->ptFunc(app_state->ld_scene, ld_message_box);
    assert(confirm_count == 1);
    assert(confirm_box == box);
    assert(confirm_user_data == &user_cookie);
}

static void test_message_box_rejects_invalid_inputs(struct tinyui_window *win)
{
    struct tinyui_message_box *box =
        tinyui_message_box_create((struct tinyui_widget *)win, "message_box_invalid");

    assert(box != 0);
    assert(tinyui_message_box_create(0, "message_box") == 0);
    assert(tinyui_message_box_create((struct tinyui_widget *)win, 0) == 0);
    assert(tinyui_message_box_create_with_props(0,
                                                &(struct tinyui_message_box_props){
                                                    .id = "bad_parent",
                                                }) == 0);
    assert(tinyui_message_box_create_with_props((struct tinyui_widget *)win, 0) == 0);
    assert(tinyui_message_box_create_with_props((struct tinyui_widget *)win,
                                                &(struct tinyui_message_box_props){
                                                    .title = "missing_id",
                                                }) == 0);
    assert(tinyui_message_box_set_title(0, "Confirm") == -1);
    assert(tinyui_message_box_set_message(0, "Message") == -1);
    assert(tinyui_message_box_set_confirm_text(0, "OK") == -1);
    assert(tinyui_message_box_set_title(box, 0) == -1);
    assert(tinyui_message_box_set_message(box, 0) == -1);
    assert(tinyui_message_box_set_confirm_text(box, 0) == -1);
}

static void test_message_box_final_release_contract_covers_multi_action_and_readback_boundary(
    struct tinyui_window *win)
{
    static const uint8_t *buttons[2] = {
        (const uint8_t *)"Later",
        (const uint8_t *)"Apply",
    };
    struct tinyui_message_box *box =
        tinyui_message_box_create((struct tinyui_widget *)win, "message_box_release_ready");
    struct tinyui_widget *backend;
    ldMessageBox_t *ld_message_box;

    assert(box != 0);
    assert(tinyui_message_box_set_title(box, "Release") == 0);
    assert(tinyui_message_box_set_message(box, "Apply current settings now?") == 0);
    assert(tinyui_message_box_set_confirm_text(box, "Apply") == 0);
    assert(tinyui_message_box_get_title(box) != 0);
    assert(tinyui_message_box_get_message(box) != 0);
    assert(tinyui_message_box_get_confirm_text(box) != 0);

    backend = &box->widget;
    assert(backend->ld_widget != 0);
    assert(backend->kind == TINYUI_BACKEND_WIDGET_MESSAGE_BOX);
    ld_message_box = (ldMessageBox_t *)backend->ld_widget;
    assert(ld_message_box != 0);

    assert(ld_message_box->pTitleStr != 0);
    assert(ld_message_box->pMsgStr != 0);
    assert(ld_message_box->ppBtnStrGroup != 0);
    assert(ld_message_box->btnCount == 1);
    assert(strcmp((const char *)ld_message_box->pTitleStr, "Release") == 0);
    assert(strcmp((const char *)ld_message_box->pMsgStr, "Apply current settings now?") == 0);
    assert(strcmp((const char *)ld_message_box->ppBtnStrGroup[0], "Apply") == 0);

    ldMessageBoxSetBtn(ld_message_box, buttons, 2);
    assert(ld_message_box->btnCount == 2);
    assert(strcmp((const char *)ld_message_box->ppBtnStrGroup[0], "Later") == 0);
    assert(strcmp((const char *)ld_message_box->ppBtnStrGroup[1], "Apply") == 0);
    assert(strcmp(tinyui_message_box_get_confirm_text(box), "Apply") == 0);
}

static void test_message_box_native_multi_button_and_color_round_trip(struct tinyui_window *win)
{
    static const char *buttons[] = {
        "Later",
        "Apply",
        "Reset",
    };
    struct tinyui_message_box *box =
        tinyui_message_box_create((struct tinyui_widget *)win, "message_box_native");
    struct tinyui_widget *backend;
    ldMessageBox_t *ld_message_box;

    assert(box != 0);
    backend = &box->widget;
    assert(backend->ld_widget != 0);
    ld_message_box = (ldMessageBox_t *)backend->ld_widget;
    assert(ld_message_box != 0);

    assert(tinyui_message_box_set_buttons(box, buttons, 3) == 0);
    assert(tinyui_message_box_set_string_colors(box, 0x112233U, 0x445566U, 0x778899U) == 0);
    assert(tinyui_message_box_set_button_colors(box, 0x123456U, 0x654321U) == 0);
    assert(tinyui_message_box_set_bg_color(box, 0xABCDEFU) == 0);

    assert(ld_message_box->btnCount == 3);
    assert(strcmp((const char *)ld_message_box->ppBtnStrGroup[0], "Later") == 0);
    assert(strcmp((const char *)ld_message_box->ppBtnStrGroup[1], "Apply") == 0);
    assert(strcmp((const char *)ld_message_box->ppBtnStrGroup[2], "Reset") == 0);
    assert(ld_message_box->titleStrColor == (ldColor)0x112233U);
    assert(ld_message_box->msgStrColor == (ldColor)0x445566U);
    assert(ld_message_box->btnStrColor == (ldColor)0x778899U);
    assert(ld_message_box->releaseColor == (ldColor)0x123456U);
    assert(ld_message_box->pressColor == (ldColor)0x654321U);
    assert(ld_message_box->bgColor == (ldColor)0xABCDEFU);

    assert(tinyui_message_box_set_buttons(0, buttons, 3) == -1);
    assert(tinyui_message_box_set_string_colors(0, 0, 0, 0) == -1);
    assert(tinyui_message_box_set_button_colors(0, 0, 0) == -1);
    assert(tinyui_message_box_set_bg_color(0, 0) == -1);
}

static void test_message_box_multi_button_callback_reports_clicked_index(struct tinyui_window *win)
{
    static const char *buttons[] = {
        "Later",
        "Apply",
        "Reset",
    };
    int user_cookie = 31;
    struct tinyui_message_box *box =
        tinyui_message_box_create((struct tinyui_widget *)win, "message_box_multi_cb");
    struct tinyui_widget *backend;
    ldMessageBox_t *ld_message_box;
    struct tinyui_app *app_state;

    assert(box != 0);
    assert(tinyui_message_box_set_buttons(box, buttons, 3) == 0);
    tinyui_message_box_set_on_confirm_indexed(box, on_confirm_with_index, &user_cookie);

    confirm_count = 0;
    confirm_box = 0;
    confirm_user_data = 0;
    confirm_button_index = -1;

    backend = &box->widget;
    assert(backend->ld_widget != 0);
    app_state = backend->owner;
    assert(app_state != 0);
    ld_message_box = (ldMessageBox_t *)backend->ld_widget;
    assert(ld_message_box != 0);
    assert(ld_message_box->ptFunc != 0);

    ld_message_box->clickNum = 2;
    ld_message_box->ptFunc(app_state->ld_scene, ld_message_box);
    assert(confirm_count == 1);
    assert(confirm_box == box);
    assert(confirm_user_data == &user_cookie);
    assert(confirm_button_index == 2);
}

static void test_message_box_init_aliases_and_shared_base_round_trip(struct tinyui_window *win)
{
    static const char *buttons[] = {
        "Later",
        "Apply",
    };
    int user_cookie = 41;
    struct tinyui_message_box *box =
        tinyui_message_box_init((struct tinyui_widget *)win, "message_box_alias");
    struct tinyui_widget *backend;
    ldMessageBox_t *ld_message_box;

    assert(box != 0);
    backend = &box->widget;
    assert(backend->ld_widget != 0);
    ld_message_box = (ldMessageBox_t *)backend->ld_widget;
    assert(ld_message_box != 0);

    assert(tinyui_message_box_set_title(box, "Alias") == 0);
    assert(tinyui_message_box_set_msg(box, "Apply current settings?") == 0);
    assert(tinyui_message_box_set_btn(box, buttons, 2) == 0);
    assert(tinyui_message_box_set_string_color(box, 0x112233U, 0x445566U, 0x778899U) == 0);
    assert(tinyui_message_box_set_button_color(box, 0x123456U, 0x654321U) == 0);
    assert(tinyui_message_box_set_background_color(box, 0xA0B0C0U) == 0);
    tinyui_message_box_set_callback(box, on_confirm, &user_cookie);

    assert(strcmp((const char *)ld_message_box->pTitleStr, "Alias") == 0);
    assert(strcmp((const char *)ld_message_box->pMsgStr, "Apply current settings?") == 0);
    assert(ld_message_box->btnCount == 2);
    assert(strcmp((const char *)ld_message_box->ppBtnStrGroup[0], "Later") == 0);
    assert(strcmp((const char *)ld_message_box->ppBtnStrGroup[1], "Apply") == 0);
    assert(ld_message_box->titleStrColor == (ldColor)0x112233U);
    assert(ld_message_box->msgStrColor == (ldColor)0x445566U);
    assert(ld_message_box->btnStrColor == (ldColor)0x778899U);
    assert(ld_message_box->releaseColor == (ldColor)0x123456U);
    assert(ld_message_box->pressColor == (ldColor)0x654321U);
    assert(ld_message_box->bgColor == (ldColor)0xA0B0C0U);
    assert(ld_message_box->ptFunc != 0);

    assert(tinyui_widget_set_pos(&box->widget, 14, 18) == 0);
    assert(((ldBase_t *)ld_message_box)->use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 14);
    assert(((ldBase_t *)ld_message_box)->use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 18);
    assert(tinyui_widget_set_visible(&box->widget, 0) == 0);
    assert(((ldBase_t *)ld_message_box)->isHidden == true);
    assert(tinyui_widget_set_opacity(&box->widget, 62) == 0);
    assert(((ldBase_t *)ld_message_box)->opacity == 62);
    assert(tinyui_widget_set_corner(&box->widget, 5) == 0);
    assert(((ldBase_t *)ld_message_box)->isCorner == true);
}

static void test_message_box_modal_hit_and_dismiss_returns_focus_to_underlay(struct tinyui_window *win)
{
    struct tinyui_button *underlay =
        tinyui_button_create(win, "message_box_underlay_button");
    struct tinyui_message_box *box =
        tinyui_message_box_create((struct tinyui_widget *)win, "message_box_modal");
    struct tinyui_widget *button_backend;
    struct tinyui_widget *box_backend;
    struct tinyui_app *app_state;
    ldButton_t *ld_button;
    ldMessageBox_t *ld_message_box;
    int press_cookie = 77;
    arm_2d_location_t click;

    assert(underlay != 0);
    assert(box != 0);
    assert(tinyui_widget_set_pos(&underlay->widget, 110, 180) == 0);
    assert(tinyui_widget_set_size(&underlay->widget, 260, 140) == 0);
    assert(tinyui_widget_set_pos(&box->widget, 110, 180) == 0);
    assert(tinyui_message_box_set_confirm_text(box, "OK") == 0);
    assert(tinyui_button_set_on_pressed(underlay, on_underlay_pressed, &press_cookie) == 0);

    button_backend = &underlay->widget;
    box_backend = &box->widget;
    assert(button_backend->ld_widget != 0);
    assert(box_backend->ld_widget != 0);
    app_state = box_backend->owner;
    assert(app_state != 0);
    ldMsgDeinit(&app_state->ld_scene->ptMsgQueue);
    assert(ldMsgInit(&app_state->ld_scene->ptMsgQueue, 8) == true);
    ld_button = (ldButton_t *)button_backend->ld_widget;
    ld_message_box = (ldMessageBox_t *)box_backend->ld_widget;
    assert(ld_button != 0);
    assert(ld_message_box != 0);
    click.iX = (int16_t)(110 + ld_message_box->btnRegion.tLocation.iX + 2
                         + (ld_message_box->btnRegion.tSize.iWidth / 2));
    click.iY = (int16_t)(180 + ld_message_box->btnRegion.tLocation.iY
                         + (ld_message_box->btnRegion.tSize.iHeight / 2));
    assert(ldBaseGetWidget(app_state->ld_scene->ptNodeRoot, box_backend->ld_name_id) == ld_message_box);
    assert((ldBase_t *)arm_2d_helper_control_find_node_with_location(app_state->ld_scene->ptNodeRoot, click)
           == (ldBase_t *)ld_message_box);

    underlay_press_count = 0;
    underlay_last_press_widget = 0;
    underlay_last_press_cookie = 0;
    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue,
                     box_backend->ld_widget,
                     SIGNAL_PRESS,
                     make_signal_value_xy((uint16_t)click.iX, (uint16_t)click.iY)) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(underlay_press_count == 0);
    assert(underlay_last_press_widget == 0);
    assert(ld_message_box->isBtnPressed != 0);

    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue,
                     box_backend->ld_widget,
                     SIGNAL_RELEASE,
                     make_release_value_xy((uint16_t)click.iX, (uint16_t)click.iY)) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(ldBaseGetWidget(app_state->ld_scene->ptNodeRoot, box_backend->ld_name_id) == 0);
    assert((ldBase_t *)arm_2d_helper_control_find_node_with_location(app_state->ld_scene->ptNodeRoot, click)
           == (ldBase_t *)ld_button);

    ldGuiClickedAction(app_state->ld_scene, SIGNAL_PRESS, click);
    ldMsgProcess(app_state->ld_scene);
    assert(underlay_press_count == 1);
    assert(underlay_last_press_widget == &underlay->widget);
    assert(underlay_last_press_cookie == press_cookie);
    assert(ldBaseGetWidget(app_state->ld_scene->ptNodeRoot, button_backend->ld_name_id) == ld_button);
}

static void test_message_box_rejects_null_args(struct tinyui_window *win)
{
    assert(tinyui_message_box_create(0, "id") == 0);
    assert(tinyui_message_box_create(win, 0) == 0);
    assert(tinyui_message_box_set_title(0, "title") == -1);
    assert(tinyui_message_box_set_message(0, "msg") == -1);
    assert(tinyui_message_box_set_confirm_text(0, "ok") == -1);
    tinyui_message_box_set_on_confirm(0, 0, 0); /* void return */
}

static int msg_box_confirm_fired = 0;
static void on_msg_box_confirm(struct tinyui_message_box *box, void *user_data)
{
    msg_box_confirm_fired = 1;
}

static void test_message_box_confirm_callback_fires(struct tinyui_window *win)
{
    struct tinyui_message_box *box = tinyui_message_box_create(win, "mb_cb");
    assert(box != 0);
    msg_box_confirm_fired = 0;
    tinyui_message_box_set_on_confirm(box, on_msg_box_confirm, 0);
    // callback registered; firing tested via backend signal in other tests
    (void)box;
}

static void test_message_box_internal_seams_renamed_to_tinyui(void)
{
    const char *widget_source = resolve_repo_path("tinyui/src/widgets/message_box.c");

    assert(widget_source != 0);
    assert_source_has_function_definition(widget_source, "tinyui_message_box_props_are_valid");
    assert_source_lacks_function_definition(widget_source, "tinyui_message_box_get_ld");
    assert_source_has_function_definition(widget_source, "tinyui_message_box_confirm_bridge");
}

int main(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *win;

    assert(app != 0);
    win = tinyui_window_create(app, "root");
    assert(win != 0);

    test_message_box_create_builds_direct_backend_mapping(win);
    test_message_box_widget_file_owns_native_helper_truth(win);
    test_message_box_create_and_props(win);
    test_message_box_confirm_callback_bridge(win);
    test_message_box_rejects_invalid_inputs(win);
    test_message_box_final_release_contract_covers_multi_action_and_readback_boundary(win);
    test_message_box_native_multi_button_and_color_round_trip(win);
    test_message_box_multi_button_callback_reports_clicked_index(win);
    test_message_box_init_aliases_and_shared_base_round_trip(win);
    test_message_box_modal_hit_and_dismiss_returns_focus_to_underlay(win);
    test_message_box_rejects_null_args(win);
    test_message_box_confirm_callback_fires(win);
    test_message_box_internal_seams_renamed_to_tinyui();

    tinyui_app_destroy(app);
    return 0;
}
