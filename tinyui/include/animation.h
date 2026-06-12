#ifndef TINYUI_ANIMATION_H
#define TINYUI_ANIMATION_H

struct picoui_window;
struct picoui_animation;
struct picoui_image_source;

struct picoui_animation_props {
    const char *id;
    const char *style_class;
    void *user_data;
    int width;
    int height;
    int period_ms;
    struct picoui_image_source *source;
};

struct picoui_animation *picoui_animation_create(struct picoui_widget *parent, const char *id);
struct picoui_animation *picoui_animation_init(struct picoui_widget *parent, const char *id);
struct picoui_animation *picoui_animation_create_with_props(
    struct picoui_widget *parent,
    const struct picoui_animation_props *props);
int picoui_animation_set_source(struct picoui_animation *animation, struct picoui_image_source *source);
int picoui_animation_set_period_ms(struct picoui_animation *animation, int period_ms);
int picoui_animation_show_frame(struct picoui_animation *animation, int frame_index);

#endif
