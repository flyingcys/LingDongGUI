#include "backend.h"
#include "internal.h"
#include "ldBase.h"
#include "ldTable.h"

#include <stdlib.h>

extern const arm_2d_a1_font_t ARM_2D_FONT_6x8;

static struct picoui_backend_app_state *picoui_backend_table_get_app_state(void *parent)
{
    struct picoui_backend_widget *parent_widget = parent;

    if (parent_widget == NULL || parent_widget->owner == NULL || parent_widget->owner->backend_app == NULL) {
        return NULL;
    }
    return (struct picoui_backend_app_state *)parent_widget->owner->backend_app;
}

static ldTable_t *picoui_backend_table_get_ld(void *backend_widget)
{
    struct picoui_backend_widget *widget = backend_widget;

    if (widget == NULL || widget->ld_widget == NULL) {
        return NULL;
    }

    return (ldTable_t *)widget->ld_widget;
}

static int picoui_backend_table_sync_host_current_cell(struct picoui_backend_widget *backend,
                                                       int *row_out,
                                                       int *column_out)
{
    struct picoui_table *table;
    ldTable_t *ld_table;
    int row;
    int column;

    if (backend == NULL || backend->host_widget == NULL) {
        return -1;
    }

    ld_table = picoui_backend_table_get_ld(backend);
    if (ld_table == NULL) {
        return -1;
    }

    row = (int)ld_table->currentRow;
    column = (int)ld_table->currentColumn;
    table = (struct picoui_table *)backend->host_widget;
    table->current_row = row;
    table->current_column = column;
    if (row_out != NULL) {
        *row_out = row;
    }
    if (column_out != NULL) {
        *column_out = column;
    }
    return 0;
}

static bool picoui_backend_table_native_slot(struct ld_scene_t *scene, ldMsg_t msg)
{
    struct picoui_backend_widget *backend;
    struct picoui_table *table;
    ldTable_t *ld_table;
    ldTableItem_t *item;
    int row = 0;
    int column = 0;
    int was_focus_owner = 0;

    (void)scene;

    if (msg.ptSender == NULL) {
        return false;
    }

    backend = (struct picoui_backend_widget *)((ldBase_t *)msg.ptSender)->pInfo;
    if (backend == NULL || backend->host_widget == NULL) {
        return false;
    }

    table = (struct picoui_table *)backend->host_widget;
    ld_table = picoui_backend_table_get_ld(backend);
    if (ld_table == NULL) {
        return false;
    }

    (void)picoui_backend_table_sync_host_current_cell(backend, &row, &column);
    item = ldTableGetItem(ld_table, (uint8_t)row, (uint8_t)column);

    if (msg.signal == SIGNAL_PRESS) {
        was_focus_owner = picoui_widget_is_focus_owner(&table->widget);
        (void)picoui_backend_widget_claim_focus(backend);
        if (item != NULL && item->isEditable && (item->isEditing || was_focus_owner)) {
            backend->edit_result_on_finish = PICOUI_EDIT_RESULT_COMMIT;
            (void)picoui_widget_claim_editing(&table->widget);
        }
        return false;
    }

    if (msg.signal != SIGNAL_FINISHED) {
        return false;
    }

    if (item != NULL) {
        item->isEditing = false;
    }
    (void)picoui_widget_mark_edit_result(&table->widget, backend->edit_result_on_finish);
    (void)picoui_widget_release_editing(&table->widget);
    backend->edit_result_on_finish = PICOUI_EDIT_RESULT_NONE;
    return false;
}

void *picoui_backend_create_table(void *parent, const char *id, int rows, int columns)
{
    struct picoui_backend_widget *widget;
    struct picoui_backend_widget *parent_widget = parent;
    struct picoui_backend_app_state *app_state;
    ldTable_t *ld_table;
    uint16_t name_id;

    if (parent == 0 || id == 0 || rows <= 0 || columns <= 0 || rows > 255 || columns > 255) {
        return 0;
    }

    app_state = picoui_backend_table_get_app_state(parent);
    if (app_state == NULL || app_state->ld_scene == NULL || parent_widget->ld_widget == NULL) {
        return 0;
    }

    widget = calloc(1, sizeof(*widget));
    if (widget == 0) {
        return 0;
    }

    name_id = ++app_state->next_ld_name_id;
    ld_table = ldTable_init(app_state->ld_scene,
                            NULL,
                            name_id,
                            parent_widget->ld_name_id,
                            0,
                            0,
                            220,
                            120,
                            (uint8_t)rows,
                            (uint8_t)columns,
                            4);
    if (ld_table == NULL) {
        free(widget);
        return 0;
    }

    widget->parent = parent;
    widget->id = id;
    widget->kind = PICOUI_BACKEND_WIDGET_TABLE;
    widget->theme = parent_widget->theme;
    widget->ld_widget = ld_table;
    widget->ld_name_id = name_id;
    if (picoui_backend_widget_attach_child(parent, widget) != 0) {
        free(widget);
        return 0;
    }
    return widget;
}

int picoui_backend_table_set_keyboard_binding(void *backend_widget, unsigned int keyboard_binding)
{
    ldTable_t *ld_table = picoui_backend_table_get_ld(backend_widget);

    if (ld_table == NULL || keyboard_binding == 0U || keyboard_binding > 0xFFFFU) {
        return -1;
    }

    ldTableSetKeyboard(ld_table, (uint16_t)keyboard_binding);
    return 0;
}

int picoui_backend_table_get_keyboard_binding(void *backend_widget, unsigned int *keyboard_binding)
{
    ldTable_t *ld_table = picoui_backend_table_get_ld(backend_widget);

    if (ld_table == NULL || keyboard_binding == NULL) {
        return -1;
    }

    *keyboard_binding = (unsigned int)ld_table->kbNameId;
    return 0;
}

int picoui_backend_table_set_cell_text(void *backend_widget, int row, int column, const char *text)
{
    ldTable_t *ld_table = picoui_backend_table_get_ld(backend_widget);
    ldTableItem_t *item;

    if (ld_table == NULL || text == NULL || row < 0 || column < 0 ||
        row >= ld_table->rowCount || column >= ld_table->columnCount) {
        return -1;
    }

    item = ldTableGetItem(ld_table, (uint8_t)row, (uint8_t)column);
    if (item != NULL && item->ptFont == NULL) {
        ldTableSetItemFont(ld_table, (uint8_t)row, (uint8_t)column, (arm_2d_font_t *)&ARM_2D_FONT_6x8);
    }
    ldTableSetItemText(ld_table, (uint8_t)row, (uint8_t)column, (uint8_t *)text);
    return 0;
}

const char *picoui_backend_table_get_cell_text(void *backend_widget, int row, int column)
{
    ldTable_t *ld_table = picoui_backend_table_get_ld(backend_widget);

    if (ld_table == NULL || row < 0 || column < 0 ||
        row >= ld_table->rowCount || column >= ld_table->columnCount) {
        return NULL;
    }

    return (const char *)ldTableGetItemText(ld_table, (uint8_t)row, (uint8_t)column);
}

int picoui_backend_table_set_cell_editable(void *backend_widget,
                                           int row,
                                           int column,
                                           int editable,
                                           unsigned int text_max)
{
    ldTable_t *ld_table = picoui_backend_table_get_ld(backend_widget);

    if (ld_table == NULL || row < 0 || column < 0 ||
        row >= ld_table->rowCount || column >= ld_table->columnCount ||
        text_max > 255U) {
        return -1;
    }

    ldTableSetItemEditable(ld_table,
                           (uint8_t)row,
                           (uint8_t)column,
                           editable != 0,
                           (uint8_t)text_max);
    if (editable != 0) {
        ldTableSetItemFont(ld_table, (uint8_t)row, (uint8_t)column, (arm_2d_font_t *)&ARM_2D_FONT_6x8);
    }
    return 0;
}

int picoui_backend_table_set_current_cell(void *backend_widget, int row, int column)
{
    struct picoui_backend_widget *backend = backend_widget;
    ldTable_t *ld_table = picoui_backend_table_get_ld(backend_widget);

    if (backend == NULL || ld_table == NULL || row < 0 || column < 0 ||
        row >= ld_table->rowCount || column >= ld_table->columnCount) {
        return -1;
    }

    ldTableSetItemSelect(ld_table, (uint8_t)row, (uint8_t)column, true);
    return picoui_backend_table_sync_host_current_cell(backend, NULL, NULL);
}

int picoui_backend_table_sync_current_cell(struct picoui_table *table, int *row_out, int *column_out)
{
    struct picoui_backend_widget *backend;

    if (table == NULL || table->widget.backend_widget == NULL) {
        return -1;
    }

    backend = (struct picoui_backend_widget *)table->widget.backend_widget;
    return picoui_backend_table_sync_host_current_cell(backend, row_out, column_out);
}

int picoui_backend_table_bind_host(void *backend_widget)
{
    struct picoui_backend_widget *backend = backend_widget;
    ldTable_t *ld_table;

    if (backend == NULL) {
        return -1;
    }

    ld_table = picoui_backend_table_get_ld(backend_widget);
    if (ld_table == NULL) {
        return -1;
    }

    if (!ldMsgConnect(ld_table, SIGNAL_PRESS, picoui_backend_table_native_slot)) {
        return -1;
    }
    if (!ldMsgConnect(ld_table, SIGNAL_FINISHED, picoui_backend_table_native_slot)) {
        return -1;
    }
    return 0;
}
