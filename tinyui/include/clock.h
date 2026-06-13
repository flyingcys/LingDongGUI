#ifndef TINYUI_CLOCK_H
#define TINYUI_CLOCK_H

struct tinyui_widget;
struct tinyui_clock;
struct tinyui_image_source;

struct tinyui_clock_props {
    const char *id;
    const char *style_class;
    void *user_data;
    struct tinyui_image_source *background_source;
    struct tinyui_image_source *hour_pointer_source;
    struct tinyui_image_source *minute_pointer_source;
    struct tinyui_image_source *second_pointer_source;
    unsigned int mask_color;
    float hour_anchor_x;
    float hour_anchor_y;
    float minute_anchor_x;
    float minute_anchor_y;
    float second_anchor_x;
    float second_anchor_y;
    int step_second;
};

struct tinyui_clock *tinyui_clock_create(struct tinyui_widget *parent, const char *id);

struct tinyui_clock *tinyui_clock_create_with_props(
    struct tinyui_widget *parent,
    const struct tinyui_clock_props *props);

struct tinyui_clock *tinyui_clock_init(struct tinyui_widget *parent, const char *id);

int tinyui_clock_set_use_system_time(struct tinyui_clock *clock, int enabled);

int tinyui_clock_set_auto_sys_time(struct tinyui_clock *clock, int enabled);

int tinyui_clock_get_use_system_time(const struct tinyui_clock *clock);

int tinyui_clock_set_step_second(struct tinyui_clock *clock, int step_second);

int tinyui_clock_get_step_second(const struct tinyui_clock *clock);

int tinyui_clock_set_background_image(struct tinyui_clock *clock, struct tinyui_image_source *source);

int tinyui_clock_set_background_source(struct tinyui_clock *clock, struct tinyui_image_source *source);

int tinyui_clock_set_hour_pointer_image(struct tinyui_clock *clock, struct tinyui_image_source *source);

int tinyui_clock_set_hour_pointer_source(struct tinyui_clock *clock, struct tinyui_image_source *source);

int tinyui_clock_set_minute_pointer_image(struct tinyui_clock *clock, struct tinyui_image_source *source);

int tinyui_clock_set_minute_pointer_source(struct tinyui_clock *clock, struct tinyui_image_source *source);

int tinyui_clock_set_second_pointer_image(struct tinyui_clock *clock, struct tinyui_image_source *source);

int tinyui_clock_set_second_pointer_source(struct tinyui_clock *clock, struct tinyui_image_source *source);

int tinyui_clock_set_mask_color(struct tinyui_clock *clock, unsigned int mask_color);

int tinyui_clock_set_hour_anchor(struct tinyui_clock *clock, float x, float y);

int tinyui_clock_set_minute_anchor(struct tinyui_clock *clock, float x, float y);

int tinyui_clock_set_second_anchor(struct tinyui_clock *clock, float x, float y);

#endif
