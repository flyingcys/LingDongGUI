#include "backend.h"
#include "internal.h"
#include "ldBase.h"
#include "ldImage.h"

#include <stdlib.h>

static ldColor picoui_backend_image_rgb_to_ld_color(unsigned int rgb)
{
    return __RGB((rgb >> 16) & 0xFFU, (rgb >> 8) & 0xFFU, rgb & 0xFFU);
}

static struct picoui_backend_app_state *picoui_backend_image_get_app_state(void *parent)
{
    struct picoui_backend_widget *parent_widget = parent;

    if (parent_widget == NULL || parent_widget->owner == NULL || parent_widget->owner->backend_app == NULL) {
        return NULL;
    }
    return (struct picoui_backend_app_state *)parent_widget->owner->backend_app;
}

void *picoui_backend_create_image(void *parent, const char *id)
{
    struct picoui_backend_widget *widget;
    struct picoui_backend_widget *parent_widget = parent;
    struct picoui_backend_app_state *app_state;
    ldImage_t *ld_image;
    uint16_t name_id;

    if (parent == 0 || id == 0) {
        return 0;
    }

    app_state = picoui_backend_image_get_app_state(parent);
    if (app_state == NULL || app_state->ld_scene == NULL || parent_widget->ld_widget == NULL) {
        return 0;
    }

    widget = calloc(1, sizeof(*widget));
    if (widget == 0) {
        return 0;
    }

    name_id = ++app_state->next_ld_name_id;
    ld_image = ldImage_init(app_state->ld_scene, NULL, name_id, parent_widget->ld_name_id, 0, 0, 220, 56, NULL, NULL);
    if (ld_image == NULL) {
        free(widget);
        return 0;
    }

    widget->parent = parent;
    widget->id = id;
    widget->kind = PICOUI_BACKEND_WIDGET_IMAGE;
    widget->theme = ((struct picoui_backend_widget *)parent)->theme;
    widget->ld_widget = ld_image;
    widget->ld_name_id = name_id;
    if (picoui_backend_widget_attach_child(parent, widget) != 0) {
        free(widget);
        return 0;
    }
    return widget;
}

int picoui_backend_set_image_source(void *backend_widget, struct picoui_image_source *source)
{
    struct picoui_backend_widget *widget = backend_widget;
    ldImage_t *ld_image;

    if (widget == 0 || (source != 0 && source->img_tile == NULL)) {
        return -1;
    }
    if (widget->kind != PICOUI_BACKEND_WIDGET_IMAGE || widget->ld_widget == NULL) {
        return -1;
    }

    ld_image = widget->ld_widget;
    widget->image_source = source;
    ldImageSetImage(ld_image,
                    source != NULL ? source->img_tile : NULL,
                    source != NULL ? source->mask_tile : NULL);
    return 0;
}

int picoui_backend_image_set_mask_color(void *backend_widget, unsigned int rgb)
{
    struct picoui_backend_widget *widget = backend_widget;
    ldImage_t *ld_image;

    if (widget == 0 || widget->kind != PICOUI_BACKEND_WIDGET_IMAGE || widget->ld_widget == NULL) {
        return -1;
    }

    ld_image = widget->ld_widget;
    ldImageSetMaskColor(ld_image, picoui_backend_image_rgb_to_ld_color(rgb));
    return 0;
}
