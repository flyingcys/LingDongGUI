/*
 * TinyUI v2.3 external C++ consumer fixture.
 * Proves <tinyui.h> is C++-linkable without an extra extern "C" wrapper.
 * Does not call runtime init/process or any platform host (SDL).
 */

#include <tinyui.h>

int main()
{
    tinyui_font_t font;
    tinyui_result_t result = tinyui_font_from_builtin(TINYUI_FONT_6X8, &font);
    if (result != TINYUI_OK) {
        return 1;
    }

    tinyui_font_deinit(&font);
    return 0;
}
