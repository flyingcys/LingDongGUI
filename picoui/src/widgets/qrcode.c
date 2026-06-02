#include "internal.h"
#include "picoui/qrcode.h"
#include "picoui/widget.h"

#include <stdlib.h>

int picoui_backend_qrcode_set_text(struct picoui_qrcode *qrcode, const char *text);
const char *picoui_backend_qrcode_get_text(struct picoui_qrcode *qrcode);
int picoui_backend_qrcode_set_qr_color(void *backend_widget, unsigned int rgb);
int picoui_backend_qrcode_set_bg_color(void *backend_widget, unsigned int rgb);
int picoui_backend_qrcode_set_ecc(void *backend_widget, int ecc);
int picoui_backend_qrcode_set_max_version(void *backend_widget, int max_version);
int picoui_backend_qrcode_set_zoom(void *backend_widget, int zoom);

static int picoui_qrcode_props_are_valid(const struct picoui_qrcode_props *props)
{
    return props != 0 && props->id != 0 && props->text != 0;
}

struct picoui_qrcode *picoui_qrcode_create(struct picoui_widget *parent, const char *id)
{
    struct picoui_qrcode *qrcode;

    if (parent == 0 || id == 0 || parent->backend_widget == 0) {
        return 0;
    }

    qrcode = calloc(1, sizeof(*qrcode));
    if (qrcode == 0) {
        return 0;
    }

    qrcode->widget.backend_widget = picoui_backend_create_qrcode(parent->backend_widget, id);
    if (qrcode->widget.backend_widget == 0) {
        free(qrcode);
        return 0;
    }

    qrcode->id = id;
    qrcode->qr_color = 0x000000U;
    qrcode->bg_color = 0xFFFFFFU;
    qrcode->ecc = 0;
    qrcode->max_version = 2;
    qrcode->zoom = 4;
    qrcode->widget.visible = 1;
    qrcode->widget.enabled = 1;
    if (picoui_backend_widget_bind_host(qrcode->widget.backend_widget, &qrcode->widget) != 0) {
        free(qrcode);
        return 0;
    }
    return qrcode;
}

struct picoui_qrcode *picoui_q_r_code_init(struct picoui_widget *parent, const char *id)
{
    return picoui_qrcode_create(parent, id);
}

struct picoui_qrcode *picoui_qrcode_create_with_props(struct picoui_widget *parent,
                                                      const struct picoui_qrcode_props *props)
{
    struct picoui_qrcode *qrcode;

    if (!picoui_qrcode_props_are_valid(props)) {
        return 0;
    }

    qrcode = picoui_qrcode_create(parent, props->id);
    if (qrcode == 0) {
        return 0;
    }

    if (props->style_class != 0
        && picoui_widget_set_style_class(&qrcode->widget, props->style_class) != 0) {
        free(qrcode);
        return 0;
    }
    if (picoui_widget_set_user_data(&qrcode->widget, props->user_data) != 0
        || picoui_qrcode_set_text(qrcode, props->text) != 0) {
        free(qrcode);
        return 0;
    }

    return qrcode;
}

int picoui_qrcode_set_text(struct picoui_qrcode *qrcode, const char *text)
{
    if (qrcode == 0 || text == 0) {
        return -1;
    }

    if (picoui_backend_qrcode_set_text(qrcode, text) != 0) {
        return -1;
    }

    qrcode->text = text;
    return 0;
}

int picoui_q_r_code_set_text(struct picoui_qrcode *qrcode, const char *text)
{
    return picoui_qrcode_set_text(qrcode, text);
}

const char *picoui_qrcode_get_text(const struct picoui_qrcode *qrcode)
{
    if (qrcode == 0) {
        return 0;
    }

    return picoui_backend_qrcode_get_text((struct picoui_qrcode *)qrcode);
}

int picoui_qrcode_set_qr_color(struct picoui_qrcode *qrcode, unsigned int rgb)
{
    if (qrcode == 0 || rgb > 0xFFFFFFU) {
        return -1;
    }

    if (picoui_backend_qrcode_set_qr_color(qrcode->widget.backend_widget, rgb) != 0) {
        return -1;
    }

    qrcode->qr_color = rgb;
    return 0;
}

int picoui_qrcode_set_bg_color(struct picoui_qrcode *qrcode, unsigned int rgb)
{
    if (qrcode == 0 || rgb > 0xFFFFFFU) {
        return -1;
    }

    if (picoui_backend_qrcode_set_bg_color(qrcode->widget.backend_widget, rgb) != 0) {
        return -1;
    }

    qrcode->bg_color = rgb;
    return 0;
}

int picoui_qrcode_set_ecc(struct picoui_qrcode *qrcode, int ecc)
{
    if (qrcode == 0 || ecc < 0 || ecc > 3) {
        return -1;
    }

    if (picoui_backend_qrcode_set_ecc(qrcode->widget.backend_widget, ecc) != 0) {
        return -1;
    }

    qrcode->ecc = ecc;
    return 0;
}

int picoui_qrcode_set_max_version(struct picoui_qrcode *qrcode, int max_version)
{
    if (qrcode == 0 || max_version <= 0 || max_version > 40) {
        return -1;
    }

    if (picoui_backend_qrcode_set_max_version(qrcode->widget.backend_widget, max_version) != 0) {
        return -1;
    }

    qrcode->max_version = max_version;
    return 0;
}

int picoui_qrcode_set_zoom(struct picoui_qrcode *qrcode, int zoom)
{
    if (qrcode == 0 || zoom <= 0) {
        return -1;
    }

    if (picoui_backend_qrcode_set_zoom(qrcode->widget.backend_widget, zoom) != 0) {
        return -1;
    }

    qrcode->zoom = zoom;
    return 0;
}
