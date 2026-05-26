#ifndef PICOUI_THEME_H
#define PICOUI_THEME_H

struct picoui_app;
struct picoui_theme;

enum picoui_color_id {
    PICOUI_COLOR_TEXT_PRIMARY,
    PICOUI_COLOR_BG,
    PICOUI_COLOR_PANEL,
    PICOUI_COLOR_BORDER,
    PICOUI_COLOR_ACCENT,
    PICOUI_COLOR_DISABLED,
    PICOUI_COLOR_COUNT,
};

enum picoui_metric_id {
    PICOUI_METRIC_PADDING,
    PICOUI_METRIC_RADIUS,
    PICOUI_METRIC_BORDER_WIDTH,
    PICOUI_METRIC_CONTROL_HEIGHT,
    PICOUI_METRIC_COUNT,
};

struct picoui_theme *picoui_theme_create(void);
void picoui_theme_destroy(struct picoui_theme *theme);
int picoui_theme_set_color(struct picoui_theme *theme, enum picoui_color_id id, unsigned int rgb);
int picoui_theme_set_metric(struct picoui_theme *theme, enum picoui_metric_id id, int value);
int picoui_app_set_theme(struct picoui_app *app, struct picoui_theme *theme);

#endif
