#ifndef TINYUI_PROGRESS_WHEEL_H
#define TINYUI_PROGRESS_WHEEL_H

struct picoui_widget;
struct picoui_progress_wheel;

struct picoui_progress_wheel_props {
    const char *id;
    const char *style_class;
    void *user_data;
    int percent;
    int dot_enabled;
};

struct picoui_progress_wheel *picoui_progress_wheel_create(struct picoui_widget *parent,
                                                           const char *id);
struct picoui_progress_wheel *picoui_progress_wheel_create_with_props(
    struct picoui_widget *parent,
    const struct picoui_progress_wheel_props *props);
struct picoui_progress_wheel *picoui_progress_wheel_init(struct picoui_widget *parent,
                                                         const char *id);
int picoui_progress_wheel_set_percent(struct picoui_progress_wheel *wheel, int percent);
int picoui_progress_wheel_set_progress(struct picoui_progress_wheel *wheel, int percent);
int picoui_progress_wheel_get_percent(const struct picoui_progress_wheel *wheel);
int picoui_progress_wheel_set_wheel_color(struct picoui_progress_wheel *wheel, unsigned int rgb);
int picoui_progress_wheel_set_dot_color(struct picoui_progress_wheel *wheel, unsigned int rgb);
int picoui_progress_wheel_set_dot_enabled(struct picoui_progress_wheel *wheel, int enabled);
int picoui_progress_wheel_get_dot_enabled(const struct picoui_progress_wheel *wheel);

#endif
