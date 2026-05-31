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
