/*
 * TinyUI qrcode unit tests — M3 Task 5 L3/L4 harness.
 */

#include "tinyui.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldQRCode.h"
#include "internal.h"
#include "widgets/qrcode.h"

#include <assert.h>
#include <string.h>

static unsigned int test_rgb_to_ld_color(unsigned int rgb)
{
    return (unsigned int)__RGB((rgb >> 16) & 0xFFU, (rgb >> 8) & 0xFFU, rgb & 0xFFU);
}

static void test_qrcode_create_and_ld_mapping(tinyui_obj_t *root)
{
    tinyui_obj_t *qr = tinyui_qrcode_create(root);
    struct tinyui_widget *backend;
    ldQRCode_t *ld_qr;

    assert(qr != 0);
    backend = (struct tinyui_widget *)(void *)qr;
    assert(backend->kind == TINYUI_BACKEND_WIDGET_QRCODE);
    ld_qr = (ldQRCode_t *)backend->ld_widget;
    assert(ld_qr != 0);
    assert(((ldBase_t *)ld_qr)->widgetType == widgetTypeQRCode);
}

static void test_qrcode_text_color_ecc_version_zoom(tinyui_obj_t *root)
{
    tinyui_obj_t *qr = tinyui_qrcode_create(root);
    ldQRCode_t *ld_qr;
    char text[] = "tinyui-qr";

    assert(qr != 0);
    ld_qr = (ldQRCode_t *)((struct tinyui_widget *)(void *)qr)->ld_widget;
    assert(ld_qr != 0);

    assert(tinyui_qrcode_set_text(qr, text) == 0);
    assert(ld_qr->pStr != 0);
    assert(strcmp((const char *)ld_qr->pStr, "tinyui-qr") == 0);
    assert(strcmp(tinyui_qrcode_get_text(qr), "tinyui-qr") == 0);

    assert(tinyui_qrcode_set_qr_color(qr, 0x101010U) == 0);
    assert(ld_qr->qrColor == (ldColor)test_rgb_to_ld_color(0x101010U));
    assert(tinyui_qrcode_set_bg_color(qr, 0xFEFEFEU) == 0);
    assert(ld_qr->bgColor == (ldColor)test_rgb_to_ld_color(0xFEFEFEU));

    assert(tinyui_qrcode_set_ecc(qr, 2) == 0);
    assert(ld_qr->qrEcc == 2);
    assert(tinyui_qrcode_set_max_version(qr, 8) == 0);
    assert(ld_qr->qrMaxVersion == 8);
    assert(tinyui_qrcode_set_zoom(qr, 3) == 0);
    assert(ld_qr->qrZoom == 3);
}

static void test_qrcode_rejects_invalid(tinyui_obj_t *root)
{
    tinyui_obj_t *qr = tinyui_qrcode_create(root);
    assert(qr != 0);
    assert(tinyui_qrcode_create(0) == 0);
    assert(tinyui_qrcode_set_text(qr, 0) == -1);
    assert(tinyui_qrcode_set_ecc(qr, 9) == -1);
    assert(tinyui_qrcode_set_max_version(qr, 0) == -1);
    assert(tinyui_qrcode_set_zoom(qr, 0) == -1);
}

int main(void)
{
    tinyui_obj_t *root;

    tinyui_deinit();
    assert(tinyui_init() == TINYUI_OK);
    root = tinyui_screen_create();
    assert(root != 0);

    test_qrcode_create_and_ld_mapping(root);
    test_qrcode_text_color_ecc_version_zoom(root);
    test_qrcode_rejects_invalid(root);

    tinyui_deinit();
    return 0;
}
