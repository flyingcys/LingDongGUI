#ifndef PICOUI_CLOCK_H
#define PICOUI_CLOCK_H

struct picoui_widget;
struct picoui_clock;

struct picoui_clock_props {
    const char *id;
    const char *style_class;
    void *user_data;
    int step_second;
};

struct picoui_clock *picoui_clock_create(struct picoui_widget *parent, const char *id);
struct picoui_clock *picoui_clock_create_with_props(
    struct picoui_widget *parent,
    const struct picoui_clock_props *props);
int picoui_clock_set_step_second(struct picoui_clock *clock, int step_second);
int picoui_clock_get_step_second(const struct picoui_clock *clock);

#endif
