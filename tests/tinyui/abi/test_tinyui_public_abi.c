#include "core/event.h"
#include "core/focus.h"
#include "core/timer.h"
#include "integration/input.h"
#include "resource/font.h"
#include "resource/image_source.h"
#include "style/style.h"
#include "theme/theme.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

_Static_assert(sizeof(tinyui_event_handle_t) == sizeof(uint32_t),
               "event handle ABI must be 32-bit");
_Static_assert(TINYUI_EVENT_CB_CAPACITY == 16, "event pool ABI changed");
_Static_assert(TINYUI_TIMER_CAPACITY == 16, "timer pool ABI changed");
_Static_assert(TINYUI_EVENT_MASK(TINYUI_EVENT_PRESSED) == UINT32_C(1),
               "event mask encoding changed");
_Static_assert(TINYUI_EVENT_MASK(TINYUI_EVENT_DELETE) == UINT32_C(128),
               "event mask encoding changed");
_Static_assert(offsetof(tinyui_style_t, fields) == 0,
               "style presence mask must be the first field");
_Static_assert(sizeof(((tinyui_style_t *)0)->fields) == sizeof(uint32_t),
               "style presence mask ABI changed");
_Static_assert(sizeof(((tinyui_style_t *)0)->font) == sizeof(void *),
               "style font descriptor must be a borrowed pointer");
_Static_assert(sizeof(tinyui_theme_t) ==
                   TINYUI_COLOR_COUNT * sizeof(uint32_t) +
                   TINYUI_METRIC_COUNT * sizeof(int16_t),
               "theme descriptor must not grow hidden runtime state");

/*
 * Descriptor budgets are always host-asserted with pointer-width awareness:
 * 32-bit ABI is the product target; 64-bit host may exceed the absolute bytes
 * because private uintptr storage doubles. Absolute 32-bit limits live in
 * test_tinyui_internal_abi.c / tinyui_abi32_compile.
 */
#if defined(UINTPTR_MAX) && defined(UINT32_MAX) && (UINTPTR_MAX == UINT32_MAX)
_Static_assert(sizeof(tinyui_image_source_t) <= 80,
               "tinyui_image_source_t exceeds 80 B on 32-bit ABI");
_Static_assert(sizeof(tinyui_font_t) <= 16,
               "tinyui_font_t exceeds 16 B on 32-bit ABI");
#endif

int main(void)
{
#if defined(UINTPTR_MAX) && defined(UINT32_MAX) && (UINTPTR_MAX == UINT32_MAX)
    printf("public_abi32 image=%zu font=%zu\n",
           sizeof(tinyui_image_source_t),
           sizeof(tinyui_font_t));
#else
    printf("ABI32_NOT_EXECUTED_ON_HOST\n");
    printf("public_abi_host image=%zu font=%zu\n",
           sizeof(tinyui_image_source_t),
           sizeof(tinyui_font_t));
#endif
    return 0;
}
