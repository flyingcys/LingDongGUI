#include "picoui/picoui.h"
#include "internal.h"

#include <assert.h>
#include <string.h>

int picoui_native_qrcode_render_buffer(const struct picoui_qrcode *qrcode,
                                       unsigned int *buffer,
                                       int width,
                                       int height,
                                       struct picoui_rect *dirty_rect);

static void test_qrcode_payload_marks_finder_pattern_pixels(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_window *window;
    struct picoui_qrcode *qrcode;
    unsigned int buffer[128 * 128] = {0};
    struct picoui_rect dirty_rect = {-1, -1, 0, 0};
    unsigned int dark_color = 0xFF000000U;
    int module_size = 4;
    int right_origin_x;
    int bottom_origin_y;
    int i;
    int changed_pixels = 0;

    assert(app != 0);
    window = picoui_window_create(app, "qrcode_renderer_root");
    assert(window != 0);
    qrcode = picoui_qrcode_create((struct picoui_widget *)window, "qr_renderer");
    assert(qrcode != 0);
    assert(picoui_widget_set_size((struct picoui_widget *)qrcode, 128, 128) == 0);
    assert(picoui_qrcode_set_text(qrcode, "PICOUI") == 0);
    assert(picoui_qrcode_set_max_version(qrcode, 4) == 0);
    assert(picoui_qrcode_set_zoom(qrcode, 4) == 0);

    assert(picoui_native_qrcode_render_buffer(qrcode, buffer, 128, 128, &dirty_rect) == 0);
    assert(dirty_rect.x >= 0);
    assert(dirty_rect.y >= 0);
    assert(dirty_rect.width > 0);
    assert(dirty_rect.height > 0);

    for (i = 0; i < (int)(sizeof(buffer) / sizeof(buffer[0])); ++i) {
        if (buffer[i] != 0U) {
            changed_pixels++;
        }
    }

    right_origin_x = dirty_rect.x + dirty_rect.width - module_size;
    bottom_origin_y = dirty_rect.y + dirty_rect.height - module_size;

    assert(changed_pixels > 0);
    assert(buffer[(dirty_rect.y + 1) * 128 + (dirty_rect.x + 1)] == dark_color);
    assert(buffer[(dirty_rect.y + 1) * 128 + (right_origin_x + 1)] == dark_color);
    assert(buffer[(bottom_origin_y + 1) * 128 + (dirty_rect.x + 1)] == dark_color);

    picoui_app_destroy(app);
}

static void test_qrcode_renderer_rejects_invalid_inputs(void)
{
    unsigned int buffer[16];
    struct picoui_rect dirty_rect = {5, 5, 5, 5};
    unsigned int snapshot[16];

    memset(buffer, 0x5A, sizeof(buffer));
    memcpy(snapshot, buffer, sizeof(buffer));
    assert(picoui_native_qrcode_render_buffer(0, buffer, 4, 4, &dirty_rect) == -1);
    assert(memcmp(buffer, snapshot, sizeof(buffer)) == 0);
}

int main(void)
{
    test_qrcode_payload_marks_finder_pattern_pixels();
    test_qrcode_renderer_rejects_invalid_inputs();
    return 0;
}
