#include "backend.h"
#include "internal.h"
#include "ldQRCode.h"

#include <stdlib.h>

static struct picoui_backend_app_state *picoui_backend_qrcode_get_app_state(void *parent)
{
    struct picoui_backend_widget *parent_widget = parent;

    if (parent_widget == NULL || parent_widget->owner == NULL || parent_widget->owner->backend_app == NULL) {
        return NULL;
    }
    return (struct picoui_backend_app_state *)parent_widget->owner->backend_app;
}

static ldQRCode_t *picoui_backend_qrcode_get_ld(struct picoui_qrcode *qrcode)
{
    struct picoui_backend_widget *backend;

    if (qrcode == NULL || qrcode->widget.backend_widget == NULL) {
        return NULL;
    }

    backend = (struct picoui_backend_widget *)qrcode->widget.backend_widget;
    if (backend->kind != PICOUI_BACKEND_WIDGET_QRCODE || backend->ld_widget == NULL) {
        return NULL;
    }

    return (ldQRCode_t *)backend->ld_widget;
}

void *picoui_backend_create_qrcode(void *parent, const char *id)
{
    struct picoui_backend_widget *widget;
    struct picoui_backend_widget *parent_widget = parent;
    struct picoui_backend_app_state *app_state;
    ldQRCode_t *ld_qrcode;
    uint16_t name_id;
    static unsigned char empty_text[] = "";

    if (parent == 0 || id == 0) {
        return 0;
    }

    app_state = picoui_backend_qrcode_get_app_state(parent);
    if (app_state == NULL || app_state->ld_scene == NULL || parent_widget->ld_widget == NULL) {
        return 0;
    }

    widget = calloc(1, sizeof(*widget));
    if (widget == 0) {
        return 0;
    }

    name_id = ++app_state->next_ld_name_id;
    ld_qrcode = ldQRCode_init(app_state->ld_scene,
                              NULL,
                              name_id,
                              parent_widget->ld_name_id,
                              0,
                              0,
                              128,
                              128,
                              empty_text,
                              GLCD_COLOR_BLACK,
                              GLCD_COLOR_WHITE,
                              QR_ECC_7,
                              2,
                              4);
    if (ld_qrcode == NULL) {
        free(widget);
        return 0;
    }

    widget->parent = parent;
    widget->id = id;
    widget->kind = PICOUI_BACKEND_WIDGET_QRCODE;
    widget->theme = ((struct picoui_backend_widget *)parent)->theme;
    widget->ld_widget = ld_qrcode;
    widget->ld_name_id = name_id;
    widget->text = (const char *)empty_text;
    if (picoui_backend_widget_attach_child(parent, widget) != 0) {
        free(widget);
        return 0;
    }
    return widget;
}

int picoui_backend_qrcode_set_text(struct picoui_qrcode *qrcode, const char *text)
{
    ldQRCode_t *ld_qrcode = picoui_backend_qrcode_get_ld(qrcode);
    struct picoui_backend_widget *backend;

    if (ld_qrcode == NULL || text == NULL) {
        return -1;
    }

    ldQRCodeSetText(ld_qrcode, (uint8_t *)text);
    backend = (struct picoui_backend_widget *)qrcode->widget.backend_widget;
    backend->text = text;
    return 0;
}

const char *picoui_backend_qrcode_get_text(struct picoui_qrcode *qrcode)
{
    ldQRCode_t *ld_qrcode = picoui_backend_qrcode_get_ld(qrcode);

    if (ld_qrcode == NULL) {
        return NULL;
    }

    return (const char *)ld_qrcode->pStr;
}
