#include "picoui/widget.h"
#include "../core/internal.h"
#include "../../../examples/common/Arm-2D/examples/common/controls/qrcode_box/qrcodegen.h"

#include <stdlib.h>

static void picoui_native_qrcode_clear_dirty_rect(struct picoui_rect *dirty_rect)
{
    if (dirty_rect == 0) {
        return;
    }

    dirty_rect->x = 0;
    dirty_rect->y = 0;
    dirty_rect->width = 0;
    dirty_rect->height = 0;
}

static unsigned int picoui_native_qrcode_pack_color(unsigned int rgb)
{
    return 0xFF000000U | (rgb & 0x00FFFFFFU);
}

static int picoui_native_qrcode_resolve_version(const struct picoui_qrcode *qrcode)
{
    int version;

    version = qrcode->max_version;
    if (version <= 0) {
        version = 1;
    }
    if (version > 40) {
        version = 40;
    }
    return version;
}

int picoui_native_qrcode_render_buffer(const struct picoui_qrcode *qrcode,
                                       unsigned int *buffer,
                                       int width,
                                       int height,
                                       struct picoui_rect *dirty_rect)
{
    uint8_t *qr0;
    uint8_t *temp_buffer;
    unsigned int dark_color;
    unsigned int light_color;
    int version;
    int module_count;
    int module_size;
    int render_size;
    int offset_x;
    int offset_y;
    int x;
    int y;
    int mx;
    int my;
    int px;
    int py;
    int rc = -1;

    picoui_native_qrcode_clear_dirty_rect(dirty_rect);

    if (qrcode == 0 || qrcode->text == 0 || buffer == 0 || width <= 0 || height <= 0 || dirty_rect == 0) {
        return -1;
    }

    version = picoui_native_qrcode_resolve_version(qrcode);
    qr0 = (uint8_t *)calloc(1, qrcodegen_BUFFER_LEN_FOR_VERSION(version));
    temp_buffer = (uint8_t *)calloc(1, qrcodegen_BUFFER_LEN_FOR_VERSION(version));
    if (qr0 == 0 || temp_buffer == 0) {
        goto done;
    }

    if (!qrcodegen_encodeText(qrcode->text,
                              temp_buffer,
                              qr0,
                              (enum qrcodegen_Ecc)qrcode->ecc,
                              version,
                              version,
                              qrcodegen_Mask_AUTO,
                              true)) {
        goto done;
    }

    module_count = qrcodegen_getSize(qr0);
    if (module_count <= 0) {
        goto done;
    }

    module_size = qrcode->zoom > 0 ? qrcode->zoom : 1;
    render_size = module_count * module_size;
    if (render_size > width) {
        module_size = width / module_count;
    }
    if (module_size <= 0) {
        goto done;
    }
    if (module_count * module_size > height) {
        module_size = height / module_count;
    }
    if (module_size <= 0) {
        goto done;
    }

    render_size = module_count * module_size;
    offset_x = (width - render_size) / 2;
    offset_y = (height - render_size) / 2;
    dark_color = picoui_native_qrcode_pack_color(qrcode->qr_color);
    light_color = picoui_native_qrcode_pack_color(qrcode->bg_color);

    for (my = 0; my < module_count; ++my) {
        for (mx = 0; mx < module_count; ++mx) {
            unsigned int color = qrcodegen_getModule(qr0, mx, my) ? dark_color : light_color;
            int start_x = offset_x + mx * module_size;
            int start_y = offset_y + my * module_size;

            for (y = 0; y < module_size; ++y) {
                py = start_y + y;
                if (py < 0 || py >= height) {
                    continue;
                }
                for (x = 0; x < module_size; ++x) {
                    px = start_x + x;
                    if (px < 0 || px >= width) {
                        continue;
                    }
                    buffer[py * width + px] = color;
                }
            }
        }
    }

    dirty_rect->x = offset_x;
    dirty_rect->y = offset_y;
    dirty_rect->width = render_size;
    dirty_rect->height = render_size;
    rc = 0;

done:
    free(qr0);
    free(temp_buffer);
    return rc;
}
