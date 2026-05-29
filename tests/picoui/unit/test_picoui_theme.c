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
                                unsigned int border,
                                int radius,
                                int padding)
{
    assert(widget != NULL);
    assert(widget->bg_color == bg);
    assert(widget->text_color == text);
    assert(widget->border_color == border);
    assert(widget->radius == radius);
    assert(widget->padding == padding);
}

static int backend_width(const struct picoui_widget *widget)
{
    const struct picoui_backend_widget *backend = (const struct picoui_backend_widget *)widget->backend_widget;
    const ldBase_t *ld_base;

    assert(backend != NULL);
    assert(backend->ld_widget != NULL);
    ld_base = (const ldBase_t *)backend->ld_widget;
    return ld_base->use_as__arm_2d_control_node_t.tRegion.tSize.iWidth;
}

static int backend_height(const struct picoui_widget *widget)
{
    const struct picoui_backend_widget *backend = (const struct picoui_backend_widget *)widget->backend_widget;
    const ldBase_t *ld_base;

    assert(backend != NULL);
    assert(backend->ld_widget != NULL);
    ld_base = (const ldBase_t *)backend->ld_widget;
    return ld_base->use_as__arm_2d_control_node_t.tRegion.tSize.iHeight;
}

static void assert_backend_height(const struct picoui_widget *widget, int height)
{
    assert(widget != NULL);
    assert(widget->height == height);
    assert(backend_height(widget) == height);
}

static void assert_backend_size(const struct picoui_widget *widget, int width, int height)
{
    assert(widget != NULL);
    assert(widget->height == height);
    assert(backend_width(widget) == width);
    assert(backend_height(widget) == height);
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

static void assert_window_backend_padding(const struct picoui_window *window, int padding)
{
    const struct picoui_backend_widget *backend = (const struct picoui_backend_widget *)window->widget.backend_widget;
    const ldWindow_t *ld_window;

    assert(backend != NULL);
    assert(backend->ld_widget != NULL);
    ld_window = (const ldWindow_t *)backend->ld_widget;
    assert(backend->window_layout.padding == padding);
    assert(ld_window->flexPadding.left == padding);
    assert(ld_window->flexPadding.top == padding);
    assert(ld_window->flexPadding.right == padding);
    assert(ld_window->flexPadding.bottom == padding);
    assert(ld_window->gridPadding.left == padding);
    assert(ld_window->gridPadding.top == padding);
    assert(ld_window->gridPadding.right == padding);
    assert(ld_window->gridPadding.bottom == padding);
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
    struct picoui_label *failed_label;
    struct picoui_backend_widget *failed_backend;
    int failed_width;
    int failed_height;
    unsigned int failed_backend_bg;
    unsigned int failed_backend_text;

    assert(theme != NULL);
    assert(picoui_theme_set_color(theme, PICOUI_COLOR_TEXT_PRIMARY, 0x111111U) == 0);
    assert(picoui_theme_set_color(theme, PICOUI_COLOR_BG, 0x222222U) == 0);
    assert(picoui_theme_set_color(theme, PICOUI_COLOR_PANEL, 0x333333U) == 0);
    assert(picoui_theme_set_color(theme, PICOUI_COLOR_BORDER, 0x444444U) == 0);
    assert(picoui_theme_set_color(theme, PICOUI_COLOR_ACCENT, 0x555555U) == 0);
    assert(picoui_theme_set_color(theme, PICOUI_COLOR_DISABLED, 0x666666U) == 0);
    assert(picoui_theme_set_metric(theme, PICOUI_METRIC_PADDING, 5) == 0);
    assert(picoui_theme_set_metric(theme, PICOUI_METRIC_RADIUS, 3) == 0);
    assert(picoui_theme_set_metric(theme, PICOUI_METRIC_BORDER_WIDTH, 2) == 0);
    assert(picoui_theme_set_metric(theme, PICOUI_METRIC_BORDER_WIDTH, -1) == -1);
    assert(picoui_theme_set_metric(theme, PICOUI_METRIC_CONTROL_HEIGHT, 19) == 0);

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
    failed_label = picoui_label_create(win, "failed_label");
    assert(win != NULL);
    assert(label != NULL);
    assert(button != NULL);
    assert(checkbox != NULL);
    assert(sw != NULL);
    assert(slider != NULL);
    assert(text != NULL);
    assert(image != NULL);
    assert(failed_label != NULL);

    assert(picoui_theme_apply_to_widget(theme,
                                        &label->widget,
                                        PICOUI_PART_MAIN,
                                        PICOUI_STATE_DISABLED)
           == 0);
    assert_widget_style(&label->widget, 0x222222U, 0x666666U, 0x666666U, 3, 5);
    assert_backend_height(&label->widget, 19);
    assert_backend_size(&label->widget, 220, 19);
    assert_label_backend_style(label, 0x222222U, 0x666666U);

    failed_backend = (struct picoui_backend_widget *)failed_label->widget.backend_widget;
    assert(failed_backend != NULL);
    assert(failed_backend->theme == theme);
    failed_width = backend_width(&failed_label->widget);
    failed_height = backend_height(&failed_label->widget);
    failed_backend_bg = ((ldLabel_t *)failed_backend->ld_widget)->bgColor;
    failed_backend_text = ((ldLabel_t *)failed_backend->ld_widget)->textColor;
    failed_backend->theme = NULL;
    assert(picoui_theme_apply_to_widget(theme,
                                        &failed_label->widget,
                                        PICOUI_PART_MAIN,
                                        PICOUI_STATE_DISABLED)
           == -1);
    failed_backend->theme = theme;
    assert_widget_style(&failed_label->widget, 0U, 0U, 0U, 0, 0);
    assert(backend_width(&failed_label->widget) == failed_width);
    assert(backend_height(&failed_label->widget) == failed_height);
    assert(((ldLabel_t *)failed_backend->ld_widget)->bgColor == failed_backend_bg);
    assert(((ldLabel_t *)failed_backend->ld_widget)->textColor == failed_backend_text);

    assert(picoui_theme_apply_to_widget(theme,
                                        &text->widget,
                                        PICOUI_PART_TEXT,
                                        PICOUI_STATE_FOCUSED)
           == 0);
    assert_widget_style(&text->widget, 0x333333U, 0x555555U, 0x444444U, 3, 5);
    assert_backend_height(&text->widget, 19);
    assert_text_backend_style(text, 0x333333U, 0x555555U);

    assert(picoui_theme_apply_to_widget(theme,
                                        &button->widget,
                                        PICOUI_PART_MAIN,
                                        PICOUI_STATE_PRESSED)
           == 0);
    assert_widget_style(&button->widget, 0x555555U, 0x111111U, 0x555555U, 3, 5);
    assert_backend_height(&button->widget, 19);
    assert_button_backend_style(button, 0x555555U, 0x555555U, 0x111111U);

    assert(picoui_theme_apply_to_widget(theme,
                                        &checkbox->widget,
                                        PICOUI_PART_INDICATOR,
                                        PICOUI_STATE_CHECKED)
           == 0);
    assert_widget_style(&checkbox->widget, 0x555555U, 0x111111U, 0x555555U, 3, 5);
    assert_backend_height(&checkbox->widget, 19);
    assert_checkbox_backend_style(checkbox, 0x333333U, 0x555555U, 0x111111U);

    assert(picoui_theme_apply_to_widget(theme,
                                        &sw->widget,
                                        PICOUI_PART_TRACK,
                                        PICOUI_STATE_DISABLED)
           == 0);
    assert_widget_style(&sw->widget, 0x222222U, 0x666666U, 0x666666U, 3, 5);
    assert_backend_height(&sw->widget, 19);
    assert_switch_backend_style(sw, 0x222222U, 0x555555U, 0x222222U, 0x666666U);

    assert(picoui_theme_apply_to_widget(theme,
                                        &slider->widget,
                                        PICOUI_PART_KNOB,
                                        PICOUI_STATE_FOCUSED)
           == 0);
    assert_widget_style(&slider->widget, 0x555555U, 0x111111U, 0x444444U, 3, 5);
    assert_backend_height(&slider->widget, 19);
    assert_slider_backend_style(slider, 0x333333U, 0x444444U, 0x555555U);

    assert(picoui_theme_apply_to_widget(theme,
                                        &win->widget,
                                        PICOUI_PART_MAIN,
                                        PICOUI_STATE_DISABLED)
           == 0);
    assert_widget_style(&win->widget, 0x222222U, 0x666666U, 0x666666U, 3, 5);
    assert_window_backend_style(win, 0x222222U);
    assert_window_backend_padding(win, 5);

    assert(picoui_theme_apply_to_widget(theme,
                                        &button->widget,
                                        PICOUI_PART_KNOB,
                                        PICOUI_STATE_DEFAULT)
           == -1);
    assert_widget_style(&button->widget, 0x555555U, 0x111111U, 0x555555U, 3, 5);

    assert(picoui_theme_apply_to_widget(theme,
                                        &label->widget,
                                        PICOUI_PART_INDICATOR,
                                        PICOUI_STATE_DEFAULT)
           == -1);
    assert_widget_style(&label->widget, 0x222222U, 0x666666U, 0x666666U, 3, 5);

    assert(picoui_theme_apply_to_widget(theme,
                                        &slider->widget,
                                        PICOUI_PART_TEXT,
                                        PICOUI_STATE_DEFAULT)
           == -1);
    assert_widget_style(&slider->widget, 0x555555U, 0x111111U, 0x444444U, 3, 5);

    assert(picoui_theme_apply_to_widget(theme,
                                        &slider->widget,
                                        PICOUI_PART_TRACK,
                                        (enum picoui_state)99)
           == -1);
    assert_widget_style(&slider->widget, 0x555555U, 0x111111U, 0x444444U, 3, 5);

    assert(picoui_theme_apply_to_widget(theme,
                                        &image->widget,
                                        PICOUI_PART_MAIN,
                                        PICOUI_STATE_DEFAULT)
           == -1);
    assert_widget_style(&image->widget, 0U, 0U, 0U, 0, 0);
    assert(image->widget.height == 0);

    assert(label->widget.visible == 1);
    assert(ldBaseIsHidden((ldBase_t *)((struct picoui_backend_widget *)label->widget.backend_widget)->ld_widget) == false);
    assert(picoui_widget_set_visible(&label->widget, 0) == 0);
    assert(label->widget.visible == 0);
    assert(ldBaseIsHidden((ldBase_t *)((struct picoui_backend_widget *)label->widget.backend_widget)->ld_widget) == true);
    assert(picoui_widget_set_visible(&label->widget, 1) == 0);
    assert(label->widget.visible == 1);
    assert(ldBaseIsHidden((ldBase_t *)((struct picoui_backend_widget *)label->widget.backend_widget)->ld_widget) == false);

    assert(sw->widget.enabled == 1);
    assert(ldSwitchIsDisabled((ldSwitch_t *)((struct picoui_backend_widget *)sw->widget.backend_widget)->ld_widget) == false);
    assert(picoui_widget_set_enabled(&sw->widget, 0) == 0);
    assert(sw->widget.enabled == 0);
    assert(ldSwitchIsDisabled((ldSwitch_t *)((struct picoui_backend_widget *)sw->widget.backend_widget)->ld_widget) == true);
    assert(picoui_widget_set_enabled(&sw->widget, 1) == 0);
    assert(sw->widget.enabled == 1);
    assert(ldSwitchIsDisabled((ldSwitch_t *)((struct picoui_backend_widget *)sw->widget.backend_widget)->ld_widget) == false);

    picoui_app_destroy(app);
    picoui_theme_destroy(theme);
    return 0;
}
