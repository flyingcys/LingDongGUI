#include "picoui/app.h"
#include "picoui/qrcode.h"
#include "picoui/widget.h"
#include "picoui/window.h"
#include "../../../src/gui/ldQRCode.h"
#include "backend.h"
#include "internal.h"

#include <assert.h>
#include <string.h>

static void test_qrcode_create_and_props(struct picoui_window *win)
{
    int user_cookie = 7;
    struct picoui_qrcode_props props = {
        .id = "qr_props",
        .style_class = "qr-code",
        .user_data = &user_cookie,
        .text = "https://example.local/props",
    };
    struct picoui_qrcode *qrcode = picoui_qrcode_create((struct picoui_widget *)win, "qr");
    struct picoui_qrcode *with_props = picoui_qrcode_create_with_props((struct picoui_widget *)win, &props);

    assert(qrcode != 0);
    assert(with_props != 0);
    assert(picoui_qrcode_get_text(qrcode) != 0);
    assert(strcmp(picoui_qrcode_get_text(qrcode), "") == 0);
    assert(picoui_qrcode_get_text(with_props) != 0);
    assert(strcmp(picoui_qrcode_get_text(with_props), props.text) == 0);
}

static void test_qrcode_set_get_text(struct picoui_window *win)
{
    struct picoui_qrcode *qrcode = picoui_qrcode_create((struct picoui_widget *)win, "qr_text");
    const char *value = "https://example.local/qrcode";

    assert(qrcode != 0);
    assert(picoui_qrcode_set_text(qrcode, value) == 0);
    assert(picoui_qrcode_get_text(qrcode) != 0);
    assert(strcmp(picoui_qrcode_get_text(qrcode), value) == 0);
}

static void test_qrcode_rejects_invalid_inputs(struct picoui_window *win)
{
    struct picoui_qrcode *qrcode = picoui_qrcode_create((struct picoui_widget *)win, "qr_invalid");

    assert(qrcode != 0);
    assert(picoui_qrcode_create(0, "qr") == 0);
    assert(picoui_qrcode_create((struct picoui_widget *)win, 0) == 0);
    assert(picoui_qrcode_create_with_props(0,
                                           &(struct picoui_qrcode_props){
                                               .id = "bad_parent",
                                               .text = "abc",
                                           }) == 0);
    assert(picoui_qrcode_create_with_props((struct picoui_widget *)win, 0) == 0);
    assert(picoui_qrcode_create_with_props((struct picoui_widget *)win,
                                           &(struct picoui_qrcode_props){
                                               .text = "abc",
                                           }) == 0);
    assert(picoui_qrcode_create_with_props((struct picoui_widget *)win,
                                           &(struct picoui_qrcode_props){
                                               .id = "bad_text",
                                           }) == 0);
    assert(picoui_qrcode_set_text(0, "abc") == -1);
    assert(picoui_qrcode_set_text(qrcode, 0) == -1);
    assert(picoui_qrcode_get_text(0) == 0);
}

static void test_qrcode_release_contract_covers_configuration_boundary(struct picoui_window *win)
{
    const char *value = "https://example.local/final-release";
    struct picoui_qrcode *qrcode = picoui_qrcode_create_with_props(
        (struct picoui_widget *)win,
        &(struct picoui_qrcode_props){
            .id = "qr_release_ready",
            .style_class = "qr-card",
            .text = value,
        });
    struct picoui_backend_widget *backend;
    ldQRCode_t *ld_qrcode;

    assert(qrcode != 0);
    backend = (struct picoui_backend_widget *)qrcode->widget.backend_widget;
    assert(backend != 0);
    assert(backend->kind == PICOUI_BACKEND_WIDGET_QRCODE);
    assert(backend->style_class == (const char *)"qr-card");
    ld_qrcode = (ldQRCode_t *)backend->ld_widget;
    assert(ld_qrcode != 0);

    assert(strcmp(picoui_qrcode_get_text(qrcode), value) == 0);
    assert(strcmp((const char *)ld_qrcode->pStr, value) == 0);
    assert(ld_qrcode->qrColor == GLCD_COLOR_BLACK);
    assert(ld_qrcode->bgColor == GLCD_COLOR_WHITE);
    assert(ld_qrcode->qrEcc == QR_ECC_7);
    assert(ld_qrcode->qrMaxVersion == 2);
    assert(ld_qrcode->qrZoom == 4);
}

int main(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_window *win;

    assert(app != 0);
    win = picoui_window_create(app, "root");
    assert(win != 0);

    test_qrcode_create_and_props(win);
    test_qrcode_set_get_text(win);
    test_qrcode_rejects_invalid_inputs(win);
    test_qrcode_release_contract_covers_configuration_boundary(win);

    picoui_app_destroy(app);
    return 0;
}
