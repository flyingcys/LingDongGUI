#include "internal.h"
#include "picoui/background.h"

#include <stdlib.h>

struct picoui_background *picoui_background_create(struct picoui_app *app, const char *id)
{
    struct picoui_background *background;
    void *backend_widget;

    if (app == 0 || id == 0) {
        return 0;
    }

    backend_widget = picoui_backend_create_background(app, id);
    if (backend_widget == 0) {
        return 0;
    }

    background = calloc(1, sizeof(*background));
    if (background == 0) {
        free(backend_widget);
        return 0;
    }

    background->window.id = id;
    background->window.widget.backend_widget = backend_widget;
    background->window.widget.visible = 1;
    background->window.widget.enabled = 1;
    background->window.flex_flow = PICOUI_FLEX_FLOW_ROW;
    background->window.flex_main_align = PICOUI_ALIGN_START;
    background->window.flex_cross_align = PICOUI_ALIGN_START;
    background->window.flex_track_align = PICOUI_ALIGN_START;
    background->window.grid_col_align = PICOUI_ALIGN_START;
    background->window.grid_row_align = PICOUI_ALIGN_START;
    if (picoui_backend_widget_bind_host(background->window.widget.backend_widget,
                                        &background->window.widget) != 0) {
        free(background);
        return 0;
    }

    return background;
}

int picoui_background_set_source(struct picoui_background *background,
                                 struct picoui_image_source *source)
{
    return picoui_window_set_background_source((struct picoui_window *)background, source);
}

int picoui_background_set_color(struct picoui_background *background, unsigned int rgb)
{
    return picoui_window_set_color((struct picoui_window *)background, rgb);
}

int picoui_background_get_color(struct picoui_background *background, unsigned int *rgb)
{
    return picoui_window_get_color((struct picoui_window *)background, rgb);
}

int picoui_background_set_offset(struct picoui_background *background, int offset_x, int offset_y)
{
    return picoui_window_set_background_offset((struct picoui_window *)background, offset_x, offset_y);
}

int picoui_background_get_offset(struct picoui_background *background,
                                 int *offset_x,
                                 int *offset_y)
{
    return picoui_window_get_background_offset((struct picoui_window *)background,
                                               offset_x,
                                               offset_y);
}
