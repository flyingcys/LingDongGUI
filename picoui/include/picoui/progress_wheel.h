#ifndef PICOUI_PROGRESS_WHEEL_H
#define PICOUI_PROGRESS_WHEEL_H

struct picoui_widget;
struct picoui_progress_wheel;

struct picoui_progress_wheel_props {
    const char *id;
    const char *style_class;
    void *user_data;
    int percent;
};

struct picoui_progress_wheel *picoui_progress_wheel_create(struct picoui_widget *parent, const char *id);
struct picoui_progress_wheel *picoui_progress_wheel_create_with_props(
    struct picoui_widget *parent,
    const struct picoui_progress_wheel_props *props);
int picoui_progress_wheel_set_percent(struct picoui_progress_wheel *wheel, int percent);
int picoui_progress_wheel_get_percent(const struct picoui_progress_wheel *wheel);

#endif
