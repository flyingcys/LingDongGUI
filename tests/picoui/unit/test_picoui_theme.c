#include "picoui/picoui.h"

#include <assert.h>
#include <stddef.h>

int main(void)
{
    struct picoui_theme *theme = picoui_theme_create();
    assert(theme != NULL);

    assert(picoui_theme_set_color(theme, PICOUI_COLOR_ACCENT, 0x112233) == 0);
    assert(picoui_theme_set_metric(theme, PICOUI_METRIC_RADIUS, 6) == 0);

    struct picoui_app *app = picoui_app_create();
    assert(app != NULL);
    assert(picoui_app_set_theme(app, theme) == 0);

    struct picoui_window *win = picoui_window_create(app, "root");
    struct picoui_label *label = picoui_label_create(win, "title");
    struct picoui_button *button = picoui_button_create(win, "ok");
    assert(label != NULL);
    assert(button != NULL);

    assert(picoui_label_set_text(label, "hello") == 0);
    assert(picoui_button_set_text(button, "OK") == 0);

    picoui_app_destroy(app);
    picoui_theme_destroy(theme);
    return 0;
}
