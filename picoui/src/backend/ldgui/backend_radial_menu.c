#include "backend.h"
#include "internal.h"
#include "ldBase.h"
#include "ldRadialMenu.h"

#include <stdlib.h>

extern const arm_2d_tile_t c_tileQuaterArcGRAY8;
extern const arm_2d_tile_t c_tileQuaterArcMask;
extern const arm_2d_tile_t c_tilePointerSecGRAY8;
extern const arm_2d_tile_t c_tilePointerSecMask;

static arm_2d_tile_t *const g_radial_menu_tiles[] = {
    (arm_2d_tile_t *)&c_tileQuaterArcGRAY8,
    (arm_2d_tile_t *)&c_tilePointerSecGRAY8,
    (arm_2d_tile_t *)&c_tileQuaterArcGRAY8,
    (arm_2d_tile_t *)&c_tilePointerSecGRAY8,
    (arm_2d_tile_t *)&c_tileQuaterArcGRAY8,
};

static arm_2d_tile_t *const g_radial_menu_masks[] = {
    (arm_2d_tile_t *)&c_tileQuaterArcMask,
    (arm_2d_tile_t *)&c_tilePointerSecMask,
    (arm_2d_tile_t *)&c_tileQuaterArcMask,
    (arm_2d_tile_t *)&c_tilePointerSecMask,
    (arm_2d_tile_t *)&c_tileQuaterArcMask,
};

static struct picoui_backend_app_state *picoui_backend_radial_menu_get_app_state(void *parent)
{
    struct picoui_backend_widget *parent_widget = parent;

    if (parent_widget == NULL || parent_widget->owner == NULL || parent_widget->owner->backend_app == NULL) {
        return NULL;
    }
    return (struct picoui_backend_app_state *)parent_widget->owner->backend_app;
}

static ldRadialMenu_t *picoui_backend_radial_menu_get_ld(void *backend_widget)
{
    struct picoui_backend_widget *widget = backend_widget;

    if (widget == NULL || widget->ld_widget == NULL) {
        return NULL;
    }

    return (ldRadialMenu_t *)widget->ld_widget;
}

static bool picoui_backend_radial_menu_native_slot(struct ld_scene_t *scene, ldMsg_t msg)
{
    struct picoui_backend_widget *backend;
    struct picoui_radial_menu *radial_menu;
    int selected_index;
    int previous_selected_index;

    (void)scene;

    if (msg.ptSender == NULL || msg.signal != SIGNAL_CLICKED_ITEM) {
        return false;
    }

    backend = (struct picoui_backend_widget *)((ldBase_t *)msg.ptSender)->pInfo;
    if (backend == NULL || backend->host_widget == NULL) {
        return false;
    }

    radial_menu = (struct picoui_radial_menu *)backend->host_widget;
    selected_index = (int)msg.value;
    if (selected_index < 0 || selected_index >= radial_menu->item_count) {
        return false;
    }

    previous_selected_index = radial_menu->selected_index;
    if (radial_menu->widget.visible == 0 || radial_menu->widget.enabled == 0) {
        return false;
    }

    if (picoui_backend_widget_claim_focus(backend) != 0) {
        return false;
    }

    radial_menu->selected_index = selected_index;
    backend->value = selected_index;
    if (previous_selected_index == selected_index) {
        return false;
    }
    backend->data_model_epoch++;
    backend->last_data_source = PICOUI_BACKEND_DATA_SOURCE_NATIVE_EVENT;
    backend->last_signal = PICOUI_BACKEND_SIGNAL_VALUE_CHANGED;
    backend->dispatch_count++;
    if (radial_menu->cb != 0) {
        radial_menu->cb(radial_menu, selected_index, radial_menu->user_data);
    }
    return false;
}

void *picoui_backend_create_radial_menu(void *parent, const char *id)
{
    struct picoui_backend_widget *widget;
    struct picoui_backend_widget *parent_widget = parent;
    struct picoui_backend_app_state *app_state;
    ldRadialMenu_t *ld_radial_menu;
    uint16_t name_id;

    if (parent == 0 || id == 0) {
        return 0;
    }

    app_state = picoui_backend_radial_menu_get_app_state(parent);
    if (app_state == NULL || app_state->ld_scene == NULL || parent_widget->ld_widget == NULL) {
        return 0;
    }

    widget = calloc(1, sizeof(*widget));
    if (widget == 0) {
        return 0;
    }

    name_id = ++app_state->next_ld_name_id;
    ld_radial_menu = ldRadialMenu_init(app_state->ld_scene,
                                       NULL,
                                       name_id,
                                       parent_widget->ld_name_id,
                                       0,
                                       0,
                                       194,
                                       96,
                                       68,
                                       46,
                                       5);
    if (ld_radial_menu == NULL) {
        free(widget);
        return 0;
    }

    widget->parent = parent;
    widget->id = id;
    widget->kind = PICOUI_BACKEND_WIDGET_RADIAL_MENU;
    widget->theme = parent_widget->theme;
    widget->ld_widget = ld_radial_menu;
    widget->ld_name_id = name_id;
    widget->value = -1;
    widget->last_signal = PICOUI_BACKEND_SIGNAL_NONE;
    if (picoui_backend_widget_attach_child(parent, widget) != 0) {
        free(widget);
        return 0;
    }
    return widget;
}

int picoui_backend_radial_menu_add_item(void *backend_widget, const char *id)
{
    struct picoui_backend_widget *widget = backend_widget;
    ldRadialMenu_t *ld_radial_menu;
    int index;

    if (widget == NULL ||
        widget->kind != PICOUI_BACKEND_WIDGET_RADIAL_MENU ||
        widget->ld_widget == NULL ||
        id == NULL ||
        widget->list_item_count >= PICOUI_BACKEND_LIST_MAX_ITEMS) {
        return -1;
    }

    ld_radial_menu = picoui_backend_radial_menu_get_ld(backend_widget);
    if (ld_radial_menu == NULL) {
        return -1;
    }

    index = widget->list_item_count;
    ldRadialMenuAddItem(ld_radial_menu,
                        g_radial_menu_tiles[index % 5],
                        g_radial_menu_masks[index % 5]);
    widget->list_item_ids[index] = id;
    widget->list_item_count++;
    if (widget->value < 0) {
        widget->value = 0;
    }
    return 0;
}

int picoui_backend_radial_menu_set_selected_index(void *backend_widget, int index)
{
    struct picoui_backend_widget *widget = backend_widget;
    ldRadialMenu_t *ld_radial_menu;

    if (widget == NULL ||
        widget->kind != PICOUI_BACKEND_WIDGET_RADIAL_MENU ||
        widget->ld_widget == NULL ||
        index < 0 ||
        index >= widget->list_item_count) {
        return -1;
    }

    ld_radial_menu = picoui_backend_radial_menu_get_ld(backend_widget);
    if (ld_radial_menu == NULL) {
        return -1;
    }

    ldRadialMenuSetDefaultItem(ld_radial_menu, (uint8_t)index);
    widget->value = index;
    return 0;
}

int picoui_backend_radial_menu_get_selected_index(void *backend_widget)
{
    ldRadialMenu_t *ld_radial_menu = picoui_backend_radial_menu_get_ld(backend_widget);

    if (ld_radial_menu == NULL) {
        return -1;
    }

    return (int)ld_radial_menu->selectItem;
}

int picoui_backend_radial_menu_offset_selection(void *backend_widget, int offset)
{
    struct picoui_backend_widget *widget = backend_widget;
    int item_count;
    int next_index;

    if (widget == NULL ||
        widget->kind != PICOUI_BACKEND_WIDGET_RADIAL_MENU ||
        widget->ld_widget == NULL ||
        widget->list_item_count <= 0) {
        return -1;
    }

    item_count = widget->list_item_count;
    next_index = picoui_backend_radial_menu_get_selected_index(backend_widget);
    if (next_index < 0) {
        next_index = 0;
    }
    next_index = (next_index + offset) % item_count;
    if (next_index < 0) {
        next_index += item_count;
    }

    return picoui_backend_radial_menu_set_selected_index(backend_widget, next_index);
}

int picoui_backend_radial_menu_bind_host(void *backend_widget)
{
    struct picoui_backend_widget *backend = backend_widget;
    ldRadialMenu_t *ld_radial_menu;

    if (backend == NULL) {
        return -1;
    }

    ld_radial_menu = picoui_backend_radial_menu_get_ld(backend_widget);
    if (ld_radial_menu == NULL) {
        return -1;
    }

    if (!ldMsgConnect(ld_radial_menu, SIGNAL_CLICKED_ITEM, picoui_backend_radial_menu_native_slot)) {
        return -1;
    }
    return 0;
}
