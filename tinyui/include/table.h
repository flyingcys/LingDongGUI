#ifndef TINYUI_TABLE_H
#define TINYUI_TABLE_H

#include "native.h"
#include "widget.h"

struct picoui_window;
struct picoui_table;
struct picoui_image_source;

struct picoui_table_region {
    int x;
    int y;
    int width;
    int height;
};

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

struct picoui_table *picoui_table_init(struct picoui_window *parent,
                                       const char *id,
                                       int rows,
                                       int columns);

int picoui_table_set_keyboard(struct picoui_table *table, unsigned int keyboard_binding);

int picoui_table_set_keyboard_binding(struct picoui_table *table, unsigned int keyboard_binding);

int picoui_table_get_keyboard_binding(const struct picoui_table *table, unsigned int *keyboard_binding);

int picoui_tabel_show_keyboard(struct picoui_table *table);

int picoui_table_set_item_text(struct picoui_table *table,
                               int row,
                               int column,
                               const char *text);

int picoui_table_set_cell_text(struct picoui_table *table,
                               int row,
                               int column,
                               const char *text);

const char *picoui_table_get_item_text(const struct picoui_table *table, int row, int column);

const char *picoui_table_get_cell_text(const struct picoui_table *table, int row, int column);

int picoui_table_set_item_editable(struct picoui_table *table,
                                   int row,
                                   int column,
                                   int editable,
                                   unsigned int text_max);

int picoui_table_set_cell_editable(struct picoui_table *table,
                                   int row,
                                   int column,
                                   int editable,
                                   unsigned int text_max);

int picoui_table_set_item_image(struct picoui_table *table,
                                int row,
                                int column,
                                int x,
                                int y,
                                struct picoui_image_source *source,
                                unsigned int mask_color);

int picoui_table_set_item_button(struct picoui_table *table,
                                 int row,
                                 int column,
                                 int x,
                                 int y,
                                 struct picoui_image_source *release_source,
                                 unsigned int release_mask_color,
                                 struct picoui_image_source *press_source,
                                 unsigned int press_mask_color,
                                 int checkable);

int picoui_table_set_background_color(struct picoui_table *table, unsigned int bg_color);

int picoui_table_set_excel_type(struct picoui_table *table);

int picoui_table_set_align_grid(struct picoui_table *table, int enabled);

int picoui_table_set_item_width(struct picoui_table *table, int column, int width);

int picoui_table_set_item_height(struct picoui_table *table, int row, int height);

int picoui_table_set_item_color(struct picoui_table *table,
                                int row,
                                int column,
                                unsigned int text_color,
                                unsigned int bg_color);

int picoui_table_set_bg_color(struct picoui_table *table, unsigned int bg_color);

int picoui_table_set_item_static_text(struct picoui_table *table, int row, int column, const char *text);

int picoui_table_set_item_font(struct picoui_table *table, int row, int column);

int picoui_table_get_align_grid(const struct picoui_table *table);

unsigned int picoui_table_get_background_color(const struct picoui_table *table);

void *picoui_table_get_item(const struct picoui_table *table, int row, int column);

int picoui_table_set_item_align(struct picoui_table *table,
                                int row,
                                int column,
                                enum picoui_align align);

int picoui_table_get_item_align(const struct picoui_table *table, int row, int column);

int picoui_table_get_item_editable(const struct picoui_table *table, int row, int column);

void *picoui_table_get_item_font(const struct picoui_table *table, int row, int column);

int picoui_table_get_item_height(const struct picoui_table *table, int row);

unsigned int picoui_table_get_item_text_color(const struct picoui_table *table, int row, int column);

unsigned int picoui_table_get_item_background_color(const struct picoui_table *table, int row, int column);

int picoui_table_get_item_width(const struct picoui_table *table, int column);

int picoui_table_navigate(struct picoui_table *table, enum picoui_native_nav_dir dir);

int picoui_table_set_item_select(struct picoui_table *table, int row, int column, int selected);

struct picoui_table_region picoui_table_get_item_region(const struct picoui_table *table,
                                                        int row,
                                                        int column);

int picoui_table_set_selected_cell(struct picoui_table *table, int row, int column);

int picoui_table_set_current_cell(struct picoui_table *table, int row, int column);

int picoui_table_get_current_row(const struct picoui_table *table);

int picoui_table_get_current_column(const struct picoui_table *table);

#endif
