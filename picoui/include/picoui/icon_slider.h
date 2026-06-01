#ifndef PICOUI_ICON_SLIDER_H
#define PICOUI_ICON_SLIDER_H

struct picoui_widget;
struct picoui_icon_slider;
struct picoui_image_source;

struct picoui_icon_slider_props {
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

struct picoui_icon_slider *picoui_icon_slider_create(struct picoui_widget *parent, const char *id);
struct picoui_icon_slider *picoui_icon_slider_create_with_props(
    struct picoui_widget *parent,
    const struct picoui_icon_slider_props *props
);
int picoui_icon_slider_add_item(struct picoui_icon_slider *icon_slider,
                                const char *id,
                                const char *text);
int picoui_icon_slider_add_item_with_source(struct picoui_icon_slider *icon_slider,
                                            const char *id,
                                            const char *text,
                                            struct picoui_image_source *source);
int picoui_icon_slider_set_selected_index(struct picoui_icon_slider *icon_slider, int index);
int picoui_icon_slider_get_selected_index(const struct picoui_icon_slider *icon_slider);
int picoui_icon_slider_set_horizontal(struct picoui_icon_slider *icon_slider, int horizontal);
int picoui_icon_slider_get_horizontal(const struct picoui_icon_slider *icon_slider, int *horizontal);
int picoui_icon_slider_set_speed(struct picoui_icon_slider *icon_slider, int speed);
void picoui_icon_slider_set_on_selected(struct picoui_icon_slider *icon_slider,
                                        void (*callback)(struct picoui_icon_slider *icon_slider,
                                                         int index,
                                                         void *user_data),
                                        void *user_data);

#endif
