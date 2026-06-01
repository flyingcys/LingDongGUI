#include "backend.h"
#include "internal.h"
#include "ldProgressBar.h"

#include <stdlib.h>

static ldColor picoui_backend_rgb_to_ld_color(unsigned int rgb)
{
    return __RGB((rgb >> 16) & 0xFFU, (rgb >> 8) & 0xFFU, rgb & 0xFFU);
}

static struct picoui_backend_app_state *picoui_backend_progress_bar_get_app_state(void *parent)
{
    struct picoui_backend_widget *parent_widget = parent;

    if (parent_widget == NULL || parent_widget->owner == NULL || parent_widget->owner->backend_app == NULL) {
        return NULL;
    }
    return (struct picoui_backend_app_state *)parent_widget->owner->backend_app;
}

static ldProgressBar_t *picoui_backend_progress_bar_get_ld(struct picoui_progress_bar *bar)
{
    struct picoui_backend_widget *backend;

    if (bar == NULL || bar->widget.backend_widget == NULL) {
        return NULL;
    }

    backend = (struct picoui_backend_widget *)bar->widget.backend_widget;
    if (backend->kind != PICOUI_BACKEND_WIDGET_PROGRESS_BAR || backend->ld_widget == NULL) {
        return NULL;
    }

    return (ldProgressBar_t *)backend->ld_widget;
}

void *picoui_backend_create_progress_bar(void *parent, const char *id)
{
    struct picoui_backend_widget *widget;
    struct picoui_backend_widget *parent_widget = parent;
    struct picoui_backend_app_state *app_state;
    ldProgressBar_t *ld_progress_bar;
    uint16_t name_id;

    if (parent == 0 || id == 0) {
        return 0;
    }

    app_state = picoui_backend_progress_bar_get_app_state(parent);
    if (app_state == NULL || app_state->ld_scene == NULL || parent_widget->ld_widget == NULL) {
        return 0;
    }

    widget = calloc(1, sizeof(*widget));
    if (widget == 0) {
        return 0;
    }

    name_id = ++app_state->next_ld_name_id;
    ld_progress_bar = ldProgressBar_init(app_state->ld_scene,
                                         NULL,
                                         name_id,
                                         parent_widget->ld_name_id,
                                         0,
                                         0,
                                         220,
                                         24);
    if (ld_progress_bar == NULL) {
        free(widget);
        return 0;
    }

    ldProgressBarSetColor(ld_progress_bar, __RGB(221, 226, 234), __RGB(32, 87, 196));
    ldProgressBarSetFrameColor(ld_progress_bar, __RGB(88, 98, 119), 1);

    widget->parent = parent;
    widget->id = id;
    widget->kind = PICOUI_BACKEND_WIDGET_PROGRESS_BAR;
    widget->theme = ((struct picoui_backend_widget *)parent)->theme;
    widget->ld_widget = ld_progress_bar;
    widget->ld_name_id = name_id;
    widget->value = 0;
    widget->last_signal = PICOUI_BACKEND_SIGNAL_NONE;
    if (picoui_backend_widget_attach_child(parent, widget) != 0) {
        free(widget);
        return 0;
    }
    return widget;
}

int picoui_backend_progress_bar_set_percent(struct picoui_progress_bar *bar, int percent)
{
    ldProgressBar_t *ld_progress_bar = picoui_backend_progress_bar_get_ld(bar);
    struct picoui_backend_widget *backend;

    if (ld_progress_bar == NULL || percent < 0 || percent > 100) {
        return -1;
    }

    ldProgressBarSetPercent(ld_progress_bar, (float)percent);
    backend = (struct picoui_backend_widget *)bar->widget.backend_widget;
    backend->value = percent;
    return 0;
}

int picoui_backend_progress_bar_get_percent(struct picoui_progress_bar *bar, int *percent)
{
    ldProgressBar_t *ld_progress_bar = picoui_backend_progress_bar_get_ld(bar);

    if (ld_progress_bar == NULL || percent == NULL) {
        return -1;
    }

    *percent = (int)(ld_progress_bar->permille / 10U);
    return 0;
}

int picoui_backend_progress_bar_set_horizontal(struct picoui_progress_bar *bar, int horizontal)
{
    ldProgressBar_t *ld_progress_bar = picoui_backend_progress_bar_get_ld(bar);

    if (ld_progress_bar == NULL) {
        return -1;
    }

    ldProgressBarSetHorizontal(ld_progress_bar, horizontal != 0);
    return 0;
}

int picoui_backend_progress_bar_get_horizontal(struct picoui_progress_bar *bar, int *horizontal)
{
    ldProgressBar_t *ld_progress_bar = picoui_backend_progress_bar_get_ld(bar);

    if (ld_progress_bar == NULL || horizontal == NULL) {
        return -1;
    }

    *horizontal = ld_progress_bar->isHorizontal ? 1 : 0;
    return 0;
}

int picoui_backend_progress_bar_set_bg_source(void *backend_widget, struct picoui_image_source *source)
{
    ldProgressBar_t *ld_progress_bar;

    if (backend_widget == NULL || source == NULL || source->img_tile == NULL) {
        return -1;
    }

    ld_progress_bar = picoui_backend_progress_bar_get_ld((struct picoui_progress_bar *)
        ((struct picoui_backend_widget *)backend_widget)->host_widget);
    if (ld_progress_bar == NULL) {
        return -1;
    }

    ldProgressBarSetImage(ld_progress_bar,
                          source->img_tile,
                          source->mask_tile,
                          ld_progress_bar->ptFgImgTile,
                          ld_progress_bar->ptFgMaskTile);
    return 0;
}

int picoui_backend_progress_bar_set_fg_source(void *backend_widget, struct picoui_image_source *source)
{
    ldProgressBar_t *ld_progress_bar;

    if (backend_widget == NULL || source == NULL || source->img_tile == NULL) {
        return -1;
    }

    ld_progress_bar = picoui_backend_progress_bar_get_ld((struct picoui_progress_bar *)
        ((struct picoui_backend_widget *)backend_widget)->host_widget);
    if (ld_progress_bar == NULL) {
        return -1;
    }

    ldProgressBarSetImage(ld_progress_bar,
                          ld_progress_bar->ptBgImgTile,
                          ld_progress_bar->ptBgMaskTile,
                          source->img_tile,
                          source->mask_tile);
    return 0;
}

int picoui_backend_progress_bar_set_frame_source(void *backend_widget, struct picoui_image_source *source)
{
    ldProgressBar_t *ld_progress_bar;

    if (backend_widget == NULL || source == NULL || source->img_tile == NULL) {
        return -1;
    }

    ld_progress_bar = picoui_backend_progress_bar_get_ld((struct picoui_progress_bar *)
        ((struct picoui_backend_widget *)backend_widget)->host_widget);
    if (ld_progress_bar == NULL) {
        return -1;
    }

    ldProgressBarSetFrameImage(ld_progress_bar, source->img_tile, source->mask_tile);
    return 0;
}

int picoui_backend_progress_bar_set_color(void *backend_widget, unsigned int bg_color, unsigned int fg_color)
{
    ldProgressBar_t *ld_progress_bar;

    if (backend_widget == NULL || bg_color > 0xFFFFFFU || fg_color > 0xFFFFFFU) {
        return -1;
    }

    ld_progress_bar = picoui_backend_progress_bar_get_ld((struct picoui_progress_bar *)
        ((struct picoui_backend_widget *)backend_widget)->host_widget);
    if (ld_progress_bar == NULL) {
        return -1;
    }

    ldProgressBarSetColor(ld_progress_bar,
                          picoui_backend_rgb_to_ld_color(bg_color),
                          picoui_backend_rgb_to_ld_color(fg_color));
    return 0;
}

int picoui_backend_progress_bar_set_frame_color(void *backend_widget,
                                                unsigned int frame_color,
                                                int frame_color_size)
{
    ldProgressBar_t *ld_progress_bar;

    if (backend_widget == NULL || frame_color > 0xFFFFFFU || frame_color_size < 0 || frame_color_size > 255) {
        return -1;
    }

    ld_progress_bar = picoui_backend_progress_bar_get_ld((struct picoui_progress_bar *)
        ((struct picoui_backend_widget *)backend_widget)->host_widget);
    if (ld_progress_bar == NULL) {
        return -1;
    }

    ldProgressBarSetFrameColor(ld_progress_bar,
                               picoui_backend_rgb_to_ld_color(frame_color),
                               (uint8_t)frame_color_size);
    return 0;
}

int picoui_backend_progress_bar_set_inverted(void *backend_widget, int inverted)
{
    ldProgressBar_t *ld_progress_bar;

    if (backend_widget == NULL) {
        return -1;
    }

    ld_progress_bar = picoui_backend_progress_bar_get_ld((struct picoui_progress_bar *)
        ((struct picoui_backend_widget *)backend_widget)->host_widget);
    if (ld_progress_bar == NULL) {
        return -1;
    }

    ldProgressBarSetInverted(ld_progress_bar, inverted != 0);
    return 0;
}

int picoui_backend_progress_bar_get_inverted(void *backend_widget)
{
    ldProgressBar_t *ld_progress_bar;

    if (backend_widget == NULL) {
        return -1;
    }

    ld_progress_bar = picoui_backend_progress_bar_get_ld((struct picoui_progress_bar *)
        ((struct picoui_backend_widget *)backend_widget)->host_widget);
    if (ld_progress_bar == NULL) {
        return -1;
    }

    return ld_progress_bar->isInverted ? 1 : 0;
}
