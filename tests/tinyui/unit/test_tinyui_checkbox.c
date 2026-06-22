#include "tinyui.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldCheckBox.h"
#include "internal.h"
#include <assert.h>
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>

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

static void assert_self_binary_lacks_symbol(const char *symbol)
{
    char command[1024];
    FILE *pipe;
    char line[512];

    assert(test_self_binary_path != 0);
    assert(symbol != 0);
    snprintf(command, sizeof(command), "nm %s 2>/dev/null", test_self_binary_path);
    pipe = popen(command, "r");
    assert(pipe != 0);
    while (fgets(line, sizeof(line), pipe) != 0) {
        size_t line_len = strlen(line);
        char *last_space;
        char *token;

        while (line_len > 0 && (line[line_len - 1] == '\n' || line[line_len - 1] == '\r')) {
            line[--line_len] = '\0';
        }
        last_space = strrchr(line, ' ');
        token = last_space != 0 ? last_space + 1 : line;
        if (strcmp(token, symbol) == 0) {
            assert(!"unexpected symbol still present in test binary");
        }
    }
    assert(pclose(pipe) == 0);
}

static void assert_source_lacks_text(const char *source_path, const char *needle)
{
    char command[1024];

    assert(source_path != 0);
    assert(needle != 0);
    snprintf(command,
             sizeof(command),
             "python3 - '%s' '%s' <<'PY'\n"
             "from pathlib import Path\n"
             "import sys\n"
             "text = Path(sys.argv[1]).read_text()\n"
             "raise SystemExit(1 if sys.argv[2] in text else 0)\n"
             "PY",
             source_path,
             needle);
    assert(system(command) == 0);
}

static void assert_source_contains_text(const char *source_path, const char *needle)
{
    char command[1024];

    assert(source_path != 0);
    assert(needle != 0);
    snprintf(command,
             sizeof(command),
             "python3 - '%s' '%s' <<'PY'\n"
             "from pathlib import Path\n"
             "import sys\n"
             "text = Path(sys.argv[1]).read_text()\n"
             "raise SystemExit(0 if sys.argv[2] in text else 1)\n"
             "PY",
             source_path,
             needle);
    assert(system(command) == 0);
}

static unsigned int test_rgb_to_ld_color(unsigned int rgb)
{
    return (unsigned int)__RGB((rgb >> 16) & 0xFFU, (rgb >> 8) & 0xFFU, rgb & 0xFFU);
}

static void assert_checkbox_has_bound_images(const struct tinyui_checkbox *checkbox,
                                             const struct tinyui_image_source *expected_unchecked,
                                             const struct tinyui_image_source *expected_checked)
{
    const struct tinyui_widget *backend;
    const ldCheckBox_t *ld_checkbox;

    assert(checkbox != 0);
    backend = &checkbox->widget;
    assert(backend->ld_widget != 0);
    ld_checkbox = (const ldCheckBox_t *)backend->ld_widget;
    assert(ld_checkbox != 0);
    assert(ld_checkbox->ptUncheckedImgTile
           == (expected_unchecked != 0 ? expected_unchecked->img_tile : 0));
    assert(ld_checkbox->ptUncheckedMaskTile
           == (expected_unchecked != 0 ? expected_unchecked->mask_tile : 0));
    assert(ld_checkbox->ptCheckedImgTile
           == (expected_checked != 0 ? expected_checked->img_tile : 0));
    assert(ld_checkbox->ptCheckedMaskTile
           == (expected_checked != 0 ? expected_checked->mask_tile : 0));
}

static void test_checkbox_create_and_ld_mapping(struct tinyui_window *win)
{
    struct tinyui_checkbox *checkbox = tinyui_checkbox_create(win, "cb_test");
    struct tinyui_widget *backend;
    ldBase_t *ld_base;

    assert(checkbox != 0);
    backend = &checkbox->widget;
    assert(backend->ld_widget != 0);
    assert(backend->kind == TINYUI_BACKEND_WIDGET_CHECKBOX);
    ld_base = (ldBase_t *)backend->ld_widget;
    assert(ld_base != 0);
    assert(ld_base->widgetType == widgetTypeCheckBox);
}

static void test_checkbox_create_with_props_pushes_all_fields(struct tinyui_window *win)
{
    struct tinyui_checkbox *checkbox = tinyui_checkbox_create_with_props(
        win,
        &(struct tinyui_checkbox_props){
            .id = "cb_props",
            .text = "Agree",
            .checked = 1,
        });
    struct tinyui_widget *backend;
    ldCheckBox_t *ld_checkbox;

    assert(checkbox != 0);
    backend = &checkbox->widget;
    assert(backend->ld_widget != 0);
    assert(backend->text != 0);
    assert(strcmp(backend->text, "Agree") == 0);
    ld_checkbox = (ldCheckBox_t *)backend->ld_widget;
    assert(ld_checkbox != 0);
    assert(ld_checkbox->isChecked == true);
}

static void test_checkbox_set_checked_round_trip(struct tinyui_window *win)
{
    struct tinyui_checkbox *checkbox = tinyui_checkbox_create(win, "cb_checked");
    struct tinyui_widget *backend;
    ldCheckBox_t *ld_checkbox;

    assert(checkbox != 0);
    backend = &checkbox->widget;
    assert(backend->ld_widget != 0);
    ld_checkbox = (ldCheckBox_t *)backend->ld_widget;
    assert(ld_checkbox != 0);

    assert(tinyui_checkbox_set_checked(checkbox, 1) == 0);
    assert(ld_checkbox->isChecked == true);
    assert(checkbox->checked == 1);
    assert(tinyui_checkbox_is_checked(checkbox) == 1);

    assert(tinyui_checkbox_set_checked(checkbox, 0) == 0);
    assert(ld_checkbox->isChecked == false);
    assert(checkbox->checked == 0);
    assert(tinyui_checkbox_is_checked(checkbox) == 0);
}

static void test_checkbox_set_text_round_trip(struct tinyui_window *win)
{
    struct tinyui_checkbox *checkbox = tinyui_checkbox_create(win, "cb_text");
    struct tinyui_widget *backend;

    assert(checkbox != 0);
    assert(tinyui_checkbox_set_text(checkbox, "Label") == 0);
    backend = &checkbox->widget;
    assert(backend->text != 0);
    assert(strcmp(backend->text, "Label") == 0);
}

static void test_checkbox_shared_text_helper_uses_tinyui_prefix(void)
{
    assert_source_contains_text(test_repo_path("tinyui/src/core/widget.c"),
                                "tinyui_widget_set_backend_text");
    assert_source_contains_text(test_repo_path("tinyui/src/widgets/checkbox.c"),
                                "tinyui_widget_set_backend_text");
    assert_self_binary_lacks_symbol("tinyui_backend_set_text");
}

static void test_checkbox_internal_seams_use_tinyui_prefix(void)
{
    const char *source_path = test_repo_path("tinyui/src/widgets/checkbox.c");

    /* C2 migration: legacy private seams collapsed into core helpers
     * (tinyui_widget_create_leaf / tinyui_widget_destroy_common /
     *  tinyui_rgb_to_ld_color). Verify the old names are gone and the
     *  new backend accessor + props validator are present. */
    assert_source_contains_text(source_path, "tinyui_checkbox_backend(");
    assert_source_contains_text(source_path, "checkbox_props_valid(");
    assert_source_lacks_text(source_path, "tinyui_checkbox_fail_next_set_check_color");
    assert_source_lacks_text(source_path, "tinyui_checkbox_rgb_to_ld_color");
    assert_source_lacks_text(source_path, "tinyui_checkbox_get_ld");
    assert_source_lacks_text(source_path, "tinyui_checkbox_dispose_partial");
    assert_source_lacks_text(source_path, "tinyui_checkbox_test_fail_next_set_check_color");
}

static void test_checkbox_native_helper_behaviors(struct tinyui_window *win)
{
    struct tinyui_checkbox *checkbox = tinyui_checkbox_create(win, "cb_native");
    struct tinyui_widget *backend;
    ldCheckBox_t *ld_checkbox;
    arm_2d_tile_t unchecked_tile = {0};
    arm_2d_tile_t unchecked_mask_tile = {0};
    arm_2d_tile_t checked_tile = {0};
    arm_2d_tile_t checked_mask_tile = {0};
    struct tinyui_image_source unchecked_source = {
        .img_tile = &unchecked_tile,
        .mask_tile = &unchecked_mask_tile,
    };
    struct tinyui_image_source checked_source = {
        .img_tile = &checked_tile,
        .mask_tile = &checked_mask_tile,
    };

    assert(checkbox != 0);
    backend = &checkbox->widget;
    assert(backend->ld_widget != 0);
    ld_checkbox = (ldCheckBox_t *)backend->ld_widget;
    assert(ld_checkbox != 0);

    assert(tinyui_checkbox_set_check_color(checkbox, 0xAA5500U) == 0);
    assert(ld_checkbox->fgColor == (ldColor)test_rgb_to_ld_color(0xAA5500U));
    assert(ld_checkbox->ptUncheckedImgTile == 0);
    assert(ld_checkbox->ptCheckedImgTile == 0);

    assert(tinyui_checkbox_set_text_color(checkbox, 0x224466U) == 0);
    assert(checkbox->widget.text_color == 0x224466U);
    assert(ld_checkbox->textColor == (ldColor)test_rgb_to_ld_color(0x224466U));

    assert(tinyui_checkbox_set_unchecked_source(checkbox, &unchecked_source) == 0);
    assert_checkbox_has_bound_images(checkbox, &unchecked_source, 0);
    assert(tinyui_checkbox_set_checked_source(checkbox, &checked_source) == 0);
    assert_checkbox_has_bound_images(checkbox, &unchecked_source, &checked_source);

    assert(tinyui_checkbox_set_radio_group(checkbox, 7) == 0);
    assert(ld_checkbox->isRadioButton == true);
    assert(ld_checkbox->radioButtonGroup == 7);

    assert(tinyui_checkbox_set_string_left_space(checkbox, 22) == 0);
    assert(ld_checkbox->boxWidth == 22);

    assert(tinyui_checkbox_set_check_color(checkbox, 0x003366U) == 0);
    assert(ld_checkbox->fgColor == (ldColor)test_rgb_to_ld_color(0x003366U));
    assert_checkbox_has_bound_images(checkbox, 0, 0);
    assert(ld_checkbox->boxWidth == 14);
}

static void test_checkbox_rejects_null_args(struct tinyui_window *win)
{
    struct tinyui_checkbox *checkbox = tinyui_checkbox_create(win, "cb_invalid");
    struct tinyui_widget *backend;
    ldCheckBox_t *ld_checkbox;
    arm_2d_tile_t unchecked_mask_tile = {0};
    arm_2d_tile_t checked_mask_tile = {0};
    struct tinyui_image_source invalid_unchecked_source = {
        .img_tile = 0,
        .mask_tile = &unchecked_mask_tile,
    };
    struct tinyui_image_source invalid_checked_source = {
        .img_tile = 0,
        .mask_tile = &checked_mask_tile,
    };

    assert(checkbox != 0);
    backend = &checkbox->widget;
    assert(backend->ld_widget != 0);
    ld_checkbox = (ldCheckBox_t *)backend->ld_widget;
    assert(ld_checkbox != 0);

    assert(tinyui_checkbox_create(0, "id") == 0);
    assert(tinyui_checkbox_create(win, 0) == 0);
    assert(tinyui_checkbox_set_text(0, "text") == -1);
    assert(tinyui_checkbox_set_checked(0, 1) == -1);
    assert(tinyui_checkbox_set_unchecked_source(0, &invalid_unchecked_source) == -1);
    assert(tinyui_checkbox_set_checked_source(0, &invalid_checked_source) == -1);
    assert(tinyui_checkbox_set_unchecked_source(checkbox, &invalid_unchecked_source) == -1);
    assert(tinyui_checkbox_set_checked_source(checkbox, &invalid_checked_source) == -1);
    assert(tinyui_checkbox_set_radio_group(0, 1) == -1);
    assert(tinyui_checkbox_set_radio_group(checkbox, -1) == -1);
    assert(tinyui_checkbox_set_radio_group(checkbox, 256) == -1);
    assert(tinyui_checkbox_set_string_left_space(0, 10) == -1);
    assert(tinyui_checkbox_set_string_left_space(checkbox, -1) == -1);
    assert(tinyui_checkbox_set_check_color(0, 0x123456U) == -1);
    assert(tinyui_checkbox_set_text_color(0, 0x123456U) == -1);
    assert(ld_checkbox->radioButtonGroup == 0);
    assert(ld_checkbox->boxWidth == 14);
}

int main(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *win;
    Dl_info self_info;
    assert(app != 0);
    assert(dladdr((void *)&main, &self_info) != 0);
    test_self_binary_path = self_info.dli_fname;
    win = tinyui_window_create(app, "root");
    assert(win != 0);

    test_checkbox_create_and_ld_mapping(win);
    test_checkbox_create_with_props_pushes_all_fields(win);
    test_checkbox_set_checked_round_trip(win);
    test_checkbox_set_text_round_trip(win);
    test_checkbox_shared_text_helper_uses_tinyui_prefix();
    test_checkbox_internal_seams_use_tinyui_prefix();
    test_checkbox_native_helper_behaviors(win);
    test_checkbox_rejects_null_args(win);

    tinyui_app_destroy(app);
    return 0;
}
