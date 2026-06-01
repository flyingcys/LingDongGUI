#include "picoui/app.h"
#include "picoui/message_box.h"
#include "picoui/widget.h"
#include "picoui/window.h"
#include "../../../src/gui/ldMessageBox.h"
#include "internal.h"

#include <assert.h>

static int confirm_count = 0;
static struct picoui_message_box *confirm_box = 0;
static void *confirm_user_data = 0;

static void on_confirm(struct picoui_message_box *box, void *user_data)
{
    confirm_count++;
    confirm_box = box;
    confirm_user_data = user_data;
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

    picoui_app_destroy(app);
    return 0;
}
