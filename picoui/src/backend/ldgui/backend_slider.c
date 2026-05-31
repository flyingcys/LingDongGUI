#include "backend.h"
#include "internal.h"
#include "ldSlider.h"

#include <stdlib.h>

static ldSlider_t *picoui_backend_slider_get_ld(struct picoui_slider *slider)
{
    struct picoui_backend_widget *backend;

    if (slider == NULL || slider->widget.backend_widget == NULL) {
        return NULL;
    }

    backend = (struct picoui_backend_widget *)slider->widget.backend_widget;
    if (backend->kind != PICOUI_BACKEND_WIDGET_SLIDER || backend->ld_widget == NULL) {
        return NULL;
    }

    return (ldSlider_t *)backend->ld_widget;
}

static struct picoui_backend_app_state *picoui_backend_slider_get_app_state(void *parent)
{
    struct picoui_backend_widget *parent_widget = parent;

    if (parent_widget == NULL || parent_widget->owner == NULL || parent_widget->owner->backend_app == NULL) {
        return NULL;
    }
    return (struct picoui_backend_app_state *)parent_widget->owner->backend_app;
}

void *picoui_backend_create_slider(void *parent, const char *id)
{
    struct picoui_backend_widget *widget;
    struct picoui_backend_widget *parent_widget = parent;
    struct picoui_backend_app_state *app_state;
    ldSlider_t *ld_slider;
    uint16_t name_id;

    if (parent == 0 || id == 0) {
        return 0;
    }

    app_state = picoui_backend_slider_get_app_state(parent);
    if (app_state == NULL || app_state->ld_scene == NULL || parent_widget->ld_widget == NULL) {
        return 0;
    }

    widget = calloc(1, sizeof(*widget));
    if (widget == 0) {
        return 0;
    }

    name_id = ++app_state->next_ld_name_id;
    ld_slider = ldSlider_init(app_state->ld_scene,
                              NULL,
                              name_id,
                              parent_widget->ld_name_id,
                              0,
                              0,
                              220,
                              30);
    if (ld_slider == NULL) {
        free(widget);
        return 0;
    }

    widget->parent = parent;
    widget->id = id;
    widget->kind = PICOUI_BACKEND_WIDGET_SLIDER;
    widget->theme = ((struct picoui_backend_widget *)parent)->theme;
    widget->ld_widget = ld_slider;
    widget->ld_name_id = name_id;
    widget->last_signal = PICOUI_BACKEND_SIGNAL_NONE;
    if (picoui_backend_widget_attach_child(parent, widget) != 0) {
        free(widget);
        return 0;
    }
    return widget;
}

int picoui_backend_slider_set_horizontal(struct picoui_slider *slider, int horizontal)
{
    ldSlider_t *ld_slider = picoui_backend_slider_get_ld(slider);

    if (ld_slider == NULL) {
        return -1;
    }

    ldSliderSetHorizontal(ld_slider, horizontal != 0);
    return 0;
}

int picoui_backend_slider_get_horizontal(struct picoui_slider *slider, int *horizontal)
{
    ldSlider_t *ld_slider = picoui_backend_slider_get_ld(slider);

    if (ld_slider == NULL || horizontal == NULL) {
        return -1;
    }

    *horizontal = ld_slider->isHorizontal ? 1 : 0;
    return 0;
}

int picoui_backend_slider_set_background_source(struct picoui_slider *slider,
                                                struct picoui_image_source *background_source)
{
    ldSlider_t *ld_slider = picoui_backend_slider_get_ld(slider);

    if (ld_slider == NULL) {
        return -1;
    }

    ldSliderSetImage(ld_slider,
                     background_source != NULL ? background_source->img_tile : NULL,
                     background_source != NULL ? background_source->mask_tile : NULL,
                     ld_slider->ptIndicImgTile,
                     ld_slider->ptIndicMaskTile);
    return 0;
}

int picoui_backend_slider_set_indicator_source(struct picoui_slider *slider,
                                               struct picoui_image_source *indicator_source)
{
    ldSlider_t *ld_slider = picoui_backend_slider_get_ld(slider);

    if (ld_slider == NULL) {
        return -1;
    }

    ldSliderSetImage(ld_slider,
                     ld_slider->ptBgImgTile,
                     ld_slider->ptBgMaskTile,
                     indicator_source != NULL ? indicator_source->img_tile : NULL,
                     indicator_source != NULL ? indicator_source->mask_tile : NULL);
    return 0;
}

int picoui_backend_slider_set_indicator_width(struct picoui_slider *slider, int indicator_width)
{
    ldSlider_t *ld_slider = picoui_backend_slider_get_ld(slider);

    if (ld_slider == NULL || indicator_width < 0 || indicator_width > 255) {
        return -1;
    }

    ldSliderSetIndicatorWidth(ld_slider, (uint8_t)indicator_width);
    return 0;
}

int picoui_backend_slider_set_slim_size(struct picoui_slider *slider, int slim_size)
{
    ldSlider_t *ld_slider = picoui_backend_slider_get_ld(slider);

    if (ld_slider == NULL || slim_size < 0 || slim_size > 255) {
        return -1;
    }

    ldSliderSetSlimSize(ld_slider, (uint8_t)slim_size);
    return 0;
}

int picoui_backend_slider_get_percent(struct picoui_slider *slider, int *percent)
{
    ldSlider_t *ld_slider = picoui_backend_slider_get_ld(slider);

    if (ld_slider == NULL || percent == NULL) {
        return -1;
    }

    *percent = (int)(ld_slider->permille / 10U);
    return 0;
}
