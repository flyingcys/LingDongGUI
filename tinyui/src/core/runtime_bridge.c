#include "internal.h"
#include "runtime_bridge.h"
#include "../../../src/porting/ldConfig.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldButton.h"
#include "../../../src/gui/ldCheckBox.h"
#include "../../../src/gui/ldList.h"
#include "../../../src/gui/ldSlider.h"
#include "../../../src/gui/ldSwitch.h"
#include "../../../src/gui/ldGui.h"
#include "../../../src/misc/ldMsg.h"
#include <stdlib.h>

void ldBaseNodeRemove(arm_2d_control_node_t *ptNode);

static bool tinyui_runtime_bridge_ld_event_bridge_slot(struct ld_scene_t *scene, ldMsg_t msg)
{
    struct tinyui_widget *widget = NULL;

    if (msg.ptSender == NULL) {
        return false;
    }

    widget = tinyui_widget_from_ld_scene(scene, msg.ptSender);
    if (widget == NULL) {
        return false;
    }

    tinyui_widget_dispatch_native_signal(widget, msg.signal, msg.value);
    return false;
}

static int tinyui_runtime_bridge_connect_native_events(struct tinyui_widget *widget)
{
    uint8_t primary_signal = SIGNAL_NO_OPERATION;
    uint8_t secondary_signal = SIGNAL_NO_OPERATION;
    uint8_t tertiary_signal = SIGNAL_NO_OPERATION;
    ldBase_t *sender = NULL;
    ldAssn_t *assn = NULL;

    if (widget == NULL || widget->ld_widget == NULL) {
        return -1;
    }

    sender = (ldBase_t *)widget->ld_widget;

    switch (widget->kind) {
    case TINYUI_BACKEND_WIDGET_BUTTON:
        primary_signal = SIGNAL_PRESS;
        secondary_signal = SIGNAL_RELEASE;
        tertiary_signal = SIGNAL_HOLD_DOWN;
        break;
    case TINYUI_BACKEND_WIDGET_LIST:
        primary_signal = SIGNAL_CLICKED_ITEM;
        break;
    case TINYUI_BACKEND_WIDGET_CHECKBOX:
    case TINYUI_BACKEND_WIDGET_SWITCH:
    case TINYUI_BACKEND_WIDGET_SLIDER:
        primary_signal = SIGNAL_VALUE_CHANGED;
        break;
    default:
        return 0;
    }

    assn = sender->ptAssn;
    while (assn != NULL) {
        if (assn->signal == primary_signal && assn->pFunc == tinyui_runtime_bridge_ld_event_bridge_slot) {
            primary_signal = SIGNAL_NO_OPERATION;
            break;
        }
        assn = assn->ptNext;
    }
    if (primary_signal != SIGNAL_NO_OPERATION
        && !ldMsgConnect(sender, primary_signal, tinyui_runtime_bridge_ld_event_bridge_slot)) {
        return -1;
    }
    if (secondary_signal != SIGNAL_NO_OPERATION) {
        assn = sender->ptAssn;
        while (assn != NULL) {
            if (assn->signal == secondary_signal
                && assn->pFunc == tinyui_runtime_bridge_ld_event_bridge_slot) {
                secondary_signal = SIGNAL_NO_OPERATION;
                break;
            }
            assn = assn->ptNext;
        }
        if (secondary_signal != SIGNAL_NO_OPERATION
            && !ldMsgConnect(sender, secondary_signal, tinyui_runtime_bridge_ld_event_bridge_slot)) {
            return -1;
        }
    }
    if (tertiary_signal != SIGNAL_NO_OPERATION) {
        assn = sender->ptAssn;
        while (assn != NULL) {
            if (assn->signal == tertiary_signal
                && assn->pFunc == tinyui_runtime_bridge_ld_event_bridge_slot) {
                tertiary_signal = SIGNAL_NO_OPERATION;
                break;
            }
            assn = assn->ptNext;
        }
        if (tertiary_signal != SIGNAL_NO_OPERATION
            && !ldMsgConnect(sender, tertiary_signal, tinyui_runtime_bridge_ld_event_bridge_slot)) {
            return -1;
        }
    }

    return 0;
}

int tinyui_runtime_bridge_bind_theme(struct tinyui_app *app, struct tinyui_theme *theme)
{
    if (app == 0 || theme == 0) {
        return -1;
    }

    app->theme = theme;
    return 0;
}

int tinyui_runtime_bridge_init_app(struct tinyui_app *app)
{
    if (app == NULL) {
        return -1;
    }

    if (app->ld_scene != NULL) {
        return 0;
    }

    app->ld_scene = calloc(1, sizeof(*app->ld_scene));
    if (app->ld_scene == NULL) {
        return -1;
    }

    app->next_ld_name_id = 0;
    app->ld_scene->bUserAllocated = true;
    return 0;
}

int tinyui_runtime_bridge_run_app(struct tinyui_app *app, struct tinyui_window *window)
{
    int running = 1;

    if (app == NULL || window == NULL) {
        return -1;
    }

    app->root_window = window;
    while (running) {
        int step = tinyui_runtime_bridge_step_app(app);

        if (step < 0) {
            return -1;
        }
        if (step > 0) {
            running = 0;
        }
    }

    return 0;
}

int tinyui_runtime_bridge_step_app(struct tinyui_app *app)
{
    return tinyui_runtime_host_step_app(app);
}

void tinyui_runtime_bridge_shutdown_app(struct tinyui_app *app)
{
    if (app == NULL) {
        return;
    }

    if (app->ld_scene == NULL) {
        return;
    }

    tinyui_runtime_host_shutdown_app(app);
    ldGuiDespose(app->ld_scene);
    free(app->ld_scene);
    app->ld_scene = NULL;
}

void tinyui_runtime_bridge_begin_screen_create(struct tinyui_app *app)
{
    (void)app;
}

int16_t tinyui_runtime_bridge_map_pointer_axis(int value, int window_extent, int target_extent)
{
    (void)window_extent;
    (void)target_extent;

    if (value < 0) {
        return 0;
    }
    if (value > 32767) {
        return 32767;
    }

    return (int16_t)value;
}

static int tinyui_runtime_bridge_touch_log_enabled(void)
{
    static int initialized = 0;
    static int enabled = 0;

    if (!initialized) {
        const char *env = getenv("TINYUI_TOUCH_LOG");
        enabled = (env != NULL && env[0] != '\0' && env[0] != '0') ? 1 : 0;
        initialized = 1;
    }

    return enabled;
}

int tinyui_runtime_bridge_bridge_pointer_from_port(struct tinyui_app *app,
                                                   int window_width,
                                                   int window_height)
{
    struct tinyui_display_config display = {0};
    int pointer_x = 0;
    int pointer_y = 0;
    int pointer_pressed = 0;
    int16_t mapped_x;
    int16_t mapped_y;

    if (app == NULL) {
        return -1;
    }

    if (tinyui_input_get_pointer(app, &pointer_x, &pointer_y, &pointer_pressed) != 0) {
        return -1;
    }

    if (tinyui_display_get_config(app, &display) != 0) {
        return -1;
    }

    mapped_x = tinyui_runtime_bridge_map_pointer_axis(pointer_x, window_width, display.width);
    mapped_y = tinyui_runtime_bridge_map_pointer_axis(pointer_y, window_height, display.height);
    if (tinyui_runtime_bridge_touch_log_enabled()) {
        printf("[TINYUI_TOUCH][PORT->LD] raw=(%d,%d) window=(%d,%d) mapped=(%d,%d) pressed=%d\n",
               pointer_x,
               pointer_y,
               window_width,
               window_height,
               mapped_x,
               mapped_y,
               pointer_pressed ? 1 : 0);
        fflush(stdout);
    }
    ldCfgTouchSetPoint(mapped_x, mapped_y, pointer_pressed != 0);
    return 0;
}

int tinyui_runtime_bridge_commit_pointer_event(struct tinyui_app *app,
                                               int window_width,
                                               int window_height,
                                               int x,
                                               int y,
                                               int pressed)
{
    if (app == NULL) {
        return -1;
    }

    if (tinyui_input_push_pointer(app, x, y, pressed) != 0) {
        return -1;
    }

    return tinyui_runtime_bridge_bridge_pointer_from_port(app, window_width, window_height);
}

int tinyui_runtime_bridge_bind_ld_event_bridge(struct tinyui_widget *widget,
                                               struct ld_scene_t *scene,
                                               void *sender)
{
    if (widget == 0 || scene == 0 || sender == 0) {
        return -1;
    }

    if (tinyui_runtime_bridge_connect_native_events(widget) != 0) {
        return -1;
    }

    widget->ld_event_bridge_scene = scene;
    widget->ld_event_bridge_sender = sender;
    return 0;
}

int tinyui_runtime_bridge_unbind_host(struct tinyui_widget *widget)
{
    if (widget == 0) {
        return -1;
    }

    widget->owner = 0;
    widget->ld_event_bridge_scene = 0;
    widget->ld_event_bridge_sender = 0;
    widget->ld_event_bridge_next = 0;
    return 0;
}

int tinyui_runtime_bridge_detach_from_parent(struct tinyui_widget *widget)
{
    if (widget == 0) {
        return -1;
    }

    if (widget->ld_widget != 0) {
        ldBaseNodeRemove((arm_2d_control_node_t *)widget->ld_widget);
        widget->ld_widget = 0;
    }

    return 0;
}

int tinyui_runtime_bridge_has_scene(const struct tinyui_app *app)
{
    return app != 0 && app->ld_scene != 0;
}

int tinyui_runtime_bridge_window_is_owned_by(const struct tinyui_app *app,
                                             const struct tinyui_window *window)
{
    if (app == 0 || window == 0) {
        return 0;
    }

    /* C1: owner is now a folded field in struct tinyui_widget */
    return window->widget.owner == app;
}

int tinyui_runtime_bridge_bind_leaf_widget(struct tinyui_widget *widget,
                                            struct tinyui_app *app)
{
    ldBase_t *sender;
    uint8_t primary_signal   = SIGNAL_NO_OPERATION;
    uint8_t secondary_signal = SIGNAL_NO_OPERATION;
    uint8_t tertiary_signal  = SIGNAL_NO_OPERATION;

    if (widget == 0 || app == 0 || widget->ld_widget == 0 || app->ld_scene == 0) {
        return -1;
    }

    sender = (ldBase_t *)widget->ld_widget;

    widget->ld_event_bridge_scene  = app->ld_scene;
    widget->ld_event_bridge_sender = widget->ld_widget;

    switch (widget->kind) {
    case TINYUI_BACKEND_WIDGET_BUTTON:
        primary_signal   = SIGNAL_PRESS;
        secondary_signal = SIGNAL_RELEASE;
        tertiary_signal  = SIGNAL_HOLD_DOWN;
        break;
    case TINYUI_BACKEND_WIDGET_LIST:
        primary_signal = SIGNAL_CLICKED_ITEM;
        break;
    case TINYUI_BACKEND_WIDGET_CHECKBOX:
    case TINYUI_BACKEND_WIDGET_SWITCH:
    case TINYUI_BACKEND_WIDGET_SLIDER:
        primary_signal = SIGNAL_VALUE_CHANGED;
        break;
    default:
        break;
    }

    if (primary_signal != SIGNAL_NO_OPERATION
        && !ldMsgConnect(sender, primary_signal, tinyui_runtime_bridge_ld_event_bridge_slot)) {
        return -1;
    }
    if (secondary_signal != SIGNAL_NO_OPERATION
        && !ldMsgConnect(sender, secondary_signal, tinyui_runtime_bridge_ld_event_bridge_slot)) {
        return -1;
    }
    if (tertiary_signal != SIGNAL_NO_OPERATION
        && !ldMsgConnect(sender, tertiary_signal, tinyui_runtime_bridge_ld_event_bridge_slot)) {
        return -1;
    }

    return 0;
}

void tinyui_runtime_bridge_reset_window_switch(struct tinyui_app *app)
{
    (void)app;  /* last_window_switch_* fields removed in Phase 0 */
}

void tinyui_runtime_bridge_set_window_switch(struct tinyui_app *app,
                                             int mode,
                                             unsigned int duration_ms)
{
    (void)app; (void)mode; (void)duration_ms;  /* last_window_switch_* fields removed in Phase 0 */
}
