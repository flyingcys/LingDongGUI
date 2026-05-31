#include "backend.h"
#include "internal.h"
#include "ldKeyboard.h"
#include "ldLineEdit.h"

#include <stdlib.h>

extern const arm_2d_a1_font_t ARM_2D_FONT_6x8;
void ldKeyboardInputAscii(ldKeyboard_t *ptWidget, uint8_t ascii);

static struct picoui_backend_app_state *picoui_backend_keyboard_get_app_state(void *parent)
{
    struct picoui_backend_widget *parent_widget = parent;

    if (parent_widget == NULL || parent_widget->owner == NULL || parent_widget->owner->backend_app == NULL) {
        return NULL;
    }
    return (struct picoui_backend_app_state *)parent_widget->owner->backend_app;
}

static ldKeyboard_t *picoui_backend_keyboard_get_ld(void *backend_widget)
{
    struct picoui_backend_widget *widget = backend_widget;

    if (widget == NULL || widget->ld_widget == NULL) {
        return NULL;
    }

    return (ldKeyboard_t *)widget->ld_widget;
}

static void picoui_backend_keyboard_prepare(ldKeyboard_t *ld_keyboard,
                                            struct picoui_line_edit *line_edit)
{
    if (ld_keyboard == NULL) {
        return;
    }

    if (line_edit != NULL) {
        ld_keyboard->editType = (ldEditType_t)line_edit->type;
    }
    if (ld_keyboard->pBtnList == NULL || ld_keyboard->isWaitInit) {
        ld_keyboard->pBtnList = ldKeyboardGetTargetBtnList(ld_keyboard);
        ld_keyboard->isWaitInit = false;
    }
    ldBaseSetHidden((ldBase_t *)ld_keyboard, false);
}

static struct picoui_line_edit *picoui_backend_keyboard_get_target_line_edit(struct picoui_backend_widget *backend)
{
    struct picoui_app *app;
    struct picoui_widget *target;
    struct picoui_backend_widget *target_backend;
    ldBase_t *ld_base;

    if (backend == NULL || backend->owner == NULL) {
        return NULL;
    }

    app = backend->owner;
    target = app->editing_owner != NULL ? app->editing_owner : app->focus_owner;
    if (target == NULL || target->backend_widget == NULL) {
        return NULL;
    }

    target_backend = (struct picoui_backend_widget *)target->backend_widget;
    if (target_backend->kind != PICOUI_BACKEND_WIDGET_TEXT || target_backend->ld_widget == NULL) {
        return NULL;
    }

    ld_base = (ldBase_t *)target_backend->ld_widget;
    if (ld_base->widgetType != widgetTypeLineEdit) {
        return NULL;
    }
    return (struct picoui_line_edit *)target;
}

void *picoui_backend_create_keyboard(void *parent, const char *id)
{
    struct picoui_backend_widget *widget;
    struct picoui_backend_widget *parent_widget = parent;
    struct picoui_backend_app_state *app_state;
    ldKeyboard_t *ld_keyboard;
    uint16_t name_id;

    if (parent == 0 || id == 0) {
        return 0;
    }

    app_state = picoui_backend_keyboard_get_app_state(parent);
    if (app_state == NULL || app_state->ld_scene == NULL || parent_widget->ld_widget == NULL) {
        return 0;
    }

    widget = calloc(1, sizeof(*widget));
    if (widget == 0) {
        return 0;
    }

    name_id = ++app_state->next_ld_name_id;
    ld_keyboard = ldKeyboard_init(app_state->ld_scene,
                                  NULL,
                                  name_id,
                                  parent_widget->ld_name_id,
                                  (arm_2d_font_t *)&ARM_2D_FONT_6x8);
    if (ld_keyboard == NULL) {
        free(widget);
        return 0;
    }

    widget->parent = parent;
    widget->id = id;
    widget->kind = PICOUI_BACKEND_WIDGET_KEYBOARD;
    widget->theme = parent_widget->theme;
    widget->ld_widget = ld_keyboard;
    widget->ld_name_id = name_id;
    if (picoui_backend_widget_attach_child(parent, widget) != 0) {
        free(widget);
        return 0;
    }
    return widget;
}

int picoui_backend_keyboard_input_ascii(void *backend_widget, unsigned int ascii)
{
    struct picoui_backend_widget *backend = backend_widget;
    struct picoui_line_edit *line_edit;
    ldKeyboard_t *ld_keyboard;
    ldLineEdit_t *ld_line_edit;

    if (backend == NULL) {
        return -1;
    }

    line_edit = picoui_backend_keyboard_get_target_line_edit(backend);
    ld_keyboard = picoui_backend_keyboard_get_ld(backend_widget);
    if (line_edit == NULL || ld_keyboard == NULL) {
        return -1;
    }
    ld_line_edit = (ldLineEdit_t *)((struct picoui_backend_widget *)line_edit->widget.backend_widget)->ld_widget;
    if (ld_line_edit == NULL) {
        return -1;
    }

    picoui_backend_keyboard_prepare(ld_keyboard, line_edit);
    ld_keyboard->ppStr = &ld_line_edit->pText;
    ld_keyboard->strMax = ld_line_edit->textMax;
    ld_keyboard->editorId = ((struct picoui_backend_widget *)line_edit->widget.backend_widget)->ld_name_id;
    ldKeyboardInputAscii(ld_keyboard, (uint8_t)ascii);
    line_edit->widget.text = picoui_backend_line_edit_get_text(line_edit->widget.backend_widget);
    return 0;
}

int picoui_backend_keyboard_navigate(void *backend_widget, int direction)
{
    struct picoui_backend_widget *backend = backend_widget;
    ldKeyboard_t *ld_keyboard;

    if (backend == NULL || backend->host_widget == NULL || !picoui_widget_is_focus_owner(backend->host_widget)) {
        return -1;
    }

    ld_keyboard = picoui_backend_keyboard_get_ld(backend_widget);
    if (ld_keyboard == NULL) {
        return -1;
    }

    picoui_backend_keyboard_prepare(ld_keyboard, picoui_backend_keyboard_get_target_line_edit(backend));
    ldKeyboardNavigate(ld_keyboard, (ldNavDir_t)direction);
    return 0;
}

int picoui_backend_keyboard_click(void *backend_widget)
{
    struct picoui_backend_widget *backend = backend_widget;
    struct picoui_backend_app_state *app_state;
    ldKeyboard_t *ld_keyboard;

    if (backend == NULL || backend->host_widget == NULL || !picoui_widget_is_focus_owner(backend->host_widget)) {
        return -1;
    }

    app_state = (struct picoui_backend_app_state *)backend->owner->backend_app;
    ld_keyboard = picoui_backend_keyboard_get_ld(backend_widget);
    if (app_state == NULL || app_state->ld_scene == NULL || ld_keyboard == NULL) {
        return -1;
    }

    picoui_backend_keyboard_prepare(ld_keyboard, picoui_backend_keyboard_get_target_line_edit(backend));
    ldKeyboardClick(app_state->ld_scene, ld_keyboard, SIGNAL_PRESS);
    return 0;
}

int picoui_backend_keyboard_exit(void *backend_widget)
{
    struct picoui_backend_widget *backend = backend_widget;
    struct picoui_line_edit *line_edit;
    ldKeyboard_t *ld_keyboard;
    ldLineEdit_t *ld_line_edit = NULL;

    if (backend == NULL) {
        return -1;
    }

    ld_keyboard = picoui_backend_keyboard_get_ld(backend_widget);
    if (ld_keyboard == NULL) {
        return -1;
    }

    line_edit = picoui_backend_keyboard_get_target_line_edit(backend);
    ldKeyboardExit(ld_keyboard);
    if (line_edit != NULL) {
        ld_line_edit = (ldLineEdit_t *)((struct picoui_backend_widget *)line_edit->widget.backend_widget)->ld_widget;
        line_edit->editing = 0;
        if (ld_line_edit != NULL) {
            ld_line_edit->isEditing = false;
            ((ldBase_t *)ld_line_edit)->isDirtyRegionUpdate = true;
        }
        ((struct picoui_backend_widget *)line_edit->widget.backend_widget)->edit_result_on_finish =
            PICOUI_EDIT_RESULT_CANCEL;
        (void)picoui_widget_mark_edit_result(&line_edit->widget, PICOUI_EDIT_RESULT_CANCEL);
        (void)picoui_widget_release_editing(&line_edit->widget);
    }
    if (backend->host_widget != NULL && picoui_widget_is_focus_owner(backend->host_widget)) {
        return picoui_widget_release_focus(backend->host_widget);
    }
    return 0;
}
