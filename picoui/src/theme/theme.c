#include "internal.h"
#include "picoui/theme.h"

#include <stdlib.h>

struct picoui_theme *picoui_theme_create(void)
{
    return calloc(1, sizeof(struct picoui_theme));
}

void picoui_theme_destroy(struct picoui_theme *theme)
{
    free(theme);
}

int picoui_theme_set_color(struct picoui_theme *theme, enum picoui_color_id id, unsigned int rgb)
{
    if (theme == 0 || id < 0 || id >= PICOUI_COLOR_COUNT) {
        return -1;
    }

    theme->colors[id] = rgb;
    return 0;
}

int picoui_theme_set_metric(struct picoui_theme *theme, enum picoui_metric_id id, int value)
{
    if (theme == 0 || id < 0 || id >= PICOUI_METRIC_COUNT || value < 0) {
        return -1;
    }

    theme->metrics[id] = value;
    return 0;
}

int picoui_app_set_theme(struct picoui_app *app, struct picoui_theme *theme)
{
    if (app == 0 || theme == 0) {
        return -1;
    }

    app->theme = theme;
    return picoui_backend_apply_theme(app, theme);
}
