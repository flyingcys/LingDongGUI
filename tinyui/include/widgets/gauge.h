#ifndef TINYUI_GAUGE_H
#define TINYUI_GAUGE_H

struct tinyui_widget;
struct tinyui_gauge;
struct tinyui_image_source;

struct tinyui_gauge_props {
    const char *id;
    const char *style_class;
    void *user_data;
    float angle;
    struct tinyui_image_source *bg_source;
    struct tinyui_image_source *pointer_source;
    int centre_offset_x;
    int centre_offset_y;
    unsigned int pointer_color;
    int auto_move;
};

struct tinyui_gauge *tinyui_gauge_create(struct tinyui_widget *parent, const char *id);
struct tinyui_gauge *tinyui_gauge_create_with_props(struct tinyui_widget *parent,
                                                    const struct tinyui_gauge_props *props);
struct tinyui_gauge *tinyui_gauge_init(struct tinyui_widget *parent, const char *id);
int tinyui_gauge_set_angle(struct tinyui_gauge *gauge, float angle);
float tinyui_gauge_get_angle(const struct tinyui_gauge *gauge);
int tinyui_gauge_set_bg_source(struct tinyui_gauge *gauge, struct tinyui_image_source *source);
int tinyui_gauge_set_background_image(struct tinyui_gauge *gauge, struct tinyui_image_source *source);
int tinyui_gauge_set_pointer_source(struct tinyui_gauge *gauge, struct tinyui_image_source *source);
int tinyui_gauge_set_centre_offset(struct tinyui_gauge *gauge, int centre_offset_x, int centre_offset_y);
int tinyui_gauge_set_trail(struct tinyui_gauge *gauge,
                           struct tinyui_image_source *bg_trail_source,
                           struct tinyui_image_source *pointer_trail_source);
int tinyui_gauge_set_progress_bar(struct tinyui_gauge *gauge,
                                  struct tinyui_image_source *bg_progress_source,
                                  struct tinyui_image_source *pointer_progress_source);
int tinyui_gauge_set_pointer_color(struct tinyui_gauge *gauge, unsigned int pointer_color);
unsigned int tinyui_gauge_get_pointer_color(const struct tinyui_gauge *gauge);
int tinyui_gauge_set_auto_move(struct tinyui_gauge *gauge, int auto_move);
int tinyui_gauge_get_auto_move(const struct tinyui_gauge *gauge);

#endif
