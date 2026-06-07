#include "../backend/ldgui/backend.h"
#include "../core/internal.h"

#include <string.h>

#define PICOUI_NATIVE_LINE_EDIT_TEXT_MAX 255

int picoui_native_text_render(const struct picoui_backend_widget *backend);

int picoui_native_line_edit_set_text(struct picoui_line_edit *line_edit, const char *text);

int picoui_native_line_edit_init(struct picoui_line_edit *line_edit)
{
    if (line_edit == 0) {
        return -1;
    }

    line_edit->text_buffer[0] = '\0';
    line_edit->editing = 0;
    if (line_edit->widget.text != 0) {
        return picoui_native_line_edit_set_text(line_edit, line_edit->widget.text);
    }
    return 0;
}

int picoui_native_line_edit_set_text(struct picoui_line_edit *line_edit, const char *text)
{
    size_t len;

    if (line_edit == 0 || text == 0) {
        return -1;
    }

    len = strlen(text);
    if (len > PICOUI_NATIVE_LINE_EDIT_TEXT_MAX) {
        return -1;
    }

    memcpy(line_edit->text_buffer, text, len + 1U);
    line_edit->widget.text = line_edit->text_buffer;
    return 0;
}

const char *picoui_native_line_edit_get_text(const struct picoui_line_edit *line_edit)
{
    if (line_edit == 0) {
        return 0;
    }

    return line_edit->text_buffer;
}

int picoui_native_line_edit_set_editing(struct picoui_line_edit *line_edit, int editing)
{
    if (line_edit == 0) {
        return -1;
    }

    line_edit->editing = editing != 0 ? 1 : 0;
    return 0;
}

int picoui_native_line_edit_get_editing(const struct picoui_line_edit *line_edit, int *editing)
{
    if (line_edit == 0 || editing == 0) {
        return -1;
    }

    *editing = line_edit->editing != 0 ? 1 : 0;
    return 0;
}

int picoui_native_line_edit_append_ascii(struct picoui_line_edit *line_edit, unsigned int ascii)
{
    size_t len;

    if (line_edit == 0 || ascii == 0 || ascii > 0x7FU) {
        return -1;
    }

    len = strlen(line_edit->text_buffer);
    if (len >= PICOUI_NATIVE_LINE_EDIT_TEXT_MAX) {
        return -1;
    }

    line_edit->text_buffer[len] = (char)ascii;
    line_edit->text_buffer[len + 1U] = '\0';
    line_edit->widget.text = line_edit->text_buffer;
    return 0;
}

int picoui_native_line_edit_backspace(struct picoui_line_edit *line_edit)
{
    size_t len;

    if (line_edit == 0) {
        return -1;
    }

    len = strlen(line_edit->text_buffer);
    if (len == 0U) {
        return 0;
    }

    line_edit->text_buffer[len - 1U] = '\0';
    line_edit->widget.text = line_edit->text_buffer;
    return 0;
}

int picoui_native_line_edit_render(const struct picoui_backend_widget *backend)
{
    if (backend == 0 || backend->kind != PICOUI_BACKEND_WIDGET_TEXT || backend->host_widget == 0) {
        return -1;
    }

    return picoui_native_text_render(backend);
}
