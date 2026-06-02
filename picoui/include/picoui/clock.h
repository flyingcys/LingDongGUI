#ifndef PICOUI_CLOCK_H
#define PICOUI_CLOCK_H

struct picoui_widget;
struct picoui_clock;
struct picoui_image_source;

struct picoui_clock_props {
    const char *id;
    const char *style_class;
    void *user_data;
    struct picoui_image_source *background_source;
    struct picoui_image_source *hour_pointer_source;
    struct picoui_image_source *minute_pointer_source;
    struct picoui_image_source *second_pointer_source;
    unsigned int mask_color;
    float hour_anchor_x;
    float hour_anchor_y;
    float minute_anchor_x;
    float minute_anchor_y;
    float second_anchor_x;
    float second_anchor_y;
    int step_second;
};

struct picoui_clock *picoui_clock_create(struct picoui_widget *parent, const char *id);
struct picoui_clock *picoui_clock_create_with_props(
    struct picoui_widget *parent,
    const struct picoui_clock_props *props);
struct picoui_clock *picoui_clock_init(struct picoui_widget *parent, const char *id);
int picoui_clock_set_use_system_time(struct picoui_clock *clock, int enabled);
int picoui_clock_get_use_system_time(const struct picoui_clock *clock);
int picoui_clock_set_step_second(struct picoui_clock *clock, int step_second);
int picoui_clock_get_step_second(const struct picoui_clock *clock);
int picoui_clock_set_background_image(struct picoui_clock *clock, struct picoui_image_source *source);
int picoui_clock_set_background_source(struct picoui_clock *clock, struct picoui_image_source *source);
int picoui_clock_set_hour_pointer_image(struct picoui_clock *clock, struct picoui_image_source *source);
int picoui_clock_set_hour_pointer_source(struct picoui_clock *clock, struct picoui_image_source *source);
int picoui_clock_set_minute_pointer_image(struct picoui_clock *clock, struct picoui_image_source *source);
int picoui_clock_set_minute_pointer_source(struct picoui_clock *clock, struct picoui_image_source *source);
int picoui_clock_set_second_pointer_image(struct picoui_clock *clock, struct picoui_image_source *source);
int picoui_clock_set_second_pointer_source(struct picoui_clock *clock, struct picoui_image_source *source);
int picoui_clock_set_mask_color(struct picoui_clock *clock, unsigned int mask_color);
int picoui_clock_set_hour_anchor(struct picoui_clock *clock, float x, float y);
int picoui_clock_set_minute_anchor(struct picoui_clock *clock, float x, float y);
int picoui_clock_set_second_anchor(struct picoui_clock *clock, float x, float y);

#endif
