#include "core/obj.h"
#include "core/runtime.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

static void test_public_types_and_symbols(void)
{
    tinyui_result_t (*init_fn)(void) = tinyui_init;
    void (*deinit_fn)(void) = tinyui_deinit;
    tinyui_obj_t *(*screen_create_fn)(void) = tinyui_screen_create;
    tinyui_obj_t *(*screen_create_with_props_fn)(const tinyui_window_props_t *) =
        tinyui_screen_create_with_props;
    tinyui_obj_t *(*background_create_fn)(void) = tinyui_background_create;
    tinyui_obj_t *(*background_create_with_props_fn)(const tinyui_background_props_t *) =
        tinyui_background_create_with_props;
    tinyui_result_t (*load_fn)(tinyui_obj_t *, tinyui_screen_transition_t, uint32_t) =
        tinyui_screen_load;
    tinyui_obj_t *(*active_fn)(void) = tinyui_screen_active;
    tinyui_result_t (*process_fn)(uint32_t *) = tinyui_process;
    tinyui_result_t (*delete_fn)(tinyui_obj_t *) = tinyui_obj_delete;
    tinyui_result_t (*get_id_fn)(const tinyui_obj_t *, uint16_t *) = tinyui_obj_get_id;
    tinyui_obj_t *(*find_fn)(tinyui_obj_t *, uint16_t) = tinyui_obj_find_by_id;
    tinyui_obj_t *(*parent_fn)(const tinyui_obj_t *) = tinyui_obj_get_parent;
    tinyui_obj_t *(*first_fn)(const tinyui_obj_t *) = tinyui_obj_get_first_child;
    tinyui_obj_t *(*sibling_fn)(const tinyui_obj_t *) = tinyui_obj_get_next_sibling;
    tinyui_obj_t *(*root_fn)(const tinyui_obj_t *) = tinyui_obj_get_root;
    tinyui_result_t (*count_fn)(const tinyui_obj_t *, uint16_t *) = tinyui_obj_get_child_count;

    (void)init_fn;
    (void)deinit_fn;
    (void)screen_create_fn;
    (void)screen_create_with_props_fn;
    (void)background_create_fn;
    (void)background_create_with_props_fn;
    (void)load_fn;
    (void)active_fn;
    (void)process_fn;
    (void)delete_fn;
    (void)get_id_fn;
    (void)find_fn;
    (void)parent_fn;
    (void)first_fn;
    (void)sibling_fn;
    (void)root_fn;
    (void)count_fn;

    _Static_assert(TINYUI_SCREEN_TRANSITION_NONE == 0, "transition order changed");
    _Static_assert(TINYUI_SCREEN_TRANSITION_FLY_IN_BOTTOM == 14,
                   "canonical transition set must contain 15 entries");
    _Static_assert(TINYUI_ERROR_BACKEND == 8, "canonical result set changed");
}

static void test_runtime_and_object_model(void)
{
    tinyui_obj_t *background;
    tinyui_obj_t *screen;
    uint32_t next_ms = UINT32_MAX;
    uint16_t id = 0;
    uint16_t child_count = 0;

    assert(tinyui_obj_delete(NULL) == TINYUI_ERROR_INVALID_OBJECT);
    assert(tinyui_obj_get_id(NULL, &id) == TINYUI_ERROR_INVALID_OBJECT);
    assert(tinyui_obj_get_id(NULL, NULL) == TINYUI_ERROR_INVALID_OBJECT);
    assert(tinyui_obj_find_by_id(NULL, 1) == NULL);
    assert(tinyui_obj_get_parent(NULL) == NULL);
    assert(tinyui_obj_get_first_child(NULL) == NULL);
    assert(tinyui_obj_get_next_sibling(NULL) == NULL);
    assert(tinyui_obj_get_root(NULL) == NULL);
    assert(tinyui_obj_get_child_count(NULL, &child_count) == TINYUI_ERROR_INVALID_OBJECT);
    assert(tinyui_obj_get_child_count(NULL, NULL) == TINYUI_ERROR_INVALID_OBJECT);

    assert(tinyui_init() == TINYUI_OK);
    assert(tinyui_screen_create_with_props(NULL) == NULL);
    assert(tinyui_background_create_with_props(NULL) == NULL);
    screen = tinyui_screen_create();
    assert(screen != NULL);
    assert(tinyui_screen_load(screen, TINYUI_SCREEN_TRANSITION_NONE, 0) == TINYUI_OK);
    assert(tinyui_screen_active() == screen);
    assert(tinyui_process(&next_ms) == TINYUI_OK);
    assert(next_ms == UINT32_MAX);

    /* M2 Task 1: transitions map to Arm-2D modes; unknown values only are OOR. */
    assert(tinyui_screen_load(screen, TINYUI_SCREEN_TRANSITION_FADE_WHITE, 100) ==
           TINYUI_OK);
    assert(tinyui_screen_active() == screen);
    assert(tinyui_screen_load(screen, (tinyui_screen_transition_t)99, 0) ==
           TINYUI_ERROR_OUT_OF_RANGE);
    assert(tinyui_obj_get_id(screen, &id) == TINYUI_OK);
    assert(id != 0);
    assert(tinyui_obj_find_by_id(screen, id) == screen);
    assert(tinyui_obj_get_parent(screen) == NULL);
    assert(tinyui_obj_get_first_child(screen) == NULL);
    assert(tinyui_obj_get_next_sibling(screen) == NULL);
    assert(tinyui_obj_get_root(screen) == screen);
    assert(tinyui_obj_get_child_count(screen, &child_count) == TINYUI_OK);
    assert(child_count == 0);

    tinyui_deinit();
    assert(tinyui_screen_active() == NULL);

    assert(tinyui_init() == TINYUI_OK);
    background = tinyui_background_create();
    assert(background != NULL);
    assert(tinyui_screen_load(background, TINYUI_SCREEN_TRANSITION_NONE, 0) == TINYUI_OK);
    assert(tinyui_screen_active() == background);
    tinyui_deinit();
}

int main(void)
{
    test_public_types_and_symbols();
    test_runtime_and_object_model();
    return 0;
}
