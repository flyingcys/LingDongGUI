#include "picoui/picoui.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldWindow.h"
#include "../../../src/porting/ldConfig.h"
#include "internal.h"
#include <assert.h>
#include <dlfcn.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

extern int tinyui_widget_has_ld_binding(const struct picoui_widget *widget);
void tinyui_window_test_fail_next_set_bg_color(void);

static const char *test_self_binary_path = 0;
static const char *test_source_file_path = __FILE__;

static FILE *open_repo_file_from_test_source(const char *relative_path)
{
    char base_path[PATH_MAX];
    char *tests_dir;
    size_t base_len;

    assert(test_source_file_path != 0);
    assert(relative_path != 0);
    assert(strlen(test_source_file_path) < sizeof(base_path));
    snprintf(base_path, sizeof(base_path), "%s", test_source_file_path);
    tests_dir = strstr(base_path, "tests/tinyui/unit/");
    assert(tests_dir != 0);
    *tests_dir = '\0';
    base_len = strlen(base_path);
    assert(base_len + strlen(relative_path) + 1 < sizeof(base_path));
    snprintf(base_path + base_len, sizeof(base_path) - base_len, "%s", relative_path);
    return fopen(base_path, "rb");
}

static void assert_repo_file_lacks(const char *relative_path, const char *needle)
{
    FILE *file;
    char content[32768];
    size_t bytes_read;

    file = open_repo_file_from_test_source(relative_path);
    assert(file != 0);
    bytes_read = fread(content, 1, sizeof(content) - 1, file);
    assert(ferror(file) == 0);
    content[bytes_read] = '\0';
    assert(fclose(file) == 0);
    assert(strstr(content, needle) == 0);
}

static void assert_self_binary_lacks_symbol(const char *symbol)
{
    char command[PATH_MAX + 32];
    FILE *pipe;
    char line[512];

    assert(test_self_binary_path != 0);
    assert(symbol != 0);
    snprintf(command, sizeof(command), "nm %s 2>/dev/null", test_self_binary_path);
    pipe = popen(command, "r");
    assert(pipe != 0);
    while (fgets(line, sizeof(line), pipe) != 0) {
        size_t line_len = strlen(line);
        size_t symbol_len = strlen(symbol);

        while (line_len > 0 && (line[line_len - 1] == '\n' || line[line_len - 1] == '\r')) {
            line[--line_len] = '\0';
        }
        if (line_len >= symbol_len &&
            strcmp(line + line_len - symbol_len, symbol) == 0) {
            assert(!"unexpected symbol still present in test binary");
        }
    }
    assert(pclose(pipe) == 0);
}

static void test_window_color_internal_seam_uses_tinyui_names(void)
{
    static const char *old_symbols[] = {
        "picoui_window_fail_next_set_bg_color",
        "picoui_window_rgb_to_ld_color",
        "picoui_window_ld_color_to_rgb",
        "picoui_backend_window_test_fail_next_set_bg_color",
    };
    size_t i;

    for (i = 0; i < sizeof(old_symbols) / sizeof(old_symbols[0]); ++i) {
        assert_repo_file_lacks("tinyui/src/widgets/window.c", old_symbols[i]);
        assert_self_binary_lacks_symbol(old_symbols[i]);
    }
}

static void test_window_layout_mapper_internal_seam_uses_tinyui_names(void)
{
    static const char *old_symbols[] = {
        "picoui_window_map_flex_flow",
        "picoui_window_map_flex_main_align",
        "picoui_window_map_flex_cross_align",
        "picoui_window_map_flex_track_align",
        "picoui_window_map_grid_align",
        "picoui_window_map_grid_track",
        "picoui_window_copy_grid_tracks",
    };
    size_t i;

    for (i = 0; i < sizeof(old_symbols) / sizeof(old_symbols[0]); ++i) {
        assert_repo_file_lacks("tinyui/src/widgets/window.c", old_symbols[i]);
        assert_self_binary_lacks_symbol(old_symbols[i]);
    }
}

static void test_window_native_access_internal_seam_uses_tinyui_names(void)
{
    static const char *old_symbols[] = {
        "picoui_window_get_backend",
        "picoui_window_get_backend_host",
        "picoui_window_get_ld_window",
        "picoui_window_map_layout_type",
    };
    size_t i;

    for (i = 0; i < sizeof(old_symbols) / sizeof(old_symbols[0]); ++i) {
        assert_repo_file_lacks("tinyui/src/widgets/window.c", old_symbols[i]);
        assert_self_binary_lacks_symbol(old_symbols[i]);
    }
}

static void test_window_validation_internal_seam_uses_tinyui_names(void)
{
    static const char *old_symbols[] = {
        "picoui_window_is_valid",
        "picoui_window_props_are_valid",
    };
    size_t i;

    for (i = 0; i < sizeof(old_symbols) / sizeof(old_symbols[0]); ++i) {
        assert_repo_file_lacks("tinyui/src/widgets/window.c", old_symbols[i]);
        assert_self_binary_lacks_symbol(old_symbols[i]);
    }
}

static void test_window_init_defaults_internal_seam_uses_tinyui_names(void)
{
    static const char *old_symbols[] = {
        "picoui_window_init_defaults",
    };
    size_t i;

    for (i = 0; i < sizeof(old_symbols) / sizeof(old_symbols[0]); ++i) {
        assert_repo_file_lacks("tinyui/src/widgets/window.c", old_symbols[i]);
        assert_self_binary_lacks_symbol(old_symbols[i]);
    }
}

static void test_window_root_size_internal_seam_uses_tinyui_names(void)
{
    static const char *old_symbols[] = {
        "picoui_window_get_root_size",
    };
    size_t i;

    for (i = 0; i < sizeof(old_symbols) / sizeof(old_symbols[0]); ++i) {
        assert_repo_file_lacks("tinyui/src/widgets/window.c", old_symbols[i]);
        assert_self_binary_lacks_symbol(old_symbols[i]);
    }
}

static void test_window_dispose_partial_internal_seam_uses_tinyui_names(void)
{
    static const char *old_symbols[] = {
        "picoui_window_dispose_partial",
    };
    size_t i;

    for (i = 0; i < sizeof(old_symbols) / sizeof(old_symbols[0]); ++i) {
        assert_repo_file_lacks("tinyui/src/widgets/window.c", old_symbols[i]);
        assert_self_binary_lacks_symbol(old_symbols[i]);
    }
}

static void test_window_generic_gap_internal_seam_uses_tinyui_names(void)
{
    static const char *old_symbols[] = {
        "picoui_window_apply_generic_gap_impl",
    };
    size_t i;

    for (i = 0; i < sizeof(old_symbols) / sizeof(old_symbols[0]); ++i) {
        assert_repo_file_lacks("tinyui/src/widgets/window.c", old_symbols[i]);
        assert_self_binary_lacks_symbol(old_symbols[i]);
    }
}

static void test_window_padding_contract_internal_seam_uses_tinyui_names(void)
{
    static const char *old_symbols[] = {
        "picoui_window_set_padding_group_impl",
        "picoui_window_apply_padding_contract",
    };
    size_t i;

    for (i = 0; i < sizeof(old_symbols) / sizeof(old_symbols[0]); ++i) {
        assert_repo_file_lacks("tinyui/src/widgets/window.c", old_symbols[i]);
        assert_self_binary_lacks_symbol(old_symbols[i]);
    }
}

static void test_window_layout_type_internal_seam_uses_tinyui_names(void)
{
    static const char *old_symbols[] = {
        "picoui_window_apply_layout_type_impl",
    };
    size_t i;

    for (i = 0; i < sizeof(old_symbols) / sizeof(old_symbols[0]); ++i) {
        assert_repo_file_lacks("tinyui/src/widgets/window.c", old_symbols[i]);
        assert_self_binary_lacks_symbol(old_symbols[i]);
    }
}

static void test_window_flex_contract_internal_seam_uses_tinyui_names(void)
{
    static const char *old_symbols[] = {
        "picoui_window_apply_flex_contract_impl",
    };
    size_t i;

    for (i = 0; i < sizeof(old_symbols) / sizeof(old_symbols[0]); ++i) {
        assert_repo_file_lacks("tinyui/src/widgets/window.c", old_symbols[i]);
        assert_self_binary_lacks_symbol(old_symbols[i]);
    }
}

static void test_window_create_and_backend_mapping(struct picoui_window *win)
{
    struct picoui_backend_widget *backend;
    ldWindow_t *ld_win;

    assert(win != 0);
    backend = (struct picoui_backend_widget *)win->widget.backend_widget;
    assert(backend != 0);
    assert(backend->kind == PICOUI_BACKEND_WIDGET_WINDOW);
    ld_win = (ldWindow_t *)backend->ld_widget;
    assert(ld_win != 0);
    assert(ld_win->use_as__ldBase_t.widgetType == widgetTypeBackground);
}

static void test_window_padding_group_round_trip(struct picoui_window *win)
{
    struct picoui_backend_widget *backend;
    ldWindow_t *ld_win;

    assert(picoui_window_set_padding_group(win, 10, 20, 30, 40) == 0);

    backend = (struct picoui_backend_widget *)win->widget.backend_widget;
    ld_win = (ldWindow_t *)backend->ld_widget;
    assert(ld_win->flexPadding.left == 10);
    assert(ld_win->flexPadding.top == 20);
    assert(ld_win->flexPadding.right == 30);
    assert(ld_win->flexPadding.bottom == 40);
    assert(ld_win->gridPadding.left == 10);
    assert(ld_win->gridPadding.top == 20);
    assert(ld_win->gridPadding.right == 30);
    assert(ld_win->gridPadding.bottom == 40);
}

static void test_window_widget_base_api_round_trip(struct picoui_window *win)
{
    struct picoui_backend_widget *backend;
    struct picoui_display_config display = {0};
    ldBase_t *ld_base;

    backend = (struct picoui_backend_widget *)win->widget.backend_widget;
    ld_base = (ldBase_t *)backend->ld_widget;

    assert(picoui_display_get_config(backend->owner, &display) == 0);
    assert(ld_base->use_as__arm_2d_control_node_t.tRegion.tSize.iWidth == display.width);
    assert(ld_base->use_as__arm_2d_control_node_t.tRegion.tSize.iHeight == display.height);

    assert(picoui_widget_set_size(&win->widget, 200, 120) == 0);
    assert(ld_base->use_as__arm_2d_control_node_t.tRegion.tSize.iWidth == 200);
    assert(ld_base->use_as__arm_2d_control_node_t.tRegion.tSize.iHeight == 120);

    assert(picoui_widget_set_selectable(&win->widget, 1) == 0);
    assert(picoui_widget_set_selected(&win->widget, 1) == 0);
    assert(ld_base->isSelectable == true);
    assert(ld_base->isSelected == true);
}

static void test_window_grid_padding_positions_switch(struct picoui_window *win)
{
    const int cols[] = {220, 0};
    const int rows[] = {24, 0};
    struct picoui_switch *sw;
    struct picoui_backend_widget *window_backend;
    struct picoui_backend_widget *switch_backend;
    ldWindow_t *ld_win;
    ldBase_t *ld_switch;

    assert(picoui_grid_set_columns(win, cols, 2) == 0);
    assert(picoui_grid_set_rows(win, rows, 2) == 0);
    assert(picoui_grid_set_gap(win, 12, 12) == 0);
    assert(picoui_window_set_padding_group(win, 16, 24, 16, 16) == 0);

    sw = picoui_switch_create(win, "window_grid_padding_switch");
    assert(sw != 0);
    assert(picoui_widget_set_size((struct picoui_widget *)sw, 48, 24) == 0);
    assert(picoui_widget_set_grid_cell((struct picoui_widget *)sw,
                                       0,
                                       0,
                                       1,
                                       1,
                                       PICOUI_ALIGN_START,
                                       PICOUI_ALIGN_START) == 0);

    window_backend = (struct picoui_backend_widget *)win->widget.backend_widget;
    switch_backend = (struct picoui_backend_widget *)sw->widget.backend_widget;
    ld_win = (ldWindow_t *)window_backend->ld_widget;
    ld_switch = (ldBase_t *)switch_backend->ld_widget;

    ldWindow_on_frame_start(0, ld_win);
    assert(ld_switch->use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 16);
    assert(ld_switch->use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 24);
    assert(ld_switch->use_as__arm_2d_control_node_t.tRegion.tSize.iWidth == 48);
    assert(ld_switch->use_as__arm_2d_control_node_t.tRegion.tSize.iHeight == 24);
}

static void test_window_constructor_binds_ld_without_backend_wrapper(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_window *win = picoui_window_create(app, "root_contract");

    assert(app != 0);
    assert(win != 0);
    assert(tinyui_widget_has_ld_binding(&win->widget) == 1);

    picoui_app_destroy(app);
}

static void test_window_create_with_props_failure_rolls_back_root_binding(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_window_props props = {
        .id = "root_props_fail",
        .bg_color = 0x112233,
    };
    struct picoui_backend_app_state *app_state;

    assert(app != 0);
    app_state = (struct picoui_backend_app_state *)app->backend_app;
    assert(app_state != 0);
    assert(app_state->ld_scene != 0);
    assert(app_state->ld_scene->ptNodeRoot == 0);

    tinyui_window_test_fail_next_set_bg_color();
    assert(picoui_window_create_with_props(app, &props) == 0);
    assert(app_state->ld_scene->ptNodeRoot == 0);

    picoui_app_destroy(app);
}

static void test_window_create_with_props_applies_bg_color_without_backend_constructor(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_window_props props = {
        .id = "root_props_color",
        .bg_color = 0x112233,
    };
    struct picoui_window *win;
    struct picoui_backend_widget *backend;
    ldWindow_t *ld_win;

    assert(app != 0);
    win = picoui_window_create_with_props(app, &props);
    assert(win != 0);
    assert(win->widget.bg_color == 0x112233);

    backend = (struct picoui_backend_widget *)win->widget.backend_widget;
    assert(backend != 0);
    ld_win = (ldWindow_t *)backend->ld_widget;
    assert(ld_win != 0);
    assert(ldWindowGetColor(ld_win) == __RGB(0x11, 0x22, 0x33));

    picoui_app_destroy(app);
}

static void test_window_public_constructors_keep_v2_direct_create_truth(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_window *win = picoui_window_create(app, "root_public_truth");
    struct picoui_background *bg = picoui_background_create(app, "bg_public_truth");
    struct picoui_backend_widget *win_backend;
    unsigned int bg_color = 0;
    int bg_offset_x = 0;
    int bg_offset_y = 0;

    assert(app != 0);
    assert(win != 0);
    assert(bg != 0);

    win_backend = (struct picoui_backend_widget *)win->widget.backend_widget;
    assert(win_backend != 0);
    assert(win_backend->kind == PICOUI_BACKEND_WIDGET_WINDOW);
    assert(win_backend->owner == app);
    assert(win_backend->root == win_backend);
    assert(picoui_background_set_color(bg, 0x224466U) == 0);
    assert(picoui_background_get_color(bg, &bg_color) == 0);
    assert(bg_color == 0x204462U);
    assert(picoui_background_set_offset(bg, 3, 7) == 0);
    assert(picoui_background_get_offset(bg, &bg_offset_x, &bg_offset_y) == 0);
    assert(bg_offset_x == 3);
    assert(bg_offset_y == 7);

    picoui_app_destroy(app);
}

int main(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_window *win;
    Dl_info self_info;

    assert(dladdr((void *)&main, &self_info) != 0);
    assert(self_info.dli_fname != 0);
    test_self_binary_path = self_info.dli_fname;

    test_window_color_internal_seam_uses_tinyui_names();
    test_window_layout_mapper_internal_seam_uses_tinyui_names();
    test_window_native_access_internal_seam_uses_tinyui_names();
    test_window_validation_internal_seam_uses_tinyui_names();
    test_window_init_defaults_internal_seam_uses_tinyui_names();
    test_window_root_size_internal_seam_uses_tinyui_names();
    test_window_dispose_partial_internal_seam_uses_tinyui_names();
    test_window_generic_gap_internal_seam_uses_tinyui_names();
    test_window_padding_contract_internal_seam_uses_tinyui_names();
    test_window_layout_type_internal_seam_uses_tinyui_names();
    test_window_flex_contract_internal_seam_uses_tinyui_names();
    test_window_constructor_binds_ld_without_backend_wrapper();
    test_window_public_constructors_keep_v2_direct_create_truth();
    test_window_create_with_props_failure_rolls_back_root_binding();
    test_window_create_with_props_applies_bg_color_without_backend_constructor();

    assert(app != 0);
    win = picoui_window_create(app, "root");
    assert(win != 0);

    test_window_create_and_backend_mapping(win);
    test_window_padding_group_round_trip(win);
    test_window_grid_padding_positions_switch(win);
    test_window_widget_base_api_round_trip(win);

    picoui_app_destroy(app);
    return 0;
}
