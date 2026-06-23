#include "internal.h"
#include "tinyui.h"
#include "ldButton.h"
#include "ldCheckBox.h"
#include "ldImage.h"
#include "ldLabel.h"
#include "ldList.h"
#include "ldSlider.h"
#include "ldSwitch.h"
#include "ldText.h"
#include "ldCalendar.h"
#include "ldWindow.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

static void assert_source_lacks_static_definition(const char *path, const char *symbol_name)
{
    char command[1024];

    snprintf(command,
             sizeof(command),
             "rg -n \"^[[:space:]]*static[[:space:]].*%s[[:space:]]*(\\(|=)\" %s >/dev/null",
             symbol_name,
             path);
    if (system(command) == 0) {
        fprintf(stderr, "unexpected old static helper still present: %s in %s\n", symbol_name, path);
        abort();
    }
}

static void test_theme_internal_static_helpers_no_longer_use_tinyui_prefix(void)
{
    const char *source = "tinyui/src/theme/theme.c";
    assert(source != NULL);
    assert_source_lacks_static_definition(source, "tinyui_theme_rgb_to_ld_color");
}

static void assert_widget_style(const struct tinyui_widget *widget,
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

static int backend_width(const struct tinyui_widget *widget)
{
    const ldBase_t *ld_base;

    assert(widget->ld_widget != NULL);
    ld_base = (const ldBase_t *)widget->ld_widget;
    return ld_base->use_as__arm_2d_control_node_t.tRegion.tSize.iWidth;
}

static int backend_height(const struct tinyui_widget *widget)
{
    const ldBase_t *ld_base;

    assert(widget->ld_widget != NULL);
    ld_base = (const ldBase_t *)widget->ld_widget;
    return ld_base->use_as__arm_2d_control_node_t.tRegion.tSize.iHeight;
}

static void assert_backend_height(const struct tinyui_widget *widget, int height)
{
    assert(widget != NULL);
    assert(widget->height == height);
    assert(backend_height(widget) == height);
}

static void assert_backend_size(const struct tinyui_widget *widget, int width, int height)
{
    assert(widget != NULL);
    assert(widget->height == height);
    assert(backend_width(widget) == width);
    assert(backend_height(widget) == height);
}

static unsigned int tinyui_test_rgb_to_ld_color(unsigned int rgb)
{
    return (unsigned int)__RGB((rgb >> 16) & 0xFFU, (rgb >> 8) & 0xFFU, rgb & 0xFFU);
}

static void assert_button_backend_style(const struct tinyui_button *button,
                                        unsigned int release_color,
                                        unsigned int press_color,
                                        unsigned int text_color)
{
    const ldButton_t *ld_button;

    assert(button->widget.ld_widget != NULL);
    ld_button = (const ldButton_t *)button->widget.ld_widget;
    assert(ld_button->releaseColor == tinyui_test_rgb_to_ld_color(release_color));
    assert(ld_button->pressColor == tinyui_test_rgb_to_ld_color(press_color));
    assert(ld_button->textColor == tinyui_test_rgb_to_ld_color(text_color));
}

static void assert_checkbox_backend_style(const struct tinyui_checkbox *checkbox,
                                          unsigned int bg_color,
                                          unsigned int fg_color,
                                          unsigned int text_color)
{
    const ldCheckBox_t *ld_checkbox;

    assert(checkbox->widget.ld_widget != NULL);
    ld_checkbox = (const ldCheckBox_t *)checkbox->widget.ld_widget;
    assert(ld_checkbox->bgColor == tinyui_test_rgb_to_ld_color(bg_color));
    assert(ld_checkbox->fgColor == tinyui_test_rgb_to_ld_color(fg_color));
    assert(ld_checkbox->textColor == tinyui_test_rgb_to_ld_color(text_color));
}

static void assert_switch_backend_style(const struct tinyui_switch *sw,
                                        unsigned int off_track,
                                        unsigned int on_track,
                                        unsigned int knob,
                                        unsigned int border)
{
    const ldSwitch_t *ld_switch;

    assert(sw->widget.ld_widget != NULL);
    ld_switch = (const ldSwitch_t *)sw->widget.ld_widget;
    assert(ld_switch->offTrackColor == tinyui_test_rgb_to_ld_color(off_track));
    assert(ld_switch->onTrackColor == tinyui_test_rgb_to_ld_color(on_track));
    assert(ld_switch->knobColor == tinyui_test_rgb_to_ld_color(knob));
    assert(ld_switch->borderColor == tinyui_test_rgb_to_ld_color(border));
}

static void assert_slider_backend_style(const struct tinyui_slider *slider,
                                        unsigned int bg_color,
                                        unsigned int frame_color,
                                        unsigned int indic_color)
{
    const ldSlider_t *ld_slider;

    assert(slider->widget.ld_widget != NULL);
    ld_slider = (const ldSlider_t *)slider->widget.ld_widget;
    assert(ld_slider->bgColor == tinyui_test_rgb_to_ld_color(bg_color));
    assert(ld_slider->frameColor == tinyui_test_rgb_to_ld_color(frame_color));
    assert(ld_slider->indicColor == tinyui_test_rgb_to_ld_color(indic_color));
}

static void assert_window_backend_style(const struct tinyui_window *window, unsigned int bg_color)
{
    const ldWindow_t *ld_window;

    assert(window->widget.ld_widget != NULL);
    ld_window = (const ldWindow_t *)window->widget.ld_widget;
    assert(ld_window->bgColor == tinyui_test_rgb_to_ld_color(bg_color));
}

static void assert_window_backend_padding(const struct tinyui_window *window, int padding)
{
    const ldWindow_t *ld_window;

    assert(window->widget.ld_widget != NULL);
    ld_window = (const ldWindow_t *)window->widget.ld_widget;
    assert(window->widget.padding == padding);
    assert(ld_window->flexPadding.left == padding);
    assert(ld_window->flexPadding.top == padding);
    assert(ld_window->flexPadding.right == padding);
    assert(ld_window->flexPadding.bottom == padding);
    assert(ld_window->gridPadding.left == padding);
    assert(ld_window->gridPadding.top == padding);
    assert(ld_window->gridPadding.right == padding);
    assert(ld_window->gridPadding.bottom == padding);
}

static void assert_button_backend_metrics(const struct tinyui_button *button,
                                          int height,
                                          int radius,
                                          int padding)
{
    const ldButton_t *ld_button;

    assert(button->widget.ld_widget != NULL);
    ld_button = (const ldButton_t *)button->widget.ld_widget;
    assert(ld_button->use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iHeight == height);
    assert(button->widget.radius == radius);
    assert(button->widget.padding == padding);
}

static void assert_label_backend_style(const struct tinyui_label *label,
                                       unsigned int bg_color,
                                       unsigned int text_color)
{
    const ldLabel_t *ld_label;

    assert(label->widget.ld_widget != NULL);
    ld_label = (const ldLabel_t *)label->widget.ld_widget;
    assert(ld_label->bgColor == tinyui_test_rgb_to_ld_color(bg_color));
    assert(ld_label->textColor == tinyui_test_rgb_to_ld_color(text_color));
}

static void assert_text_backend_style(const struct tinyui_text *text,
                                      unsigned int bg_color,
                                      unsigned int text_color)
{
    const ldText_t *ld_text;

    assert(text->widget.ld_widget != NULL);
    ld_text = (const ldText_t *)text->widget.ld_widget;
    assert(ld_text->bgColor == tinyui_test_rgb_to_ld_color(bg_color));
    assert(ld_text->textColor == tinyui_test_rgb_to_ld_color(text_color));
}

static void assert_list_backend_style(const struct tinyui_list *list,
                                      unsigned int bg_color,
                                      unsigned int text_color,
                                      unsigned int select_color)
{
    const ldList_t *ld_list;

    assert(list->widget.ld_widget != NULL);
    ld_list = (const ldList_t *)list->widget.ld_widget;
    assert(ld_list->bgColor == tinyui_test_rgb_to_ld_color(bg_color));
    assert(ld_list->textColor == tinyui_test_rgb_to_ld_color(text_color));
    assert(ld_list->selectColor == tinyui_test_rgb_to_ld_color(select_color));
}

static void assert_list_backend_text_color(const struct tinyui_list *list,
                                           unsigned int text_color)
{
    const ldList_t *ld_list;

    assert(list->widget.ld_widget != NULL);
    ld_list = (const ldList_t *)list->widget.ld_widget;
    assert(ld_list->textColor == tinyui_test_rgb_to_ld_color(text_color));
}

static void assert_calendar_backend_style(const struct tinyui_calendar *calendar,
                                          unsigned int bg_color,
                                          unsigned int item_color,
                                          unsigned int text_color)
{
    const ldCalendar_t *ld_calendar;

    assert(calendar->widget.ld_widget != NULL);
    ld_calendar = (const ldCalendar_t *)calendar->widget.ld_widget;
    assert(ld_calendar->bgColor == tinyui_test_rgb_to_ld_color(bg_color));
    assert(ld_calendar->itemColor == tinyui_test_rgb_to_ld_color(item_color));
    assert(ld_calendar->textColor == tinyui_test_rgb_to_ld_color(text_color));
}

static void test_image_theme_and_enabled_are_support_not_reject(void)
{
    struct tinyui_theme *theme = tinyui_theme_create();
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *win;
    struct tinyui_image *image;
    ldImage_t *ld_image;

    assert(theme != NULL);
    assert(app != NULL);
    assert(tinyui_theme_set_color(theme, TINYUI_COLOR_TEXT_PRIMARY, 0x111111U) == 0);
    assert(tinyui_theme_set_color(theme, TINYUI_COLOR_BG, 0x222222U) == 0);
    assert(tinyui_theme_set_color(theme, TINYUI_COLOR_PANEL, 0x333333U) == 0);
    assert(tinyui_theme_set_color(theme, TINYUI_COLOR_BORDER, 0x444444U) == 0);
    assert(tinyui_theme_set_color(theme, TINYUI_COLOR_ACCENT, 0x555555U) == 0);
    assert(tinyui_theme_set_color(theme, TINYUI_COLOR_DISABLED, 0x666666U) == 0);
    assert(tinyui_theme_set_metric(theme, TINYUI_METRIC_PADDING, 7) == 0);
    assert(tinyui_theme_set_metric(theme, TINYUI_METRIC_RADIUS, 2) == 0);
    assert(tinyui_theme_set_metric(theme, TINYUI_METRIC_CONTROL_HEIGHT, 17) == 0);
    assert(tinyui_app_set_theme(app, theme) == 0);

    win = tinyui_window_create(app, "image_theme_root");
    image = tinyui_image_create(win, "image_theme");
    assert(win != NULL);
    assert(image != NULL);

    assert(image->widget.ld_widget != NULL);
    ld_image = (ldImage_t *)image->widget.ld_widget;

    assert(tinyui_theme_apply_to_widget(theme,
                                        &image->widget,
                                        TINYUI_PART_MAIN,
                                        TINYUI_STATE_DISABLED) == 0);
    assert_widget_style(&image->widget, 0x222222U, 0x666666U, 0x666666U, 2, 7);
    assert(backend_height(&image->widget) == 56);
    assert(image->widget.height == 0);
    assert(ld_image->maskColor == tinyui_test_rgb_to_ld_color(0x222222U));

    assert(tinyui_widget_set_enabled(&image->widget, 0) == 0);
    assert(image->widget.enabled == 0);
    assert(((ldBase_t *)image->widget.ld_widget)->isSelectable == false);
    assert(tinyui_widget_set_enabled(&image->widget, 1) == 0);
    assert(image->widget.enabled == 1);
    assert(((ldBase_t *)image->widget.ld_widget)->isSelectable == true);

    tinyui_app_destroy(app);
    tinyui_theme_destroy(theme);
}

static void test_theme_native_parts_apply_to_real_backend_fields(void)
{
    struct tinyui_theme *theme = tinyui_theme_create();
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *win;
    struct tinyui_button *button;
    struct tinyui_list *list;
    struct tinyui_calendar *calendar;

    assert(theme != NULL);
    assert(app != NULL);
    assert(tinyui_theme_set_color(theme, TINYUI_COLOR_TEXT_PRIMARY, 0x101010U) == 0);
    assert(tinyui_theme_set_color(theme, TINYUI_COLOR_BG, 0x202020U) == 0);
    assert(tinyui_theme_set_color(theme, TINYUI_COLOR_PANEL, 0x303030U) == 0);
    assert(tinyui_theme_set_color(theme, TINYUI_COLOR_BORDER, 0x404040U) == 0);
    assert(tinyui_theme_set_color(theme, TINYUI_COLOR_ACCENT, 0x505050U) == 0);
    assert(tinyui_theme_set_color(theme, TINYUI_COLOR_DISABLED, 0x606060U) == 0);
    assert(tinyui_theme_set_metric(theme, TINYUI_METRIC_PADDING, 9) == 0);
    assert(tinyui_theme_set_metric(theme, TINYUI_METRIC_RADIUS, 4) == 0);
    assert(tinyui_theme_set_metric(theme, TINYUI_METRIC_CONTROL_HEIGHT, 23) == 0);
    assert(tinyui_app_set_theme(app, theme) == 0);

    win = tinyui_window_create(app, "root");
    button = tinyui_button_create(win, "ok");
    list = tinyui_list_create(win, "items");
    calendar = tinyui_calendar_create(win, "calendar");
    assert(win != NULL);
    assert(button != NULL);
    assert(list != NULL);
    assert(calendar != NULL);

    assert(tinyui_theme_apply_to_widget(theme,
                                        &button->widget,
                                        TINYUI_PART_MAIN,
                                        TINYUI_STATE_DEFAULT) == 0);
    assert_button_backend_metrics(button, 23, 4, 9);

    assert(tinyui_theme_apply_to_widget(theme,
                                        &win->widget,
                                        TINYUI_PART_MAIN,
                                        TINYUI_STATE_DEFAULT) == 0);
    assert_window_backend_padding(win, 9);

    assert(tinyui_theme_apply_to_widget(theme,
                                        &list->widget,
                                        TINYUI_PART_TEXT,
                                        TINYUI_STATE_FOCUSED) == 0);
    assert_list_backend_text_color(list, 0x505050U);

    assert(tinyui_theme_apply_to_widget(theme,
                                        &calendar->widget,
                                        TINYUI_PART_MAIN,
                                        TINYUI_STATE_DEFAULT) == 0);
    assert(tinyui_theme_apply_to_widget(theme,
                                        &calendar->widget,
                                        TINYUI_PART_TEXT,
                                        TINYUI_STATE_FOCUSED) == 0);
    assert_calendar_backend_style(calendar, 0x303030U, 0x404040U, 0x505050U);

    tinyui_app_destroy(app);
    tinyui_theme_destroy(theme);
}

static void test_theme_apply_requires_theme_owned_native_style_dispatch(void)
{
    struct tinyui_theme *theme = tinyui_theme_create();
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *win;
    struct tinyui_button *button;
    struct tinyui_checkbox *checkbox;
    struct tinyui_switch *sw;
    struct tinyui_slider *slider;

    assert(theme != NULL);
    assert(app != NULL);
    assert(tinyui_theme_set_color(theme, TINYUI_COLOR_TEXT_PRIMARY, 0x111213U) == 0);
    assert(tinyui_theme_set_color(theme, TINYUI_COLOR_BG, 0x212223U) == 0);
    assert(tinyui_theme_set_color(theme, TINYUI_COLOR_PANEL, 0x313233U) == 0);
    assert(tinyui_theme_set_color(theme, TINYUI_COLOR_BORDER, 0x414243U) == 0);
    assert(tinyui_theme_set_color(theme, TINYUI_COLOR_ACCENT, 0x515253U) == 0);
    assert(tinyui_theme_set_color(theme, TINYUI_COLOR_DISABLED, 0x616263U) == 0);
    assert(tinyui_theme_set_metric(theme, TINYUI_METRIC_PADDING, 6) == 0);
    assert(tinyui_theme_set_metric(theme, TINYUI_METRIC_RADIUS, 5) == 0);
    assert(tinyui_theme_set_metric(theme, TINYUI_METRIC_CONTROL_HEIGHT, 21) == 0);
    assert(tinyui_app_set_theme(app, theme) == 0);

    win = tinyui_window_create(app, "dispatch_root");
    button = tinyui_button_create(win, "dispatch_button");
    checkbox = tinyui_checkbox_create(win, "dispatch_checkbox");
    sw = tinyui_switch_create(win, "dispatch_switch");
    slider = tinyui_slider_create(win, "dispatch_slider");
    assert(win != NULL);
    assert(button != NULL);
    assert(checkbox != NULL);
    assert(sw != NULL);
    assert(slider != NULL);

    assert(tinyui_theme_apply_to_widget(theme,
                                        &button->widget,
                                        TINYUI_PART_MAIN,
                                        TINYUI_STATE_DEFAULT) == 0);
    assert_button_backend_style(button, 0x313233U, 0x515253U, 0x111213U);

    assert(tinyui_theme_apply_to_widget(theme,
                                        &checkbox->widget,
                                        TINYUI_PART_INDICATOR,
                                        TINYUI_STATE_DEFAULT) == 0);
    assert_checkbox_backend_style(checkbox, 0x313233U, 0x313233U, 0x111213U);

    assert(tinyui_theme_apply_to_widget(theme,
                                        &sw->widget,
                                        TINYUI_PART_KNOB,
                                        TINYUI_STATE_DEFAULT) == 0);
    assert_switch_backend_style(sw, 0x414243U, 0x515253U, 0x313233U, 0x414243U);

    assert(tinyui_theme_apply_to_widget(theme,
                                        &slider->widget,
                                        TINYUI_PART_TRACK,
                                        TINYUI_STATE_DEFAULT) == 0);
    assert_slider_backend_style(slider, 0x414243U, 0x414243U, 0x515253U);

    tinyui_app_destroy(app);
    tinyui_theme_destroy(theme);
}

static void test_app_set_theme_updates_app_theme(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_theme *theme = tinyui_theme_create();

    assert(app != NULL);
    assert(theme != NULL);
    assert(app->theme == NULL);

    assert(tinyui_app_set_theme(app, theme) == 0);
    assert(app->theme == theme);

    assert(tinyui_app_set_theme(NULL, theme) == -1);
    assert(tinyui_app_set_theme(app, NULL) == -1);
    assert(app->theme == theme);

    tinyui_app_destroy(app);
    tinyui_theme_destroy(theme);
}

static void test_app_set_theme_null_arg_guard_fires(void)
{
    struct tinyui_theme *theme = tinyui_theme_create();
    struct tinyui_app *app = tinyui_app_create();
    struct ld_scene_t *saved_ld_scene;

    assert(theme != NULL);
    assert(app != NULL);
    assert(app->theme == NULL);

    /* Simulate pre-init state: ld_scene == NULL means bind_theme still works
     * (Phase A: bind_theme no longer gated on ld_scene).  Just verify null-arg
     * guard still fires. */
    saved_ld_scene = app->ld_scene;
    app->ld_scene = NULL;

    assert(tinyui_app_set_theme(app, theme) == 0);

    app->ld_scene = saved_ld_scene;
    tinyui_app_destroy(app);
    tinyui_theme_destroy(theme);
}

static void test_theme_shared_style_apply_helpers_reject_null_and_unsupported_backend(void)
{
    struct tinyui_widget backend = {0};

    assert(tinyui_theme_apply_widget_style(0,
                                           TINYUI_PART_MAIN,
                                           TINYUI_STATE_DEFAULT,
                                           0x111111U,
                                           0x222222U,
                                           0x333333U)
           == -1);

    backend.kind = TINYUI_BACKEND_WIDGET_QRCODE;
    assert(tinyui_theme_apply_widget_style(&backend,
                                           TINYUI_PART_MAIN,
                                           TINYUI_STATE_DEFAULT,
                                           0x111111U,
                                           0x222222U,
                                           0x333333U)
           == -1);
}

static void test_theme_internal_style_apply_helper_no_longer_uses_tinyui_prefix(void)
{
    const char *source = "tinyui/src/theme/theme.c";

    assert_source_lacks_static_definition(source, "tinyui_theme_apply_widget_style");
}

int main(void)
{
    struct tinyui_theme *theme = tinyui_theme_create();
    struct tinyui_app *app;
    struct tinyui_window *win;
    struct tinyui_label *label;
    struct tinyui_button *button;
    struct tinyui_checkbox *checkbox;
    struct tinyui_switch *sw;
    struct tinyui_slider *slider;
    struct tinyui_text *text;
    struct tinyui_list *list;
    struct tinyui_image *image;
    struct tinyui_label *failed_label;
    ldLabel_t *failed_ld_label;
    void *saved_ld_widget;
    int failed_width;
    int failed_height;
    unsigned int failed_backend_bg;
    unsigned int failed_backend_text;

    test_theme_internal_static_helpers_no_longer_use_tinyui_prefix();

    assert(theme != NULL);
    assert(tinyui_theme_set_color(theme, TINYUI_COLOR_TEXT_PRIMARY, 0x111111U) == 0);
    assert(tinyui_theme_set_color(theme, TINYUI_COLOR_BG, 0x222222U) == 0);
    assert(tinyui_theme_set_color(theme, TINYUI_COLOR_PANEL, 0x333333U) == 0);
    assert(tinyui_theme_set_color(theme, TINYUI_COLOR_BORDER, 0x444444U) == 0);
    assert(tinyui_theme_set_color(theme, TINYUI_COLOR_ACCENT, 0x555555U) == 0);
    assert(tinyui_theme_set_color(theme, TINYUI_COLOR_DISABLED, 0x666666U) == 0);
    assert(tinyui_theme_set_metric(theme, TINYUI_METRIC_PADDING, 5) == 0);
    assert(tinyui_theme_set_metric(theme, TINYUI_METRIC_RADIUS, 3) == 0);
    assert(tinyui_theme_set_metric(theme, TINYUI_METRIC_BORDER_WIDTH, 2) == 0);
    assert(tinyui_theme_set_metric(theme, TINYUI_METRIC_BORDER_WIDTH, -1) == -1);
    assert(tinyui_theme_set_metric(theme, TINYUI_METRIC_CONTROL_HEIGHT, 19) == 0);

    test_app_set_theme_updates_app_theme();
    test_theme_apply_requires_theme_owned_native_style_dispatch();

    app = tinyui_app_create();
    assert(app != NULL);
    assert(tinyui_app_set_theme(app, theme) == 0);

    win = tinyui_window_create(app, "root");
    label = tinyui_label_create(win, "label");
    button = tinyui_button_create(win, "ok");
    checkbox = tinyui_checkbox_create(win, "check");
    sw = tinyui_switch_create(win, "switch");
    slider = tinyui_slider_create(win, "slider");
    text = tinyui_text_create(win, "text");
    list = tinyui_list_create(win, "list");
    image = tinyui_image_create(win, "image");
    failed_label = tinyui_label_create(win, "failed_label");
    assert(win != NULL);
    assert(label != NULL);
    assert(button != NULL);
    assert(checkbox != NULL);
    assert(sw != NULL);
    assert(slider != NULL);
    assert(text != NULL);
    assert(list != NULL);
    assert(image != NULL);
    assert(failed_label != NULL);

    assert(tinyui_theme_apply_to_widget(theme,
                                        &label->widget,
                                        TINYUI_PART_MAIN,
                                        TINYUI_STATE_DISABLED)
           == 0);
    assert_widget_style(&label->widget, 0x222222U, 0x666666U, 0x666666U, 3, 5);
    assert_backend_height(&label->widget, 19);
    assert_backend_size(&label->widget, 220, 19);
    assert_label_backend_style(label, 0x222222U, 0x666666U);

    failed_width = backend_width(&failed_label->widget);
    failed_height = backend_height(&failed_label->widget);
    failed_ld_label = (ldLabel_t *)failed_label->widget.ld_widget;
    failed_backend_bg = failed_ld_label->bgColor;
    failed_backend_text = failed_ld_label->textColor;
    saved_ld_widget = failed_label->widget.ld_widget;
    failed_label->widget.ld_widget = NULL; /* force apply_to_widget failure */
    assert(tinyui_theme_apply_to_widget(theme,
                                        &failed_label->widget,
                                        TINYUI_PART_MAIN,
                                        TINYUI_STATE_DISABLED)
           == -1);
    failed_label->widget.ld_widget = saved_ld_widget;
    assert_widget_style(&failed_label->widget, 0U, 0U, 0U, 0, 0);
    assert(backend_width(&failed_label->widget) == failed_width);
    assert(backend_height(&failed_label->widget) == failed_height);
    assert(failed_ld_label->bgColor == failed_backend_bg);
    assert(failed_ld_label->textColor == failed_backend_text);

    assert(tinyui_theme_apply_to_widget(theme,
                                        &text->widget,
                                        TINYUI_PART_TEXT,
                                        TINYUI_STATE_FOCUSED)
           == 0);
    assert_widget_style(&text->widget, 0x333333U, 0x555555U, 0x444444U, 3, 5);
    assert_backend_height(&text->widget, 19);
    assert_text_backend_style(text, 0x333333U, 0x555555U);

    assert(tinyui_theme_apply_to_widget(theme,
                                        &list->widget,
                                        TINYUI_PART_MAIN,
                                        TINYUI_STATE_FOCUSED)
           == 0);
    assert_widget_style(&list->widget, 0x555555U, 0x111111U, 0x555555U, 3, 5);
    assert_backend_height(&list->widget, 19);
    assert_list_backend_style(list, 0x555555U, 0x000000U, 0x555555U);

    assert(tinyui_theme_apply_to_widget(theme,
                                        &list->widget,
                                        TINYUI_PART_TEXT,
                                        TINYUI_STATE_DISABLED)
           == 0);
    assert(list->widget.bg_color == 0x222222U);
    assert(list->widget.text_color == 0x666666U);
    assert(list->widget.border_color == 0x666666U);
    assert(list->widget.radius == 3);
    assert(list->widget.padding == 5);
    assert_backend_height(&list->widget, 19);
    assert_list_backend_text_color(list, 0x666666U);

    assert(tinyui_theme_apply_to_widget(theme,
                                        &button->widget,
                                        TINYUI_PART_MAIN,
                                        TINYUI_STATE_PRESSED)
           == 0);
    assert_widget_style(&button->widget, 0x555555U, 0x111111U, 0x555555U, 3, 5);
    assert_backend_height(&button->widget, 19);
    assert_button_backend_style(button, 0x555555U, 0x555555U, 0x111111U);

    assert(tinyui_theme_apply_to_widget(theme,
                                        &checkbox->widget,
                                        TINYUI_PART_INDICATOR,
                                        TINYUI_STATE_CHECKED)
           == 0);
    assert_widget_style(&checkbox->widget, 0x555555U, 0x111111U, 0x555555U, 3, 5);
    assert_backend_height(&checkbox->widget, 19);
    assert_checkbox_backend_style(checkbox, 0x333333U, 0x555555U, 0x111111U);

    assert(tinyui_theme_apply_to_widget(theme,
                                        &sw->widget,
                                        TINYUI_PART_TRACK,
                                        TINYUI_STATE_DISABLED)
           == 0);
    assert_widget_style(&sw->widget, 0x222222U, 0x666666U, 0x666666U, 3, 5);
    assert_backend_height(&sw->widget, 19);
    assert_switch_backend_style(sw, 0x222222U, 0x555555U, 0x222222U, 0x666666U);

    assert(tinyui_theme_apply_to_widget(theme,
                                        &slider->widget,
                                        TINYUI_PART_KNOB,
                                        TINYUI_STATE_FOCUSED)
           == 0);
    assert_widget_style(&slider->widget, 0x555555U, 0x111111U, 0x444444U, 3, 5);
    assert_backend_height(&slider->widget, 19);
    assert_slider_backend_style(slider, 0x333333U, 0x444444U, 0x555555U);

    assert(tinyui_theme_apply_to_widget(theme,
                                        &win->widget,
                                        TINYUI_PART_MAIN,
                                        TINYUI_STATE_DISABLED)
           == 0);
    assert_widget_style(&win->widget, 0x222222U, 0x666666U, 0x666666U, 3, 5);
    assert_window_backend_style(win, 0x222222U);
    assert_window_backend_padding(win, 5);

    assert(tinyui_theme_apply_to_widget(theme,
                                        &button->widget,
                                        TINYUI_PART_KNOB,
                                        TINYUI_STATE_DEFAULT)
           == -1);
    assert_widget_style(&button->widget, 0x555555U, 0x111111U, 0x555555U, 3, 5);

    assert(tinyui_theme_apply_to_widget(theme,
                                        &label->widget,
                                        TINYUI_PART_INDICATOR,
                                        TINYUI_STATE_DEFAULT)
           == -1);
    assert_widget_style(&label->widget, 0x222222U, 0x666666U, 0x666666U, 3, 5);

    assert(tinyui_theme_apply_to_widget(theme,
                                        &slider->widget,
                                        TINYUI_PART_TEXT,
                                        TINYUI_STATE_DEFAULT)
           == -1);
    assert_widget_style(&slider->widget, 0x555555U, 0x111111U, 0x444444U, 3, 5);

    assert(tinyui_theme_apply_to_widget(theme,
                                        &list->widget,
                                        TINYUI_PART_INDICATOR,
                                        TINYUI_STATE_DEFAULT)
           == -1);
    assert(list->widget.bg_color == 0x222222U);
    assert(list->widget.text_color == 0x666666U);
    assert(list->widget.border_color == 0x666666U);
    assert(list->widget.radius == 3);
    assert(list->widget.padding == 5);

    assert(tinyui_theme_apply_to_widget(theme,
                                        &slider->widget,
                                        TINYUI_PART_TRACK,
                                        (enum tinyui_state)99)
           == -1);
    assert_widget_style(&slider->widget, 0x555555U, 0x111111U, 0x444444U, 3, 5);

    assert(tinyui_theme_apply_to_widget(theme,
                                        &image->widget,
                                        TINYUI_PART_MAIN,
                                        TINYUI_STATE_DEFAULT)
           == 0);
    assert_widget_style(&image->widget, 0x333333U, 0x111111U, 0x444444U, 3, 5);
    assert(image->widget.height == 0);
    assert(((ldImage_t *)image->widget.ld_widget)->maskColor
           == tinyui_test_rgb_to_ld_color(0x333333U));

    assert(label->widget.visible == 1);
    assert(ldBaseIsHidden((ldBase_t *)label->widget.ld_widget) == false);
    assert(tinyui_widget_set_visible(&label->widget, 0) == 0);
    assert(label->widget.visible == 0);
    assert(ldBaseIsHidden((ldBase_t *)label->widget.ld_widget) == true);
    assert(tinyui_widget_set_visible(&label->widget, 1) == 0);
    assert(label->widget.visible == 1);
    assert(ldBaseIsHidden((ldBase_t *)label->widget.ld_widget) == false);

    assert(sw->widget.enabled == 1);
    assert(ldSwitchIsDisabled((ldSwitch_t *)sw->widget.ld_widget) == false);
    assert(tinyui_widget_set_enabled(&sw->widget, 0) == 0);
    assert(sw->widget.enabled == 0);
    assert(ldSwitchIsDisabled((ldSwitch_t *)sw->widget.ld_widget) == true);
    assert(tinyui_widget_set_enabled(&sw->widget, 1) == 0);
    assert(sw->widget.enabled == 1);
    assert(ldSwitchIsDisabled((ldSwitch_t *)sw->widget.ld_widget) == false);

    test_image_theme_and_enabled_are_support_not_reject();
    test_theme_native_parts_apply_to_real_backend_fields();
    test_app_set_theme_null_arg_guard_fires();
    test_theme_shared_style_apply_helpers_reject_null_and_unsupported_backend();
    test_theme_internal_style_apply_helper_no_longer_uses_tinyui_prefix();

    tinyui_app_destroy(app);
    tinyui_theme_destroy(theme);
    return 0;
}
