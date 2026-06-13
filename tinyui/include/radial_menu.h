#ifndef TINYUI_RADIAL_MENU_H
#define TINYUI_RADIAL_MENU_H

struct tinyui_widget;
struct tinyui_radial_menu;
struct tinyui_image_source;

struct tinyui_radial_menu_props {
    const char *id;
    const char *style_class;
    void *user_data;
    int width;
    int height;
    int x_axis;
    int y_axis;
    int item_max;
    int default_index;
};

struct tinyui_radial_menu *tinyui_radial_menu_create(struct tinyui_widget *parent, const char *id);
struct tinyui_radial_menu *tinyui_radial_menu_create_with_props(
    struct tinyui_widget *parent,
    const struct tinyui_radial_menu_props *props
);
struct tinyui_radial_menu *tinyui_radial_menu_init(struct tinyui_widget *parent, const char *id);
int tinyui_radial_menu_add_item(struct tinyui_radial_menu *radial_menu, const char *id);
int tinyui_radial_menu_add_item_with_source(struct tinyui_radial_menu *radial_menu,
                                            const char *id,
                                            struct tinyui_image_source *source);
int tinyui_radial_menu_add_item_with_image(struct tinyui_radial_menu *radial_menu,
                                           const char *id,
                                           struct tinyui_image_source *source);
int tinyui_radial_menu_set_selected_index(struct tinyui_radial_menu *radial_menu, int index);
int tinyui_radial_menu_get_selected_index(const struct tinyui_radial_menu *radial_menu);
int tinyui_radial_menu_offset_selection(struct tinyui_radial_menu *radial_menu, int offset);
int tinyui_radial_menu_set_default_item(struct tinyui_radial_menu *radial_menu, int index);
int tinyui_radial_menu_click_item(struct tinyui_radial_menu *radial_menu, int index);
int tinyui_radial_menu_set_click_item(struct tinyui_radial_menu *radial_menu, int index);
int tinyui_radial_menu_offset_item(struct tinyui_radial_menu *radial_menu, int offset);
void tinyui_radial_menu_set_on_selected(struct tinyui_radial_menu *radial_menu,
                                        void (*callback)(struct tinyui_radial_menu *radial_menu,
                                                         int index,
                                                         void *user_data),
                                        void *user_data);

#endif
