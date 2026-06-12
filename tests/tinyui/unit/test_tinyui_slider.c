#include "tinyui.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldSlider.h"
#include "internal.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int tinyui_widget_has_ld_binding(const struct picoui_widget *widget);
void tinyui_slider_test_fail_indicator_width_for_id(const char *id);
const struct picoui_backend_widget *tinyui_slider_test_last_disposed_backend(void);

static const char *test_source_file_path = __FILE__;

static void test_slider_create_and_backend_mapping(struct picoui_window *win)
{
    struct picoui_slider *slider = picoui_slider_create(win, "sl_test");
    struct picoui_backend_widget *backend;
    struct picoui_backend_widget *parent_backend;
    ldSlider_t *ld_slider;

    assert(slider != 0);
    backend = (struct picoui_backend_widget *)slider->widget.backend_widget;
    assert(backend != 0);
    parent_backend = (struct picoui_backend_widget *)win->widget.backend_widget;
    assert(parent_backend != 0);
    assert(backend->kind == PICOUI_BACKEND_WIDGET_SLIDER);
    assert(backend->owner == parent_backend->owner);
    assert(backend->root == parent_backend->root);
    assert(backend->parent == parent_backend);
    assert(backend->ld_name_id != 0);
    assert(backend->host_widget == &slider->widget);
    assert(backend->ld_event_bridge_scene != 0);
    assert(backend->ld_event_bridge_sender == backend->ld_widget);
    ld_slider = (ldSlider_t *)backend->ld_widget;
    assert(ld_slider != 0);
    assert(((ldBase_t *)ld_slider)->pInfo == backend);
    assert(slider->min_value == 0);
    assert(slider->max_value == 100);
    assert(slider->value == 0);
    assert(tinyui_widget_has_ld_binding(&slider->widget) == 1);
}

static void test_slider_create_with_props_pushes_range(struct picoui_window *win)
{
    struct picoui_slider *slider = picoui_slider_create_with_props(
        win,
        &(struct picoui_slider_props){
            .id = "sl_props",
            .min_value = 10,
            .max_value = 100,
            .value = 50,
        });
    struct picoui_backend_widget *backend;
    ldSlider_t *ld_slider;

    assert(slider != 0);
    assert(slider->value == 50);
    assert(slider->min_value == 10);
    assert(slider->max_value == 100);

    backend = (struct picoui_backend_widget *)slider->widget.backend_widget;
    assert(backend != 0);
    assert(backend->kind == PICOUI_BACKEND_WIDGET_SLIDER);

    ld_slider = (ldSlider_t *)backend->ld_widget;
    assert(ld_slider != 0);
    /* value=50 in range [10,100] => percent=(50-10)*100/(100-10)=44 => permille=440 */
    assert(ld_slider->permille == 440U);
}

static void test_slider_set_range_and_value_round_trip(struct picoui_window *win)
{
    struct picoui_slider *slider = picoui_slider_create(win, "sl_rnd");
    int percent;
    int horizontal;
    arm_2d_tile_t bg_tile = {0};
    arm_2d_tile_t bg_mask = {0};
    arm_2d_tile_t indic_tile = {0};
    arm_2d_tile_t indic_mask = {0};
    struct picoui_image_source background_source = {
        .img_tile = &bg_tile,
        .mask_tile = &bg_mask,
    };
    struct picoui_image_source indicator_source = {
        .img_tile = &indic_tile,
        .mask_tile = &indic_mask,
    };
    struct picoui_backend_widget *backend;
    ldSlider_t *ld_slider;

    assert(slider != 0);
    backend = (struct picoui_backend_widget *)slider->widget.backend_widget;
    assert(backend != 0);
    ld_slider = (ldSlider_t *)backend->ld_widget;
    assert(ld_slider != 0);

    assert(picoui_slider_set_range(slider, 0, 200) == 0);
    assert(slider->min_value == 0);
    assert(slider->max_value == 200);

    assert(picoui_slider_set_value(slider, 75) == 0);
    assert(slider->value == 75);

    /* percent = (75-0)*100/(200-0) = 37 */
    assert(picoui_slider_get_percent(slider, &percent) == 0);
    assert(percent == 37);

    /* set_value rejects out-of-range */
    assert(picoui_slider_set_value(slider, -1) == -1);
    assert(picoui_slider_set_value(slider, 201) == -1);
    assert(slider->value == 75);

    /* set_percent updates value */
    assert(picoui_slider_set_percent(slider, 100) == 0);
    assert(slider->value == 200);
    assert(picoui_slider_get_percent(slider, &percent) == 0);
    assert(percent == 100);

    /* set_percent rejects bounds */
    assert(picoui_slider_set_percent(slider, -1) == -1);
    assert(picoui_slider_set_percent(slider, 101) == -1);
    assert(slider->value == 200);

    assert(picoui_slider_set_horizontal(slider, 0) == 0);
    assert(picoui_slider_get_horizontal(slider, &horizontal) == 0);
    assert(horizontal == 0);
    assert(ld_slider->isHorizontal == false);

    assert(picoui_slider_set_horizontal(slider, 1) == 0);
    assert(picoui_slider_get_horizontal(slider, &horizontal) == 0);
    assert(horizontal == 1);
    assert(ld_slider->isHorizontal == true);

    assert(picoui_slider_set_background_source(slider, &background_source) == 0);
    assert(picoui_slider_set_indicator_source(slider, &indicator_source) == 0);
    assert(ld_slider->ptBgImgTile == &bg_tile);
    assert(ld_slider->ptBgMaskTile == &bg_mask);
    assert(ld_slider->ptIndicImgTile == &indic_tile);
    assert(ld_slider->ptIndicMaskTile == &indic_mask);

    assert(picoui_slider_set_indicator_width(slider, 21) == 0);
    assert(ld_slider->indicWidth == 21U);

    assert(picoui_slider_set_slim_size(slider, 9) == 0);
    assert(ld_slider->slimSize == 9U);
}

static void test_slider_create_with_props_failure_rolls_back_attached_child(struct picoui_window *win)
{
    struct picoui_backend_widget *parent_backend =
        (struct picoui_backend_widget *)win->widget.backend_widget;
    struct picoui_backend_widget *tail = parent_backend->first_child;
    struct picoui_backend_widget *next_before = 0;
    struct picoui_slider *slider;
    const struct picoui_backend_widget *disposed_backend;

    while (tail != 0 && tail->next_sibling != 0) {
        tail = tail->next_sibling;
    }
    if (tail != 0) {
        next_before = tail->next_sibling;
    }

    tinyui_slider_test_fail_indicator_width_for_id("sl_fail_indicator_width");
    slider = picoui_slider_create_with_props(
        win,
        &(struct picoui_slider_props){
            .id = "sl_fail_indicator_width",
            .min_value = 0,
            .max_value = 100,
            .value = 10,
            .has_indicator_width = 1,
            .indicator_width = 12,
        });

    assert(slider == 0);
    disposed_backend = tinyui_slider_test_last_disposed_backend();
    assert(disposed_backend != 0);
    assert(disposed_backend->kind == PICOUI_BACKEND_WIDGET_SLIDER);
    assert(disposed_backend->parent == 0);
    assert(disposed_backend->owner == 0);
    assert(disposed_backend->root == 0);
    assert(disposed_backend->host_widget == 0);
    assert(disposed_backend->ld_event_bridge_scene == 0);
    assert(disposed_backend->ld_event_bridge_sender == 0);
    assert(disposed_backend->ld_event_bridge_next == 0);
    assert(disposed_backend->next_sibling == 0);
    assert(((const ldBase_t *)disposed_backend->ld_widget)->pInfo == 0);
    if (tail != 0) {
        assert(tail->next_sibling == next_before);
    } else {
        assert(parent_backend->first_child == 0);
    }

    assert(picoui_slider_create_with_props(
               win,
               &(struct picoui_slider_props){
                   .id = "sl_fail_indicator_width",
                   .min_value = 0,
                   .max_value = 100,
                   .value = 10,
                   .has_indicator_width = 1,
                   .indicator_width = 12,
               }) != 0);
}

static void test_slider_rejects_null_args(struct picoui_window *win)
{
    assert(picoui_slider_create(0, "id") == 0);
    assert(picoui_slider_create(win, 0) == 0);
    assert(picoui_slider_init(0, "id") == 0);
    assert(picoui_slider_init(win, 0) == 0);
    assert(picoui_slider_create_with_props(0, &(struct picoui_slider_props){.id = "p"}) == 0);
    assert(picoui_slider_create_with_props(win, 0) == 0);
    assert(picoui_slider_create_with_props(win, &(struct picoui_slider_props){.id = 0}) == 0);
    assert(picoui_slider_set_value(0, 10) == -1);
    assert(picoui_slider_set_range(0, 0, 100) == -1);
    assert(picoui_slider_set_percent(0, 50) == -1);
    assert(picoui_slider_set_horizontal(0, 1) == -1);
    assert(picoui_slider_get_percent(0, &(int){0}) == -1);
    assert(picoui_slider_set_on_value_changed(0, 0, 0) == -1);
}

static int file_contains_pattern(const char *path, const char *pattern)
{
    FILE *fp = fopen(path, "rb");
    long size;
    char *buffer;
    int found = 0;

    if (fp == NULL) {
        return 0;
    }

    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        return 0;
    }
    size = ftell(fp);
    if (size < 0 || fseek(fp, 0, SEEK_SET) != 0) {
        fclose(fp);
        return 0;
    }

    buffer = (char *)malloc((size_t)size + 1U);
    if (buffer == NULL) {
        fclose(fp);
        return 0;
    }

    if (fread(buffer, 1, (size_t)size, fp) == (size_t)size) {
        buffer[size] = '\0';
        found = strstr(buffer, pattern) != NULL;
    }

    free(buffer);
    fclose(fp);
    return found;
}

static const char *resolve_repo_path(const char *repo_relative_path)
{
    static char resolved_path[1024];
    char base_path[1024];
    char *tests_dir;
    size_t base_len;

    assert(test_source_file_path != 0);
    assert(repo_relative_path != 0);
    assert(strlen(test_source_file_path) < sizeof(base_path));
    snprintf(base_path, sizeof(base_path), "%s", test_source_file_path);
    tests_dir = strstr(base_path, "tests/tinyui/unit/");
    assert(tests_dir != 0);
    *tests_dir = '\0';
    base_len = strlen(base_path);
    assert(base_len + strlen(repo_relative_path) + 1 < sizeof(resolved_path));
    snprintf(resolved_path, sizeof(resolved_path), "%s%s", base_path, repo_relative_path);
    return resolved_path;
}

static void test_slider_internal_seams_are_renamed(void)
{
    static const char *old_internal_symbols[] = {
        "picoui_slider_fail_indicator_width_id",
        "picoui_slider_last_disposed_backend_snapshot",
        "picoui_slider_last_disposed_backend_valid",
        "picoui_slider_backend(",
        "picoui_slider_get_ld(",
        "picoui_slider_should_fail_indicator_width(",
        "picoui_slider_dispose_partial(",
        "picoui_slider_props_are_valid(",
        "picoui_backend_slider_test_fail_indicator_width_for_id(",
        "picoui_backend_slider_test_last_disposed_backend(",
    };
    const char *slider_source_path = resolve_repo_path("tinyui/src/widgets/slider.c");
    size_t i;

    assert(file_contains_pattern(slider_source_path, "tinyui_slider_backend(") != 0);
    assert(file_contains_pattern(slider_source_path, "tinyui_slider_get_ld(") != 0);
    assert(file_contains_pattern(slider_source_path, "tinyui_slider_props_are_valid(") != 0);
    assert(file_contains_pattern(slider_source_path,
                                 "tinyui_slider_test_last_disposed_backend(")
           != 0);

    for (i = 0; i < sizeof(old_internal_symbols) / sizeof(old_internal_symbols[0]); ++i) {
        assert(file_contains_pattern(slider_source_path, old_internal_symbols[i]) == 0);
    }
}

int main(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_window *win;

    assert(app != 0);
    win = picoui_window_create(app, "root");
    assert(win != 0);

    test_slider_create_and_backend_mapping(win);
    test_slider_create_with_props_pushes_range(win);
    test_slider_create_with_props_failure_rolls_back_attached_child(win);
    test_slider_set_range_and_value_round_trip(win);
    test_slider_rejects_null_args(win);
    test_slider_internal_seams_are_renamed();

    picoui_app_destroy(app);
    return 0;
}
