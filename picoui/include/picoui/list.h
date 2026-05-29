#ifndef PICOUI_LIST_H
#define PICOUI_LIST_H

struct picoui_widget;
struct picoui_list;

struct picoui_list_props {
    const char *id;
    const char *style_class;
    void *user_data;
};

struct picoui_list *picoui_list_create(struct picoui_widget *parent, const char *id);
struct picoui_list *picoui_list_create_with_props(struct picoui_widget *parent,
                                                  const struct picoui_list_props *props);
int picoui_list_add_item(struct picoui_list *list, const char *id, const char *text);
int picoui_list_set_selected_index(struct picoui_list *list, int index);
int picoui_list_get_selected_index(const struct picoui_list *list);
void picoui_list_set_on_selected(struct picoui_list *list,
                                 void (*callback)(struct picoui_list *list,
                                                  int index,
                                                  void *user_data),
                                 void *user_data);

#endif
