#include "../backend/ldgui/backend.h"
#include "../core/internal.h"

#define PICOUI_NATIVE_TABLE_RENDER_READY (1u << 28)

static struct picoui_backend_widget *picoui_native_table_backend(const struct picoui_table *table)
{
    if (table == 0 || table->widget.backend_widget == 0) {
        return 0;
    }

    return (struct picoui_backend_widget *)table->widget.backend_widget;
}

void picoui_native_table_reset_render_state(struct picoui_table *table)
{
    struct picoui_backend_widget *backend = picoui_native_table_backend(table);

    if (backend == 0) {
        return;
    }

    backend->runtime_evidence_flags &= ~PICOUI_NATIVE_TABLE_RENDER_READY;
}

int picoui_native_table_set_current_cell(struct picoui_table *table, int row, int column, int emit_callback)
{
    struct picoui_backend_widget *backend;

    if (table == 0 || row < 0 || column < 0 || row >= table->row_count || column >= table->column_count) {
        return -1;
    }

    table->current_row = row;
    table->current_column = column;

    backend = picoui_native_table_backend(table);
    if (backend == 0) {
        return -1;
    }

    backend->value = (((uint32_t)row & 0xFFFFu) << 16) | ((uint32_t)column & 0xFFFFu);
    backend->last_data_source = emit_callback ? PICOUI_BACKEND_DATA_SOURCE_NATIVE_EVENT
                                              : PICOUI_BACKEND_DATA_SOURCE_SETTER;
    backend->data_model_epoch++;
    picoui_native_table_reset_render_state(table);

    if (emit_callback && table->on_selected != 0) {
        table->on_selected(table, row, column, table->on_selected_user_data);
    }

    return 0;
}

int picoui_native_table_render(const struct picoui_backend_widget *backend)
{
    struct picoui_backend_widget *mutable_backend;
    struct picoui_table *table;
    int row;
    int column;

    if (backend == 0 || backend->kind != PICOUI_BACKEND_WIDGET_TABLE || backend->host_widget == 0) {
        return -1;
    }

    mutable_backend = (struct picoui_backend_widget *)backend;
    table = (struct picoui_table *)backend->host_widget;

    for (column = 0; column < table->column_count; ++column) {
        int width = picoui_table_get_item_width(table, column);

        if (width > 0 && picoui_backend_table_set_item_width(mutable_backend, column, width) != 0) {
            return -1;
        }
    }

    for (row = 0; row < table->row_count; ++row) {
        int height = picoui_table_get_item_height(table, row);

        if (height > 0 && picoui_backend_table_set_item_height(mutable_backend, row, height) != 0) {
            return -1;
        }
    }

    for (row = 0; row < table->row_count; ++row) {
        for (column = 0; column < table->column_count; ++column) {
            const char *text = picoui_table_get_cell_text(table, row, column);

            if (text != 0 && picoui_backend_table_set_cell_text(mutable_backend, row, column, text) != 0) {
                return -1;
            }
        }
    }

    if (picoui_backend_table_set_current_cell(mutable_backend, table->current_row, table->current_column) != 0) {
        return -1;
    }

    mutable_backend->runtime_evidence_flags |= PICOUI_NATIVE_TABLE_RENDER_READY;
    return 0;
}

int picoui_native_table_get_rendered_state(const struct picoui_table *table,
                                           int *row_count,
                                           int *column_count,
                                           int *current_row,
                                           int *current_column)
{
    const struct picoui_backend_widget *backend = picoui_native_table_backend(table);

    if (table == 0 || row_count == 0 || column_count == 0 || current_row == 0 || current_column == 0 ||
        backend == 0 || (backend->runtime_evidence_flags & PICOUI_NATIVE_TABLE_RENDER_READY) == 0) {
        return -1;
    }

    *row_count = table->row_count;
    *column_count = table->column_count;
    *current_row = table->current_row;
    *current_column = table->current_column;
    return 0;
}

int picoui_native_table_get_rendered_cell_text(const struct picoui_table *table,
                                               int row,
                                               int column,
                                               const char **text)
{
    const struct picoui_backend_widget *backend = picoui_native_table_backend(table);

    if (table == 0 || text == 0 ||
        row < 0 || column < 0 || row >= table->row_count || column >= table->column_count ||
        backend == 0 || (backend->runtime_evidence_flags & PICOUI_NATIVE_TABLE_RENDER_READY) == 0) {
        return -1;
    }

    *text = picoui_table_get_cell_text(table, row, column);
    return *text == 0 ? -1 : 0;
}

int picoui_native_table_get_item_center(const struct picoui_table *table,
                                        int row,
                                        int column,
                                        int *x,
                                        int *y)
{
    struct picoui_table_region region;
    struct picoui_point origin;
    const struct picoui_backend_widget *backend = picoui_native_table_backend(table);

    if (table == 0 || x == 0 || y == 0 ||
        row < 0 || column < 0 || row >= table->row_count || column >= table->column_count ||
        backend == 0 || (backend->runtime_evidence_flags & PICOUI_NATIVE_TABLE_RENDER_READY) == 0) {
        return -1;
    }

    region = picoui_table_get_item_region(table, row, column);
    if (region.width <= 0 || region.height <= 0) {
        return -1;
    }

    origin = picoui_widget_get_absolute_pos((const struct picoui_widget *)table, (struct picoui_point){0, 0});
    *x = origin.x + region.x + (region.width / 2);
    *y = origin.y + region.y + (region.height / 2);
    return 0;
}

int picoui_native_table_select_point(struct picoui_table *table, int x, int y)
{
    int row;
    int column;
    struct picoui_point origin;

    if (table == 0) {
        return -1;
    }

    origin = picoui_widget_get_absolute_pos((const struct picoui_widget *)table, (struct picoui_point){0, 0});
    for (row = 0; row < table->row_count; ++row) {
        for (column = 0; column < table->column_count; ++column) {
            struct picoui_table_region region = picoui_table_get_item_region(table, row, column);

            if (region.width <= 0 || region.height <= 0) {
                continue;
            }

            if (x >= origin.x + region.x
                && x < origin.x + region.x + region.width
                && y >= origin.y + region.y
                && y < origin.y + region.y + region.height) {
                return picoui_native_table_set_current_cell(table, row, column, 1);
            }
        }
    }

    return -1;
}
