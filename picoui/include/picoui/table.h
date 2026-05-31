#ifndef PICOUI_TABLE_H
#define PICOUI_TABLE_H

#include "picoui/widget.h"

struct picoui_window;
struct picoui_table;

struct picoui_table_props {
    const char *id;
    unsigned int keyboard_binding;
    int rows;
    int columns;
    const char *style_class;
    void *user_data;
    int width;
    int height;
    unsigned int bg_color;
    unsigned int text_color;
    unsigned int border_color;
    int radius;
    int padding;
    int has_keyboard_binding;
};

struct picoui_table *picoui_table_create(struct picoui_window *parent,
                                         const char *id,
                                         int rows,
                                         int columns);
struct picoui_table *picoui_table_create_with_props(struct picoui_window *parent,
                                                    const struct picoui_table_props *props);
int picoui_table_set_keyboard_binding(struct picoui_table *table, unsigned int keyboard_binding);
int picoui_table_get_keyboard_binding(const struct picoui_table *table, unsigned int *keyboard_binding);
int picoui_table_set_cell_text(struct picoui_table *table,
                               int row,
                               int column,
                               const char *text);
const char *picoui_table_get_cell_text(const struct picoui_table *table, int row, int column);
int picoui_table_set_cell_editable(struct picoui_table *table,
                                   int row,
                                   int column,
                                   int editable,
                                   unsigned int text_max);
int picoui_table_set_current_cell(struct picoui_table *table, int row, int column);
int picoui_table_get_current_row(const struct picoui_table *table);
int picoui_table_get_current_column(const struct picoui_table *table);

#endif
