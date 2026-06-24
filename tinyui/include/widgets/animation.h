#ifndef TINYUI_ANIMATION_H
#define TINYUI_ANIMATION_H

struct tinyui_window;
struct tinyui_animation;
struct tinyui_image_source;

struct tinyui_animation_props {
    const char *id;
    const char *style_class;
    void *user_data;
    int width;
    int height;
    int period_ms;
    struct tinyui_image_source *source;
};

struct tinyui_animation *tinyui_animation_create(struct tinyui_widget *parent, const char *id);
struct tinyui_animation *tinyui_animation_init(struct tinyui_widget *parent, const char *id);
struct tinyui_animation *tinyui_animation_create_with_props(
    struct tinyui_widget *parent,
    const struct tinyui_animation_props *props);
int tinyui_animation_set_source(struct tinyui_animation *animation, struct tinyui_image_source *source);
int tinyui_animation_set_period_ms(struct tinyui_animation *animation, int period_ms);
int tinyui_animation_show_frame(struct tinyui_animation *animation, int frame_index);

#endif
