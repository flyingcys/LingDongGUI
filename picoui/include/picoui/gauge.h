#ifndef PICOUI_GAUGE_H
#define PICOUI_GAUGE_H

struct picoui_widget;
struct picoui_gauge;

struct picoui_gauge_props {
    const char *id;
    const char *style_class;
    void *user_data;
    float angle;
    unsigned int pointer_color;
    int auto_move;
};

struct picoui_gauge *picoui_gauge_create(struct picoui_widget *parent, const char *id);
struct picoui_gauge *picoui_gauge_create_with_props(struct picoui_widget *parent,
                                                    const struct picoui_gauge_props *props);
int picoui_gauge_set_angle(struct picoui_gauge *gauge, float angle);
float picoui_gauge_get_angle(const struct picoui_gauge *gauge);
int picoui_gauge_set_pointer_color(struct picoui_gauge *gauge, unsigned int pointer_color);
unsigned int picoui_gauge_get_pointer_color(const struct picoui_gauge *gauge);
int picoui_gauge_set_auto_move(struct picoui_gauge *gauge, int auto_move);
int picoui_gauge_get_auto_move(const struct picoui_gauge *gauge);

#endif
