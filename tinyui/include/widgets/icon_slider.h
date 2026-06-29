#ifndef TINYUI_ICON_SLIDER_H
#define TINYUI_ICON_SLIDER_H

struct tinyui_widget;
struct tinyui_icon_slider;
struct tinyui_image_source;

struct tinyui_icon_slider_props {
    const char *id;
    const char *style_class;
    void *user_data;
    int width;
    int height;
    int icon_width;
    int icon_space;
    int columns;
    int rows;
    int pages;
    int horizontal;
};

struct tinyui_icon_slider *tinyui_icon_slider_create(struct tinyui_widget *parent, const char *id);
struct tinyui_icon_slider *tinyui_icon_slider_create_with_props(
    struct tinyui_widget *parent,
    const struct tinyui_icon_slider_props *props
);
struct tinyui_icon_slider *tinyui_icon_slider_init(struct tinyui_widget *parent, const char *id);
int tinyui_icon_slider_add_item(struct tinyui_icon_slider *icon_slider,
                                const char *id,
                                const char *text);
int tinyui_icon_slider_add_item_with_source(struct tinyui_icon_slider *icon_slider,
                                            const char *id,
                                            const char *text,
                                            struct tinyui_image_source *source);
int tinyui_icon_slider_add_icon(struct tinyui_icon_slider *icon_slider,
                                const char *id,
                                const char *text,
                                struct tinyui_image_source *source);
int tinyui_icon_slider_set_layout(struct tinyui_icon_slider *icon_slider,
                                  int width,
                                  int height,
                                  int icon_width,
                                  int icon_space,
                                  int columns,
                                  int rows,
                                  int pages);
int tinyui_icon_slider_set_selected_index(struct tinyui_icon_slider *icon_slider, int index);
int tinyui_icon_slider_get_selected_index(const struct tinyui_icon_slider *icon_slider);
int tinyui_icon_slider_set_horizontal(struct tinyui_icon_slider *icon_slider, int horizontal);
int tinyui_icon_slider_set_horizontal_scroll(struct tinyui_icon_slider *icon_slider, int horizontal);
int tinyui_icon_slider_get_horizontal(const struct tinyui_icon_slider *icon_slider, int *horizontal);
int tinyui_icon_slider_set_speed(struct tinyui_icon_slider *icon_slider, int speed);
void tinyui_icon_slider_set_on_selected(struct tinyui_icon_slider *icon_slider,
                                        void (*callback)(struct tinyui_icon_slider *icon_slider,
                                                         int index,
                                                         void *user_data),
                                        void *user_data);

#endif
