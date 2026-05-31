#include "internal.h"

static struct picoui_app *picoui_widget_get_owner_app(struct picoui_widget *widget)
{
    struct picoui_backend_widget *backend;

    if (widget == 0 || widget->backend_widget == 0) {
        return 0;
    }

    backend = (struct picoui_backend_widget *)widget->backend_widget;
    return backend->owner;
}

static void picoui_widget_note_focus_event(struct picoui_widget *widget,
                                           enum picoui_focus_event event)
{
    if (widget == 0 || event == PICOUI_FOCUS_EVENT_NONE) {
        return;
    }

    widget->last_focus_event = event;
    widget->focus_change_count++;
    if (event == PICOUI_FOCUS_EVENT_ENTER) {
        widget->has_focus = 1;
        widget->focus_enter_count++;
        return;
    }

    widget->has_focus = 0;
    widget->focus_leave_count++;
}

int picoui_widget_claim_focus(struct picoui_widget *widget)
{
    struct picoui_app *owner;
    struct picoui_widget *previous;

    if (widget == 0 || widget->visible == 0 || widget->enabled == 0) {
        return -1;
    }

    owner = picoui_widget_get_owner_app(widget);
    if (owner == 0) {
        return -1;
    }

    previous = owner->focus_owner;
    if (previous == widget) {
        if (widget->has_focus == 0) {
            picoui_widget_note_focus_event(widget, PICOUI_FOCUS_EVENT_ENTER);
        }
        return 0;
    }

    if (previous != 0) {
        picoui_widget_note_focus_event(previous, PICOUI_FOCUS_EVENT_LEAVE);
    }

    owner->focus_owner = widget;
    picoui_widget_note_focus_event(widget, PICOUI_FOCUS_EVENT_ENTER);
    return 0;
}

int picoui_widget_release_focus(struct picoui_widget *widget)
{
    struct picoui_app *owner;

    if (widget == 0) {
        return -1;
    }

    owner = picoui_widget_get_owner_app(widget);
    if (owner == 0) {
        return -1;
    }

    if (owner->focus_owner != widget) {
        if (widget->has_focus != 0) {
            picoui_widget_note_focus_event(widget, PICOUI_FOCUS_EVENT_LEAVE);
        }
        return 0;
    }

    owner->focus_owner = 0;
    picoui_widget_note_focus_event(widget, PICOUI_FOCUS_EVENT_LEAVE);
    return 0;
}

int picoui_widget_is_focus_owner(const struct picoui_widget *widget)
{
    struct picoui_app *owner;

    if (widget == 0) {
        return 0;
    }

    owner = picoui_widget_get_owner_app((struct picoui_widget *)widget);
    if (owner == 0) {
        return 0;
    }

    return owner->focus_owner == widget;
}

int picoui_widget_mark_edit_result(struct picoui_widget *widget, enum picoui_edit_result result)
{
    if (widget == 0) {
        return -1;
    }

    if (result != PICOUI_EDIT_RESULT_COMMIT && result != PICOUI_EDIT_RESULT_CANCEL) {
        return -1;
    }

    widget->pending_edit_result = result;
    return 0;
}

int picoui_widget_claim_editing(struct picoui_widget *widget)
{
    struct picoui_app *owner;

    if (widget == 0 || widget->visible == 0 || widget->enabled == 0) {
        return -1;
    }

    owner = picoui_widget_get_owner_app(widget);
    if (owner == 0) {
        return -1;
    }

    widget->pending_edit_result = PICOUI_EDIT_RESULT_NONE;
    owner->editing_owner = widget;
    return 0;
}

int picoui_widget_release_editing(struct picoui_widget *widget)
{
    struct picoui_app *owner;

    if (widget == 0) {
        return -1;
    }

    owner = picoui_widget_get_owner_app(widget);
    if (owner == 0) {
        return -1;
    }

    if (owner->editing_owner == widget) {
        owner->editing_owner = 0;
    }
    widget->last_edit_result = widget->pending_edit_result;
    widget->pending_edit_result = PICOUI_EDIT_RESULT_NONE;
    return 0;
}

int picoui_widget_is_editing_owner(const struct picoui_widget *widget)
{
    struct picoui_app *owner;

    if (widget == 0) {
        return 0;
    }

    owner = picoui_widget_get_owner_app((struct picoui_widget *)widget);
    if (owner == 0) {
        return 0;
    }

    return owner->editing_owner == widget;
}

int picoui_event_stub(void)
{
    return 0;
}
