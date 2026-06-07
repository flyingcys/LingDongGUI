#include "picoui/picoui.h"
#include "internal.h"
#include "../../../src/gui/ldTable.h"

#include <assert.h>
#include <string.h>

int picoui_native_event_bind_root(struct picoui_screen *screen, struct picoui_window *root_window);
int picoui_native_table_get_rendered_state(const struct picoui_table *table,
                                           int *row_count,
                                           int *column_count,
                                           int *current_row,
                                           int *current_column);
int picoui_native_table_get_rendered_cell_text(const struct picoui_table *table,
                                               int row,
                                               int column,
                                               const char **text);
static int g_callback_count = 0;
static int g_callback_row = -1;
static int g_callback_column = -1;
static void *g_callback_user_data = 0;
static struct picoui_table *g_callback_table = 0;

static void on_table_selected(struct picoui_table *table, int row, int column, void *user_data)
{
    g_callback_count++;
    g_callback_row = row;
    g_callback_column = column;
    g_callback_user_data = user_data;
    g_callback_table = table;
}

int main(void)
{
    struct picoui_display *display;
    struct picoui_screen *screen;
    struct picoui_window *window;
    struct picoui_table *table;
    struct picoui_backend_widget *backend;
    ldTable_t *ld_table;
    arm_2d_region_t item_region;
    struct picoui_point origin;
    struct picoui_app *app;
    int row_count = -1;
    int column_count = -1;
    int current_row = -1;
    int current_column = -1;
    const char *cell_text = 0;
    int cell_x = -1;
    int cell_y = -1;
    int next_cell_x = -1;
    int next_cell_y = -1;
    int callback_cookie = 73;
    int rc;

    g_callback_count = 0;
    g_callback_row = -1;
    g_callback_column = -1;
    g_callback_user_data = 0;
    g_callback_table = 0;

    assert(picoui_init() == 0);

    display = picoui_display_create(320, 480);
    assert(display != 0);
    assert(picoui_display_set_default(display) == 0);

    screen = picoui_screen_active();
    assert(screen != 0);

    window = picoui_window_create_root(screen, "root");
    assert(window != 0);

    table = picoui_table_create(window, "settings_table", 3, 3);
    assert(table != 0);
    assert(picoui_widget_set_pos((struct picoui_widget *)table, 20, 40) == 0);
    assert(picoui_widget_set_size((struct picoui_widget *)table, 240, 150) == 0);

    assert(picoui_table_set_cell_text(table, 0, 0, "A1") == 0);
    assert(picoui_table_set_cell_text(table, 1, 1, "B2") == 0);
    assert(picoui_table_set_cell_text(table, 1, 2, "C2") == 0);
    assert(picoui_table_set_cell_text(table, 2, 2, "C3") == 0);
    picoui_table_set_on_selected(table, on_table_selected, &callback_cookie);
    assert(picoui_table_get_cell_text(table, 1, 1) != 0);
    assert(strcmp(picoui_table_get_cell_text(table, 1, 1), "B2") == 0);

    assert(picoui_table_set_selected_cell(table, 1, 1) == 0);
    assert(picoui_table_get_current_row(table) == 1);
    assert(picoui_table_get_current_column(table) == 1);
    assert(g_callback_count == 1);
    assert(g_callback_row == 1);
    assert(g_callback_column == 1);
    assert(g_callback_user_data == &callback_cookie);
    assert(g_callback_table == table);

    backend = (struct picoui_backend_widget *)table->widget.backend_widget;
    assert(backend != 0);
    assert(backend->kind == PICOUI_BACKEND_WIDGET_TABLE);
    ld_table = (ldTable_t *)backend->ld_widget;
    assert(ld_table != 0);
    app = backend->owner;
    assert(app != 0);

    assert(picoui_native_event_bind_root(screen, window) == 0);
    assert(picoui_native_table_get_rendered_state(table,
                                                  &row_count,
                                                  &column_count,
                                                  &current_row,
                                                  &current_column) == -1);
    assert(picoui_native_table_get_rendered_cell_text(table, 1, 1, &cell_text) == -1);
    rc = picoui_timer_handler();
    assert(rc == 0 || rc == 1);

    assert(picoui_native_table_get_rendered_state(table,
                                                  &row_count,
                                                  &column_count,
                                                  &current_row,
                                                  &current_column) == 0);
    assert(row_count == 3);
    assert(column_count == 3);
    assert(current_row == 1);
    assert(current_column == 1);

    assert(picoui_native_table_get_rendered_cell_text(table, 1, 1, &cell_text) == 0);
    assert(cell_text != 0);
    assert(strcmp(cell_text, "B2") == 0);

    origin = picoui_widget_get_absolute_pos((const struct picoui_widget *)table, (struct picoui_point){0, 0});
    item_region = ldTableGetItemRegion(ld_table, 1, 1);
    cell_x = origin.x + item_region.tLocation.iX + (item_region.tSize.iWidth / 2);
    cell_y = origin.y + item_region.tLocation.iY + (item_region.tSize.iHeight / 2);
    item_region = ldTableGetItemRegion(ld_table, 1, 2);
    next_cell_x = origin.x + item_region.tLocation.iX + (item_region.tSize.iWidth / 2);
    next_cell_y = origin.y + item_region.tLocation.iY + (item_region.tSize.iHeight / 2);
    assert(cell_x > 0);
    assert(cell_y > 0);
    assert(next_cell_x > cell_x);
    assert(next_cell_y > 0);

    assert(picoui_input_push_pointer(app, next_cell_x, next_cell_y, 1) == 0);
    rc = picoui_timer_handler();
    assert(rc == 0 || rc == 1);
    assert(picoui_input_push_pointer(app, next_cell_x, next_cell_y, 0) == 0);
    rc = picoui_timer_handler();
    assert(rc == 0 || rc == 1);

    assert(picoui_table_get_current_row(table) == 1);
    assert(picoui_table_get_current_column(table) == 2);
    assert(g_callback_count == 2);
    assert(g_callback_row == 1);
    assert(g_callback_column == 2);
    assert(backend->dispatch_count >= 0);
    assert(backend->value == ((((uint32_t)1) & 0xFFFFu) << 16 | (((uint32_t)2) & 0xFFFFu)));

    picoui_deinit();
    return 0;
}
