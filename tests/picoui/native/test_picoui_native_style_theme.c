#include "picoui/picoui.h"

#include <assert.h>

int picoui_native_style_get_bg_color(const struct picoui_widget *widget,
                                     enum picoui_part part,
                                     enum picoui_state state,
                                     unsigned int *rgb);
int picoui_native_style_get_text_color(const struct picoui_widget *widget,
                                       enum picoui_part part,
                                       enum picoui_state state,
                                       unsigned int *rgb);
int picoui_native_style_get_border_color(const struct picoui_widget *widget,
                                         enum picoui_part part,
                                         enum picoui_state state,
                                         unsigned int *rgb);
int picoui_native_style_get_radius(const struct picoui_widget *widget, int *radius);
int picoui_native_style_get_padding(const struct picoui_widget *widget, int *padding);

int main(void)
{
    struct picoui_display *display;
    struct picoui_screen *screen;
    struct picoui_window *root_window;
    struct picoui_label *label;
    struct picoui_button *button;
    struct picoui_theme *theme;
    unsigned int rgb;
    int metric;

    assert(picoui_init() == 0);

    display = picoui_display_create(320, 480);
    assert(display != 0);
    assert(picoui_display_set_default(display) == 0);

    screen = picoui_screen_active();
    assert(screen != 0);

    root_window = picoui_window_create_root(screen, "root");
    assert(root_window != 0);
    label = picoui_label_create(root_window, "status_label");
    button = picoui_button_create(root_window, "confirm_button");
    assert(label != 0);
    assert(button != 0);

    assert(picoui_widget_set_bg_color((struct picoui_widget *)root_window, 0x112233U) == 0);
    assert(picoui_label_set_bg_color(label, 0x223344U) == 0);
    assert(picoui_label_set_text_color(label, 0x334455U) == 0);
    assert(picoui_widget_set_border_color((struct picoui_widget *)button, 0x445566U) == 0);
    assert(picoui_widget_set_radius((struct picoui_widget *)button, 9) == 0);
    assert(picoui_widget_set_padding((struct picoui_widget *)button, 11) == 0);

    assert(picoui_native_style_get_bg_color((const struct picoui_widget *)label,
                                            PICOUI_PART_MAIN,
                                            PICOUI_STATE_DEFAULT,
                                            &rgb) == 0);
    assert(rgb == 0x223344U);
    assert(picoui_native_style_get_text_color((const struct picoui_widget *)label,
                                              PICOUI_PART_MAIN,
                                              PICOUI_STATE_DEFAULT,
                                              &rgb) == 0);
    assert(rgb == 0x334455U);

    assert(picoui_native_style_get_bg_color((const struct picoui_widget *)root_window,
                                            PICOUI_PART_MAIN,
                                            PICOUI_STATE_DEFAULT,
                                            &rgb) == 0);
    assert(rgb == 0x112233U);
    assert(picoui_native_style_get_border_color((const struct picoui_widget *)button,
                                                PICOUI_PART_MAIN,
                                                PICOUI_STATE_DEFAULT,
                                                &rgb) == 0);
    assert(rgb == 0x445566U);
    assert(picoui_native_style_get_radius((const struct picoui_widget *)button, &metric) == 0);
    assert(metric == 9);
    assert(picoui_native_style_get_padding((const struct picoui_widget *)button, &metric) == 0);
    assert(metric == 11);

    theme = picoui_theme_create();
    assert(theme != 0);
    assert(picoui_theme_set_color(theme, PICOUI_COLOR_PANEL, 0x556677U) == 0);
    assert(picoui_theme_set_color(theme, PICOUI_COLOR_TEXT_PRIMARY, 0x667788U) == 0);
    assert(picoui_theme_set_color(theme, PICOUI_COLOR_BORDER, 0x778899U) == 0);
    assert(picoui_theme_set_color(theme, PICOUI_COLOR_ACCENT, 0x8899AAU) == 0);
    assert(picoui_theme_set_metric(theme, PICOUI_METRIC_RADIUS, 5) == 0);
    assert(picoui_theme_set_metric(theme, PICOUI_METRIC_PADDING, 7) == 0);
    assert(picoui_theme_apply_to_widget(theme,
                                        (struct picoui_widget *)button,
                                        PICOUI_PART_MAIN,
                                        PICOUI_STATE_PRESSED) == 0);

    assert(picoui_native_style_get_bg_color((const struct picoui_widget *)button,
                                            PICOUI_PART_MAIN,
                                            PICOUI_STATE_PRESSED,
                                            &rgb) == 0);
    assert(rgb == 0x8899AAU);
    assert(picoui_native_style_get_border_color((const struct picoui_widget *)button,
                                                PICOUI_PART_MAIN,
                                                PICOUI_STATE_PRESSED,
                                                &rgb) == 0);
    assert(rgb == 0x8899AAU);
    assert(picoui_native_style_get_radius((const struct picoui_widget *)button, &metric) == 0);
    assert(metric == 5);
    assert(picoui_native_style_get_padding((const struct picoui_widget *)button, &metric) == 0);
    assert(metric == 7);

    picoui_theme_destroy(theme);
    picoui_deinit();
    return 0;
}
