#ifndef TINYUI_TABLE_H
#define TINYUI_TABLE_H

#include "core/native.h"
#include "core/widget.h"

struct tinyui_window;
struct tinyui_table;
struct tinyui_image_source;

struct tinyui_table_region {
    int x;
    int y;
    int width;
    int height;
};

struct tinyui_table_props {
    const char *id;
    /* Sentinel default: 0 = unset (no keyboard binding applied) */
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
};

struct tinyui_table *tinyui_table_create(struct tinyui_window *parent,
                                         const char *id,
                                         int rows,
                                         int columns);

struct tinyui_table *tinyui_table_create_with_props(struct tinyui_window *parent,
                                                    const struct tinyui_table_props *props);

int tinyui_table_set_keyboard(struct tinyui_table *table, unsigned int keyboard_binding);

int tinyui_table_set_keyboard_binding(struct tinyui_table *table, unsigned int keyboard_binding);

int tinyui_table_get_keyboard_binding(const struct tinyui_table *table, unsigned int *keyboard_binding);

int tinyui_table_show_keyboard(struct tinyui_table *table);

int tinyui_tabel_show_keyboard(struct tinyui_table *table);

int tinyui_table_set_item_text(struct tinyui_table *table,
                               int row,
                               int column,
                               const char *text);

int tinyui_table_set_cell_text(struct tinyui_table *table,
                               int row,
                               int column,
                               const char *text);

const char *tinyui_table_get_item_text(const struct tinyui_table *table, int row, int column);

const char *tinyui_table_get_cell_text(const struct tinyui_table *table, int row, int column);

int tinyui_table_set_item_editable(struct tinyui_table *table,
                                   int row,
                                   int column,
                                   int editable,
                                   unsigned int text_max);

int tinyui_table_set_cell_editable(struct tinyui_table *table,
                                   int row,
                                   int column,
                                   int editable,
                                   unsigned int text_max);

int tinyui_table_set_item_image(struct tinyui_table *table,
                                int row,
                                int column,
                                int x,
                                int y,
                                struct tinyui_image_source *source,
                                unsigned int mask_color);

int tinyui_table_set_item_button(struct tinyui_table *table,
                                 int row,
                                 int column,
                                 int x,
                                 int y,
                                 struct tinyui_image_source *release_source,
                                 unsigned int release_mask_color,
                                 struct tinyui_image_source *press_source,
                                 unsigned int press_mask_color,
                                 int checkable);

int tinyui_table_set_background_color(struct tinyui_table *table, unsigned int bg_color);

int tinyui_table_set_excel_type(struct tinyui_table *table);

int tinyui_table_set_align_grid(struct tinyui_table *table, int enabled);

int tinyui_table_set_item_width(struct tinyui_table *table, int column, int width);

int tinyui_table_set_item_height(struct tinyui_table *table, int row, int height);

int tinyui_table_set_item_color(struct tinyui_table *table,
                                int row,
                                int column,
                                unsigned int text_color,
                                unsigned int bg_color);

int tinyui_table_set_bg_color(struct tinyui_table *table, unsigned int bg_color);

int tinyui_table_set_item_static_text(struct tinyui_table *table, int row, int column, const char *text);

int tinyui_table_set_item_font(struct tinyui_table *table, int row, int column);

int tinyui_table_get_align_grid(const struct tinyui_table *table);

unsigned int tinyui_table_get_background_color(const struct tinyui_table *table);

void *tinyui_table_get_item(const struct tinyui_table *table, int row, int column);

int tinyui_table_set_item_align(struct tinyui_table *table,
                                int row,
                                int column,
                                enum tinyui_align align);

int tinyui_table_get_item_align(const struct tinyui_table *table, int row, int column);

int tinyui_table_get_item_editable(const struct tinyui_table *table, int row, int column);

void *tinyui_table_get_item_font(const struct tinyui_table *table, int row, int column);

int tinyui_table_get_item_height(const struct tinyui_table *table, int row);

unsigned int tinyui_table_get_item_text_color(const struct tinyui_table *table, int row, int column);

unsigned int tinyui_table_get_item_background_color(const struct tinyui_table *table, int row, int column);

int tinyui_table_get_item_width(const struct tinyui_table *table, int column);

int tinyui_table_navigate(struct tinyui_table *table, enum tinyui_native_nav_dir dir);

int tinyui_table_set_item_select(struct tinyui_table *table, int row, int column, int selected);

struct tinyui_table_region tinyui_table_get_item_region(const struct tinyui_table *table,
                                                        int row,
                                                        int column);

int tinyui_table_set_selected_cell(struct tinyui_table *table, int row, int column);

int tinyui_table_set_current_cell(struct tinyui_table *table, int row, int column);

int tinyui_table_get_current_row(const struct tinyui_table *table);

int tinyui_table_get_current_column(const struct tinyui_table *table);

#endif
