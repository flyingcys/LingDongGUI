#include "internal.h"
#include "ldWindow.h"

#include <stdlib.h>

#define PICOUI_RUNTIME_ROOT_WIDTH 480
#define PICOUI_RUNTIME_ROOT_HEIGHT 320

struct picoui_backend_window_host {
    struct picoui_backend_widget widget;
    ldPadding_t padding_group;
    int has_padding_group;
};

static ldColor picoui_backend_rgb_to_ld_color(unsigned int rgb)
{
    return __RGB((rgb >> 16) & 0xFFU, (rgb >> 8) & 0xFFU, rgb & 0xFFU);
}

static unsigned int picoui_backend_ld_color_to_rgb(ldColor color)
{
    uint32_t red = ((uint32_t)color >> 11) & 0x1FU;
    uint32_t green = ((uint32_t)color >> 5) & 0x3FU;
    uint32_t blue = (uint32_t)color & 0x1FU;

    red = (red * 255U) / 31U;
    green = (green * 255U) / 63U;
    blue = (blue * 255U) / 31U;
    return (red << 16) | (green << 8) | blue;
}

static struct picoui_backend_app_state *picoui_backend_window_get_app_state(struct picoui_app *app)
{
    if (app == NULL || app->backend_app == NULL) {
        return NULL;
    }
    return (struct picoui_backend_app_state *)app->backend_app;
}

void *picoui_backend_create_window(struct picoui_app *app, const char *id)
{
    struct picoui_backend_window_host *host;
    struct picoui_backend_app_state *app_state;
    ldWindow_t *ld_root;

    if (app == 0 || id == 0) {
        return 0;
    }

    app_state = picoui_backend_window_get_app_state(app);
    if (app_state == NULL || app_state->ld_scene == NULL) {
        return 0;
    }

    host = calloc(1, sizeof(*host));
    if (host == 0) {
        return 0;
    }

    ld_root = ldWindow_init(app_state->ld_scene,
                            NULL,
                            0,
                            0,
                            0,
                            0,
                            PICOUI_RUNTIME_ROOT_WIDTH,
                            PICOUI_RUNTIME_ROOT_HEIGHT);
    if (ld_root == NULL) {
        free(host);
        return 0;
    }

    host->widget.id = id;
    host->widget.owner = app;
    host->widget.kind = PICOUI_BACKEND_WIDGET_WINDOW;
    host->widget.root = &host->widget;
    host->widget.theme = app->theme;
    host->widget.ld_widget = ld_root;
    host->widget.ld_name_id = 0;
    return &host->widget;
}

static struct picoui_backend_widget *picoui_backend_window_get(struct picoui_window *window)
{
    if (window == NULL) {
        return NULL;
    }
    return (struct picoui_backend_widget *)window->widget.backend_widget;
}

static struct picoui_backend_window_host *picoui_backend_window_get_host(struct picoui_window *window)
{
    return (struct picoui_backend_window_host *)picoui_backend_window_get(window);
}

static ldWindow_t *picoui_backend_window_get_ld(struct picoui_window *window)
{
    struct picoui_backend_widget *backend = picoui_backend_window_get(window);

    if (backend == NULL || backend->ld_widget == NULL) {
        return NULL;
    }
    return (ldWindow_t *)backend->ld_widget;
}

static int picoui_backend_window_set_padding_group_ptr(struct picoui_window *window,
                                                       int left,
                                                       int top,
                                                       int right,
                                                       int bottom)
{
    struct picoui_backend_window_host *host = picoui_backend_window_get_host(window);
    ldWindow_t *ld_window;

    if (host == NULL) {
        return -1;
    }

    ld_window = picoui_backend_window_get_ld(window);
    if (ld_window == NULL) {
        return -1;
    }

    if (!host->has_padding_group || ld_window->pLayoutPaddingGroup != &host->padding_group) {
        ldWindowSetPaddingGroup(ld_window, &host->padding_group);
        host->has_padding_group = 1;
    }

    host->padding_group.left = (int16_t)left;
    host->padding_group.top = (int16_t)top;
    host->padding_group.right = (int16_t)right;
    host->padding_group.bottom = (int16_t)bottom;
    return 0;
}

int picoui_backend_window_set_background_source(struct picoui_window *window,
                                                struct picoui_image_source *source)
{
    ldWindow_t *ld_window;

    if (picoui_backend_window_get(window) == NULL) {
        return -1;
    }
    if (source != NULL && source->img_tile == NULL) {
        return -1;
    }

    ld_window = picoui_backend_window_get_ld(window);
    if (ld_window == NULL) {
        return -1;
    }

    ldWindowSetImage(ld_window,
                     source != NULL ? source->img_tile : NULL,
                     source != NULL ? source->mask_tile : NULL);
    return 0;
}

int picoui_backend_window_set_bg_color(struct picoui_window *window, unsigned int rgb)
{
    ldWindow_t *ld_window = picoui_backend_window_get_ld(window);

    if (ld_window == NULL) {
        return -1;
    }

    ldWindowSetColor(ld_window, picoui_backend_rgb_to_ld_color(rgb));
    return 0;
}

int picoui_backend_window_get_bg_color(struct picoui_window *window, unsigned int *rgb)
{
    ldWindow_t *ld_window = picoui_backend_window_get_ld(window);

    if (ld_window == NULL || rgb == NULL) {
        return -1;
    }

    *rgb = picoui_backend_ld_color_to_rgb(ldWindowGetColor(ld_window));
    return 0;
}

int picoui_backend_window_set_padding_group(struct picoui_window *window,
                                            int left,
                                            int top,
                                            int right,
                                            int bottom)
{
    if (left < 0 || top < 0 || right < 0 || bottom < 0) {
        return -1;
    }
    return picoui_backend_window_set_padding_group_ptr(window, left, top, right, bottom);
}

int picoui_backend_window_get_padding_left(struct picoui_window *window)
{
    ldWindow_t *ld_window = picoui_backend_window_get_ld(window);

    if (ld_window == NULL || ld_window->pLayoutPaddingGroup == NULL) {
        return -1;
    }
    return ld_window->pLayoutPaddingGroup->left;
}

int picoui_backend_window_get_padding_top(struct picoui_window *window)
{
    ldWindow_t *ld_window = picoui_backend_window_get_ld(window);

    if (ld_window == NULL || ld_window->pLayoutPaddingGroup == NULL) {
        return -1;
    }
    return ld_window->pLayoutPaddingGroup->top;
}

int picoui_backend_window_get_padding_right(struct picoui_window *window)
{
    ldWindow_t *ld_window = picoui_backend_window_get_ld(window);

    if (ld_window == NULL || ld_window->pLayoutPaddingGroup == NULL) {
        return -1;
    }
    return ld_window->pLayoutPaddingGroup->right;
}

int picoui_backend_window_get_padding_bottom(struct picoui_window *window)
{
    ldWindow_t *ld_window = picoui_backend_window_get_ld(window);

    if (ld_window == NULL || ld_window->pLayoutPaddingGroup == NULL) {
        return -1;
    }
    return ld_window->pLayoutPaddingGroup->bottom;
}
