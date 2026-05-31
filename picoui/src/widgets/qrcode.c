#include "internal.h"
#include "picoui/qrcode.h"
#include "picoui/widget.h"

#include <stdlib.h>

int picoui_backend_qrcode_set_text(struct picoui_qrcode *qrcode, const char *text);
const char *picoui_backend_qrcode_get_text(struct picoui_qrcode *qrcode);

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
    qrcode->widget.visible = 1;
    qrcode->widget.enabled = 1;
    if (picoui_backend_widget_bind_host(qrcode->widget.backend_widget, &qrcode->widget) != 0) {
        free(qrcode);
        return 0;
    }
    return qrcode;
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

const char *picoui_qrcode_get_text(const struct picoui_qrcode *qrcode)
{
    if (qrcode == 0) {
        return 0;
    }

    return picoui_backend_qrcode_get_text((struct picoui_qrcode *)qrcode);
}
