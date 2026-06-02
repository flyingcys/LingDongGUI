#ifndef PICOUI_ARC_H
#define PICOUI_ARC_H

struct picoui_widget;
struct picoui_arc;
struct picoui_image_source;

struct picoui_arc_props {
    const char *id;
    const char *style_class;
    void *user_data;
    float bg_start_angle;
    float bg_end_angle;
    float fg_end_angle;
    float rotation_angle;
    struct picoui_image_source *quarter_source;
    unsigned int parent_color;
    unsigned int bg_color;
    unsigned int fg_color;
};

struct picoui_arc *picoui_arc_create(struct picoui_widget *parent, const char *id);
struct picoui_arc *picoui_arc_create_with_props(struct picoui_widget *parent,
                                                const struct picoui_arc_props *props);
struct picoui_arc *picoui_arc_init(struct picoui_widget *parent, const char *id);
int picoui_arc_set_background_angle(struct picoui_arc *arc, float bg_start_angle, float bg_end_angle);
int picoui_arc_set_foreground_angle(struct picoui_arc *arc, float fg_end_angle);
int picoui_arc_set_rotation_angle(struct picoui_arc *arc, float rotation_angle);
int picoui_arc_set_quarter_source(struct picoui_arc *arc, struct picoui_image_source *source);
int picoui_arc_set_parent_color(struct picoui_arc *arc, unsigned int parent_color);
int picoui_arc_set_color(struct picoui_arc *arc, unsigned int bg_color, unsigned int fg_color);
float picoui_arc_get_background_start_angle(const struct picoui_arc *arc);
float picoui_arc_get_background_angle(const struct picoui_arc *arc);
float picoui_arc_get_foreground_angle(const struct picoui_arc *arc);
float picoui_arc_get_rotation_angle(const struct picoui_arc *arc);
unsigned int picoui_arc_get_background_color(const struct picoui_arc *arc);
unsigned int picoui_arc_get_foreground_color(const struct picoui_arc *arc);

#endif
