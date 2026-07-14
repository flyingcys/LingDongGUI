#include "core/runtime.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

struct tinyui_test_allocator_stats {
    size_t alloc_calls;
    size_t calloc_calls;
    size_t realloc_calls;
    size_t free_calls;
    size_t bytes_requested;
};

void tinyui_test_allocator_reset(void);
struct tinyui_test_allocator_stats tinyui_test_allocator_snapshot(void);
void *tinyui_test_allocator_malloc(uint32_t size);
void *tinyui_test_allocator_calloc(uint32_t num, uint32_t size);
void *tinyui_test_allocator_realloc(void *ptr, uint32_t size);
void tinyui_test_allocator_free(void *ptr);

void *ldMalloc(uint32_t size)
{
    return tinyui_test_allocator_malloc(size);
}

void *ldCalloc(uint32_t num, uint32_t size)
{
    return tinyui_test_allocator_calloc(num, size);
}

void *ldRealloc(void *ptr, uint32_t size)
{
    return tinyui_test_allocator_realloc(ptr, size);
}

void ldFree(void *ptr)
{
    tinyui_test_allocator_free(ptr);
}

static void assert_zero_allocations(const struct tinyui_test_allocator_stats *stats)
{
    assert(stats->alloc_calls == 0U);
    assert(stats->calloc_calls == 0U);
    assert(stats->realloc_calls == 0U);
    assert(stats->free_calls == 0U);
    assert(stats->bytes_requested == 0U);
}

int main(void)
{
    tinyui_obj_t *screen;
    struct tinyui_test_allocator_stats stats;
    uint32_t next_ms = UINT32_MAX;
    unsigned int round;

    assert(tinyui_init() == TINYUI_OK);
    screen = tinyui_screen_create();
    assert(screen != NULL);
    assert(tinyui_screen_load(screen, TINYUI_SCREEN_TRANSITION_NONE, 0) == TINYUI_OK);
    assert(tinyui_process(&next_ms) == TINYUI_OK);

    tinyui_test_allocator_reset();
    for (round = 0U; round < 100U; ++round) {
        assert(tinyui_process(&next_ms) == TINYUI_OK);
        stats = tinyui_test_allocator_snapshot();
        assert_zero_allocations(&stats);
    }

    stats = tinyui_test_allocator_snapshot();
    assert_zero_allocations(&stats);
    printf("TINYUI_STEADY_STATE_ALLOCATIONS=0\n");
    tinyui_deinit();
    return 0;
}
