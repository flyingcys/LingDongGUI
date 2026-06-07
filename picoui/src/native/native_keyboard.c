#include "../backend/ldgui/backend.h"
#include "../core/internal.h"

#include "../../../src/gui/ldBase.h"

int picoui_native_line_edit_set_editing(struct picoui_line_edit *line_edit, int editing);
int picoui_native_line_edit_append_ascii(struct picoui_line_edit *line_edit, unsigned int ascii);
int picoui_native_line_edit_backspace(struct picoui_line_edit *line_edit);
const char *picoui_native_line_edit_get_text(const struct picoui_line_edit *line_edit);
int picoui_native_line_edit_set_text(struct picoui_line_edit *line_edit, const char *text);

static struct picoui_line_edit *picoui_native_keyboard_get_target_line_edit(
    const struct picoui_keyboard *keyboard)
{
    struct picoui_backend_widget *backend;
    struct picoui_app *app;
    struct picoui_widget *target;

    if (keyboard == 0 || keyboard->widget.backend_widget == 0) {
        return 0;
    }

    backend = (struct picoui_backend_widget *)keyboard->widget.backend_widget;
    app = backend->owner;
    if (app == 0) {
        return 0;
    }

    target = app->editing_owner != 0 ? app->editing_owner : app->focus_owner;
    if (target == 0 || target->backend_widget == 0 || target->accepts_text_input == 0) {
        return 0;
    }

    if (picoui_widget_get_type(target) != PICOUI_WIDGET_TYPE_LINE_EDIT) {
        struct picoui_backend_widget *target_backend = (struct picoui_backend_widget *)target->backend_widget;
        ldBase_t *ld_base;

        if (target_backend == 0 || target_backend->kind != PICOUI_BACKEND_WIDGET_TEXT
            || target_backend->ld_widget == 0) {
            return 0;
        }

        ld_base = (ldBase_t *)target_backend->ld_widget;
        if (ld_base->widgetType != widgetTypeLineEdit) {
            return 0;
        }
    } else {
        return (struct picoui_line_edit *)target;
    }

    if (picoui_widget_get_type(target) != PICOUI_WIDGET_TYPE_LINE_EDIT
        && ((ldBase_t *)((struct picoui_backend_widget *)target->backend_widget)->ld_widget)->widgetType
               != widgetTypeLineEdit) {
        return 0;
    }

    return (struct picoui_line_edit *)target;
}

int picoui_native_keyboard_input_ascii(struct picoui_keyboard *keyboard, unsigned int ascii)
{
    struct picoui_line_edit *line_edit;
    const char *text;

    if (keyboard == 0) {
        return -1;
    }

    keyboard->selected_key_code = ascii;
    line_edit = picoui_native_keyboard_get_target_line_edit(keyboard);
    if (line_edit == 0) {
        return -1;
    }

    if (picoui_widget_is_editing_owner(&line_edit->widget) == 0
        && picoui_widget_claim_editing(&line_edit->widget) != 0) {
        return -1;
    }

    if (ascii == '\r' || ascii == '\n') {
        if (picoui_native_line_edit_set_editing(line_edit, 0) != 0) {
            return -1;
        }
        if (picoui_widget_mark_edit_result(&line_edit->widget, PICOUI_EDIT_RESULT_COMMIT) != 0) {
            return -1;
        }
        if (picoui_widget_release_editing(&line_edit->widget) != 0) {
            return -1;
        }
        if (line_edit->on_edit_finished != 0) {
            line_edit->on_edit_finished(line_edit, line_edit->on_edit_finished_user_data);
        }
        return 0;
    }

    if (picoui_native_line_edit_set_editing(line_edit, 1) != 0) {
        return -1;
    }

    if (ascii == '\b') {
        if (picoui_native_line_edit_backspace(line_edit) != 0) {
            return -1;
        }
    } else {
        if (picoui_native_line_edit_append_ascii(line_edit, ascii) != 0) {
            return -1;
        }
    }

    text = picoui_native_line_edit_get_text(line_edit);
    if (text == 0 || picoui_native_line_edit_set_text(line_edit, text) != 0) {
        return -1;
    }
    (void)picoui_backend_line_edit_set_text(line_edit->widget.backend_widget, text);
    return 0;
}
