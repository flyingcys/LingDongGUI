#include "picoui/app.h"
#include "picoui/button.h"
#include "picoui/message_box.h"
#include "picoui/widget.h"
#include "picoui/window.h"
#include "../../../src/gui/ldGui.h"
#include "../../../src/gui/ldMessageBox.h"
#include "../../../src/misc/ldMsg.h"
#include "internal.h"

#include <assert.h>

void ldGuiClickedAction(ld_scene_t *ptScene, uint8_t touchSignal, arm_2d_location_t tLocation);

static int confirm_count = 0;
static struct picoui_message_box *confirm_box = 0;
static void *confirm_user_data = 0;
static int confirm_button_index = -1;
static int underlay_press_count = 0;
static struct picoui_widget *underlay_last_press_widget = 0;
static int underlay_last_press_cookie = 0;

static uint64_t make_signal_value_xy(uint16_t x, uint16_t y)
{
    return ((uint64_t)x << 16) | (uint64_t)y;
}

static uint64_t make_release_value_xy(uint16_t x, uint16_t y)
{
    return ((uint64_t)x << 16) | (uint64_t)y;
}

static void on_confirm(struct picoui_message_box *box, void *user_data)
{
    confirm_count++;
    confirm_box = box;
    confirm_user_data = user_data;
}

static void on_confirm_with_index(struct picoui_message_box *box, int index, void *user_data)
{
    confirm_count++;
    confirm_box = box;
    confirm_user_data = user_data;
    confirm_button_index = index;
}

static void on_underlay_pressed(struct picoui_widget *widget, void *user_data)
{
    underlay_press_count++;
    underlay_last_press_widget = widget;
    underlay_last_press_cookie = user_data != 0 ? *(const int *)user_data : -1;
}

static void test_message_box_create_and_props(struct picoui_window *win)
{
    int user_cookie = 11;
    struct picoui_message_box_props props = {
        .id = "message_box_props",
        .style_class = "dialog",
        .user_data = &user_cookie,
        .title = "Update",
        .message = "Apply settings?",
        .confirm_text = "OK",
    };
    struct picoui_message_box *box =
        picoui_message_box_create((struct picoui_widget *)win, "message_box");
    struct picoui_message_box *with_props =
        picoui_message_box_create_with_props((struct picoui_widget *)win, &props);

    assert(box != 0);
    assert(with_props != 0);
    assert(picoui_message_box_get_title(box) == 0);
    assert(picoui_message_box_get_message(box) == 0);
    assert(picoui_message_box_get_confirm_text(box) == 0);
    assert(picoui_message_box_get_title(with_props) == props.title);
    assert(picoui_message_box_get_message(with_props) == props.message);
    assert(picoui_message_box_get_confirm_text(with_props) == props.confirm_text);
}

static void test_message_box_confirm_callback_bridge(struct picoui_window *win)
{
    int user_cookie = 23;
    struct picoui_message_box *box =
        picoui_message_box_create((struct picoui_widget *)win, "message_box_state");
    struct picoui_backend_widget *backend;
    ldMessageBox_t *ld_message_box;
    struct picoui_backend_app_state *app_state;

    assert(box != 0);
    assert(picoui_message_box_set_title(box, "Confirm") == 0);
    assert(picoui_message_box_set_message(box, "Save changes?") == 0);
    assert(picoui_message_box_set_confirm_text(box, "OK") == 0);
    confirm_count = 0;
    confirm_box = 0;
    confirm_user_data = 0;
    picoui_message_box_set_on_confirm(box, on_confirm, &user_cookie);

    assert(picoui_message_box_get_title(box) != 0);
    assert(picoui_message_box_get_message(box) != 0);
    assert(picoui_message_box_get_confirm_text(box) != 0);
    backend = (struct picoui_backend_widget *)box->widget.backend_widget;
    assert(backend != 0);
    app_state = (struct picoui_backend_app_state *)backend->owner->backend_app;
    assert(app_state != 0);
    ld_message_box = (ldMessageBox_t *)backend->ld_widget;
    assert(ld_message_box != 0);
    assert(ld_message_box->ptFunc != 0);

    ld_message_box->ptFunc(app_state->ld_scene, ld_message_box);
    assert(confirm_count == 1);
    assert(confirm_box == box);
    assert(confirm_user_data == &user_cookie);
}

static void test_message_box_rejects_invalid_inputs(struct picoui_window *win)
{
    struct picoui_message_box *box =
        picoui_message_box_create((struct picoui_widget *)win, "message_box_invalid");

    assert(box != 0);
    assert(picoui_message_box_create(0, "message_box") == 0);
    assert(picoui_message_box_create((struct picoui_widget *)win, 0) == 0);
    assert(picoui_message_box_create_with_props(0,
                                                &(struct picoui_message_box_props){
                                                    .id = "bad_parent",
                                                }) == 0);
    assert(picoui_message_box_create_with_props((struct picoui_widget *)win, 0) == 0);
    assert(picoui_message_box_create_with_props((struct picoui_widget *)win,
                                                &(struct picoui_message_box_props){
                                                    .title = "missing_id",
                                                }) == 0);
    assert(picoui_message_box_set_title(0, "Confirm") == -1);
    assert(picoui_message_box_set_message(0, "Message") == -1);
    assert(picoui_message_box_set_confirm_text(0, "OK") == -1);
    assert(picoui_message_box_set_title(box, 0) == -1);
    assert(picoui_message_box_set_message(box, 0) == -1);
    assert(picoui_message_box_set_confirm_text(box, 0) == -1);
}

static void test_message_box_final_release_contract_covers_multi_action_and_readback_boundary(
    struct picoui_window *win)
{
    static const uint8_t *buttons[2] = {
        (const uint8_t *)"Later",
        (const uint8_t *)"Apply",
    };
    struct picoui_message_box *box =
        picoui_message_box_create((struct picoui_widget *)win, "message_box_release_ready");
    struct picoui_backend_widget *backend;
    ldMessageBox_t *ld_message_box;

    assert(box != 0);
    assert(picoui_message_box_set_title(box, "Release") == 0);
    assert(picoui_message_box_set_message(box, "Apply current settings now?") == 0);
    assert(picoui_message_box_set_confirm_text(box, "Apply") == 0);
    assert(picoui_message_box_get_title(box) != 0);
    assert(picoui_message_box_get_message(box) != 0);
    assert(picoui_message_box_get_confirm_text(box) != 0);

    backend = (struct picoui_backend_widget *)box->widget.backend_widget;
    assert(backend != 0);
    assert(backend->kind == PICOUI_BACKEND_WIDGET_MESSAGE_BOX);
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
    assert(strcmp(picoui_message_box_get_confirm_text(box), "Apply") == 0);
}

static void test_message_box_native_multi_button_and_color_round_trip(struct picoui_window *win)
{
    static const char *buttons[] = {
        "Later",
        "Apply",
        "Reset",
    };
    struct picoui_message_box *box =
        picoui_message_box_create((struct picoui_widget *)win, "message_box_native");
    struct picoui_backend_widget *backend;
    ldMessageBox_t *ld_message_box;

    assert(box != 0);
    backend = (struct picoui_backend_widget *)box->widget.backend_widget;
    assert(backend != 0);
    ld_message_box = (ldMessageBox_t *)backend->ld_widget;
    assert(ld_message_box != 0);

    assert(picoui_message_box_set_buttons(box, buttons, 3) == 0);
    assert(picoui_message_box_set_string_colors(box, 0x112233U, 0x445566U, 0x778899U) == 0);
    assert(picoui_message_box_set_button_colors(box, 0x123456U, 0x654321U) == 0);
    assert(picoui_message_box_set_bg_color(box, 0xABCDEFU) == 0);

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

    assert(picoui_message_box_set_buttons(0, buttons, 3) == -1);
    assert(picoui_message_box_set_string_colors(0, 0, 0, 0) == -1);
    assert(picoui_message_box_set_button_colors(0, 0, 0) == -1);
    assert(picoui_message_box_set_bg_color(0, 0) == -1);
}

static void test_message_box_multi_button_callback_reports_clicked_index(struct picoui_window *win)
{
    static const char *buttons[] = {
        "Later",
        "Apply",
        "Reset",
    };
    int user_cookie = 31;
    struct picoui_message_box *box =
        picoui_message_box_create((struct picoui_widget *)win, "message_box_multi_cb");
    struct picoui_backend_widget *backend;
    ldMessageBox_t *ld_message_box;
    struct picoui_backend_app_state *app_state;

    assert(box != 0);
    assert(picoui_message_box_set_buttons(box, buttons, 3) == 0);
    picoui_message_box_set_on_confirm_indexed(box, on_confirm_with_index, &user_cookie);

    confirm_count = 0;
    confirm_box = 0;
    confirm_user_data = 0;
    confirm_button_index = -1;

    backend = (struct picoui_backend_widget *)box->widget.backend_widget;
    assert(backend != 0);
    app_state = (struct picoui_backend_app_state *)backend->owner->backend_app;
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

static void test_message_box_init_aliases_and_shared_base_round_trip(struct picoui_window *win)
{
    static const char *buttons[] = {
        "Later",
        "Apply",
    };
    int user_cookie = 41;
    struct picoui_message_box *box =
        picoui_message_box_init((struct picoui_widget *)win, "message_box_alias");
    struct picoui_backend_widget *backend;
    ldMessageBox_t *ld_message_box;

    assert(box != 0);
    backend = (struct picoui_backend_widget *)box->widget.backend_widget;
    assert(backend != 0);
    ld_message_box = (ldMessageBox_t *)backend->ld_widget;
    assert(ld_message_box != 0);

    assert(picoui_message_box_set_title(box, "Alias") == 0);
    assert(picoui_message_box_set_msg(box, "Apply current settings?") == 0);
    assert(picoui_message_box_set_btn(box, buttons, 2) == 0);
    assert(picoui_message_box_set_string_color(box, 0x112233U, 0x445566U, 0x778899U) == 0);
    assert(picoui_message_box_set_button_color(box, 0x123456U, 0x654321U) == 0);
    assert(picoui_message_box_set_background_color(box, 0xA0B0C0U) == 0);
    picoui_message_box_set_callback(box, on_confirm, &user_cookie);

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

    assert(picoui_widget_set_pos(&box->widget, 14, 18) == 0);
    assert(((ldBase_t *)ld_message_box)->use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 14);
    assert(((ldBase_t *)ld_message_box)->use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 18);
    assert(picoui_widget_set_visible(&box->widget, 0) == 0);
    assert(((ldBase_t *)ld_message_box)->isHidden == true);
    assert(picoui_widget_set_opacity(&box->widget, 62) == 0);
    assert(((ldBase_t *)ld_message_box)->opacity == 62);
    assert(picoui_widget_set_corner(&box->widget, 5) == 0);
    assert(((ldBase_t *)ld_message_box)->isCorner == true);
}

static void test_message_box_modal_hit_and_dismiss_returns_focus_to_underlay(struct picoui_window *win)
{
    struct picoui_button *underlay =
        picoui_button_create(win, "message_box_underlay_button");
    struct picoui_message_box *box =
        picoui_message_box_create((struct picoui_widget *)win, "message_box_modal");
    struct picoui_backend_widget *button_backend;
    struct picoui_backend_widget *box_backend;
    struct picoui_backend_app_state *app_state;
    ldButton_t *ld_button;
    ldMessageBox_t *ld_message_box;
    int press_cookie = 77;
    arm_2d_location_t click;

    assert(underlay != 0);
    assert(box != 0);
    assert(picoui_widget_set_pos(&underlay->widget, 110, 180) == 0);
    assert(picoui_widget_set_size(&underlay->widget, 260, 140) == 0);
    assert(picoui_widget_set_pos(&box->widget, 110, 180) == 0);
    assert(picoui_message_box_set_confirm_text(box, "OK") == 0);
    assert(picoui_button_set_on_pressed(underlay, on_underlay_pressed, &press_cookie) == 0);

    button_backend = (struct picoui_backend_widget *)underlay->widget.backend_widget;
    box_backend = (struct picoui_backend_widget *)box->widget.backend_widget;
    assert(button_backend != 0);
    assert(box_backend != 0);
    app_state = (struct picoui_backend_app_state *)box_backend->owner->backend_app;
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

int main(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_window *win;

    assert(app != 0);
    win = picoui_window_create(app, "root");
    assert(win != 0);

    test_message_box_create_and_props(win);
    test_message_box_confirm_callback_bridge(win);
    test_message_box_rejects_invalid_inputs(win);
    test_message_box_final_release_contract_covers_multi_action_and_readback_boundary(win);
    test_message_box_native_multi_button_and_color_round_trip(win);
    test_message_box_multi_button_callback_reports_clicked_index(win);
    test_message_box_init_aliases_and_shared_base_round_trip(win);
    test_message_box_modal_hit_and_dismiss_returns_focus_to_underlay(win);

    picoui_app_destroy(app);
    return 0;
}
