#ifndef PICOUI_QRCODE_H
#define PICOUI_QRCODE_H

struct picoui_widget;
struct picoui_qrcode;

struct picoui_qrcode_props {
    const char *id;
    const char *style_class;
    void *user_data;
    const char *text;
    unsigned int qr_color;
    unsigned int bg_color;
    int ecc;
    int max_version;
    int zoom;
};

struct picoui_qrcode *picoui_qrcode_create(struct picoui_widget *parent, const char *id);
struct picoui_qrcode *picoui_qrcode_create_with_props(struct picoui_widget *parent,
                                                      const struct picoui_qrcode_props *props);
int picoui_qrcode_set_text(struct picoui_qrcode *qrcode, const char *text);
const char *picoui_qrcode_get_text(const struct picoui_qrcode *qrcode);
int picoui_qrcode_set_qr_color(struct picoui_qrcode *qrcode, unsigned int rgb);
int picoui_qrcode_set_bg_color(struct picoui_qrcode *qrcode, unsigned int rgb);
int picoui_qrcode_set_ecc(struct picoui_qrcode *qrcode, int ecc);
int picoui_qrcode_set_max_version(struct picoui_qrcode *qrcode, int max_version);
int picoui_qrcode_set_zoom(struct picoui_qrcode *qrcode, int zoom);

#endif
