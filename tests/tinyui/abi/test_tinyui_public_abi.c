#include "core/event.h"
#include "core/focus.h"
#include "core/timer.h"
#include "integration/input.h"
#include "style/style.h"
#include "theme/theme.h"

#include <stddef.h>
#include <stdint.h>

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

int main(void)
{
    return 0;
}
