#include "backend.h"
#include "internal.h"
#include "ldBase.h"
#include "ldMessageBox.h"

#include <stdlib.h>

extern const arm_2d_a1_font_t ARM_2D_FONT_6x8;

static struct picoui_backend_app_state *picoui_backend_message_box_get_app_state(void *parent)
{
    struct picoui_backend_widget *parent_widget = parent;

    if (parent_widget == NULL || parent_widget->owner == NULL || parent_widget->owner->backend_app == NULL) {
        return NULL;
    }
    return (struct picoui_backend_app_state *)parent_widget->owner->backend_app;
}

static ldMessageBox_t *picoui_backend_message_box_get_ld(struct picoui_message_box *box)
{
    struct picoui_backend_widget *backend;

    if (box == NULL || box->widget.backend_widget == NULL) {
        return NULL;
    }

    backend = (struct picoui_backend_widget *)box->widget.backend_widget;
    if (backend->kind != PICOUI_BACKEND_WIDGET_MESSAGE_BOX || backend->ld_widget == NULL) {
        return NULL;
    }

    return (ldMessageBox_t *)backend->ld_widget;
}

static void picoui_backend_message_box_confirm_bridge(ld_scene_t *scene, ldMessageBox_t *ld_message_box)
{
    struct picoui_backend_widget *backend;
    struct picoui_message_box *box;

    (void)scene;

    if (ld_message_box == NULL) {
        return;
    }

    backend = (struct picoui_backend_widget *)((ldBase_t *)ld_message_box)->pInfo;
    if (backend == NULL || backend->host_widget == NULL) {
        return;
    }

    box = (struct picoui_message_box *)backend->host_widget;
    if (box->widget.enabled == 0 || box->widget.visible == 0) {
        return;
    }

    if (box->on_confirm != 0) {
        box->on_confirm(box, box->on_confirm_user_data);
    }
}

void *picoui_backend_create_message_box(void *parent, const char *id)
{
    struct picoui_backend_widget *widget;
    struct picoui_backend_widget *parent_widget = parent;
    struct picoui_backend_app_state *app_state;
    ldMessageBox_t *ld_message_box;
    uint16_t name_id;

    if (parent == 0 || id == 0) {
        return 0;
    }

    app_state = picoui_backend_message_box_get_app_state(parent);
    if (app_state == NULL || app_state->ld_scene == NULL || parent_widget->ld_widget == NULL) {
        return 0;
    }

    widget = calloc(1, sizeof(*widget));
    if (widget == 0) {
        return 0;
    }

    name_id = ++app_state->next_ld_name_id;
    ld_message_box = ldMessageBox_init(app_state->ld_scene,
                                       NULL,
                                       name_id,
                                       parent_widget->ld_name_id,
                                       0,
                                       0,
                                       260,
                                       140,
                                       (arm_2d_font_t *)&ARM_2D_FONT_6x8);
    if (ld_message_box == NULL) {
        free(widget);
        return 0;
    }

    widget->parent = parent;
    widget->id = id;
    widget->kind = PICOUI_BACKEND_WIDGET_MESSAGE_BOX;
    widget->theme = ((struct picoui_backend_widget *)parent)->theme;
    widget->ld_widget = ld_message_box;
    widget->ld_name_id = name_id;
    widget->runtime_evidence_flags = PICOUI_BACKEND_EVIDENCE_EXCLUDE_FORMAL_MAPPING |
                                     PICOUI_BACKEND_EVIDENCE_ALLOW_SMOKE_LAYOUT;
    if (picoui_backend_widget_attach_child(parent, widget) != 0) {
        free(widget);
        return 0;
    }
    return widget;
}

int picoui_backend_message_box_set_title(struct picoui_message_box *box, const char *title)
{
    ldMessageBox_t *ld_message_box = picoui_backend_message_box_get_ld(box);

    if (ld_message_box == NULL || title == NULL) {
        return -1;
    }

    ldMessageBoxSetTitle(ld_message_box, (const uint8_t *)title);
    return 0;
}

int picoui_backend_message_box_set_message(struct picoui_message_box *box, const char *message)
{
    ldMessageBox_t *ld_message_box = picoui_backend_message_box_get_ld(box);

    if (ld_message_box == NULL || message == NULL) {
        return -1;
    }

    ldMessageBoxSetMsg(ld_message_box, (const uint8_t *)message);
    return 0;
}

int picoui_backend_message_box_set_confirm_text(struct picoui_message_box *box, const char *text)
{
    ldMessageBox_t *ld_message_box = picoui_backend_message_box_get_ld(box);
    const uint8_t *buttons[1];

    if (ld_message_box == NULL || text == NULL) {
        return -1;
    }

    buttons[0] = (const uint8_t *)text;
    ldMessageBoxSetBtn(ld_message_box, buttons, 1);
    return 0;
}

int picoui_backend_message_box_set_on_confirm(struct picoui_message_box *box)
{
    ldMessageBox_t *ld_message_box = picoui_backend_message_box_get_ld(box);

    if (ld_message_box == NULL) {
        return -1;
    }

    ldMessageBoxSetCallback(ld_message_box, picoui_backend_message_box_confirm_bridge);
    return 0;
}
