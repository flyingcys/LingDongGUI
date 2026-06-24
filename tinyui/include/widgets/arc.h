#ifndef TINYUI_ARC_H
#define TINYUI_ARC_H

struct tinyui_widget;
struct tinyui_arc;
struct tinyui_image_source;

struct tinyui_arc_props {
    const char *id;
    const char *style_class;
    void *user_data;
    float bg_start_angle;
    float bg_end_angle;
    float fg_end_angle;
    float rotation_angle;
    struct tinyui_image_source *quarter_source;
    unsigned int parent_color;
    unsigned int bg_color;
    unsigned int fg_color;
};

struct tinyui_arc *tinyui_arc_create(struct tinyui_widget *parent, const char *id);
struct tinyui_arc *tinyui_arc_create_with_props(struct tinyui_widget *parent,
                                                const struct tinyui_arc_props *props);
struct tinyui_arc *tinyui_arc_init(struct tinyui_widget *parent, const char *id);
int tinyui_arc_set_background_angle(struct tinyui_arc *arc, float bg_start_angle, float bg_end_angle);
int tinyui_arc_set_foreground_angle(struct tinyui_arc *arc, float fg_end_angle);
int tinyui_arc_set_rotation_angle(struct tinyui_arc *arc, float rotation_angle);
int tinyui_arc_set_quarter_source(struct tinyui_arc *arc, struct tinyui_image_source *source);
int tinyui_arc_set_quarter_image(struct tinyui_arc *arc, struct tinyui_image_source *source);
int tinyui_arc_set_parent_color(struct tinyui_arc *arc, unsigned int parent_color);
int tinyui_arc_set_color(struct tinyui_arc *arc, unsigned int bg_color, unsigned int fg_color);
float tinyui_arc_get_background_start_angle(const struct tinyui_arc *arc);
float tinyui_arc_get_background_angle(const struct tinyui_arc *arc);
float tinyui_arc_get_foreground_angle(const struct tinyui_arc *arc);
float tinyui_arc_get_rotation_angle(const struct tinyui_arc *arc);
unsigned int tinyui_arc_get_background_color(const struct tinyui_arc *arc);
unsigned int tinyui_arc_get_foreground_color(const struct tinyui_arc *arc);

#endif
