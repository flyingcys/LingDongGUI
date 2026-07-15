#include "internal.h"
#include "internal/runtime_pools.h"

#include <stdio.h>
#include <stdint.h>

static int assert_size_le(const char *name, size_t actual, size_t limit)
{
    if (actual > limit) {
        fprintf(stderr,
                "%s size regression: expected <= %zu, got %zu\n",
                name,
                limit,
                actual);
        return 1;
    }
    return 0;
}

int main(void)
{
    int failed = 0;
    size_t timer_pool_bytes = sizeof(struct tinyui_timer_pool);
    size_t event_pool_bytes = sizeof(struct tinyui_event_callback_pool);
    size_t bookkeeping_bytes = sizeof(struct tinyui_runtime_bookkeeping);
    size_t pool_sum_bytes = timer_pool_bytes + event_pool_bytes + bookkeeping_bytes;

    printf("TINYUI_POOL_TIMER_BYTES=%zu\n", timer_pool_bytes);
    printf("TINYUI_POOL_EVENT_CALLBACK_BYTES=%zu\n", event_pool_bytes);
    printf("TINYUI_POOL_BOOKKEEPING_BYTES=%zu\n", bookkeeping_bytes);
    printf("TINYUI_POOL_SUM_BYTES=%zu\n", pool_sum_bytes);

    /*
     * 32-bit hard RAM budget is only enforceable on a 32-bit ABI. Host 64-bit
     * sizes are printed for diagnostics and must not be recorded as a pass of
     * the 1024 B gate.
     */
#if defined(UINTPTR_MAX) && defined(UINT32_MAX) && (UINTPTR_MAX == UINT32_MAX)
    failed |= assert_size_le("32-bit runtime pool sum", pool_sum_bytes, 1024U);
    printf("TINYUI_ABI32_POOL_BUDGET_OK sum=%zu\n", pool_sum_bytes);
#else
    printf("TINYUI_ABI32_POOL_BUDGET_HOST_ONLY sum=%zu (not 32-bit ABI)\n",
           pool_sum_bytes);
#endif

    return failed;
}
