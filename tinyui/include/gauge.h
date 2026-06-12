#ifndef TINYUI_GAUGE_H
#define TINYUI_GAUGE_H

struct picoui_widget;
struct picoui_gauge;
struct picoui_image_source;

struct picoui_gauge_props {
    const char *id;
    const char *style_class;
    void *user_data;
    float angle;
    struct picoui_image_source *bg_source;
    struct picoui_image_source *pointer_source;
    int centre_offset_x;
    int centre_offset_y;
    unsigned int pointer_color;
    int auto_move;
};

struct picoui_gauge *picoui_gauge_create(struct picoui_widget *parent, const char *id);
struct picoui_gauge *picoui_gauge_create_with_props(struct picoui_widget *parent,
                                                    const struct picoui_gauge_props *props);
struct picoui_gauge *picoui_gauge_init(struct picoui_widget *parent, const char *id);
int picoui_gauge_set_angle(struct picoui_gauge *gauge, float angle);
float picoui_gauge_get_angle(const struct picoui_gauge *gauge);
int picoui_gauge_set_bg_source(struct picoui_gauge *gauge, struct picoui_image_source *source);
int picoui_gauge_set_pointer_source(struct picoui_gauge *gauge, struct picoui_image_source *source);
int picoui_gauge_set_centre_offset(struct picoui_gauge *gauge, int centre_offset_x, int centre_offset_y);
int picoui_gauge_set_trail(struct picoui_gauge *gauge,
                           struct picoui_image_source *bg_trail_source,
                           struct picoui_image_source *pointer_trail_source);
int picoui_gauge_set_progress_bar(struct picoui_gauge *gauge,
                                  struct picoui_image_source *bg_progress_source,
                                  struct picoui_image_source *pointer_progress_source);
int picoui_gauge_set_pointer_color(struct picoui_gauge *gauge, unsigned int pointer_color);
unsigned int picoui_gauge_get_pointer_color(const struct picoui_gauge *gauge);
int picoui_gauge_set_auto_move(struct picoui_gauge *gauge, int auto_move);
int picoui_gauge_get_auto_move(const struct picoui_gauge *gauge);

#endif
