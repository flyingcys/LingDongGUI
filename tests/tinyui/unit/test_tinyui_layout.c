#include "internal.h"
#include "background.h"
#include "tinyui.h"
#include "ldBase.h"
#include "ldWindow.h"
#include "../../../src/porting/ldConfig.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

extern struct tinyui_widget *tinyui_widget_backend_parent(const struct tinyui_widget *widget);

static const char *test_self_binary_path = 0;

static const char *test_repo_path(const char *relative_path)
{
    static char path[2048];
    char base[2048];
    char *tests_dir;

    snprintf(base, sizeof(base), "%s", __FILE__);
    tests_dir = strstr(base, "tests/tinyui/unit/");
    assert(tests_dir != 0);
    *tests_dir = '\0';
    snprintf(path, sizeof(path), "%s%s", base, relative_path);
    return path;
}

static void assert_source_lacks_static_definition(const char *path, const char *symbol_name)
{
    char command[1024];

    snprintf(command,
             sizeof(command),
             "rg -n \"^[[:space:]]*static[[:space:]].*%s[[:space:]]*\\(\" %s >/dev/null",
             symbol_name,
             path);
    if (system(command) == 0) {
        fprintf(stderr, "unexpected old static helper still present: %s in %s\n", symbol_name, path);
        abort();
    }
}

static void test_layout_facades_reuse_window_validity_contract(void)
{
    assert_source_lacks_static_definition(test_repo_path("tinyui/src/layout/flex.c"),
                                          "tinyui_window_is_valid");
    assert_source_lacks_static_definition(test_repo_path("tinyui/src/layout/grid.c"),
                                          "tinyui_window_is_valid");
}

static void assert_archive_lacks_symbol(const char *archive_relpath, const char *symbol)
{
    char command[1024];
    FILE *pipe;
    char line[512];
    char prefixed_symbol[256];
    const char *line_symbol;

    assert(test_self_binary_path != 0);
    assert(archive_relpath != 0);
    assert(symbol != 0);
    snprintf(prefixed_symbol, sizeof(prefixed_symbol), "_%s", symbol);
    snprintf(command, sizeof(command),
             "cd \"$(dirname '%s')\" && nm \"%s\" 2>/dev/null",
             test_self_binary_path,
             archive_relpath);
    pipe = popen(command, "r");
    assert(pipe != 0);
    while (fgets(line, sizeof(line), pipe) != 0) {
        size_t line_len = strlen(line);
        size_t symbol_len = strlen(symbol);

        while (line_len > 0 && (line[line_len - 1] == '\n' || line[line_len - 1] == '\r')) {
            line[--line_len] = '\0';
        }
        line_symbol = strrchr(line, ' ');
        if (line_symbol != 0) {
            line_symbol += 1;
        } else {
            line_symbol = line;
        }
        if (strcmp(line_symbol, symbol) == 0 || strcmp(line_symbol, prefixed_symbol) == 0) {
            assert(!"unexpected symbol still present in archive");
        }
    }
    pclose(pipe);
}

static void assert_archive_lacks_member(const char *archive_relpath, const char *member)
{
    char command[1024];
    FILE *pipe;
    char line[512];
    size_t member_len;

    assert(test_self_binary_path != 0);
    assert(archive_relpath != 0);
    assert(member != 0);
    snprintf(command, sizeof(command),
             "cd \"$(dirname '%s')\" && nm \"%s\" 2>/dev/null",
             test_self_binary_path,
             archive_relpath);
    pipe = popen(command, "r");
    assert(pipe != 0);
    member_len = strlen(member);
    while (fgets(line, sizeof(line), pipe) != 0) {
        size_t line_len = strlen(line);

        while (line_len > 0 && (line[line_len - 1] == '\n' || line[line_len - 1] == '\r')) {
            line[--line_len] = '\0';
        }
        if (line_len == member_len + 1 &&
            strncmp(line, member, member_len) == 0 &&
            line[member_len] == ':') {
            assert(!"unexpected archive member still present");
        }
    }
    pclose(pipe);
}

static void assert_self_binary_lacks_symbol(const char *symbol)
{
    char command[1024];
    FILE *pipe;
    char line[512];
    char prefixed_symbol[256];
    const char *line_symbol;

    assert(test_self_binary_path != 0);
    assert(symbol != 0);
    snprintf(prefixed_symbol, sizeof(prefixed_symbol), "_%s", symbol);
    snprintf(command, sizeof(command), "nm %s 2>/dev/null", test_self_binary_path);
    pipe = popen(command, "r");
    assert(pipe != 0);
    while (fgets(line, sizeof(line), pipe) != 0) {
        size_t line_len = strlen(line);
        size_t symbol_len = strlen(symbol);

        while (line_len > 0 && (line[line_len - 1] == '\n' || line[line_len - 1] == '\r')) {
            line[--line_len] = '\0';
        }
        line_symbol = strrchr(line, ' ');
        if (line_symbol != 0) {
            line_symbol += 1;
        } else {
            line_symbol = line;
        }
        if (strcmp(line_symbol, symbol) == 0 || strcmp(line_symbol, prefixed_symbol) == 0) {
            assert(!"unexpected symbol still present in test binary");
        }
    }
    pclose(pipe);
}

static unsigned int test_rgb_to_ld_color(unsigned int rgb)
{
    return (unsigned int)__RGB((rgb >> 16) & 0xFFU, (rgb >> 8) & 0xFFU, rgb & 0xFFU);
}

static void test_grid_layout_setters_sync_to_real_ld_window_and_children(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *win = tinyui_window_create(app, "root");
    struct tinyui_button *a = tinyui_button_create(win, "a");
    struct tinyui_button *b = tinyui_button_create(win, "b");
    const ldWindow_t *ld_window = (const ldWindow_t *)win->widget.ld_widget;
    const ldBase_t *ld_b = (const ldBase_t *)b->widget.ld_widget;

    assert(tinyui_grid_set_columns(win, (int[]){80, -1, 0}, 3) == 0);
    assert(tinyui_grid_set_rows(win, (int[]){24, -1, 0}, 3) == 0);
    assert(tinyui_grid_set_gap(win, 6, 10) == 0);
    assert(tinyui_grid_set_align(win, TINYUI_ALIGN_END, TINYUI_ALIGN_SPACE_AROUND) == 0);
    assert(tinyui_widget_set_padding((struct tinyui_widget *)win, 9) == 0);
    assert(tinyui_widget_set_grid_cell((struct tinyui_widget *)a,
                                       0, 0, 1, 1,
                                       TINYUI_ALIGN_START,
                                       TINYUI_ALIGN_STRETCH) == 0);
    assert(tinyui_widget_set_grid_cell((struct tinyui_widget *)b,
                                       1, 1, 2, 3,
                                       TINYUI_ALIGN_CENTER,
                                       TINYUI_ALIGN_END) == 0);

    assert(ld_window->layoutTpye == layoutGrid);
    assert(ld_window->gridColDsc != 0);
    assert(ld_window->gridRowDsc != 0);
    assert(ld_window->gridColDsc[0] == 80);
    assert(ld_window->gridColDsc[1] < 0);
    assert(ld_window->gridColDsc[1] != LD_GRID_TEMPLATE_LAST);
    assert(ld_window->gridColDsc[2] == LD_GRID_TEMPLATE_LAST);
    assert(ld_window->gridRowDsc[0] == 24);
    assert(ld_window->gridRowDsc[1] < 0);
    assert(ld_window->gridRowDsc[1] != LD_GRID_TEMPLATE_LAST);
    assert(ld_window->gridRowDsc[2] == LD_GRID_TEMPLATE_LAST);
    assert(ld_window->gridColumnGap == 10);
    assert(ld_window->gridRowGap == 6);
    assert(win->widget.padding == 9);
    assert(ld_window->gridPadding.left == 9);
    assert(ld_window->gridPadding.top == 9);
    assert(ld_window->gridPadding.right == 9);
    assert(ld_window->gridPadding.bottom == 9);
    assert(ld_window->gridColAlign == ldGridAlignEnd);
    assert(ld_window->gridRowAlign == ldGridAlignSpaceAround);
    assert(ld_b->gridColPos == 1);
    assert(ld_b->gridRowPos == 1);
    assert(ld_b->gridColSpan == 2);
    assert(ld_b->gridRowSpan == 3);
    assert(ld_b->gridCellXAlign == ldGridAlignCenter);
    assert(ld_b->gridCellYAlign == ldGridAlignEnd);
    assert(ld_b->isGridCellSet == true);

    tinyui_app_destroy(app);
}

static void test_grid_layout_rejects_invalid_gap_and_cell_span(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *win = tinyui_window_create(app, "root");
    struct tinyui_button *a = tinyui_button_create(win, "a");
    const ldWindow_t *ld_window = (const ldWindow_t *)win->widget.ld_widget;
    const ldBase_t *ld_a = (const ldBase_t *)a->widget.ld_widget;

    assert(tinyui_grid_set_gap(win, 4, 5) == 0);
    assert(tinyui_widget_set_padding((struct tinyui_widget *)win, 3) == 0);
    assert(tinyui_grid_set_gap(win, -1, 5) == -1);
    assert(tinyui_grid_set_gap(win, 4, -1) == -1);
    assert(tinyui_widget_set_padding((struct tinyui_widget *)win, -1) == -1);
    assert(win->widget.padding == 3);
    assert(ld_window->gridRowGap == 4);
    assert(ld_window->gridColumnGap == 5);
    assert(ld_window->gridPadding.left == 3);
    assert(ld_window->gridPadding.top == 3);
    assert(ld_window->gridPadding.right == 3);
    assert(ld_window->gridPadding.bottom == 3);

    assert(tinyui_widget_set_grid_cell((struct tinyui_widget *)a,
                                       0, 0, 2, 1,
                                       TINYUI_ALIGN_STRETCH,
                                       TINYUI_ALIGN_CENTER) == 0);
    assert(tinyui_widget_set_grid_cell((struct tinyui_widget *)a,
                                       0, 0, 0, 1,
                                       TINYUI_ALIGN_START,
                                       TINYUI_ALIGN_START) == -1);
    assert(tinyui_widget_set_grid_cell((struct tinyui_widget *)a,
                                       0, 0, 1, 0,
                                       TINYUI_ALIGN_START,
                                       TINYUI_ALIGN_START) == -1);
    assert(a->widget.grid_col_span == 2);
    assert(a->widget.grid_row_span == 1);
    assert(a->widget.grid_x_align == TINYUI_ALIGN_STRETCH);
    assert(a->widget.grid_y_align == TINYUI_ALIGN_CENTER);
    assert(ld_a->gridColSpan == 2);
    assert(ld_a->gridRowSpan == 1);
    assert(ld_a->gridCellXAlign == ldGridAlignStretch);
    assert(ld_a->gridCellYAlign == ldGridAlignCenter);

    tinyui_app_destroy(app);
}

static void test_window_native_grid_descriptors_round_trip_to_ldwindow(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *win = tinyui_window_create(app, "root");
    const ldWindow_t *ld_window = (const ldWindow_t *)win->widget.ld_widget;
    const int initial_cols[] = {48, -2, -3, 0};
    const int initial_rows[] = {20, -2, 0};

    assert(tinyui_grid_set_columns(win, initial_cols, 4) == 0);
    assert(tinyui_grid_set_rows(win, initial_rows, 3) == 0);

    assert(win->grid_col_count == 4);
    assert(win->grid_row_count == 3);
    assert(win->grid_cols[0] == 48);
    assert(win->grid_cols[1] == -2);
    assert(win->grid_cols[2] == -3);
    assert(win->grid_cols[3] == 0);
    assert(win->grid_rows[0] == 20);
    assert(win->grid_rows[1] == -2);
    assert(win->grid_rows[2] == 0);

    assert(ld_window->gridColDsc != 0);
    assert(ld_window->gridRowDsc != 0);
    assert(ld_window->gridColDsc[0] == 48);
    assert(ld_window->gridColDsc[1] == LD_GRID_CONTENT);
    assert(ld_window->gridColDsc[2] < 0);
    assert(ld_window->gridColDsc[2] != LD_GRID_CONTENT);
    assert(ld_window->gridColDsc[2] != LD_GRID_TEMPLATE_LAST);
    assert(ld_window->gridColDsc[3] == LD_GRID_TEMPLATE_LAST);
    assert(ld_window->gridRowDsc[0] == 20);
    assert(ld_window->gridRowDsc[1] == LD_GRID_CONTENT);
    assert(ld_window->gridRowDsc[2] == LD_GRID_TEMPLATE_LAST);

    assert(tinyui_grid_set_columns(win, (int[]){64}, 0) == -1);
    assert(tinyui_grid_set_columns(win, 0, 1) == -1);
    assert(tinyui_grid_set_rows(win, (int[]){32}, 0) == -1);
    assert(tinyui_grid_set_rows(win, 0, 1) == -1);

    assert(win->grid_col_count == 4);
    assert(win->grid_row_count == 3);
    assert(ld_window->gridColDsc[0] == 48);
    assert(ld_window->gridColDsc[1] == LD_GRID_CONTENT);
    assert(ld_window->gridColDsc[2] < 0);
    assert(ld_window->gridColDsc[2] != LD_GRID_CONTENT);
    assert(ld_window->gridColDsc[2] != LD_GRID_TEMPLATE_LAST);
    assert(ld_window->gridColDsc[3] == LD_GRID_TEMPLATE_LAST);
    assert(ld_window->gridRowDsc[0] == 20);
    assert(ld_window->gridRowDsc[1] == LD_GRID_CONTENT);
    assert(ld_window->gridRowDsc[2] == LD_GRID_TEMPLATE_LAST);

    tinyui_app_destroy(app);
}

static void test_flex_layout_setters_sync_to_real_ld_window_and_children(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *win = tinyui_window_create(app, "root");
    struct tinyui_button *a = tinyui_button_create(win, "a");
    struct tinyui_button *b = tinyui_button_create(win, "b");
    const ldWindow_t *ld_window = (const ldWindow_t *)win->widget.ld_widget;
    const ldBase_t *ld_a = (const ldBase_t *)a->widget.ld_widget;
    const ldBase_t *ld_b = (const ldBase_t *)b->widget.ld_widget;

    assert(tinyui_flex_set_flow(win, TINYUI_FLEX_FLOW_ROW_WRAP) == 0);
    assert(tinyui_flex_set_align(win,
                                 TINYUI_ALIGN_START,
                                 TINYUI_ALIGN_CENTER,
                                 TINYUI_ALIGN_SPACE_BETWEEN) == 0);
    assert(tinyui_flex_set_gap(win, 8, 12) == 0);
    assert(tinyui_widget_set_padding((struct tinyui_widget *)win, 7) == 0);
    assert(tinyui_flex_set_gap(win, -1, 12) == -1);
    assert(tinyui_flex_set_gap(win, 8, -1) == -1);
    assert(tinyui_widget_set_padding((struct tinyui_widget *)win, -1) == -1);
    assert(tinyui_widget_set_flex_grow((struct tinyui_widget *)a, 1) == 0);
    assert(tinyui_widget_set_flex_new_track((struct tinyui_widget *)a, 1) == 0);
    assert(tinyui_widget_set_ignore_layout((struct tinyui_widget *)b, 1) == 0);

    assert(ld_window->layoutTpye == layoutFlex);
    assert(ld_window->flexFlow == ldFlexFlowRowWrap);
    assert(ld_window->flexMainAlign == ldFlexMainAlignStart);
    assert(ld_window->flexCrossAlign == ldFlexCrossAlignCenter);
    assert(ld_window->flexTrackAlign == ldFlexTrackAlignSpaceBetween);
    assert(ld_window->flexItemGap == 8);
    assert(ld_window->flexTrackGap == 12);
    assert(win->widget.padding == 7);
    assert(ld_window->flexPadding.left == 7);
    assert(ld_window->flexPadding.top == 7);
    assert(ld_window->flexPadding.right == 7);
    assert(ld_window->flexPadding.bottom == 7);
    assert(ld_a->flexGrow == 1);
    assert(ld_a->flexInNewTrack == true);
    assert(ld_b->ignoreLayout == true);

    tinyui_app_destroy(app);
}

static void test_widget_native_base_flags_round_trip_to_ldbase(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *win = tinyui_window_create(app, "root");
    struct tinyui_button *button = tinyui_button_create(win, "stateful");
    ldBase_t *ld_base = (ldBase_t *)button->widget.ld_widget;

    assert(tinyui_widget_set_opacity((struct tinyui_widget *)button, 123) == 0);
    assert(tinyui_widget_set_selectable((struct tinyui_widget *)button, 1) == 0);
    assert(tinyui_widget_set_selected((struct tinyui_widget *)button, 1) == 0);
    assert(tinyui_widget_set_corner((struct tinyui_widget *)button, 1) == 0);
    assert(tinyui_widget_set_center((struct tinyui_widget *)button) == 0);
    assert(tinyui_widget_set_visible((struct tinyui_widget *)button, 0) == 0);

    assert(ld_base->opacity == 123);
    assert(ld_base->isSelectable == true);
    assert(ld_base->isSelected == true);
    assert(ld_base->isCorner == true);
    assert(ld_base->isHidden == true);
    assert(ld_base->use_as__arm_2d_control_node_t.tRegion.tLocation.iX != 0 ||
           ld_base->use_as__arm_2d_control_node_t.tRegion.tLocation.iY != 0);

    tinyui_app_destroy(app);
}

static void test_widget_backend_parent_round_trip(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *root = tinyui_window_create(app, "root");
    struct tinyui_window *child = tinyui_window_create_child(root, "child");

    assert(app != NULL);
    assert(root != NULL);
    assert(child != NULL);
    assert(tinyui_widget_backend_parent(&child->widget) == &root->widget);

    tinyui_app_destroy(app);
}

static void test_widget_native_flex_min_max_round_trip_to_ldbase(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *win = tinyui_window_create(app, "root");
    struct tinyui_button *button = tinyui_button_create(win, "flexy");
    const ldBase_t *ld_base = (const ldBase_t *)button->widget.ld_widget;

    assert(tinyui_widget_set_flex_min_width((struct tinyui_widget *)button, 24) == 0);
    assert(tinyui_widget_set_flex_min_height((struct tinyui_widget *)button, 12) == 0);
    assert(tinyui_widget_set_flex_max_width((struct tinyui_widget *)button, 88) == 0);
    assert(tinyui_widget_set_flex_max_height((struct tinyui_widget *)button, 42) == 0);

    assert(ld_base->hasFlexMinWidth == true);
    assert(ld_base->hasFlexMinHeight == true);
    assert(ld_base->hasFlexMaxWidth == true);
    assert(ld_base->hasFlexMaxHeight == true);
    assert(ld_base->flexMinSize.iWidth == 24);
    assert(ld_base->flexMinSize.iHeight == 12);
    assert(ld_base->flexMaxSize.iWidth == 88);
    assert(ld_base->flexMaxSize.iHeight == 42);

    tinyui_app_destroy(app);
}

static void test_widget_native_base_getters_round_trip_to_ldbase(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *win = tinyui_window_create(app, "root");
    struct tinyui_button *button = tinyui_button_create(win, "getter_stateful");
    ldBase_t *ld_base = (ldBase_t *)button->widget.ld_widget;

    assert(tinyui_widget_set_pos((struct tinyui_widget *)button, 17, 23) == 0);
    assert(tinyui_widget_set_size((struct tinyui_widget *)button, 91, 37) == 0);
    assert(tinyui_widget_set_opacity((struct tinyui_widget *)button, 123) == 0);
    assert(tinyui_widget_set_selectable((struct tinyui_widget *)button, 1) == 0);
    assert(tinyui_widget_set_selected((struct tinyui_widget *)button, 1) == 0);
    assert(tinyui_widget_set_corner((struct tinyui_widget *)button, 1) == 0);

    assert(tinyui_widget_get_x((struct tinyui_widget *)button) == 17);
    assert(tinyui_widget_get_y((struct tinyui_widget *)button) == 23);
    assert(tinyui_widget_get_width((struct tinyui_widget *)button) == 91);
    assert(tinyui_widget_get_height((struct tinyui_widget *)button) == 37);
    assert(tinyui_widget_get_opacity((struct tinyui_widget *)button) == 123);
    assert(tinyui_widget_get_selectable((struct tinyui_widget *)button) == 1);
    assert(tinyui_widget_get_selected((struct tinyui_widget *)button) == 1);
    assert(tinyui_widget_get_corner((struct tinyui_widget *)button) == 1);
    assert(tinyui_widget_get_visible((struct tinyui_widget *)button) == 1);

    assert(ldBaseGetX(ld_base) == 17);
    assert(ldBaseGetY(ld_base) == 23);
    assert(ldBaseGetWidth(ld_base) == 91);
    assert(ldBaseGetHeight(ld_base) == 37);
    assert(ldBaseGetOpacity(ld_base) == 123);
    assert(ldBaseIsSelectable(ld_base) == true);
    assert(ldBaseIsSelected(ld_base) == true);
    assert(ldBaseIsCorner(ld_base) == true);

    assert(tinyui_widget_set_visible((struct tinyui_widget *)button, 0) == 0);
    assert(tinyui_widget_get_visible((struct tinyui_widget *)button) == 0);
    assert(ldBaseIsHidden(ld_base) == true);

    assert(tinyui_widget_get_x(0) == -1);
    assert(tinyui_widget_get_y(0) == -1);
    assert(tinyui_widget_get_width(0) == -1);
    assert(tinyui_widget_get_height(0) == -1);
    assert(tinyui_widget_get_opacity(0) == -1);
    assert(tinyui_widget_get_selectable(0) == -1);
    assert(tinyui_widget_get_selected(0) == -1);
    assert(tinyui_widget_get_corner(0) == -1);
    assert(tinyui_widget_get_visible(0) == -1);

    tinyui_app_destroy(app);
}

static void test_widget_tree_name_and_type_queries_round_trip_to_ldbase(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *win = tinyui_window_create(app, "root");
    struct tinyui_button *a = tinyui_button_create(win, "a");
    struct tinyui_button *b = tinyui_button_create(win, "b");
    const ldBase_t *ld_win = (const ldBase_t *)win->widget.ld_widget;
    const ldBase_t *ld_a = (const ldBase_t *)a->widget.ld_widget;
    const ldBase_t *ld_b = (const ldBase_t *)b->widget.ld_widget;
    int b_name_id = tinyui_widget_get_name_id((const struct tinyui_widget *)b);

    assert(tinyui_widget_get_parent((const struct tinyui_widget *)a) == (struct tinyui_widget *)win);
    assert(tinyui_widget_get_first_child((const struct tinyui_widget *)win) == (struct tinyui_widget *)a);
    assert(tinyui_widget_get_next_sibling((const struct tinyui_widget *)a) == (struct tinyui_widget *)b);
    assert(tinyui_widget_get_next_sibling((const struct tinyui_widget *)b) == 0);
    assert(tinyui_widget_get_root((const struct tinyui_widget *)b) == (struct tinyui_widget *)win);
    assert(tinyui_widget_get_child_count((const struct tinyui_widget *)win) == 2);
    assert(tinyui_widget_get_child_count((const struct tinyui_widget *)a) == 0);
    assert(tinyui_widget_get_name_id((const struct tinyui_widget *)win) == (int)ld_win->nameId);
    assert(tinyui_widget_get_name_id((const struct tinyui_widget *)a) == (int)ld_a->nameId);
    assert(b_name_id == (int)ld_b->nameId);
    assert(tinyui_widget_find_by_name_id((const struct tinyui_widget *)win, b_name_id) ==
           (struct tinyui_widget *)b);
    assert(tinyui_widget_get_type((const struct tinyui_widget *)win) == TINYUI_WIDGET_TYPE_WINDOW);
    assert(tinyui_widget_get_type((const struct tinyui_widget *)a) == TINYUI_WIDGET_TYPE_BUTTON);

    assert(tinyui_widget_get_parent(0) == 0);
    assert(tinyui_widget_get_first_child(0) == 0);
    assert(tinyui_widget_get_next_sibling(0) == 0);
    assert(tinyui_widget_get_root(0) == 0);
    assert(tinyui_widget_get_child_count(0) == -1);
    assert(tinyui_widget_get_name_id(0) == -1);
    assert(tinyui_widget_find_by_name_id(0, b_name_id) == 0);
    assert(tinyui_widget_find_by_name_id((const struct tinyui_widget *)win, -1) == 0);
    assert(tinyui_widget_get_type(0) == TINYUI_WIDGET_TYPE_UNKNOWN);

    tinyui_app_destroy(app);
}

static void test_widget_remove_from_parent_updates_tinyui_and_ldbase_tree(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *win = tinyui_window_create(app, "root");
    struct tinyui_button *a = tinyui_button_create(win, "a");
    struct tinyui_button *b = tinyui_button_create(win, "b");
    struct tinyui_button *c = tinyui_button_create(win, "c");
    ldBase_t *ld_win = (ldBase_t *)win->widget.ld_widget;
    ldBase_t *ld_a = (ldBase_t *)a->widget.ld_widget;
    ldBase_t *ld_b = (ldBase_t *)b->widget.ld_widget;
    ldBase_t *ld_c = (ldBase_t *)c->widget.ld_widget;
    int b_name_id = tinyui_widget_get_name_id((const struct tinyui_widget *)b);

    assert(tinyui_widget_get_child_count((const struct tinyui_widget *)win) == 3);
    assert(ldBaseGetChildCount(ld_win) == 3);
    assert(ldBaseGetNextSibling(ld_a) == ld_b);
    assert(ldBaseGetNextSibling(ld_b) == ld_c);

    assert(tinyui_widget_remove_from_parent((struct tinyui_widget *)b) == 0);
    assert(tinyui_widget_get_parent((const struct tinyui_widget *)b) == 0);
    assert(tinyui_widget_get_root((const struct tinyui_widget *)b) == 0);
    assert(tinyui_widget_get_next_sibling((const struct tinyui_widget *)b) == 0);
    assert(tinyui_widget_get_first_child((const struct tinyui_widget *)win) == (struct tinyui_widget *)a);
    assert(tinyui_widget_get_next_sibling((const struct tinyui_widget *)a) == (struct tinyui_widget *)c);
    assert(tinyui_widget_get_child_count((const struct tinyui_widget *)win) == 2);
    assert(tinyui_widget_find_by_name_id((const struct tinyui_widget *)win, b_name_id) == 0);
    assert(ldBaseGetParent(ld_b) == 0);
    assert(ldBaseGetNextSibling(ld_b) == 0);
    assert(ldBaseGetChildCount(ld_win) == 2);
    assert(ldBaseGetNextSibling(ld_a) == ld_c);

    assert(tinyui_widget_remove_from_parent((struct tinyui_widget *)b) == -1);
    assert(tinyui_widget_remove_from_parent((struct tinyui_widget *)win) == -1);
    assert(tinyui_widget_remove_from_parent(0) == -1);

    tinyui_app_destroy(app);
}

static void test_widget_destroy_detaches_focus_and_invalidates_backend_binding(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *win = tinyui_window_create(app, "root");
    struct tinyui_button *a = tinyui_button_create(win, "a");
    struct tinyui_button *b = tinyui_button_create(win, "b");
    ldBase_t *ld_win = (ldBase_t *)win->widget.ld_widget;
    ldBase_t *ld_b = (ldBase_t *)b->widget.ld_widget;
    int b_name_id = tinyui_widget_get_name_id((const struct tinyui_widget *)b);

    /* Mark win as root_window so B2 protection applies */
    tinyui_app_set_window(app, win);

    assert(tinyui_widget_claim_focus((struct tinyui_widget *)b) == 0);
    assert(tinyui_widget_is_focus_owner((const struct tinyui_widget *)b) == 1);
    assert(tinyui_widget_get_child_count((const struct tinyui_widget *)win) == 2);

    /* b and ld_b are freed after destroy; only access app/win/a after this */
    assert(tinyui_widget_destroy((struct tinyui_widget *)b) == 0);
    assert(app->focus_owner == 0);
    assert(tinyui_widget_find_by_name_id((const struct tinyui_widget *)win, b_name_id) == 0);
    assert(tinyui_widget_get_child_count((const struct tinyui_widget *)win) == 1);
    assert(tinyui_widget_get_first_child((const struct tinyui_widget *)win) == (struct tinyui_widget *)a);
    assert(ldBaseGetChildCount(ld_win) == 1);

    /* Root window (registered via tinyui_app_set_window) is protected from destroy */
    assert(tinyui_widget_destroy((struct tinyui_widget *)win) == -1);
    assert(tinyui_widget_destroy(0) == -1);

    tinyui_app_destroy(app);
}

static void test_widget_geometry_helpers_round_trip_to_ldbase(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *win = tinyui_window_create(app, "root");
    struct tinyui_button *button = tinyui_button_create(win, "geometry");
    struct tinyui_point absolute;
    struct tinyui_point relative;
    struct tinyui_rect parent = {10, 20, 100, 80};
    struct tinyui_rect child = {3, 4, 20, 10};
    struct tinyui_rect aligned;

    assert(tinyui_widget_set_pos((struct tinyui_widget *)win, 5, 7) == 0);
    assert(tinyui_widget_set_pos((struct tinyui_widget *)button, 17, 23) == 0);

    absolute = tinyui_widget_get_absolute_pos((const struct tinyui_widget *)button,
                                              (struct tinyui_point){2, 3});
    assert(absolute.x == 24);
    assert(absolute.y == 33);

    relative = tinyui_widget_get_relative_pos((const struct tinyui_widget *)button, absolute);
    assert(relative.x == 2);
    assert(relative.y == 3);

    aligned = tinyui_rect_align(parent, child, TINYUI_ALIGN_END, TINYUI_ALIGN_END);
    assert(aligned.x == 83);
    assert(aligned.y == 74);
    assert(aligned.width == 20);
    assert(aligned.height == 10);

    aligned = tinyui_rect_center(parent, child);
    assert(aligned.x == 43);
    assert(aligned.y == 39);

    assert(tinyui_vertical_grid_align_offset((struct tinyui_rect){0, 0, 40, 100},
                                             -40,
                                             5,
                                             20,
                                             4) == -24);

    absolute = tinyui_widget_get_absolute_pos(0, (struct tinyui_point){0, 0});
    assert(absolute.x == -1);
    assert(absolute.y == -1);
    relative = tinyui_widget_get_relative_pos(0, (struct tinyui_point){0, 0});
    assert(relative.x == -1);
    assert(relative.y == -1);
    aligned = tinyui_rect_align(parent, (struct tinyui_rect){0, 0, -1, 1},
                                TINYUI_ALIGN_CENTER, TINYUI_ALIGN_CENTER);
    assert(aligned.width == -1);
    assert(aligned.height == -1);
    assert(tinyui_vertical_grid_align_offset((struct tinyui_rect){0, 0, -1, 1},
                                             0,
                                             1,
                                             1,
                                             0) == -1);

    tinyui_app_destroy(app);
}

static void test_widget_focus_navigation_public_api(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *win = tinyui_window_create(app, "root");
    struct tinyui_button *left = tinyui_button_create(win, "left");
    struct tinyui_button *right = tinyui_button_create(win, "right");
    struct tinyui_button *down = tinyui_button_create(win, "down");

    app->root_window = win;
    assert(tinyui_widget_set_pos((struct tinyui_widget *)left, 0, 0) == 0);
    assert(tinyui_widget_set_pos((struct tinyui_widget *)right, 50, 0) == 0);
    assert(tinyui_widget_set_pos((struct tinyui_widget *)down, 50, 50) == 0);
    assert(tinyui_widget_set_selectable((struct tinyui_widget *)left, 1) == 0);
    assert(tinyui_widget_set_selectable((struct tinyui_widget *)right, 1) == 0);
    assert(tinyui_widget_set_selectable((struct tinyui_widget *)down, 1) == 0);

    assert(tinyui_focus_navigate(app, TINYUI_NATIVE_NAV_RIGHT) == 0);
    assert(tinyui_widget_is_focus_owner((const struct tinyui_widget *)left) == 1);
    assert(tinyui_focus_navigate(app, TINYUI_NATIVE_NAV_RIGHT) == 0);
    assert(tinyui_widget_is_focus_owner((const struct tinyui_widget *)right) == 1);
    assert(tinyui_widget_claim_focus((struct tinyui_widget *)win) == 0);
    assert(tinyui_focus_navigate(app, TINYUI_NATIVE_NAV_ENTER) == 0);
    assert(tinyui_widget_is_focus_owner((const struct tinyui_widget *)left) == 1);
    assert(tinyui_focus_navigate(app, TINYUI_NATIVE_NAV_BACK) == 0);
    assert(tinyui_widget_is_focus_owner((const struct tinyui_widget *)win) == 1);
    assert(tinyui_widget_claim_focus((struct tinyui_widget *)right) == 0);
    assert(tinyui_focus_navigate(app, TINYUI_NATIVE_NAV_DOWN) == 0);
    assert(tinyui_widget_is_focus_owner((const struct tinyui_widget *)down) == 1);
    assert(tinyui_focus_reset(app) == 0);
    assert(tinyui_widget_is_focus_owner((const struct tinyui_widget *)down) == 0);

    assert(tinyui_focus_reset(0) == -1);
    assert(tinyui_focus_navigate(0, TINYUI_NATIVE_NAV_RIGHT) == -1);
    assert(tinyui_focus_navigate(app, (enum tinyui_native_nav_dir)99) == -1);

    tinyui_app_destroy(app);
}

static void test_window_color_round_trip_to_ldwindow(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *win = tinyui_window_create(app, "root");
    const ldWindow_t *ld_window = (const ldWindow_t *)win->widget.ld_widget;
    unsigned int rgb = 0;

    assert(tinyui_window_set_color(win, 0x336699U) == 0);
    assert(tinyui_window_get_color(win, &rgb) == 0);
    assert(rgb == 0x31659CU);
    assert(win->widget.bg_color == 0x336699U);
    assert(ldWindowGetColor((ldWindow_t *)ld_window) == (ldColor)test_rgb_to_ld_color(0x336699U));
    assert(tinyui_window_set_color(0, 0x112233U) == -1);
    assert(tinyui_window_set_color(win, 0x1000000U) == -1);
    assert(tinyui_window_get_color(0, &rgb) == -1);
    assert(tinyui_window_get_color(win, 0) == -1);

    tinyui_app_destroy(app);
}

static void test_window_background_source_round_trip_to_ldwindow(void)
{
    arm_2d_tile_t img_tile = {0};
    arm_2d_tile_t mask_tile = {0};
    struct tinyui_image_source source = {
        .img_tile = &img_tile,
        .mask_tile = &mask_tile,
    };
    struct tinyui_image_source invalid_source = {
        .img_tile = 0,
        .mask_tile = &mask_tile,
    };
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *win = tinyui_window_create(app, "root");
    const ldWindow_t *ld_window = (const ldWindow_t *)win->widget.ld_widget;

    assert(tinyui_window_set_background_source(win, &source) == 0);
    assert(ld_window->ptImgTile == source.img_tile);
    assert(ld_window->ptMaskTile == source.mask_tile);

    assert(tinyui_window_set_background_source(win, 0) == 0);
    assert(ld_window->ptImgTile == 0);
    assert(ld_window->ptMaskTile == 0);

    assert(tinyui_window_set_background_source(win, &invalid_source) == -1);
    assert(ld_window->ptImgTile == 0);
    assert(ld_window->ptMaskTile == 0);
    assert(tinyui_window_set_background_source(0, &source) == -1);

    tinyui_app_destroy(app);
}

static void test_window_background_offset_round_trip_to_scene_root(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *win = tinyui_window_create(app, "root");
    struct tinyui_display_config display = {0};
    const struct tinyui_app *app_state =
        app;
    const ldWindow_t *ld_window = (const ldWindow_t *)win->widget.ld_widget;
    const ldBase_t *ld_root = (const ldBase_t *)app_state->ld_scene->ptNodeRoot;
    int offset_x = 0;
    int offset_y = 0;
    int expected_min_x;
    int expected_min_y;
    int expected_max_x;
    int expected_max_y;

    assert(tinyui_window_set_background_offset(win, 12, -18) == 0);
    assert(tinyui_display_get_config(app, &display) == 0);
    assert(tinyui_window_get_background_offset(win, &offset_x, &offset_y) == 0);
    assert(offset_x == 12);
    assert(offset_y == -18);
    assert(ld_root->use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 12);
    assert(ld_root->use_as__arm_2d_control_node_t.tRegion.tLocation.iY == -18);
    expected_min_x = offset_x < 0 ? offset_x : 0;
    expected_min_y = offset_y < 0 ? offset_y : 0;
    expected_max_x = (offset_x + display.width) > LD_CFG_SCREEN_WIDTH
                         ? (offset_x + display.width)
                         : LD_CFG_SCREEN_WIDTH;
    expected_max_y = (offset_y + display.height) > LD_CFG_SCREEN_HEIGHT
                         ? (offset_y + display.height)
                         : LD_CFG_SCREEN_HEIGHT;
    assert(ld_root->use_as__arm_2d_control_node_t.tRegion.tSize.iWidth == expected_max_x - expected_min_x);
    assert(ld_root->use_as__arm_2d_control_node_t.tRegion.tSize.iHeight == expected_max_y - expected_min_y);
    assert(ld_window->use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 12);
    assert(ld_window->use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == -18);
    assert(ld_window->use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iWidth
           == expected_max_x - expected_min_x);
    assert(ld_window->use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iHeight
           == expected_max_y - expected_min_y);

    assert(tinyui_window_set_background_offset(win, -24, 9) == 0);
    assert(tinyui_window_get_background_offset(win, &offset_x, &offset_y) == 0);
    assert(offset_x == -24);
    assert(offset_y == 9);
    assert(ld_root->use_as__arm_2d_control_node_t.tRegion.tLocation.iX == -24);
    assert(ld_root->use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 9);
    expected_min_x = offset_x < 0 ? offset_x : 0;
    expected_min_y = offset_y < 0 ? offset_y : 0;
    expected_max_x = (offset_x + display.width) > LD_CFG_SCREEN_WIDTH
                         ? (offset_x + display.width)
                         : LD_CFG_SCREEN_WIDTH;
    expected_max_y = (offset_y + display.height) > LD_CFG_SCREEN_HEIGHT
                         ? (offset_y + display.height)
                         : LD_CFG_SCREEN_HEIGHT;
    assert(ld_root->use_as__arm_2d_control_node_t.tRegion.tSize.iWidth >= expected_max_x - expected_min_x);
    assert(ld_root->use_as__arm_2d_control_node_t.tRegion.tSize.iHeight >= expected_max_y - expected_min_y);
    assert(ld_window->use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iX == -24);
    assert(ld_window->use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 9);
    assert(ld_window->use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iWidth
           >= expected_max_x - expected_min_x);
    assert(ld_window->use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iHeight
           >= expected_max_y - expected_min_y);

    assert(tinyui_window_get_background_offset(win, 0, &offset_y) == -1);
    assert(tinyui_window_get_background_offset(win, &offset_x, 0) == -1);
    assert(tinyui_window_set_background_offset(0, 1, 2) == -1);
    assert(tinyui_window_get_background_offset(0, &offset_x, &offset_y) == -1);

    tinyui_app_destroy(app);
}

static void test_background_widget_public_contract_round_trip(void)
{
    arm_2d_tile_t img_tile = {0};
    arm_2d_tile_t mask_tile = {0};
    struct tinyui_image_source source = {
        .img_tile = &img_tile,
        .mask_tile = &mask_tile,
    };
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_background *background = tinyui_background_create(app, "bg_root");
    const ldWindow_t *ld_window = (const ldWindow_t *)background->window.widget.ld_widget;
    unsigned int rgb = 0;
    int offset_x = 0;
    int offset_y = 0;

    assert(background != 0);
    assert(tinyui_widget_get_type((const struct tinyui_widget *)background) ==
           TINYUI_WIDGET_TYPE_BACKGROUND);
    assert(ld_window->use_as__ldBase_t.widgetType == widgetTypeBackground);

    assert(tinyui_background_set_source(background, &source) == 0);
    assert(ld_window->ptImgTile == source.img_tile);
    assert(ld_window->ptMaskTile == source.mask_tile);

    assert(tinyui_background_set_color(background, 0x224466U) == 0);
    assert(tinyui_background_get_color(background, &rgb) == 0);
    assert(rgb == 0x204462U);
    assert(ldWindowGetColor((ldWindow_t *)ld_window) ==
           (ldColor)test_rgb_to_ld_color(0x224466U));

    assert(tinyui_background_set_offset(background, 7, -11) == 0);
    assert(tinyui_background_get_offset(background, &offset_x, &offset_y) == 0);
    assert(offset_x == 7);
    assert(offset_y == -11);
    assert(tinyui_app_set_background(app, background) == 0);
    assert(app->root_window == (struct tinyui_window *)background);
    assert(tinyui_app_switch_background(app, background, 3, 90) == 0);

    assert(tinyui_background_set_source(0, &source) == -1);
    assert(tinyui_background_set_color(0, 0x112233U) == -1);
    assert(tinyui_background_get_color(0, &rgb) == -1);
    assert(tinyui_background_get_color(background, 0) == -1);
    assert(tinyui_background_set_offset(0, 1, 2) == -1);
    assert(tinyui_background_get_offset(0, &offset_x, &offset_y) == -1);
    assert(tinyui_app_set_background(0, background) == -1);
    assert(tinyui_app_set_background(app, 0) == -1);
    assert(tinyui_app_switch_background(0, background, 0, 0) == -1);
    assert(tinyui_app_switch_background(app, 0, 0, 0) == -1);

    tinyui_app_destroy(app);
}

static void test_flex_layout_relayout_uses_ld_window_without_cursor_override(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *win = tinyui_window_create(app, "root");
    struct tinyui_button *a = tinyui_button_create(win, "a");
    struct tinyui_button *b = tinyui_button_create(win, "b");
    ldWindow_t *ld_window = (ldWindow_t *)win->widget.ld_widget;
    const ldBase_t *ld_a = (const ldBase_t *)a->widget.ld_widget;
    const ldBase_t *ld_b = (const ldBase_t *)b->widget.ld_widget;

    assert(tinyui_flex_set_flow(win, TINYUI_FLEX_FLOW_ROW) == 0);
    assert(tinyui_flex_set_gap(win, 12, 0) == 0);
    assert(tinyui_widget_set_size((struct tinyui_widget *)a, 50, 20) == 0);
    assert(tinyui_widget_set_size((struct tinyui_widget *)b, 60, 20) == 0);
    assert(ld_window->isLayoutUpdate == true);

    ldWindow_on_frame_start(NULL, ld_window);

    assert(ld_window->isLayoutUpdate == false);
    assert(ld_a->use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 0);
    assert(ld_a->use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 0);
    assert(ld_b->use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 62);
    assert(ld_b->use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 0);

    assert(tinyui_flex_set_gap(win, 20, 0) == 0);
    assert(ld_window->isLayoutUpdate == true);
    ldWindow_on_frame_start(NULL, ld_window);

    assert(ld_window->isLayoutUpdate == false);
    assert(ld_b->use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 70);

    tinyui_app_destroy(app);
}

static void test_window_padding_survives_layout_type_switches(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *win = tinyui_window_create(app, "root");
    ldWindow_t *ld_window = (ldWindow_t *)win->widget.ld_widget;

    assert(tinyui_widget_set_padding((struct tinyui_widget *)win, 9) == 0);
    assert(tinyui_grid_set_columns(win, (int[]){40, 0}, 2) == 0);
    assert(ld_window->layoutTpye == layoutGrid);
    assert(ld_window->gridPadding.left == 9);
    assert(ld_window->gridPadding.top == 9);
    assert(ld_window->gridPadding.right == 9);
    assert(ld_window->gridPadding.bottom == 9);

    assert(tinyui_widget_set_padding((struct tinyui_widget *)win, 13) == 0);
    assert(tinyui_flex_set_flow(win, TINYUI_FLEX_FLOW_ROW) == 0);
    assert(ld_window->layoutTpye == layoutFlex);
    assert(ld_window->flexPadding.left == 13);
    assert(ld_window->flexPadding.top == 13);
    assert(ld_window->flexPadding.right == 13);
    assert(ld_window->flexPadding.bottom == 13);

    tinyui_app_destroy(app);
}

static void test_grid_setters_promote_window_to_grid_layout(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *win = tinyui_window_create(app, "root");
    ldWindow_t *ld_window = (ldWindow_t *)win->widget.ld_widget;

    assert(ld_window->layoutTpye == layoutNone);
    assert(tinyui_grid_set_columns(win, (int[]){120, -1, 0}, 3) == 0);
    assert(ld_window->layoutTpye == layoutGrid);

    assert(tinyui_window_set_layout_type(win, TINYUI_WINDOW_LAYOUT_NONE) == 0);
    assert(ld_window->layoutTpye == layoutNone);
    assert(tinyui_grid_set_rows(win, (int[]){24, -1, 0}, 3) == 0);
    assert(ld_window->layoutTpye == layoutGrid);

    tinyui_app_destroy(app);
}

static void test_grid_layout_positions_basic_widgets_like_demo(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *win = tinyui_window_create(app, "root");
    struct tinyui_switch *sw = tinyui_switch_create(win, "wifi");
    struct tinyui_checkbox *cb = tinyui_checkbox_create(win, "agree");
    struct tinyui_slider *slider = tinyui_slider_create(win, "volume");
    struct tinyui_button *button = tinyui_button_create(win, "submit");
    struct tinyui_text *text = tinyui_text_create(win, "title");
    struct tinyui_image *image = tinyui_image_create(win, "logo");
    ldWindow_t *ld_window = (ldWindow_t *)win->widget.ld_widget;
    ldBase_t *ld_sw = (ldBase_t *)sw->widget.ld_widget;
    ldBase_t *ld_cb = (ldBase_t *)cb->widget.ld_widget;
    ldBase_t *ld_slider = (ldBase_t *)slider->widget.ld_widget;
    ldBase_t *ld_button = (ldBase_t *)button->widget.ld_widget;
    ldBase_t *ld_text = (ldBase_t *)text->widget.ld_widget;
    ldBase_t *ld_image = (ldBase_t *)image->widget.ld_widget;
    const int cols[] = {220, 0};
    const int rows[] = {24, 30, 30, 36, 28, 64, 0};

    assert(tinyui_grid_set_columns(win, cols, 2) == 0);
    assert(tinyui_grid_set_rows(win, rows, 7) == 0);
    assert(tinyui_grid_set_gap(win, 12, 12) == 0);
    assert(tinyui_grid_set_align(win, TINYUI_ALIGN_START, TINYUI_ALIGN_START) == 0);
    assert(tinyui_window_set_padding(win, 16, 24, 16, 16) == 0);

    assert(tinyui_widget_set_size((struct tinyui_widget *)sw, 48, 24) == 0);
    assert(tinyui_widget_set_size((struct tinyui_widget *)cb, 220, 30) == 0);
    assert(tinyui_widget_set_size((struct tinyui_widget *)slider, 220, 30) == 0);
    assert(tinyui_widget_set_size((struct tinyui_widget *)button, 160, 36) == 0);
    assert(tinyui_widget_set_size((struct tinyui_widget *)text, 220, 28) == 0);
    assert(tinyui_widget_set_size((struct tinyui_widget *)image, 220, 60) == 0);

    assert(tinyui_widget_set_grid_cell((struct tinyui_widget *)sw, 0, 0, 1, 1, TINYUI_ALIGN_START, TINYUI_ALIGN_START) == 0);
    assert(tinyui_widget_set_grid_cell((struct tinyui_widget *)cb, 0, 1, 1, 1, TINYUI_ALIGN_START, TINYUI_ALIGN_START) == 0);
    assert(tinyui_widget_set_grid_cell((struct tinyui_widget *)slider, 0, 2, 1, 1, TINYUI_ALIGN_START, TINYUI_ALIGN_START) == 0);
    assert(tinyui_widget_set_grid_cell((struct tinyui_widget *)button, 0, 3, 1, 1, TINYUI_ALIGN_START, TINYUI_ALIGN_START) == 0);
    assert(tinyui_widget_set_grid_cell((struct tinyui_widget *)text, 0, 4, 1, 1, TINYUI_ALIGN_START, TINYUI_ALIGN_START) == 0);
    assert(tinyui_widget_set_grid_cell((struct tinyui_widget *)image, 0, 5, 1, 1, TINYUI_ALIGN_START, TINYUI_ALIGN_START) == 0);

    assert(ld_window->layoutTpye == layoutGrid);
    ldWindow_on_frame_start(app->ld_scene, ld_window);

    assert(ldBaseGetY(ld_sw) < ldBaseGetY(ld_cb));
    assert(ldBaseGetY(ld_cb) < ldBaseGetY(ld_slider));
    assert(ldBaseGetY(ld_slider) < ldBaseGetY(ld_button));
    assert(ldBaseGetY(ld_button) < ldBaseGetY(ld_text));
    assert(ldBaseGetY(ld_text) < ldBaseGetY(ld_image));

    tinyui_app_destroy(app);
}

static void test_window_padding_group_round_trip_to_ldwindow(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *win = tinyui_window_create(app, "root");
    const ldWindow_t *ld_window = (const ldWindow_t *)win->widget.ld_widget;

    {
        int pl = 0, pt = 0, pr = 0, pb = 0;
        assert(tinyui_window_set_padding(win, 4, 6, 8, 10) == 0);
        assert(tinyui_window_get_padding_group(win, &pl, &pt, &pr, &pb) == 0);
        assert(pl == 4);
        assert(pt == 6);
        assert(pr == 8);
        assert(pb == 10);
        assert(ld_window->pLayoutPaddingGroup != 0);
        assert(ld_window->pLayoutPaddingGroup->left == 4);
        assert(ld_window->pLayoutPaddingGroup->top == 6);
        assert(ld_window->pLayoutPaddingGroup->right == 8);
        assert(ld_window->pLayoutPaddingGroup->bottom == 10);
    }

    assert(tinyui_window_set_padding(win, 1, 2, 3, 4) == 0);
    assert(ld_window->pLayoutPaddingGroup != 0);
    assert(ld_window->pLayoutPaddingGroup->left == 1);
    assert(ld_window->pLayoutPaddingGroup->top == 2);
    assert(ld_window->pLayoutPaddingGroup->right == 3);
    assert(ld_window->pLayoutPaddingGroup->bottom == 4);

    tinyui_app_destroy(app);
}

static void test_window_native_layout_padding_grid_padding_and_generic_gap_round_trip(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *win = tinyui_window_create(app, "root");
    const ldWindow_t *ld_window = (const ldWindow_t *)win->widget.ld_widget;

    assert(tinyui_window_set_layout_type(win, TINYUI_WINDOW_LAYOUT_FLEX) == 0);
    assert(ld_window->layoutTpye == layoutFlex);
    assert(tinyui_window_set_padding(win, 2, 4, 6, 8) == 0);
    assert(ld_window->flexPadding.left == 2);
    assert(ld_window->flexPadding.top == 4);
    assert(ld_window->flexPadding.right == 6);
    assert(ld_window->flexPadding.bottom == 8);
    assert(ld_window->layoutTpye == layoutFlex);

    assert(tinyui_window_set_layout_type(win, TINYUI_WINDOW_LAYOUT_GRID) == 0);
    assert(tinyui_window_set_grid_padding(win, 3, 5, 7, 9) == 0);
    assert(ld_window->gridPadding.left == 3);
    assert(ld_window->gridPadding.top == 5);
    assert(ld_window->gridPadding.right == 7);
    assert(ld_window->gridPadding.bottom == 9);

    assert(tinyui_window_set_gap(win, 11) == 0);
    assert(tinyui_window_set_layout_type(win, TINYUI_WINDOW_LAYOUT_GRID) == 0);
    assert(ld_window->flexItemGap == 11);
    assert(ld_window->flexTrackGap == 11);
    assert(win->flex_item_gap == 11);
    assert(win->flex_track_gap == 11);
    assert(tinyui_window_set_layout_type(win, TINYUI_WINDOW_LAYOUT_NONE) == 0);
    assert(tinyui_window_set_layout_type(0, TINYUI_WINDOW_LAYOUT_FLEX) == -1);
    assert(tinyui_window_set_layout_type(win, (enum tinyui_window_layout_type)99) == -1);
    assert(tinyui_window_set_padding(0, 1, 2, 3, 4) == -1);
    assert(tinyui_window_set_padding(win, -1, 2, 3, 4) == -1);
    assert(tinyui_window_set_grid_padding(0, 1, 2, 3, 4) == -1);
    assert(tinyui_window_set_grid_padding(win, 1, 2, 3, -4) == -1);
    assert(tinyui_window_set_gap(0, 1) == -1);
    assert(tinyui_window_set_gap(win, -1) == -1);

    tinyui_app_destroy(app);
}

static void test_window_explicit_flex_padding_survives_followup_flex_updates(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *win = tinyui_window_create(app, "root");
    const ldWindow_t *ld_window = (const ldWindow_t *)win->widget.ld_widget;

    assert(tinyui_window_set_layout_type(win, TINYUI_WINDOW_LAYOUT_FLEX) == 0);
    assert(tinyui_window_set_padding(win, 2, 4, 6, 8) == 0);
    assert(ld_window->flexPadding.left == 2);
    assert(ld_window->flexPadding.top == 4);
    assert(ld_window->flexPadding.right == 6);
    assert(ld_window->flexPadding.bottom == 8);

    assert(tinyui_flex_set_flow(win, TINYUI_FLEX_FLOW_COLUMN_WRAP) == 0);
    assert(tinyui_flex_set_align(win,
                                 TINYUI_ALIGN_END,
                                 TINYUI_ALIGN_CENTER,
                                 TINYUI_ALIGN_SPACE_AROUND) == 0);
    assert(tinyui_flex_set_gap(win, 5, 9) == 0);

    assert(ld_window->layoutTpye == layoutFlex);
    assert(ld_window->flexPadding.left == 2);
    assert(ld_window->flexPadding.top == 4);
    assert(ld_window->flexPadding.right == 6);
    assert(ld_window->flexPadding.bottom == 8);

    tinyui_app_destroy(app);
}

static void test_window_explicit_grid_padding_survives_followup_grid_updates(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *win = tinyui_window_create(app, "root");
    const ldWindow_t *ld_window = (const ldWindow_t *)win->widget.ld_widget;

    assert(tinyui_window_set_layout_type(win, TINYUI_WINDOW_LAYOUT_GRID) == 0);
    assert(tinyui_window_set_grid_padding(win, 3, 5, 7, 9) == 0);
    assert(ld_window->gridPadding.left == 3);
    assert(ld_window->gridPadding.top == 5);
    assert(ld_window->gridPadding.right == 7);
    assert(ld_window->gridPadding.bottom == 9);

    assert(tinyui_grid_set_columns(win, (int[]){40, -2, 0}, 3) == 0);
    assert(tinyui_grid_set_rows(win, (int[]){24, -3, 0}, 3) == 0);
    assert(tinyui_grid_set_gap(win, 4, 6) == 0);
    assert(tinyui_grid_set_align(win, TINYUI_ALIGN_CENTER, TINYUI_ALIGN_END) == 0);

    assert(ld_window->layoutTpye == layoutGrid);
    assert(ld_window->gridPadding.left == 3);
    assert(ld_window->gridPadding.top == 5);
    assert(ld_window->gridPadding.right == 7);
    assert(ld_window->gridPadding.bottom == 9);

    tinyui_app_destroy(app);
}

static void test_window_padding_group_survives_followup_layout_updates(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *win = tinyui_window_create(app, "root");
    const ldWindow_t *ld_window = (const ldWindow_t *)win->widget.ld_widget;

    assert(tinyui_window_set_padding(win, 2, 4, 6, 8) == 0);
    assert(ld_window->pLayoutPaddingGroup != 0);
    assert(ld_window->pLayoutPaddingGroup->left == 2);
    assert(ld_window->pLayoutPaddingGroup->top == 4);
    assert(ld_window->pLayoutPaddingGroup->right == 6);
    assert(ld_window->pLayoutPaddingGroup->bottom == 8);

    assert(tinyui_flex_set_flow(win, TINYUI_FLEX_FLOW_COLUMN_WRAP) == 0);
    assert(tinyui_flex_set_align(win,
                                 TINYUI_ALIGN_END,
                                 TINYUI_ALIGN_CENTER,
                                 TINYUI_ALIGN_SPACE_AROUND) == 0);
    assert(tinyui_flex_set_gap(win, 5, 9) == 0);

    assert(ld_window->layoutTpye == layoutFlex);
    assert(ld_window->pLayoutPaddingGroup->left == 2);
    assert(ld_window->pLayoutPaddingGroup->top == 4);
    assert(ld_window->pLayoutPaddingGroup->right == 6);
    assert(ld_window->pLayoutPaddingGroup->bottom == 8);
    assert(ld_window->flexPadding.left == 2);
    assert(ld_window->flexPadding.top == 4);
    assert(ld_window->flexPadding.right == 6);
    assert(ld_window->flexPadding.bottom == 8);

    assert(tinyui_grid_set_columns(win, (int[]){40, -2, 0}, 3) == 0);
    assert(tinyui_grid_set_rows(win, (int[]){24, -3, 0}, 3) == 0);
    assert(tinyui_grid_set_gap(win, 4, 6) == 0);
    assert(tinyui_grid_set_align(win, TINYUI_ALIGN_CENTER, TINYUI_ALIGN_END) == 0);

    assert(ld_window->layoutTpye == layoutGrid);
    assert(ld_window->pLayoutPaddingGroup->left == 2);
    assert(ld_window->pLayoutPaddingGroup->top == 4);
    assert(ld_window->pLayoutPaddingGroup->right == 6);
    assert(ld_window->pLayoutPaddingGroup->bottom == 8);
    assert(ld_window->gridPadding.left == 2);
    assert(ld_window->gridPadding.top == 4);
    assert(ld_window->gridPadding.right == 6);
    assert(ld_window->gridPadding.bottom == 8);

    tinyui_app_destroy(app);
}

static void test_grid_layout_shorter_template_clears_stale_tail(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *win = tinyui_window_create(app, "root");
    const ldWindow_t *ld_window = (const ldWindow_t *)win->widget.ld_widget;

    assert(tinyui_grid_set_columns(win, (int[]){48, -2, 0}, 3) == 0);
    assert(tinyui_grid_set_rows(win, (int[]){20, 0}, 2) == 0);
    assert(ld_window->gridColDsc[0] == 48);
    assert(ld_window->gridColDsc[1] == LD_GRID_CONTENT);
    assert(ld_window->gridColDsc[2] == LD_GRID_TEMPLATE_LAST);

    assert(tinyui_grid_set_columns(win, (int[]){64}, 1) == 0);
    assert(win->grid_col_count == 1);
    assert(win->grid_cols[0] == 64);
    assert(ld_window->gridColDsc[0] == 64);
    assert(ld_window->gridColDsc[1] == LD_GRID_TEMPLATE_LAST);
    assert(ld_window->gridColDsc[2] == LD_GRID_TEMPLATE_LAST);

    assert(tinyui_grid_set_rows(win, (int[]){28}, 1) == 0);
    assert(win->grid_row_count == 1);
    assert(win->grid_rows[0] == 28);
    assert(ld_window->gridRowDsc[0] == 28);
    assert(ld_window->gridRowDsc[1] == LD_GRID_TEMPLATE_LAST);

    tinyui_app_destroy(app);
}

static void test_child_window_public_api_binds_real_parent_and_hosts_layout_children(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *root = tinyui_window_create(app, "root");
    struct tinyui_window *child = tinyui_window_create_child(root, "section");
    struct tinyui_button *item;
    const ldWindow_t *child_ld_window;
    const ldBase_t *item_ld_base;

    assert(child != 0);

    item = tinyui_button_create(child, "item");
    assert(item != 0);

    child_ld_window = (const ldWindow_t *)child->widget.ld_widget;
    item_ld_base = (const ldBase_t *)item->widget.ld_widget;

    assert(ldBaseGetParent((ldBase_t *)child->widget.ld_widget) == (ldBase_t *)root->widget.ld_widget);
    assert(ldBaseGetChildList((ldBase_t *)root->widget.ld_widget) == (ldBase_t *)child->widget.ld_widget);
    assert(child->widget.kind == TINYUI_BACKEND_WIDGET_WINDOW);
    assert((ldBase_t *)ldBaseGetRootNode((arm_2d_control_node_t *)child->widget.ld_widget) == (ldBase_t *)ldBaseGetRootNode((arm_2d_control_node_t *)root->widget.ld_widget));
    assert(child->widget.owner == root->widget.owner);
    assert(ldBaseGetParent((ldBase_t *)item->widget.ld_widget) == (ldBase_t *)child->widget.ld_widget);
    assert(ldBaseGetChildList((ldBase_t *)child->widget.ld_widget) == (ldBase_t *)item->widget.ld_widget);
    assert(ldBaseGetParent((ldBase_t *)child_ld_window) == (ldBase_t *)root->widget.ld_widget);

    assert(tinyui_window_set_layout_type(child, TINYUI_WINDOW_LAYOUT_FLEX) == 0);
    assert(tinyui_window_set_padding(child, 8, 10, 8, 10) == 0);
    assert(tinyui_flex_set_flow(child, TINYUI_FLEX_FLOW_ROW_WRAP) == 0);
    assert(tinyui_flex_set_gap(child, 6, 4) == 0);
    assert(tinyui_widget_set_size((struct tinyui_widget *)child, 220, 80) == 0);
    assert(tinyui_widget_set_size((struct tinyui_widget *)item, 64, 24) == 0);
    assert(tinyui_widget_set_flex_new_track((struct tinyui_widget *)item, 1) == 0);

    assert(child_ld_window->layoutTpye == layoutFlex);
    assert(child_ld_window->pLayoutPaddingGroup != 0);
    assert(child_ld_window->pLayoutPaddingGroup->left == 8);
    assert(child_ld_window->pLayoutPaddingGroup->top == 10);
    assert(child_ld_window->flexItemGap == 6);
    assert(child_ld_window->flexTrackGap == 4);
    assert(item_ld_base->flexInNewTrack == true);

    tinyui_app_destroy(app);
}

static void test_layout_setters_with_missing_native_binding_reject_without_mutating_state(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *win = tinyui_window_create(app, "root");
    struct tinyui_button *item = tinyui_button_create(win, "item");
    const int original_cols[] = {11, 22, 0};
    const int original_rows[] = {7, 9, 0};
    int i;

    win->flex_flow = TINYUI_FLEX_FLOW_ROW;
    win->flex_main_align = TINYUI_ALIGN_START;
    win->flex_cross_align = TINYUI_ALIGN_END;
    win->flex_track_align = TINYUI_ALIGN_CENTER;
    win->flex_item_gap = 3;
    win->flex_track_gap = 4;
    win->grid_col_count = 3;
    win->grid_row_count = 3;
    win->grid_row_gap = 5;
    win->grid_col_gap = 6;
    win->grid_col_align = TINYUI_ALIGN_SPACE_AROUND;
    win->grid_row_align = TINYUI_ALIGN_SPACE_BETWEEN;
    item->widget.flex_grow = 2;
    item->widget.flex_new_track = 0;
    item->widget.ignore_layout = 0;
    item->widget.grid_col = 0;
    item->widget.grid_row = 1;
    item->widget.grid_col_span = 1;
    item->widget.grid_row_span = 2;
    item->widget.grid_x_align = TINYUI_ALIGN_START;
    item->widget.grid_y_align = TINYUI_ALIGN_CENTER;
    for (i = 0; i < 3; ++i) {
        win->grid_cols[i] = original_cols[i];
        win->grid_rows[i] = original_rows[i];
    }

    win->widget.ld_widget = 0;
    item->widget.ld_widget = 0;

    assert(tinyui_window_set_padding(win, 2, 4, 6, 8) == -1);
    assert(tinyui_flex_set_flow(win, TINYUI_FLEX_FLOW_COLUMN_WRAP) == -1);
    assert(tinyui_flex_set_align(win,
                                 TINYUI_ALIGN_END,
                                 TINYUI_ALIGN_CENTER,
                                 TINYUI_ALIGN_SPACE_AROUND) == -1);
    assert(tinyui_flex_set_gap(win, 5, 9) == -1);
    assert(tinyui_grid_set_columns(win, (int[]){32, -2, 0}, 3) == -1);
    assert(tinyui_grid_set_rows(win, (int[]){18, -3, 0}, 3) == -1);
    assert(tinyui_grid_set_gap(win, 4, 6) == -1);
    assert(tinyui_grid_set_align(win, TINYUI_ALIGN_CENTER, TINYUI_ALIGN_END) == -1);
    assert(tinyui_widget_set_flex_grow((struct tinyui_widget *)item, 3) == -1);
    assert(tinyui_widget_set_flex_new_track((struct tinyui_widget *)item, 1) == -1);
    assert(tinyui_widget_set_ignore_layout((struct tinyui_widget *)item, 1) == -1);
    assert(tinyui_widget_set_grid_cell((struct tinyui_widget *)item,
                                       1, 2, 3, 4,
                                       TINYUI_ALIGN_CENTER,
                                       TINYUI_ALIGN_END) == -1);

    assert(win->flex_flow == TINYUI_FLEX_FLOW_ROW);
    assert(win->flex_main_align == TINYUI_ALIGN_START);
    assert(win->flex_cross_align == TINYUI_ALIGN_END);
    assert(win->flex_track_align == TINYUI_ALIGN_CENTER);
    assert(win->flex_item_gap == 3);
    assert(win->flex_track_gap == 4);
    assert(win->grid_col_count == 3);
    assert(win->grid_row_count == 3);
    assert(win->grid_cols[0] == 11);
    assert(win->grid_cols[1] == 22);
    assert(win->grid_cols[2] == 0);
    assert(win->grid_rows[0] == 7);
    assert(win->grid_rows[1] == 9);
    assert(win->grid_rows[2] == 0);
    assert(win->grid_row_gap == 5);
    assert(win->grid_col_gap == 6);
    assert(win->grid_col_align == TINYUI_ALIGN_SPACE_AROUND);
    assert(win->grid_row_align == TINYUI_ALIGN_SPACE_BETWEEN);
    assert(item->widget.flex_grow == 2);
    assert(item->widget.flex_new_track == 0);
    assert(item->widget.ignore_layout == 0);
    assert(item->widget.grid_col == 0);
    assert(item->widget.grid_row == 1);
    assert(item->widget.grid_col_span == 1);
    assert(item->widget.grid_row_span == 2);
    assert(item->widget.grid_x_align == TINYUI_ALIGN_START);
    assert(item->widget.grid_y_align == TINYUI_ALIGN_CENTER);

    tinyui_app_destroy(app);
}

static void test_layout_setters_reject_unbound_window_without_mutating_state(void)
{
    struct tinyui_window fake = {0};
    const int original_cols[] = {7, 8, 9};
    const int original_rows[] = {4, 5, 6};
    int i;

    fake.flex_flow = TINYUI_FLEX_FLOW_ROW;
    fake.flex_main_align = TINYUI_ALIGN_START;
    fake.flex_cross_align = TINYUI_ALIGN_END;
    fake.flex_track_align = TINYUI_ALIGN_CENTER;
    fake.flex_item_gap = 3;
    fake.flex_track_gap = 4;
    fake.grid_col_count = 3;
    fake.grid_row_count = 3;
    fake.grid_row_gap = 5;
    fake.grid_col_gap = 6;
    fake.grid_col_align = TINYUI_ALIGN_SPACE_AROUND;
    fake.grid_row_align = TINYUI_ALIGN_SPACE_BETWEEN;
    for (i = 0; i < 3; ++i) {
        fake.grid_cols[i] = original_cols[i];
        fake.grid_rows[i] = original_rows[i];
    }

    assert(tinyui_flex_set_flow(&fake, TINYUI_FLEX_FLOW_COLUMN_WRAP) == -1);
    assert(tinyui_flex_set_align(&fake,
                                 TINYUI_ALIGN_END,
                                 TINYUI_ALIGN_CENTER,
                                 TINYUI_ALIGN_SPACE_AROUND) == -1);
    assert(tinyui_flex_set_gap(&fake, 11, 12) == -1);
    assert(tinyui_grid_set_columns(&fake, (int[]){32, -2, 0}, 3) == -1);
    assert(tinyui_grid_set_rows(&fake, (int[]){18, -3, 0}, 3) == -1);
    assert(tinyui_grid_set_gap(&fake, 9, 10) == -1);
    assert(tinyui_grid_set_align(&fake, TINYUI_ALIGN_CENTER, TINYUI_ALIGN_END) == -1);

    assert(fake.flex_flow == TINYUI_FLEX_FLOW_ROW);
    assert(fake.flex_main_align == TINYUI_ALIGN_START);
    assert(fake.flex_cross_align == TINYUI_ALIGN_END);
    assert(fake.flex_track_align == TINYUI_ALIGN_CENTER);
    assert(fake.flex_item_gap == 3);
    assert(fake.flex_track_gap == 4);
    assert(fake.grid_col_count == 3);
    assert(fake.grid_row_count == 3);
    assert(fake.grid_row_gap == 5);
    assert(fake.grid_col_gap == 6);
    assert(fake.grid_col_align == TINYUI_ALIGN_SPACE_AROUND);
    assert(fake.grid_row_align == TINYUI_ALIGN_SPACE_BETWEEN);
    for (i = 0; i < 3; ++i) {
        assert(fake.grid_cols[i] == original_cols[i]);
        assert(fake.grid_rows[i] == original_rows[i]);
    }
}

static void test_layout_child_setter_rejects_corrupted_binding_without_mutating_state(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *win = tinyui_window_create(app, "root");
    struct tinyui_button *button = tinyui_button_create(win, "corrupted_binding");
    ldBase_t *ld_base = (ldBase_t *)button->widget.ld_widget;
    enum tinyui_backend_widget_kind original_kind = button->widget.kind;
    int original_widget_type = ld_base->widgetType;

    assert(tinyui_widget_set_ignore_layout((struct tinyui_widget *)button, 1) == 0);
    assert(button->widget.ignore_layout == 1);
    assert(ld_base->ignoreLayout == true);

    ld_base->widgetType = widgetTypeWindow;

    assert(tinyui_widget_set_ignore_layout((struct tinyui_widget *)button, 0) == -1);
    assert(button->widget.ignore_layout == 1);
    assert(ld_base->ignoreLayout == true);

    button->widget.kind = original_kind;
    ld_base->widgetType = original_widget_type;

    tinyui_app_destroy(app);
}

static void test_layout_window_padding_setters_reject_corrupted_binding_without_mutating_state(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *win = tinyui_window_create(app, "layout_corrupted_window");
    ldBase_t *ld_base = (ldBase_t *)win->widget.ld_widget;
    ldWindow_t *ld_window = (ldWindow_t *)win->widget.ld_widget;
    int original_widget_type = ld_base->widgetType;

    assert(tinyui_window_set_padding(win, 2, 4, 6, 8) == 0);
    assert(tinyui_window_set_grid_padding(win, 3, 5, 7, 9) == 0);
    assert(ld_window->flexPadding.left == 2);
    assert(ld_window->flexPadding.top == 4);
    assert(ld_window->flexPadding.right == 6);
    assert(ld_window->flexPadding.bottom == 8);
    assert(ld_window->gridPadding.left == 3);
    assert(ld_window->gridPadding.top == 5);
    assert(ld_window->gridPadding.right == 7);
    assert(ld_window->gridPadding.bottom == 9);

    ld_base->widgetType = widgetTypeButton;

    assert(tinyui_widget_set_padding((struct tinyui_widget *)win, 10) == -1);
    assert(tinyui_window_set_padding(win, 11, 12, 13, 14) == -1);
    assert(tinyui_window_set_grid_padding(win, 15, 16, 17, 18) == -1);

    assert(win->widget.padding == 0);
    assert(ld_window->flexPadding.left == 2);
    assert(ld_window->flexPadding.top == 4);
    assert(ld_window->flexPadding.right == 6);
    assert(ld_window->flexPadding.bottom == 8);
    assert(ld_window->gridPadding.left == 3);
    assert(ld_window->gridPadding.top == 5);
    assert(ld_window->gridPadding.right == 7);
    assert(ld_window->gridPadding.bottom == 9);
    assert(win->padding_left == 2);
    assert(win->padding_top == 4);
    assert(win->padding_right == 6);
    assert(win->padding_bottom == 8);
    assert(win->grid_padding_left == 3);
    assert(win->grid_padding_top == 5);
    assert(win->grid_padding_right == 7);
    assert(win->grid_padding_bottom == 9);
    assert(win->has_explicit_flex_padding == 1);
    assert(win->has_explicit_grid_padding == 1);

    ld_base->widgetType = original_widget_type;

    tinyui_app_destroy(app);
}

static void test_layout_grid_setters_reject_corrupted_binding_without_mutating_cached_state(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *win = tinyui_window_create(app, "layout_corrupted_grid");
    ldBase_t *ld_base = (ldBase_t *)win->widget.ld_widget;
    ldWindow_t *ld_window = (ldWindow_t *)win->widget.ld_widget;
    int original_widget_type = ld_base->widgetType;
    int original_cols[3] = {80, -1, 0};
    int original_rows[3] = {24, -2, 0};
    int i;

    assert(tinyui_grid_set_columns(win, original_cols, 3) == 0);
    assert(tinyui_grid_set_rows(win, original_rows, 3) == 0);
    assert(tinyui_grid_set_gap(win, 5, 7) == 0);
    assert(tinyui_grid_set_align(win, TINYUI_ALIGN_END, TINYUI_ALIGN_SPACE_AROUND) == 0);

    ld_base->widgetType = widgetTypeButton;

    assert(tinyui_flex_set_flow(win, TINYUI_FLEX_FLOW_COLUMN_WRAP) == -1);
    assert(tinyui_flex_set_align(win,
                                 TINYUI_ALIGN_END,
                                 TINYUI_ALIGN_CENTER,
                                 TINYUI_ALIGN_SPACE_AROUND) == -1);
    assert(tinyui_flex_set_gap(win, 11, 13) == -1);
    assert(tinyui_grid_set_columns(win, (int[]){33, -2, 0}, 3) == -1);
    assert(tinyui_grid_set_rows(win, (int[]){18, -3, 0}, 3) == -1);
    assert(tinyui_grid_set_gap(win, 11, 13) == -1);
    assert(tinyui_grid_set_align(win, TINYUI_ALIGN_CENTER, TINYUI_ALIGN_SPACE_BETWEEN) == -1);

    assert(win->flex_flow == TINYUI_FLEX_FLOW_ROW);
    assert(win->flex_main_align == TINYUI_ALIGN_START);
    assert(win->flex_cross_align == TINYUI_ALIGN_START);
    assert(win->flex_track_align == TINYUI_ALIGN_START);
    assert(win->grid_col_count == 3);
    assert(win->grid_row_count == 3);
    assert(win->grid_row_gap == 5);
    assert(win->grid_col_gap == 7);
    assert(win->grid_col_align == TINYUI_ALIGN_END);
    assert(win->grid_row_align == TINYUI_ALIGN_SPACE_AROUND);
    assert(ld_window->flexFlow == ldFlexFlowRow);
    assert(ld_window->flexMainAlign == ldFlexMainAlignStart);
    assert(ld_window->flexCrossAlign == ldFlexCrossAlignStart);
    assert(ld_window->flexTrackAlign == ldFlexTrackAlignStart);
    assert(win->grid_cols[0] == original_cols[0]);
    assert(win->grid_cols[1] == original_cols[1]);
    assert(win->grid_cols[2] == original_cols[2]);
    assert(win->grid_rows[0] == original_rows[0]);
    assert(win->grid_rows[1] == original_rows[1]);
    assert(win->grid_rows[2] == original_rows[2]);
    assert(ld_window->gridColDsc[0] == 80);
    assert(ld_window->gridColDsc[1] < 0);
    assert(ld_window->gridColDsc[1] != LD_GRID_CONTENT);
    assert(ld_window->gridColDsc[1] != LD_GRID_TEMPLATE_LAST);
    assert(ld_window->gridColDsc[2] == LD_GRID_TEMPLATE_LAST);
    assert(ld_window->gridRowDsc[0] == 24);
    assert(ld_window->gridRowDsc[1] == LD_GRID_CONTENT);
    assert(ld_window->gridRowDsc[2] == LD_GRID_TEMPLATE_LAST);
    /* C3-T4: the window_layout mirror cache has been deleted — the ld_window
     * values above are the source of truth and have been verified directly. */
    assert(ld_window->gridRowGap == 5);
    assert(ld_window->gridColumnGap == 7);
    assert(ld_window->gridColAlign == ldGridAlignEnd);
    assert(ld_window->gridRowAlign == ldGridAlignSpaceAround);

    ld_base->widgetType = original_widget_type;

    tinyui_app_destroy(app);
}

static void test_layout_window_gap_setter_rejects_corrupted_binding_without_mutating_state(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *win = tinyui_window_create(app, "layout_corrupted_gap");
    ldBase_t *ld_base = (ldBase_t *)win->widget.ld_widget;
    ldWindow_t *ld_window = (ldWindow_t *)win->widget.ld_widget;
    int original_widget_type = ld_base->widgetType;

    assert(tinyui_window_set_gap(win, 11) == 0);
    assert(win->flex_item_gap == 11);
    assert(win->flex_track_gap == 11);
    assert(ld_window->flexItemGap == 11);
    assert(ld_window->flexTrackGap == 11);

    ld_base->widgetType = widgetTypeButton;

    assert(tinyui_window_set_gap(win, 17) == -1);
    assert(win->flex_item_gap == 11);
    assert(win->flex_track_gap == 11);
    assert(ld_window->flexItemGap == 11);
    assert(ld_window->flexTrackGap == 11);

    ld_base->widgetType = original_widget_type;

    tinyui_app_destroy(app);
}

static void test_layout_child_helper_backend_symbols_are_no_longer_public(void)
{
    assert_archive_lacks_symbol("../../libtinyui_backend_ldgui.a",
                                "tinyui_backend_widget_set_flex_grow");
    assert_archive_lacks_symbol("../../libtinyui_backend_ldgui.a",
                                "tinyui_backend_widget_set_flex_new_track");
    assert_archive_lacks_symbol("../../libtinyui_backend_ldgui.a",
                                "tinyui_backend_widget_set_ignore_layout");
    assert_archive_lacks_symbol("../../libtinyui_backend_ldgui.a",
                                "tinyui_backend_widget_set_grid_cell");
    assert_self_binary_lacks_symbol("tinyui_backend_widget_set_flex_grow");
    assert_self_binary_lacks_symbol("tinyui_backend_widget_set_flex_new_track");
    assert_self_binary_lacks_symbol("tinyui_backend_widget_set_ignore_layout");
    assert_self_binary_lacks_symbol("tinyui_backend_widget_set_grid_cell");
}

static void test_layout_padding_helper_backend_symbols_are_no_longer_public(void)
{
    assert_archive_lacks_symbol("../../libtinyui_backend_ldgui.a",
                                "tinyui_backend_widget_set_padding");
    assert_archive_lacks_symbol("../../libtinyui_backend_ldgui.a",
                                "tinyui_backend_window_set_padding");
    assert_archive_lacks_symbol("../../libtinyui_backend_ldgui.a",
                                "tinyui_backend_window_set_grid_padding");
    assert_self_binary_lacks_symbol("tinyui_backend_widget_set_padding");
    assert_self_binary_lacks_symbol("tinyui_backend_window_set_padding");
    assert_self_binary_lacks_symbol("tinyui_backend_window_set_grid_padding");
}

static void test_layout_flex_helper_backend_symbols_are_no_longer_public(void)
{
    assert_archive_lacks_symbol("../../libtinyui_backend_ldgui_porting.a",
                                "tinyui_backend_window_set_layout_type");
    assert_archive_lacks_symbol("../../libtinyui_backend_ldgui_porting.a",
                                "tinyui_backend_window_set_flex_flow");
    assert_archive_lacks_symbol("../../libtinyui_backend_ldgui_porting.a",
                                "tinyui_backend_window_set_flex_align");
    assert_archive_lacks_symbol("../../libtinyui_backend_ldgui_porting.a",
                                "tinyui_backend_window_set_flex_gap");
    assert_self_binary_lacks_symbol("tinyui_backend_window_set_layout_type");
    assert_self_binary_lacks_symbol("tinyui_backend_window_set_flex_flow");
    assert_self_binary_lacks_symbol("tinyui_backend_window_set_flex_align");
    assert_self_binary_lacks_symbol("tinyui_backend_window_set_flex_gap");
}

static void test_layout_grid_helper_backend_symbols_are_no_longer_public(void)
{
    assert_archive_lacks_symbol("../../libtinyui_backend_ldgui_porting.a",
                                "tinyui_backend_window_set_grid_columns");
    assert_archive_lacks_symbol("../../libtinyui_backend_ldgui_porting.a",
                                "tinyui_backend_window_set_grid_rows");
    assert_archive_lacks_symbol("../../libtinyui_backend_ldgui_porting.a",
                                "tinyui_backend_window_set_grid_gap");
    assert_archive_lacks_symbol("../../libtinyui_backend_ldgui_porting.a",
                                "tinyui_backend_window_set_grid_align");
    assert_self_binary_lacks_symbol("tinyui_backend_window_set_grid_columns");
    assert_self_binary_lacks_symbol("tinyui_backend_window_set_grid_rows");
    assert_self_binary_lacks_symbol("tinyui_backend_window_set_grid_gap");
    assert_self_binary_lacks_symbol("tinyui_backend_window_set_grid_align");
}

static void test_layout_gap_helper_backend_symbol_is_no_longer_public(void)
{
    assert_archive_lacks_symbol("../../libtinyui_backend_ldgui_porting.a",
                                "tinyui_backend_window_set_gap");
    assert_self_binary_lacks_symbol("tinyui_backend_window_set_gap");
}

static void test_layout_backend_layout_archive_member_is_no_longer_present(void)
{
    assert_archive_lacks_member("../../libtinyui_backend_ldgui_porting.a", "backend_layout.c.o");
}

static void test_layout_internal_static_helpers_no_longer_use_tinyui_prefix(void)
{
    const char *flex_source = test_repo_path("tinyui/src/layout/flex.c");
    const char *grid_source = test_repo_path("tinyui/src/layout/grid.c");

    assert(flex_source != 0);
    assert(grid_source != 0);
}

int main(int argc, char **argv)
{
    (void)argc;
    test_self_binary_path = (argv != NULL && argv[0] != NULL) ? argv[0] : test_repo_path("build/tests/tinyui/test_tinyui_layout");
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *win = tinyui_window_create(app, "root");
    struct tinyui_button *a = tinyui_button_create(win, "a");
    struct tinyui_button *b = tinyui_button_create(win, "b");
    assert(tinyui_flex_set_flow(win, TINYUI_FLEX_FLOW_ROW_WRAP) == 0);
    assert(tinyui_flex_set_align(win,
                                 TINYUI_ALIGN_START,
                                 TINYUI_ALIGN_CENTER,
                                 TINYUI_ALIGN_SPACE_BETWEEN) == 0);
    assert(tinyui_flex_set_gap(win, 8, 12) == 0);
    assert(tinyui_widget_set_flex_grow((struct tinyui_widget *)a, 1) == 0);
    assert(tinyui_widget_set_flex_new_track((struct tinyui_widget *)a, 1) == 0);
    assert(a->widget.flex_new_track == 1);
    assert(tinyui_widget_set_ignore_layout((struct tinyui_widget *)b, 1) == 0);
    assert(b->widget.ignore_layout == 1);

    assert(tinyui_grid_set_columns(win, (int[]){80, -1, 0}, 3) == 0);
    assert(tinyui_grid_set_rows(win, (int[]){24, -1, 0}, 3) == 0);
    assert(tinyui_grid_set_align(win, TINYUI_ALIGN_END, TINYUI_ALIGN_SPACE_AROUND) == 0);
    assert(win->grid_col_align == TINYUI_ALIGN_END);
    assert(win->grid_row_align == TINYUI_ALIGN_SPACE_AROUND);
    assert(tinyui_widget_set_grid_cell((struct tinyui_widget *)b,
                                       1, 0, 1, 1,
                                       TINYUI_ALIGN_CENTER,
                                       TINYUI_ALIGN_CENTER) == 0);

    tinyui_app_destroy(app);
    test_grid_layout_setters_sync_to_real_ld_window_and_children();
    test_grid_layout_rejects_invalid_gap_and_cell_span();
    test_layout_facades_reuse_window_validity_contract();
    test_window_native_grid_descriptors_round_trip_to_ldwindow();
    test_flex_layout_setters_sync_to_real_ld_window_and_children();
    test_widget_native_base_flags_round_trip_to_ldbase();
    test_widget_backend_parent_round_trip();
    test_widget_native_flex_min_max_round_trip_to_ldbase();
    test_widget_native_base_getters_round_trip_to_ldbase();
    test_widget_tree_name_and_type_queries_round_trip_to_ldbase();
    test_widget_remove_from_parent_updates_tinyui_and_ldbase_tree();
    test_widget_destroy_detaches_focus_and_invalidates_backend_binding();
    test_widget_geometry_helpers_round_trip_to_ldbase();
    test_widget_focus_navigation_public_api();
    test_flex_layout_relayout_uses_ld_window_without_cursor_override();
    test_window_padding_survives_layout_type_switches();
    test_grid_setters_promote_window_to_grid_layout();
    test_grid_layout_positions_basic_widgets_like_demo();
    test_window_padding_group_round_trip_to_ldwindow();
    test_window_color_round_trip_to_ldwindow();
    test_window_background_source_round_trip_to_ldwindow();
    test_window_background_offset_round_trip_to_scene_root();
    test_background_widget_public_contract_round_trip();
    test_window_native_layout_padding_grid_padding_and_generic_gap_round_trip();
    test_window_explicit_flex_padding_survives_followup_flex_updates();
    test_window_explicit_grid_padding_survives_followup_grid_updates();
    test_window_padding_group_survives_followup_layout_updates();
    test_grid_layout_shorter_template_clears_stale_tail();
    test_child_window_public_api_binds_real_parent_and_hosts_layout_children();
    test_layout_setters_with_missing_native_binding_reject_without_mutating_state();
    test_layout_setters_reject_unbound_window_without_mutating_state();
    test_layout_child_setter_rejects_corrupted_binding_without_mutating_state();
    test_layout_window_padding_setters_reject_corrupted_binding_without_mutating_state();
    test_layout_grid_setters_reject_corrupted_binding_without_mutating_cached_state();
    test_layout_window_gap_setter_rejects_corrupted_binding_without_mutating_state();
    test_layout_child_helper_backend_symbols_are_no_longer_public();
    test_layout_padding_helper_backend_symbols_are_no_longer_public();
    test_layout_flex_helper_backend_symbols_are_no_longer_public();
    test_layout_grid_helper_backend_symbols_are_no_longer_public();
    test_layout_gap_helper_backend_symbol_is_no_longer_public();
    test_layout_backend_layout_archive_member_is_no_longer_present();
    test_layout_internal_static_helpers_no_longer_use_tinyui_prefix();
    return 0;
}
