#include "picoui/display.h"
#include "picoui/screen.h"
#include "picoui/widget.h"

static const struct picoui_widget *g_picoui_dirty_widget;
static const struct picoui_screen *g_picoui_dirty_screen;
static const struct picoui_display *g_picoui_dirty_display;

void picoui_native_dirty_reset(void)
{
    g_picoui_dirty_widget = 0;
    g_picoui_dirty_screen = 0;
    g_picoui_dirty_display = 0;
}

void picoui_native_dirty_mark(const struct picoui_widget *widget, const struct picoui_screen *screen)
{
    g_picoui_dirty_widget = widget;
    g_picoui_dirty_screen = screen;
    g_picoui_dirty_display = picoui_display_get_default();
}

int picoui_native_dirty_is_widget_dirty(const struct picoui_widget *widget)
{
    return widget != 0 && widget == g_picoui_dirty_widget;
}

int picoui_native_dirty_is_screen_dirty(const struct picoui_screen *screen)
{
    return screen != 0 && screen == g_picoui_dirty_screen;
}

int picoui_native_dirty_is_display_dirty(const struct picoui_display *display)
{
    return display != 0 && display == g_picoui_dirty_display;
}
