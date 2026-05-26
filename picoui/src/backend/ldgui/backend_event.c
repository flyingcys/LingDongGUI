#include "backend.h"

void picoui_backend_emit_value_changed(picoui_value_changed_cb cb,
                                       struct picoui_widget *widget,
                                       int value,
                                       void *user_data)
{
    if (cb != 0) {
        cb(widget, value, user_data);
    }
}
