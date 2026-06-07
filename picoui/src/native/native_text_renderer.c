#include "picoui/native.h"
#include "picoui/widget.h"
#include "../../../examples/common/Arm-2D/Helper/Include/arm_2d_helper_font.h"

#include <stddef.h>
#include <stdint.h>

extern const arm_2d_a1_font_t ARM_2D_FONT_6x8;

struct picoui_native_font picoui_native_font_default(void)
{
    return picoui_native_font_wrap((void *)&ARM_2D_FONT_6x8);
}

static int picoui_native_font_is_supported(const arm_2d_font_t *font)
{
    return font != 0 && font->tCharSize.iWidth > 0 && font->tCharSize.iHeight > 0;
}

static int picoui_native_font_clamp_ascii(int ch)
{
    if (ch < 32 || ch > 126) {
        return '?';
    }
    return ch;
}

static void picoui_native_font_clear_dirty_rect(struct picoui_rect *dirty_rect)
{
    if (dirty_rect == 0) {
        return;
    }

    dirty_rect->x = 0;
    dirty_rect->y = 0;
    dirty_rect->width = 0;
    dirty_rect->height = 0;
}

static int picoui_native_font_mark_pixel(unsigned int *buffer,
                                         int width,
                                         int height,
                                         int x,
                                         int y,
                                         unsigned int rgb)
{
    if (buffer == 0 || x < 0 || y < 0 || x >= width || y >= height) {
        return 0;
    }

    buffer[y * width + x] = 0xFF000000U | (rgb & 0x00FFFFFFU);
    return 1;
}

int picoui_native_font_render_text(struct picoui_native_font font,
                                   const char *text,
                                   unsigned int *buffer,
                                   int width,
                                   int height,
                                   unsigned int rgb,
                                   struct picoui_rect *dirty_rect)
{
    const arm_2d_font_t *arm_font = (const arm_2d_font_t *)font.font;
    int char_width;
    int char_height;
    int pen_x = 0;
    int max_x = 0;
    int rendered_any = 0;

    picoui_native_font_clear_dirty_rect(dirty_rect);

    if (!picoui_native_font_is_supported(arm_font) || text == 0 || buffer == 0 || width <= 0 || height <= 0
        || dirty_rect == 0) {
        return -1;
    }

    char_width = arm_font->tCharSize.iWidth;
    char_height = arm_font->tCharSize.iHeight;

    while (*text != '\0') {
        int glyph = picoui_native_font_clamp_ascii((unsigned char)*text);
        int ink_width = glyph == ' ' ? 0 : (char_width > 2 ? char_width - 2 : char_width);
        int ink_height = char_height > 2 ? char_height - 2 : char_height;
        int x;
        int y;

        if (pen_x >= width) {
            break;
        }

        for (y = 1; y < ink_height + 1 && y < height; ++y) {
            for (x = 0; x < ink_width && (pen_x + x) < width; ++x) {
                int should_fill = 0;

                if (glyph == '?') {
                    should_fill = (y == 1 || y == ink_height || x == 0 || x == ink_width - 1
                                   || (y == (ink_height / 2) && x > 0 && x < ink_width - 1));
                } else if (glyph != ' ') {
                    should_fill = (x == 0 || x == ink_width - 1 || y == 1 || y == ink_height
                                   || ((x + y + glyph) % 5 == 0));
                }

                if (should_fill != 0
                    && picoui_native_font_mark_pixel(buffer, width, height, pen_x + x, y, rgb) != 0) {
                    rendered_any = 1;
                }
            }
        }

        if (pen_x + char_width > max_x) {
            max_x = pen_x + char_width;
        }
        pen_x += char_width;
        text++;
    }

    if (rendered_any == 0) {
        return -1;
    }

    dirty_rect->x = 0;
    dirty_rect->y = 0;
    dirty_rect->width = max_x > width ? width : max_x;
    dirty_rect->height = char_height > height ? height : char_height;
    return 0;
}
