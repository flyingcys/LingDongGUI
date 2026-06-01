#include "internal.h"
#include "picoui/table.h"

#include <stdlib.h>

static int picoui_table_dims_are_valid(int rows, int columns)
{
    return rows > 0 && rows <= 255 && columns > 0 && columns <= 255;
}

static int picoui_table_keyboard_binding_is_valid(unsigned int keyboard_binding)
{
    return keyboard_binding > 0U && keyboard_binding <= 0xFFFFU;
}

static int picoui_table_props_are_valid(const struct picoui_table_props *props)
{
    return props != 0 &&
           props->id != 0 &&
           picoui_table_dims_are_valid(props->rows, props->columns) &&
           props->width >= 0 &&
           props->height >= 0 &&
           props->radius >= 0 &&
           props->padding >= 0 &&
           (props->has_keyboard_binding == 0 ||
            picoui_table_keyboard_binding_is_valid(props->keyboard_binding));
}

struct picoui_table *picoui_table_create(struct picoui_window *parent,
                                         const char *id,
                                         int rows,
                                         int columns)
{
    struct picoui_table *table;

    if (parent == 0 || id == 0 || !picoui_table_dims_are_valid(rows, columns)) {
        return 0;
    }

    table = calloc(1, sizeof(*table));
    if (table == 0) {
        return 0;
    }

    table->widget.backend_widget =
        picoui_backend_create_table(parent->widget.backend_widget, id, rows, columns);
    if (table->widget.backend_widget == 0) {
        free(table);
        return 0;
    }

    table->id = id;
    table->row_count = rows;
    table->column_count = columns;
    table->current_row = 0;
    table->current_column = 0;
    table->widget.visible = 1;
    table->widget.enabled = 1;
    if (picoui_backend_widget_bind_host(table->widget.backend_widget, &table->widget) != 0) {
        free(table);
        return 0;
    }
    if (picoui_backend_table_bind_host(table->widget.backend_widget) != 0) {
        free(table);
        return 0;
    }
    return table;
}

struct picoui_table *picoui_table_create_with_props(struct picoui_window *parent,
                                                    const struct picoui_table_props *props)
{
    struct picoui_table *table;

    if (!picoui_table_props_are_valid(props)) {
        return 0;
    }

    table = picoui_table_create(parent, props->id, props->rows, props->columns);
    if (table == 0) {
        return 0;
    }

    if ((props->has_keyboard_binding != 0 &&
         picoui_table_set_keyboard_binding(table, props->keyboard_binding) != 0) ||
        picoui_widget_set_user_data(&table->widget, props->user_data) != 0 ||
        picoui_widget_set_bg_color(&table->widget, props->bg_color) != 0 ||
        picoui_widget_set_text_color(&table->widget, props->text_color) != 0 ||
        picoui_widget_set_border_color(&table->widget, props->border_color) != 0 ||
        picoui_widget_set_radius(&table->widget, props->radius) != 0 ||
        picoui_widget_set_padding(&table->widget, props->padding) != 0) {
        free(table);
        return 0;
    }
    if (props->style_class != 0 &&
        picoui_widget_set_style_class(&table->widget, props->style_class) != 0) {
        free(table);
        return 0;
    }
    if ((props->width > 0 || props->height > 0) &&
        picoui_widget_set_size(&table->widget, props->width, props->height) != 0) {
        free(table);
        return 0;
    }

    return table;
}

int picoui_table_set_keyboard_binding(struct picoui_table *table, unsigned int keyboard_binding)
{
    if (table == 0 || !picoui_table_keyboard_binding_is_valid(keyboard_binding)) {
        return -1;
    }

    if (picoui_backend_table_set_keyboard_binding(table->widget.backend_widget,
                                                  keyboard_binding) != 0) {
        return -1;
    }
    table->keyboard_binding = keyboard_binding;
    return 0;
}

int picoui_table_get_keyboard_binding(const struct picoui_table *table, unsigned int *keyboard_binding)
{
    if (table == 0 || keyboard_binding == 0) {
        return -1;
    }

    return picoui_backend_table_get_keyboard_binding((void *)table->widget.backend_widget,
                                                     keyboard_binding);
}

int picoui_table_set_cell_text(struct picoui_table *table,
                               int row,
                               int column,
                               const char *text)
{
    if (table == 0 || text == 0) {
        return -1;
    }

    return picoui_backend_table_set_cell_text(table->widget.backend_widget, row, column, text);
}

const char *picoui_table_get_cell_text(const struct picoui_table *table, int row, int column)
{
    if (table == 0) {
        return 0;
    }

    return picoui_backend_table_get_cell_text((void *)table->widget.backend_widget, row, column);
}

int picoui_table_set_cell_editable(struct picoui_table *table,
                                   int row,
                                   int column,
                                   int editable,
                                   unsigned int text_max)
{
    if (table == 0) {
        return -1;
    }

    return picoui_backend_table_set_cell_editable(table->widget.backend_widget,
                                                  row,
                                                  column,
                                                  editable != 0,
                                                  text_max);
}

int picoui_table_set_item_image(struct picoui_table *table,
                                int row,
                                int column,
                                int x,
                                int y,
                                struct picoui_image_source *source,
                                unsigned int mask_color)
{
    if (table == 0 || source == 0 || source->img_tile == 0 || source->mask_tile == 0 ||
        mask_color > 0xFFFFFFU) {
        return -1;
    }

    return picoui_backend_table_set_item_image(table->widget.backend_widget,
                                               row,
                                               column,
                                               x,
                                               y,
                                               source,
                                               mask_color);
}

int picoui_table_set_item_button(struct picoui_table *table,
                                 int row,
                                 int column,
                                 int x,
                                 int y,
                                 struct picoui_image_source *release_source,
                                 unsigned int release_mask_color,
                                 struct picoui_image_source *press_source,
                                 unsigned int press_mask_color,
                                 int checkable)
{
    if (table == 0 || release_source == 0 || press_source == 0 ||
        release_source->img_tile == 0 || release_source->mask_tile == 0 ||
        press_source->img_tile == 0 || press_source->mask_tile == 0 ||
        release_mask_color > 0xFFFFFFU || press_mask_color > 0xFFFFFFU) {
        return -1;
    }

    return picoui_backend_table_set_item_button(table->widget.backend_widget,
                                                row,
                                                column,
                                                x,
                                                y,
                                                release_source,
                                                release_mask_color,
                                                press_source,
                                                press_mask_color,
                                                checkable != 0);
}

int picoui_table_set_excel_type(struct picoui_table *table)
{
    if (table == 0) {
        return -1;
    }

    return picoui_backend_table_set_excel_type(table->widget.backend_widget);
}

int picoui_table_set_item_width(struct picoui_table *table, int column, int width)
{
    if (table == 0 || width <= 0) {
        return -1;
    }

    return picoui_backend_table_set_item_width(table->widget.backend_widget, column, width);
}

int picoui_table_set_item_height(struct picoui_table *table, int row, int height)
{
    if (table == 0 || height <= 0) {
        return -1;
    }

    return picoui_backend_table_set_item_height(table->widget.backend_widget, row, height);
}

int picoui_table_set_item_color(struct picoui_table *table,
                                int row,
                                int column,
                                unsigned int text_color,
                                unsigned int bg_color)
{
    if (table == 0 || text_color > 0xFFFFFFU || bg_color > 0xFFFFFFU) {
        return -1;
    }

    return picoui_backend_table_set_item_color(table->widget.backend_widget,
                                               row,
                                               column,
                                               text_color,
                                               bg_color);
}

int picoui_table_set_bg_color(struct picoui_table *table, unsigned int bg_color)
{
    if (table == 0 || bg_color > 0xFFFFFFU) {
        return -1;
    }

    return picoui_backend_table_set_bg_color(table->widget.backend_widget, bg_color);
}

int picoui_table_set_item_static_text(struct picoui_table *table, int row, int column, const char *text)
{
    if (table == 0 || text == 0) {
        return -1;
    }

    return picoui_backend_table_set_item_static_text(table->widget.backend_widget, row, column, text);
}

int picoui_table_set_item_font(struct picoui_table *table, int row, int column)
{
    if (table == 0) {
        return -1;
    }

    return picoui_backend_table_set_item_font(table->widget.backend_widget, row, column);
}

int picoui_table_set_item_align(struct picoui_table *table,
                                int row,
                                int column,
                                enum picoui_align align)
{
    if (table == 0) {
        return -1;
    }

    return picoui_backend_table_set_item_align(table->widget.backend_widget, row, column, align);
}

int picoui_table_get_item_align(const struct picoui_table *table, int row, int column)
{
    if (table == 0) {
        return -1;
    }

    return picoui_backend_table_get_item_align((void *)table->widget.backend_widget, row, column);
}

int picoui_table_get_item_editable(const struct picoui_table *table, int row, int column)
{
    if (table == 0) {
        return -1;
    }

    return picoui_backend_table_get_item_editable((void *)table->widget.backend_widget, row, column);
}

int picoui_table_navigate(struct picoui_table *table, enum picoui_native_nav_dir dir)
{
    if (table == 0 ||
        (dir != PICOUI_NATIVE_NAV_LEFT &&
         dir != PICOUI_NATIVE_NAV_RIGHT &&
         dir != PICOUI_NATIVE_NAV_UP &&
         dir != PICOUI_NATIVE_NAV_DOWN)) {
        return -1;
    }

    return picoui_backend_table_navigate(table->widget.backend_widget, dir);
}

struct picoui_table_region picoui_table_get_item_region(const struct picoui_table *table, int row, int column)
{
    struct picoui_table_region region = {0};

    if (table == 0) {
        return region;
    }

    if (picoui_backend_table_get_item_region((void *)table->widget.backend_widget, row, column, &region) != 0) {
        struct picoui_table_region empty = {0};
        return empty;
    }
    return region;
}

int picoui_table_set_selected_cell(struct picoui_table *table, int row, int column)
{
    if (table == 0) {
        return -1;
    }

    if (picoui_backend_table_set_selected_cell(table->widget.backend_widget, row, column) != 0) {
        return -1;
    }
    table->current_row = row;
    table->current_column = column;
    return 0;
}

int picoui_table_set_current_cell(struct picoui_table *table, int row, int column)
{
    if (table == 0) {
        return -1;
    }

    if (picoui_backend_table_set_current_cell(table->widget.backend_widget, row, column) != 0) {
        return -1;
    }
    table->current_row = row;
    table->current_column = column;
    return 0;
}

int picoui_table_get_current_row(const struct picoui_table *table)
{
    int row;
    int column;

    if (table == 0) {
        return -1;
    }

    if (picoui_backend_table_sync_current_cell((struct picoui_table *)table, &row, &column) == 0) {
        return row;
    }
    return table->current_row;
}

int picoui_table_get_current_column(const struct picoui_table *table)
{
    int row;
    int column;

    if (table == 0) {
        return -1;
    }

    if (picoui_backend_table_sync_current_cell((struct picoui_table *)table, &row, &column) == 0) {
        return column;
    }
    return table->current_column;
}
