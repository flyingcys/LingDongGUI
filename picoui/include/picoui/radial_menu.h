#ifndef PICOUI_RADIAL_MENU_H
#define PICOUI_RADIAL_MENU_H

struct picoui_widget;
struct picoui_radial_menu;

struct picoui_radial_menu_props {
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

struct picoui_radial_menu *picoui_radial_menu_create(struct picoui_widget *parent, const char *id);
struct picoui_radial_menu *picoui_radial_menu_create_with_props(
    struct picoui_widget *parent,
    const struct picoui_radial_menu_props *props
);
int picoui_radial_menu_add_item(struct picoui_radial_menu *radial_menu, const char *id);
int picoui_radial_menu_set_selected_index(struct picoui_radial_menu *radial_menu, int index);
int picoui_radial_menu_get_selected_index(const struct picoui_radial_menu *radial_menu);
int picoui_radial_menu_offset_selection(struct picoui_radial_menu *radial_menu, int offset);
void picoui_radial_menu_set_on_selected(struct picoui_radial_menu *radial_menu,
                                        void (*callback)(struct picoui_radial_menu *radial_menu,
                                                         int index,
                                                         void *user_data),
                                        void *user_data);

#endif
