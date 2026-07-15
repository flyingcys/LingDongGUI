/*
 * TinyUI v2.3 external C consumer fixture.
 * Proves installed public headers + TinyUI::tinyui can compile and link.
 * Does not call runtime init/process or any platform host (SDL).
 */

#include <tinyui.h>

int main(void)
{
    uint16_t pixels[4] = {0, 0, 0, 0};
    tinyui_image_source_t source;
    tinyui_result_t result;

    /* stride is bytes: 2 pixels * sizeof(uint16_t) = 4 */
    result = tinyui_image_source_from_rgb565(pixels,
                                            2,
                                            2,
                                            4,
                                            0,
                                            0,
                                            &source);
    if (result != TINYUI_OK) {
        return 1;
    }

    tinyui_image_source_deinit(&source);
    return 0;
}
