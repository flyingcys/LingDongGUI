#ifndef PICOUI_QRCODE_H
#define PICOUI_QRCODE_H

struct picoui_widget;
struct picoui_qrcode;

struct picoui_qrcode_props {
    const char *id;
    const char *style_class;
    void *user_data;
    const char *text;
};

struct picoui_qrcode *picoui_qrcode_create(struct picoui_widget *parent, const char *id);
struct picoui_qrcode *picoui_qrcode_create_with_props(struct picoui_widget *parent,
                                                      const struct picoui_qrcode_props *props);
int picoui_qrcode_set_text(struct picoui_qrcode *qrcode, const char *text);
const char *picoui_qrcode_get_text(const struct picoui_qrcode *qrcode);

#endif
