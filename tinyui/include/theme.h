#ifndef TINYUI_THEME_H
#define TINYUI_THEME_H

struct tinyui_app;
struct tinyui_theme;
struct tinyui_widget;

enum tinyui_state {
    TINYUI_STATE_DEFAULT = 0,
    TINYUI_STATE_DISABLED = 1,
    TINYUI_STATE_PRESSED = 2,
    TINYUI_STATE_CHECKED = 3,
    TINYUI_STATE_FOCUSED = 4,
};

enum tinyui_part {
    TINYUI_PART_MAIN,
    TINYUI_PART_TEXT,
    TINYUI_PART_INDICATOR,
    TINYUI_PART_KNOB,
    TINYUI_PART_TRACK,
};

enum tinyui_color_id {
    TINYUI_COLOR_TEXT_PRIMARY,
    TINYUI_COLOR_BG,
    TINYUI_COLOR_PANEL,
    TINYUI_COLOR_BORDER,
    TINYUI_COLOR_ACCENT,
    TINYUI_COLOR_DISABLED,
    TINYUI_COLOR_COUNT,
};

enum tinyui_metric_id {
    TINYUI_METRIC_PADDING,
    TINYUI_METRIC_RADIUS,
    TINYUI_METRIC_BORDER_WIDTH,
    TINYUI_METRIC_CONTROL_HEIGHT,
    TINYUI_METRIC_COUNT,
};

struct tinyui_theme *tinyui_theme_create(void);

void tinyui_theme_destroy(struct tinyui_theme *theme);

int tinyui_theme_set_color(struct tinyui_theme *theme, enum tinyui_color_id id, unsigned int rgb);

int tinyui_theme_set_metric(struct tinyui_theme *theme, enum tinyui_metric_id id, int value);

int tinyui_theme_apply_to_widget(struct tinyui_theme *theme,
                                 struct tinyui_widget *widget,
                                 enum tinyui_part part,
                                 enum tinyui_state state);

int tinyui_app_set_theme(struct tinyui_app *app, struct tinyui_theme *theme);

#endif
