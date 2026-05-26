#include "internal.h"
#include "picoui/picoui.h"

#include <assert.h>
#include <stddef.h>

static void assert_widget_style(const struct picoui_widget *widget,
                                unsigned int bg,
                                unsigned int text,
                                unsigned int border)
{
    assert(widget != NULL);
    assert(widget->bg_color == bg);
    assert(widget->text_color == text);
    assert(widget->border_color == border);
}

int main(void)
{
    struct picoui_theme *theme = picoui_theme_create();
    struct picoui_app *app;
    struct picoui_window *win;
    struct picoui_button *button;
    struct picoui_checkbox *checkbox;
    struct picoui_switch *sw;
    struct picoui_slider *slider;

    assert(theme != NULL);
    assert(picoui_theme_set_color(theme, PICOUI_COLOR_TEXT_PRIMARY, 0x111111U) == 0);
    assert(picoui_theme_set_color(theme, PICOUI_COLOR_BG, 0x222222U) == 0);
    assert(picoui_theme_set_color(theme, PICOUI_COLOR_PANEL, 0x333333U) == 0);
    assert(picoui_theme_set_color(theme, PICOUI_COLOR_BORDER, 0x444444U) == 0);
    assert(picoui_theme_set_color(theme, PICOUI_COLOR_ACCENT, 0x555555U) == 0);
    assert(picoui_theme_set_color(theme, PICOUI_COLOR_DISABLED, 0x666666U) == 0);

    app = picoui_app_create();
    assert(app != NULL);
    assert(picoui_app_set_theme(app, theme) == 0);

    win = picoui_window_create(app, "root");
    button = picoui_button_create(win, "ok");
    checkbox = picoui_checkbox_create(win, "check");
    sw = picoui_switch_create(win, "switch");
    slider = picoui_slider_create(win, "slider");
    assert(win != NULL);
    assert(button != NULL);
    assert(checkbox != NULL);
    assert(sw != NULL);
    assert(slider != NULL);

    assert(picoui_theme_apply_to_widget(theme,
                                        &button->widget,
                                        PICOUI_PART_MAIN,
                                        PICOUI_STATE_PRESSED)
           == 0);
    assert_widget_style(&button->widget, 0x555555U, 0x111111U, 0x555555U);

    assert(picoui_theme_apply_to_widget(theme,
                                        &checkbox->widget,
                                        PICOUI_PART_INDICATOR,
                                        PICOUI_STATE_CHECKED)
           == 0);
    assert_widget_style(&checkbox->widget, 0x555555U, 0x111111U, 0x555555U);

    assert(picoui_theme_apply_to_widget(theme,
                                        &sw->widget,
                                        PICOUI_PART_TRACK,
                                        PICOUI_STATE_DISABLED)
           == 0);
    assert_widget_style(&sw->widget, 0x222222U, 0x666666U, 0x666666U);

    assert(picoui_theme_apply_to_widget(theme,
                                        &slider->widget,
                                        PICOUI_PART_KNOB,
                                        PICOUI_STATE_FOCUSED)
           == 0);
    assert_widget_style(&slider->widget, 0x555555U, 0x111111U, 0x444444U);

    assert(picoui_theme_apply_to_widget(theme,
                                        &button->widget,
                                        PICOUI_PART_KNOB,
                                        PICOUI_STATE_DEFAULT)
           == -1);
    assert_widget_style(&button->widget, 0x555555U, 0x111111U, 0x555555U);

    assert(picoui_theme_apply_to_widget(theme,
                                        &slider->widget,
                                        PICOUI_PART_TRACK,
                                        (enum picoui_state)99)
           == -1);
    assert_widget_style(&slider->widget, 0x555555U, 0x111111U, 0x444444U);

    picoui_app_destroy(app);
    picoui_theme_destroy(theme);
    return 0;
}
