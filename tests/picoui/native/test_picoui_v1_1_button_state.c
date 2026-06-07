#include "picoui/picoui.h"
#include "../../../picoui/src/core/internal_v1_1.h"

#include <assert.h>

static int g_clicked_count;

static void on_button_clicked(struct picoui_widget *widget, void *user_data)
{
    (void)widget;
    (void)user_data;
    ++g_clicked_count;
}

int main(void)
{
    struct picoui_screen *screen;
    struct picoui_window *root;
    struct picoui_button *button;
    int pressed;

    assert(picoui_init() == 0);

    screen = picoui_screen_active();
    assert(screen != 0);

    root = picoui_window_create_root(screen, "root");
    assert(root != 0);

    button = picoui_button_create(root, "button");
    assert(button != 0);

    assert(picoui_button_set_on_clicked(button, 0, 0) == 0);

    assert(picoui_button_set_pressed(button, 1) == 0);
    assert(picoui_button_set_pressed(button, 0) == 0);
    assert(g_clicked_count == 0);

    assert(picoui_button_set_on_clicked(button, on_button_clicked, 0) == 0);

    assert(picoui_button_set_pressed(button, 0) == 0);
    assert(g_clicked_count == 0);

    assert(picoui_button_set_pressed(button, 1) == 0);
    assert(picoui_button_get_pressed(button, &pressed) == 0);
    assert(pressed == 1);
    assert(button->widget.dirty == 1);

    button->widget.dirty = 0;
    assert(picoui_button_set_pressed(button, 1) == 0);
    assert(g_clicked_count == 0);
    assert(button->widget.dirty == 1);

    button->widget.dirty = 0;
    assert(picoui_button_set_pressed(button, 0) == 0);
    assert(picoui_button_get_pressed(button, &pressed) == 0);
    assert(pressed == 0);
    assert(g_clicked_count == 1);
    assert(button->widget.dirty == 1);

    button->widget.dirty = 0;
    assert(picoui_button_set_pressed(button, 0) == 0);
    assert(g_clicked_count == 1);
    assert(button->widget.dirty == 1);

    picoui_deinit();
    return 0;
}
