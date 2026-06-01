#include "backend.h"
#include "internal.h"
#include "ldGauge.h"

#include <stdlib.h>

extern const arm_2d_tile_t c_tileQuaterArcGRAY8;
extern const arm_2d_tile_t c_tileQuaterArcMask;
extern const arm_2d_tile_t c_tilePointerSecGRAY8;
extern const arm_2d_tile_t c_tilePointerSecMask;

static struct picoui_backend_app_state *picoui_backend_gauge_get_app_state(void *parent)
{
    struct picoui_backend_widget *parent_widget = parent;

    if (parent_widget == NULL || parent_widget->owner == NULL || parent_widget->owner->backend_app == NULL) {
        return NULL;
    }
    return (struct picoui_backend_app_state *)parent_widget->owner->backend_app;
}

static ldGauge_t *picoui_backend_gauge_get_ld(struct picoui_gauge *gauge)
{
    struct picoui_backend_widget *backend;

    if (gauge == NULL || gauge->widget.backend_widget == NULL) {
        return NULL;
    }
    backend = (struct picoui_backend_widget *)gauge->widget.backend_widget;
    if (backend->kind != PICOUI_BACKEND_WIDGET_GAUGE || backend->ld_widget == NULL) {
        return NULL;
    }
    return (ldGauge_t *)backend->ld_widget;
}

void *picoui_backend_create_gauge(void *parent, const char *id)
{
    struct picoui_backend_widget *widget;
    struct picoui_backend_widget *parent_widget = parent;
    struct picoui_backend_app_state *app_state;
    ldGauge_t *ld_gauge;
    arm_2d_tile_t *bg_img_tile;
    arm_2d_tile_t *bg_mask_tile;
    arm_2d_tile_t *pointer_img_tile;
    arm_2d_tile_t *pointer_mask_tile;
    uint16_t name_id;

    if (parent == 0 || id == 0) {
        return 0;
    }

    app_state = picoui_backend_gauge_get_app_state(parent);
    if (app_state == NULL || app_state->ld_scene == NULL || parent_widget->ld_widget == NULL) {
        return 0;
    }

    widget = calloc(1, sizeof(*widget));
    if (widget == 0) {
        return 0;
    }

    bg_img_tile = malloc(sizeof(*bg_img_tile));
    if (bg_img_tile == NULL) {
        free(widget);
        return 0;
    }
    *bg_img_tile = c_tileQuaterArcGRAY8;

    bg_mask_tile = malloc(sizeof(*bg_mask_tile));
    if (bg_mask_tile == NULL) {
        free(bg_img_tile);
        free(widget);
        return 0;
    }
    *bg_mask_tile = c_tileQuaterArcMask;

    pointer_img_tile = malloc(sizeof(*pointer_img_tile));
    if (pointer_img_tile == NULL) {
        free(bg_mask_tile);
        free(bg_img_tile);
        free(widget);
        return 0;
    }
    *pointer_img_tile = c_tilePointerSecGRAY8;

    pointer_mask_tile = malloc(sizeof(*pointer_mask_tile));
    if (pointer_mask_tile == NULL) {
        free(pointer_img_tile);
        free(bg_mask_tile);
        free(bg_img_tile);
        free(widget);
        return 0;
    }
    *pointer_mask_tile = c_tilePointerSecMask;

    name_id = ++app_state->next_ld_name_id;
    ld_gauge = ldGauge_init(app_state->ld_scene,
                            NULL,
                            name_id,
                            parent_widget->ld_name_id,
                            0,
                            0,
                            160,
                            160,
                            bg_img_tile,
                            bg_mask_tile,
                            0,
                            0);
    if (ld_gauge == NULL) {
        free(pointer_mask_tile);
        free(pointer_img_tile);
        free(bg_img_tile);
        free(bg_mask_tile);
        free(widget);
        return 0;
    }

    ldGaugeSetPointerImage(ld_gauge,
                           pointer_img_tile,
                           pointer_mask_tile,
                           (int16_t)(pointer_mask_tile->tRegion.tSize.iWidth >> 1),
                           (int16_t)(pointer_mask_tile->tRegion.tSize.iHeight));

    widget->parent = parent;
    widget->id = id;
    widget->kind = PICOUI_BACKEND_WIDGET_GAUGE;
    widget->theme = ((struct picoui_backend_widget *)parent)->theme;
    widget->ld_widget = ld_gauge;
    widget->ld_name_id = name_id;
    if (picoui_backend_widget_attach_child(parent, widget) != 0) {
        free(widget);
        return 0;
    }
    return widget;
}

int picoui_backend_gauge_set_angle(struct picoui_gauge *gauge, float angle)
{
    ldGauge_t *ld_gauge = picoui_backend_gauge_get_ld(gauge);
    struct picoui_backend_widget *backend;

    if (ld_gauge == NULL) {
        return -1;
    }
    ldGaugeSetAngle(ld_gauge, angle);
    backend = (struct picoui_backend_widget *)gauge->widget.backend_widget;
    backend->value = (int)angle;
    return 0;
}

int picoui_backend_gauge_get_angle(struct picoui_gauge *gauge, float *angle)
{
    ldGauge_t *ld_gauge = picoui_backend_gauge_get_ld(gauge);

    if (ld_gauge == NULL || angle == NULL) {
        return -1;
    }
    *angle = (float)ld_gauge->_nowAngle_x10 / 10.0f;
    return 0;
}

int picoui_backend_gauge_set_pointer_color(struct picoui_gauge *gauge, unsigned int pointer_color)
{
    ldGauge_t *ld_gauge = picoui_backend_gauge_get_ld(gauge);

    if (ld_gauge == NULL) {
        return -1;
    }
    ldGaugeSetPointerColor(ld_gauge, (ldColor)pointer_color);
    return 0;
}

int picoui_backend_gauge_get_pointer_color(struct picoui_gauge *gauge, unsigned int *pointer_color)
{
    ldGauge_t *ld_gauge = picoui_backend_gauge_get_ld(gauge);

    if (ld_gauge == NULL || pointer_color == NULL) {
        return -1;
    }
    *pointer_color = (unsigned int)ld_gauge->maskColor;
    return 0;
}

int picoui_backend_gauge_set_auto_move(struct picoui_gauge *gauge, int auto_move)
{
    ldGauge_t *ld_gauge = picoui_backend_gauge_get_ld(gauge);

    if (ld_gauge == NULL) {
        return -1;
    }
    ldGaugeSetAutoMove(ld_gauge, auto_move != 0);
    return 0;
}

int picoui_backend_gauge_get_auto_move(struct picoui_gauge *gauge, int *auto_move)
{
    ldGauge_t *ld_gauge = picoui_backend_gauge_get_ld(gauge);

    if (ld_gauge == NULL || auto_move == NULL) {
        return -1;
    }
    *auto_move = ld_gauge->isAutoMove ? 1 : 0;
    return 0;
}
