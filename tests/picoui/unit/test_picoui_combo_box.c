#include "picoui/app.h"
#include "picoui/combo_box.h"
#include "picoui/window.h"
#include "../../../src/gui/ldComboBox.h"
#include "../../../src/misc/ldMsg.h"
#include "backend.h"
#include "internal.h"

#include <assert.h>

static int combo_selected_count = 0;
static int combo_selected_index = -1;
static void *combo_selected_user_data = 0;
static int native_combo_clicked_count = 0;
static int native_combo_clicked_index = -1;

static uint64_t make_signal_value_xy(uint16_t x, uint16_t y)
{
    return ((uint64_t)x << 16) | (uint64_t)y;
}

static void on_combo_selected(struct picoui_combo_box *combo_box, int index, void *user_data)
{
    combo_selected_count++;
    combo_selected_index = index;
    combo_selected_user_data = user_data;
    assert(combo_box != 0);
}

static bool on_native_combo_clicked_probe(ld_scene_t *scene, ldMsg_t msg)
{
    (void)scene;
    native_combo_clicked_count++;
    native_combo_clicked_index = (int)msg.value;
    return false;
}

static void test_combo_box_open_close_and_selected_item_truth(void)
{
    struct picoui_app *app;
    struct picoui_window *win;
    struct picoui_combo_box *combo_box;
    struct picoui_backend_widget *backend;
    struct picoui_backend_app_state *app_state;
    ldComboBox_t *ld_combo_box;
    int is_open = -1;

    app = picoui_app_create();
    assert(app != 0);
    win = picoui_window_create(app, "root");
    assert(win != 0);
    combo_box = picoui_combo_box_create(win, "combo");
    assert(combo_box != 0);
    assert(picoui_combo_box_add_item(combo_box, "wifi", "Wi-Fi") == 0);
    assert(picoui_combo_box_add_item(combo_box, "bluetooth", "Bluetooth") == 0);
    assert(picoui_combo_box_add_item(combo_box, "display", "Display") == 0);
    assert(picoui_combo_box_set_selected_index(combo_box, 0) == 0);

    backend = (struct picoui_backend_widget *)combo_box->widget.backend_widget;
    assert(backend != 0);
    app_state = (struct picoui_backend_app_state *)backend->owner->backend_app;
    assert(app_state != 0);
    assert(app_state->ld_scene != 0);
    assert(ldMsgInit(&app_state->ld_scene->ptMsgQueue, 8) == true);
    ld_combo_box = (ldComboBox_t *)backend->ld_widget;
    assert(ld_combo_box != 0);
    assert(ldMsgConnect(backend->ld_widget, SIGNAL_CLICKED_ITEM, on_native_combo_clicked_probe) == true);
    native_combo_clicked_count = 0;
    native_combo_clicked_index = -1;

    assert(picoui_combo_box_is_open(combo_box, &is_open) == 0);
    assert(is_open == 0);

    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue, backend->ld_widget, SIGNAL_PRESS, 0) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(picoui_combo_box_is_open(combo_box, &is_open) == 0);
    assert(is_open == 1);

    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue,
                     backend->ld_widget,
                     SIGNAL_HOLD_DOWN,
                     make_signal_value_xy(10, 112)) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue,
                     backend->ld_widget,
                     SIGNAL_RELEASE,
                     make_signal_value_xy(10, 112)) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(native_combo_clicked_count == 1);
    assert(native_combo_clicked_index == 2);
    assert(picoui_combo_box_get_selected_index(combo_box) == 2);
    assert(((struct picoui_backend_widget *)combo_box->widget.backend_widget)->value == 2);

    ld_combo_box->isExpand = false;
    assert(picoui_combo_box_is_open(combo_box, &is_open) == 0);
    assert(is_open == 0);
    picoui_app_destroy(app);
}

static void test_combo_box_reuses_selection_contract(void)
{
    struct picoui_app *app;
    struct picoui_window *win;
    struct picoui_combo_box *combo_box;
    struct picoui_backend_widget *backend;
    struct picoui_backend_app_state *app_state;
    ldComboBox_t *ld_combo_box;
    int cookie = 11;

    app = picoui_app_create();
    assert(app != 0);
    win = picoui_window_create(app, "root");
    assert(win != 0);
    combo_box = picoui_combo_box_create(win, "combo_contract");
    assert(combo_box != 0);
    assert(picoui_combo_box_add_item(combo_box, "wifi", "Wi-Fi") == 0);
    assert(picoui_combo_box_add_item(combo_box, "bluetooth", "Bluetooth") == 0);
    assert(picoui_combo_box_add_item(combo_box, "display", "Display") == 0);
    picoui_combo_box_set_on_selected(combo_box, on_combo_selected, &cookie);
    combo_selected_count = 0;
    combo_selected_index = -1;
    combo_selected_user_data = 0;

    backend = (struct picoui_backend_widget *)combo_box->widget.backend_widget;
    assert(backend != 0);
    app_state = (struct picoui_backend_app_state *)backend->owner->backend_app;
    assert(app_state != 0);
    assert(app_state->ld_scene != 0);
    assert(ldMsgInit(&app_state->ld_scene->ptMsgQueue, 8) == true);
    ld_combo_box = (ldComboBox_t *)backend->ld_widget;
    assert(ld_combo_box != 0);

    assert(picoui_combo_box_set_selected_index(combo_box, 0) == 0);
    assert(picoui_widget_set_enabled(&combo_box->widget, 0) == 0);
    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue,
                     backend->ld_widget,
                     SIGNAL_PRESS,
                     make_signal_value_xy(10, 10)) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue,
                     backend->ld_widget,
                     SIGNAL_RELEASE,
                     make_signal_value_xy(10, 48)) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(combo_selected_count == 0);
    assert(combo_selected_index == -1);
    assert(combo_selected_user_data == 0);
    assert(picoui_combo_box_get_selected_index(combo_box) == 0);
    assert(ldComboBoxGetSelectItem(ld_combo_box) == 0);
    assert(backend->value == 0);
    assert(backend->last_signal == PICOUI_BACKEND_SIGNAL_NONE);
    assert(backend->dispatch_count == 0);
    picoui_app_destroy(app);
}

static void test_combo_box_final_visual_and_selection_contract_is_release_ready(void)
{
    struct picoui_app *app;
    struct picoui_window *win;
    struct picoui_combo_box *combo_box;
    struct picoui_backend_widget *backend;
    ldComboBox_t *ld_combo_box;
    int is_open = -1;

    app = picoui_app_create();
    assert(app != 0);
    win = picoui_window_create(app, "combo_release_root");
    assert(win != 0);
    combo_box = picoui_combo_box_create(win, "combo_release_ready");
    assert(combo_box != 0);
    assert(picoui_combo_box_add_item(combo_box, "wifi", "Wi-Fi") == 0);
    assert(picoui_combo_box_add_item(combo_box, "bluetooth", "Bluetooth") == 0);
    assert(picoui_combo_box_add_item(combo_box, "display", "Display") == 0);
    assert(picoui_combo_box_set_selected_index(combo_box, 1) == 0);

    backend = (struct picoui_backend_widget *)combo_box->widget.backend_widget;
    assert(backend != 0);
    assert(backend->kind == PICOUI_BACKEND_WIDGET_COMBO_BOX);
    ld_combo_box = (ldComboBox_t *)backend->ld_widget;
    assert(ld_combo_box != 0);
    assert(picoui_combo_box_get_selected_index(combo_box) == 1);
    assert(backend->value == 1);
    assert(ldComboBoxGetSelectItem(ld_combo_box) == 1);
    assert(picoui_combo_box_is_open(combo_box, &is_open) == 0);
    assert(is_open == 0);

    picoui_app_destroy(app);
}

int main(void)
{
    test_combo_box_open_close_and_selected_item_truth();
    test_combo_box_reuses_selection_contract();
    test_combo_box_final_visual_and_selection_contract_is_release_ready();
    return 0;
}
