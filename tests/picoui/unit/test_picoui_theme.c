#include "internal.h"
#include "picoui/picoui.h"
#include "ldButton.h"
#include "ldCheckBox.h"
#include "ldLabel.h"
#include "ldSlider.h"
#include "ldSwitch.h"
#include "ldText.h"
#include "ldWindow.h"

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

static unsigned int picoui_test_rgb_to_ld_color(unsigned int rgb)
{
    return (unsigned int)__RGB((rgb >> 16) & 0xFFU, (rgb >> 8) & 0xFFU, rgb & 0xFFU);
}

static void assert_button_backend_style(const struct picoui_button *button,
                                        unsigned int release_color,
                                        unsigned int press_color,
                                        unsigned int text_color)
{
    const struct picoui_backend_widget *backend = (const struct picoui_backend_widget *)button->widget.backend_widget;
    const ldButton_t *ld_button;

    assert(backend != NULL);
    assert(backend->ld_widget != NULL);
    ld_button = (const ldButton_t *)backend->ld_widget;
    assert(ld_button->releaseColor == picoui_test_rgb_to_ld_color(release_color));
    assert(ld_button->pressColor == picoui_test_rgb_to_ld_color(press_color));
    assert(ld_button->textColor == picoui_test_rgb_to_ld_color(text_color));
}

static void assert_checkbox_backend_style(const struct picoui_checkbox *checkbox,
                                          unsigned int bg_color,
                                          unsigned int fg_color,
                                          unsigned int text_color)
{
    const struct picoui_backend_widget *backend = (const struct picoui_backend_widget *)checkbox->widget.backend_widget;
    const ldCheckBox_t *ld_checkbox;

    assert(backend != NULL);
    assert(backend->ld_widget != NULL);
    ld_checkbox = (const ldCheckBox_t *)backend->ld_widget;
    assert(ld_checkbox->bgColor == picoui_test_rgb_to_ld_color(bg_color));
    assert(ld_checkbox->fgColor == picoui_test_rgb_to_ld_color(fg_color));
    assert(ld_checkbox->textColor == picoui_test_rgb_to_ld_color(text_color));
}

static void assert_switch_backend_style(const struct picoui_switch *sw,
                                        unsigned int off_track,
                                        unsigned int on_track,
                                        unsigned int knob,
                                        unsigned int border)
{
    const struct picoui_backend_widget *backend = (const struct picoui_backend_widget *)sw->widget.backend_widget;
    const ldSwitch_t *ld_switch;

    assert(backend != NULL);
    assert(backend->ld_widget != NULL);
    ld_switch = (const ldSwitch_t *)backend->ld_widget;
    assert(ld_switch->offTrackColor == picoui_test_rgb_to_ld_color(off_track));
    assert(ld_switch->onTrackColor == picoui_test_rgb_to_ld_color(on_track));
    assert(ld_switch->knobColor == picoui_test_rgb_to_ld_color(knob));
    assert(ld_switch->borderColor == picoui_test_rgb_to_ld_color(border));
}

static void assert_slider_backend_style(const struct picoui_slider *slider,
                                        unsigned int bg_color,
                                        unsigned int frame_color,
                                        unsigned int indic_color)
{
    const struct picoui_backend_widget *backend = (const struct picoui_backend_widget *)slider->widget.backend_widget;
    const ldSlider_t *ld_slider;

    assert(backend != NULL);
    assert(backend->ld_widget != NULL);
    ld_slider = (const ldSlider_t *)backend->ld_widget;
    assert(ld_slider->bgColor == picoui_test_rgb_to_ld_color(bg_color));
    assert(ld_slider->frameColor == picoui_test_rgb_to_ld_color(frame_color));
    assert(ld_slider->indicColor == picoui_test_rgb_to_ld_color(indic_color));
}

static void assert_window_backend_style(const struct picoui_window *window, unsigned int bg_color)
{
    const struct picoui_backend_widget *backend = (const struct picoui_backend_widget *)window->widget.backend_widget;
    const ldWindow_t *ld_window;

    assert(backend != NULL);
    assert(backend->ld_widget != NULL);
    ld_window = (const ldWindow_t *)backend->ld_widget;
    assert(ld_window->bgColor == picoui_test_rgb_to_ld_color(bg_color));
}

static void assert_label_backend_style(const struct picoui_label *label,
                                       unsigned int bg_color,
                                       unsigned int text_color)
{
    const struct picoui_backend_widget *backend = (const struct picoui_backend_widget *)label->widget.backend_widget;
    const ldLabel_t *ld_label;

    assert(backend != NULL);
    assert(backend->ld_widget != NULL);
    ld_label = (const ldLabel_t *)backend->ld_widget;
    assert(ld_label->bgColor == picoui_test_rgb_to_ld_color(bg_color));
    assert(ld_label->textColor == picoui_test_rgb_to_ld_color(text_color));
}

static void assert_text_backend_style(const struct picoui_text *text,
                                      unsigned int bg_color,
                                      unsigned int text_color)
{
    const struct picoui_backend_widget *backend = (const struct picoui_backend_widget *)text->widget.backend_widget;
    const ldText_t *ld_text;

    assert(backend != NULL);
    assert(backend->ld_widget != NULL);
    ld_text = (const ldText_t *)backend->ld_widget;
    assert(ld_text->bgColor == picoui_test_rgb_to_ld_color(bg_color));
    assert(ld_text->textColor == picoui_test_rgb_to_ld_color(text_color));
}

int main(void)
{
    struct picoui_theme *theme = picoui_theme_create();
    struct picoui_app *app;
    struct picoui_window *win;
    struct picoui_label *label;
    struct picoui_button *button;
    struct picoui_checkbox *checkbox;
    struct picoui_switch *sw;
    struct picoui_slider *slider;
    struct picoui_text *text;
    struct picoui_image *image;

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
    label = picoui_label_create(win, "label");
    button = picoui_button_create(win, "ok");
    checkbox = picoui_checkbox_create(win, "check");
    sw = picoui_switch_create(win, "switch");
    slider = picoui_slider_create(win, "slider");
    text = picoui_text_create(win, "text");
    image = picoui_image_create(win, "image");
    assert(win != NULL);
    assert(label != NULL);
    assert(button != NULL);
    assert(checkbox != NULL);
    assert(sw != NULL);
    assert(slider != NULL);
    assert(text != NULL);
    assert(image != NULL);

    assert(picoui_theme_apply_to_widget(theme,
                                        &label->widget,
                                        PICOUI_PART_MAIN,
                                        PICOUI_STATE_DISABLED)
           == 0);
    assert_widget_style(&label->widget, 0x222222U, 0x666666U, 0x666666U);
    assert_label_backend_style(label, 0x222222U, 0x666666U);

    assert(picoui_theme_apply_to_widget(theme,
                                        &text->widget,
                                        PICOUI_PART_TEXT,
                                        PICOUI_STATE_FOCUSED)
           == 0);
    assert_widget_style(&text->widget, 0x333333U, 0x555555U, 0x444444U);
    assert_text_backend_style(text, 0x333333U, 0x555555U);

    assert(picoui_theme_apply_to_widget(theme,
                                        &button->widget,
                                        PICOUI_PART_MAIN,
                                        PICOUI_STATE_PRESSED)
           == 0);
    assert_widget_style(&button->widget, 0x555555U, 0x111111U, 0x555555U);
    assert_button_backend_style(button, 0x333333U, 0x555555U, 0x111111U);

    assert(picoui_theme_apply_to_widget(theme,
                                        &checkbox->widget,
                                        PICOUI_PART_INDICATOR,
                                        PICOUI_STATE_CHECKED)
           == 0);
    assert_widget_style(&checkbox->widget, 0x555555U, 0x111111U, 0x555555U);
    assert_checkbox_backend_style(checkbox, 0x333333U, 0x555555U, 0x111111U);

    assert(picoui_theme_apply_to_widget(theme,
                                        &sw->widget,
                                        PICOUI_PART_TRACK,
                                        PICOUI_STATE_DISABLED)
           == 0);
    assert_widget_style(&sw->widget, 0x222222U, 0x666666U, 0x666666U);
    assert_switch_backend_style(sw, 0x222222U, 0x555555U, 0x333333U, 0x666666U);

    assert(picoui_theme_apply_to_widget(theme,
                                        &slider->widget,
                                        PICOUI_PART_KNOB,
                                        PICOUI_STATE_FOCUSED)
           == 0);
    assert_widget_style(&slider->widget, 0x555555U, 0x111111U, 0x444444U);
    assert_slider_backend_style(slider, 0x333333U, 0x444444U, 0x555555U);

    assert(picoui_theme_apply_to_widget(theme,
                                        &win->widget,
                                        PICOUI_PART_MAIN,
                                        PICOUI_STATE_DISABLED)
           == 0);
    assert_widget_style(&win->widget, 0x222222U, 0x666666U, 0x666666U);
    assert_window_backend_style(win, 0x222222U);

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

    assert(picoui_theme_apply_to_widget(theme,
                                        &image->widget,
                                        PICOUI_PART_MAIN,
                                        PICOUI_STATE_DEFAULT)
           == -1);
    assert_widget_style(&image->widget, 0U, 0U, 0U);

    picoui_app_destroy(app);
    picoui_theme_destroy(theme);
    return 0;
}
