#ifndef TINYUI_PROGRESS_WHEEL_H
#define TINYUI_PROGRESS_WHEEL_H

struct tinyui_widget;
struct tinyui_progress_wheel;

struct tinyui_progress_wheel_props {
    const char *id;
    const char *style_class;
    void *user_data;
    int percent;
    int dot_enabled;
};

struct tinyui_progress_wheel *tinyui_progress_wheel_create(struct tinyui_widget *parent,
                                                           const char *id);
struct tinyui_progress_wheel *tinyui_progress_wheel_create_with_props(
    struct tinyui_widget *parent,
    const struct tinyui_progress_wheel_props *props);
struct tinyui_progress_wheel *tinyui_progress_wheel_init(struct tinyui_widget *parent,
                                                         const char *id);
int tinyui_progress_wheel_set_percent(struct tinyui_progress_wheel *wheel, int percent);
int tinyui_progress_wheel_set_progress(struct tinyui_progress_wheel *wheel, int percent);
int tinyui_progress_wheel_get_percent(const struct tinyui_progress_wheel *wheel);
int tinyui_progress_wheel_set_wheel_color(struct tinyui_progress_wheel *wheel, unsigned int rgb);
int tinyui_progress_wheel_set_dot_color(struct tinyui_progress_wheel *wheel, unsigned int rgb);
int tinyui_progress_wheel_set_dot_enabled(struct tinyui_progress_wheel *wheel, int enabled);
int tinyui_progress_wheel_get_dot_enabled(const struct tinyui_progress_wheel *wheel);

#endif
