#ifndef TINYUI_THEME_H
#define TINYUI_THEME_H

struct picoui_app;
struct picoui_theme;
struct picoui_widget;

enum picoui_state {
    PICOUI_STATE_DEFAULT = 0,
    PICOUI_STATE_DISABLED = 1,
    PICOUI_STATE_PRESSED = 2,
    PICOUI_STATE_CHECKED = 3,
    PICOUI_STATE_FOCUSED = 4,
};

enum picoui_part {
    PICOUI_PART_MAIN,
    PICOUI_PART_TEXT,
    PICOUI_PART_INDICATOR,
    PICOUI_PART_KNOB,
    PICOUI_PART_TRACK,
};

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

int picoui_theme_apply_to_widget(struct picoui_theme *theme,
                                 struct picoui_widget *widget,
                                 enum picoui_part part,
                                 enum picoui_state state);

int picoui_app_set_theme(struct picoui_app *app, struct picoui_theme *theme);

#endif
