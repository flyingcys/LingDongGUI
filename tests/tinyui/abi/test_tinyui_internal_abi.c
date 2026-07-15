#include "internal/runtime_pools.h"
#include "resource/font.h"
#include "resource/image_source.h"

#include <stdio.h>
#include <stdint.h>

/*
 * Host may be 64-bit; the hard 32-bit budget is only asserted when the
 * translation unit is compiled for a 32-bit ABI. CMake also exposes
 * tinyui_abi32_compile for an MCU/32-bit toolchain compile-only check.
 */
#if defined(UINTPTR_MAX) && defined(UINT32_MAX) && (UINTPTR_MAX == UINT32_MAX)

_Static_assert(sizeof(struct tinyui_timer_pool) +
                   sizeof(struct tinyui_event_callback_pool) +
                   sizeof(struct tinyui_runtime_bookkeeping) <= 1024,
               "TinyUI static runtime budget exceeds 1024 B on 32-bit ABI");
_Static_assert(sizeof(tinyui_image_source_t) <= 80,
               "tinyui_image_source_t exceeds 80 B on 32-bit ABI");
_Static_assert(sizeof(tinyui_font_t) <= 16,
               "tinyui_font_t exceeds 16 B on 32-bit ABI");

#define TINYUI_ABI32_EXECUTED 1

#else

#define TINYUI_ABI32_EXECUTED 0

#endif

int main(void)
{
#if TINYUI_ABI32_EXECUTED
    printf("ABI32_EXECUTED sizes timer_pool=%zu event_pool=%zu bookkeeping=%zu "
           "image=%zu font=%zu\n",
           sizeof(struct tinyui_timer_pool),
           sizeof(struct tinyui_event_callback_pool),
           sizeof(struct tinyui_runtime_bookkeeping),
           sizeof(tinyui_image_source_t),
           sizeof(tinyui_font_t));
#else
    /* Do not record host skip as a 32-bit pass. */
    printf("ABI32_NOT_EXECUTED_ON_HOST\n");
    printf("host_probe_sizes timer_pool=%zu event_pool=%zu bookkeeping=%zu "
           "image=%zu font=%zu (not 32-bit ABI)\n",
           sizeof(struct tinyui_timer_pool),
           sizeof(struct tinyui_event_callback_pool),
           sizeof(struct tinyui_runtime_bookkeeping),
           sizeof(tinyui_image_source_t),
           sizeof(tinyui_font_t));
#endif
    return 0;
}
