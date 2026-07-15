/*
 * TinyUI message_box unit tests — M3 Task 5 L3/L4 harness.
 *
 * title/message/buttons/colors map to ldMessageBox; confirm uses fixed event pool.
 */

#include "tinyui.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldMessageBox.h"
#include "internal.h"
#include "widgets/message_box.h"

#include <assert.h>
#include <string.h>

static unsigned int test_rgb_to_ld_color(unsigned int rgb)
{
    return (unsigned int)__RGB((rgb >> 16) & 0xFFU, (rgb >> 8) & 0xFFU, rgb & 0xFFU);
}

static int g_confirm_count;
static tinyui_obj_t *g_confirm_target;
static void *g_confirm_user;
static int g_confirm_index;
static int g_pool_count;
static tinyui_obj_t *g_pool_target;
static void *g_pool_user;
static int g_pool_index;

static void reset_confirm(void)
{
    g_confirm_count = 0;
    g_confirm_target = 0;
    g_confirm_user = 0;
    g_confirm_index = -99;
    g_pool_count = 0;
    g_pool_target = 0;
    g_pool_user = 0;
    g_pool_index = -99;
}

static void on_confirm_event(const tinyui_event_t *event)
{
    assert(event != 0);
    assert(event->code == TINYUI_EVENT_CLICKED);
    g_pool_count += 1;
    g_pool_target = event->target;
    g_pool_user = event->user_data;
    g_pool_index = (int)event->data.value;
}

static void on_legacy_confirm(tinyui_obj_t *box, void *user_data)
{
    g_confirm_count += 1;
    g_confirm_target = box;
    g_confirm_user = user_data;
}

static void test_message_box_create_and_content(tinyui_obj_t *root)
{
    tinyui_obj_t *box = tinyui_message_box_create(root);
    ldMessageBox_t *ld_box;
    const char *buttons[] = {"Cancel", "OK"};

    assert(box != 0);
    ld_box = (ldMessageBox_t *)((struct tinyui_widget *)(void *)box)->ld_widget;
    assert(ld_box != 0);
    assert(((ldBase_t *)ld_box)->widgetType == widgetTypeMessageBox);

    assert(tinyui_message_box_set_title(box, "Title") == 0);
    assert(ld_box->pTitleStr != 0);
    assert(strcmp((const char *)ld_box->pTitleStr, "Title") == 0);
    assert(strcmp(tinyui_message_box_get_title(box), "Title") == 0);

    assert(tinyui_message_box_set_msg(box, "Hello") == 0);
    assert(ld_box->pMsgStr != 0);
    assert(strcmp((const char *)ld_box->pMsgStr, "Hello") == 0);
    assert(strcmp(tinyui_message_box_get_message(box), "Hello") == 0);

    assert(tinyui_message_box_set_btn(box, buttons, 2) == 0);
    assert(ld_box->btnCount == 2);
    assert(ld_box->ppBtnStrGroup != 0);
    assert(strcmp((const char *)ld_box->ppBtnStrGroup[0], "Cancel") == 0);
    assert(strcmp((const char *)ld_box->ppBtnStrGroup[1], "OK") == 0);

    assert(tinyui_message_box_set_string_color(box, 0x111111U, 0x222222U, 0x333333U) == 0);
    assert(ld_box->titleStrColor == (ldColor)test_rgb_to_ld_color(0x111111U));
    assert(ld_box->msgStrColor == (ldColor)test_rgb_to_ld_color(0x222222U));
    assert(ld_box->btnStrColor == (ldColor)test_rgb_to_ld_color(0x333333U));

    assert(tinyui_message_box_set_button_color(box, 0x444444U, 0x555555U) == 0);
    assert(ld_box->releaseColor == (ldColor)test_rgb_to_ld_color(0x444444U));
    assert(ld_box->pressColor == (ldColor)test_rgb_to_ld_color(0x555555U));

    assert(tinyui_message_box_set_background_color(box, 0x666666U) == 0);
    assert(ld_box->bgColor == (ldColor)test_rgb_to_ld_color(0x666666U));
}

static void test_message_box_confirm_event_pool(tinyui_obj_t *root)
{
    tinyui_obj_t *box = tinyui_message_box_create(root);
    ldMessageBox_t *ld_box;
    tinyui_event_handle_t handle = 0;
    int cookie = 42;

    assert(box != 0);
    ld_box = (ldMessageBox_t *)((struct tinyui_widget *)(void *)box)->ld_widget;
    assert(ld_box != 0);

    reset_confirm();
    /* Fixed event pool listener must receive confirm without relying on host mirror only. */
    assert(tinyui_obj_add_event_cb(box,
                                   TINYUI_EVENT_MASK(TINYUI_EVENT_CLICKED),
                                   on_confirm_event,
                                   &cookie,
                                   &handle) == TINYUI_OK);
    assert(handle != 0);

    tinyui_message_box_set_callback(box, on_legacy_confirm, &cookie);
    assert(ld_box->ptFunc != 0);

    /* Simulate LD confirm callback — pool + dedicated callback must fire. */
    ld_box->clickNum = 1;
    ld_box->ptFunc(((struct tinyui_widget *)(void *)box)->owner->ld_scene, ld_box);

    assert(g_confirm_count == 1);
    assert(g_confirm_target == box);
    assert(g_confirm_user == &cookie);

    assert(g_pool_count == 1);
    assert(g_pool_target == box);
    assert(g_pool_user == &cookie);
    assert(g_pool_index == 1);
}

static void test_message_box_pool_without_legacy_callback(tinyui_obj_t *root)
{
    tinyui_obj_t *box = tinyui_message_box_create(root);
    ldMessageBox_t *ld_box;
    tinyui_event_handle_t handle = 0;
    int cookie = 7;

    assert(box != 0);
    ld_box = (ldMessageBox_t *)((struct tinyui_widget *)(void *)box)->ld_widget;
    assert(ld_box != 0);
    /* create path must install LD bridge so pool listeners work alone */
    assert(ld_box->ptFunc != 0);

    reset_confirm();
    assert(tinyui_obj_add_event_cb(box,
                                   TINYUI_EVENT_MASK(TINYUI_EVENT_CLICKED),
                                   on_confirm_event,
                                   &cookie,
                                   &handle) == TINYUI_OK);

    ld_box->clickNum = 0;
    ld_box->ptFunc(((struct tinyui_widget *)(void *)box)->owner->ld_scene, ld_box);

    assert(g_confirm_count == 0);
    assert(g_pool_count == 1);
    assert(g_pool_target == box);
    assert(g_pool_user == &cookie);
    assert(g_pool_index == 0);
}

static void test_message_box_rejects_invalid(tinyui_obj_t *root)
{
    tinyui_obj_t *box = tinyui_message_box_create(root);
    assert(box != 0);
    assert(tinyui_message_box_create(0) == 0);
    assert(tinyui_message_box_set_title(0, "x") == -1);
    assert(tinyui_message_box_set_btn(box, 0, 1) == -1);
}

int main(void)
{
    tinyui_obj_t *root;

    tinyui_deinit();
    assert(tinyui_init() == TINYUI_OK);
    root = tinyui_screen_create();
    assert(root != 0);

    test_message_box_create_and_content(root);
    test_message_box_confirm_event_pool(root);
    test_message_box_pool_without_legacy_callback(root);
    test_message_box_rejects_invalid(root);

    tinyui_deinit();
    return 0;
}
