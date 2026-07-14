#include "core/obj.h"
#include "core/runtime.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

uint32_t arm_2d_helper_get_reference_clock_frequency(void)
{
    return 1000000u;
}

static void test_process_requires_initialized_runtime(void)
{
    uint32_t next_ms = 123u;

    tinyui_deinit();
    assert(tinyui_process(&next_ms) == TINYUI_ERROR_INVALID_STATE);
    assert(next_ms == 123u);
    assert(tinyui_process(NULL) == TINYUI_ERROR_INVALID_ARG);
}

static void test_canonical_runtime_and_object_tree(void)
{
    tinyui_obj_t *screen;
    uint32_t next_ms = 0;
    uint16_t id = 0;
    uint16_t child_count = 0;

    assert(tinyui_init() == TINYUI_OK);
    assert(tinyui_init() == TINYUI_OK);
    screen = tinyui_screen_create();
    assert(screen != NULL);
    assert(tinyui_screen_load(screen, TINYUI_SCREEN_TRANSITION_NONE, 0) == TINYUI_OK);
    assert(tinyui_screen_active() == screen);
    assert(tinyui_process(&next_ms) == TINYUI_OK);
    assert(next_ms == UINT32_MAX);

    assert(tinyui_screen_load(screen, TINYUI_SCREEN_TRANSITION_SLIDE_LEFT, 25) ==
           TINYUI_ERROR_NOT_SUPPORTED);
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
}

int main(void)
{
    test_process_requires_initialized_runtime();
    test_canonical_runtime_and_object_tree();
    return 0;
}
