#include "internal.h"
#include "picoui/image.h"
#include "../../../src/gui/ldBase.h"

#include <string.h>

int picoui_image_source_from_vres(unsigned int addr, struct picoui_image_source *out)
{
    if (addr == 0 || out == 0) {
        return -1;
    }

    memset(out, 0, sizeof(*out));
    out->img_tile = ldBaseGetVresImage(addr);
    if (out->img_tile == 0) {
        return -1;
    }
    out->vres_addr = addr;

    return 0;
}

int picoui_font_from_vres(unsigned int addr, struct picoui_font *out)
{
    if (addr == 0 || out == 0) {
        return -1;
    }

    memset(out, 0, sizeof(*out));
    out->kind = PICOUI_FONT_KIND_VRES;
    out->vres_addr = addr;
    return 0;
}

void picoui_image_source_destroy(struct picoui_image_source *source)
{
    if (source == 0) {
        return;
    }

    if (source->img_tile != 0) {
        ldFree(source->img_tile);
    }
    memset(source, 0, sizeof(*source));
}

void picoui_font_destroy(struct picoui_font *font)
{
    if (font == 0) {
        return;
    }

    memset(font, 0, sizeof(*font));
}
