#include "picoui/picoui.h"
#include "../../../src/gui/ldBase.h"
#include "internal.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

int picoui_native_font_render_text(struct picoui_native_font font,
                                   const char *text,
                                   uint32_t *buffer,
                                   int width,
                                   int height,
                                   unsigned int rgb,
                                   struct picoui_rect *dirty_rect);
struct picoui_native_font picoui_native_font_default(void);

static void test_default_font_ascii_text_marks_dirty_area(void)
{
    uint32_t buffer[64 * 32] = {0};
    struct picoui_rect dirty_rect = {-1, -1, 0, 0};
    struct picoui_native_font font = picoui_native_font_default();
    int rc;
    int i;
    int changed_pixels = 0;

    assert(font.font != 0);
    rc = picoui_native_font_render_text(font,
                                        "P5-B",
                                        buffer,
                                        64,
                                        32,
                                        0x00FFFFFFU,
                                        &dirty_rect);
    assert(rc == 0);
    assert(dirty_rect.x >= 0);
    assert(dirty_rect.y >= 0);
    assert(dirty_rect.width > 0);
    assert(dirty_rect.height > 0);

    for (i = 0; i < (int)(sizeof(buffer) / sizeof(buffer[0])); ++i) {
        if (buffer[i] != 0) {
            changed_pixels++;
        }
    }

    assert(changed_pixels > 0);
}

static void test_invalid_font_render_inputs_fail_without_dirty_area(void)
{
    uint32_t buffer[16] = {0};
    struct picoui_rect dirty_rect = {9, 9, 9, 9};
    struct picoui_native_font null_font = picoui_native_font_wrap(0);

    assert(picoui_native_font_render_text(null_font, "A", buffer, 4, 4, 0x00FFFFFFU, &dirty_rect) == -1);
    assert(picoui_native_font_render_text(picoui_native_font_default(),
                                          0,
                                          buffer,
                                          4,
                                          4,
                                          0x00FFFFFFU,
                                          &dirty_rect)
           == -1);
    assert(picoui_native_font_render_text(picoui_native_font_default(),
                                          "A",
                                          0,
                                          4,
                                          4,
                                          0x00FFFFFFU,
                                          &dirty_rect)
           == -1);
}

int main(void)
{
    test_default_font_ascii_text_marks_dirty_area();
    test_invalid_font_render_inputs_fail_without_dirty_area();
    return 0;
}
