#include "picoui/app.h"
#include "picoui/scroll_selecter.h"
#include "picoui/window.h"
#include "../../../src/gui/ldScrollSelecter.h"
#include "backend.h"
#include "internal.h"

#include <assert.h>

static void test_scroll_selecter_selected_item_matches_backend_truth(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_window *win;
    struct picoui_scroll_selecter *scroll_selecter;
    struct picoui_backend_widget *backend;
    ldScrollSelecter_t *ld_scroll_selecter;

    assert(app != 0);
    win = picoui_window_create(app, "root");
    assert(win != 0);
    scroll_selecter = picoui_scroll_selecter_create(win, "scroll");
    assert(scroll_selecter != 0);
    assert(picoui_scroll_selecter_add_item(scroll_selecter, "wifi", "Wi-Fi") == 0);
    assert(picoui_scroll_selecter_add_item(scroll_selecter, "bluetooth", "Bluetooth") == 0);
    assert(picoui_scroll_selecter_add_item(scroll_selecter, "display", "Display") == 0);
    assert(picoui_scroll_selecter_set_selected_index(scroll_selecter, 0) == 0);

    backend = (struct picoui_backend_widget *)scroll_selecter->widget.backend_widget;
    assert(backend != 0);
    ld_scroll_selecter = (ldScrollSelecter_t *)backend->ld_widget;
    assert(ld_scroll_selecter != 0);

    ldScrollSelecterSetSelectItemNum(ld_scroll_selecter, 2);
    assert(picoui_scroll_selecter_get_selected_index(scroll_selecter) == 2);
    picoui_app_destroy(app);
}

static void test_scroll_selecter_edit_mode_and_navigation_mode_are_distinct(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_window *win;
    struct picoui_scroll_selecter *scroll_selecter;
    struct picoui_backend_widget *backend;
    ldScrollSelecter_t *ld_scroll_selecter;
    int is_edit = -1;

    assert(app != 0);
    win = picoui_window_create(app, "root");
    assert(win != 0);
    scroll_selecter = picoui_scroll_selecter_create(win, "scroll_mode");
    assert(scroll_selecter != 0);

    backend = (struct picoui_backend_widget *)scroll_selecter->widget.backend_widget;
    assert(backend != 0);
    ld_scroll_selecter = (ldScrollSelecter_t *)backend->ld_widget;
    assert(ld_scroll_selecter != 0);

    assert(picoui_scroll_selecter_get_edit_mode(scroll_selecter, &is_edit) == 0);
    assert(is_edit == 1);
    assert(picoui_scroll_selecter_set_edit_mode(scroll_selecter, 0) == 0);
    assert(picoui_scroll_selecter_get_edit_mode(scroll_selecter, &is_edit) == 0);
    assert(is_edit == 0);
    assert(ld_scroll_selecter->isEdit == false);

    picoui_app_destroy(app);
}

int main(void)
{
    test_scroll_selecter_selected_item_matches_backend_truth();
    test_scroll_selecter_edit_mode_and_navigation_mode_are_distinct();
    return 0;
}
