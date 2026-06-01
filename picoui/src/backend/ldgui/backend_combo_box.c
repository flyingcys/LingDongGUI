#include "backend.h"
#include "internal.h"
#include "ldBase.h"
#include "ldComboBox.h"

#include <stdlib.h>

extern const arm_2d_a1_font_t ARM_2D_FONT_6x8;

static ldColor picoui_backend_combo_box_rgb_to_ld_color(unsigned int rgb)
{
    unsigned int red = (rgb >> 16) & 0xFFU;
    unsigned int green = (rgb >> 8) & 0xFFU;
    unsigned int blue = rgb & 0xFFU;

    return (ldColor)(((red >> 3) << 11) | ((green >> 2) << 5) | (blue >> 3));
}

static struct picoui_backend_app_state *picoui_backend_combo_box_get_app_state(void *parent)
{
    struct picoui_backend_widget *parent_widget = parent;

    if (parent_widget == NULL || parent_widget->owner == NULL || parent_widget->owner->backend_app == NULL) {
        return NULL;
    }
    return (struct picoui_backend_app_state *)parent_widget->owner->backend_app;
}

static ldComboBox_t *picoui_backend_combo_box_get_ld(void *backend_widget)
{
    struct picoui_backend_widget *widget = backend_widget;

    if (widget == NULL || widget->ld_widget == NULL) {
        return NULL;
    }

    return (ldComboBox_t *)widget->ld_widget;
}

static bool picoui_backend_combo_box_native_slot(struct ld_scene_t *scene, ldMsg_t msg)
{
    struct picoui_backend_widget *backend;
    struct picoui_combo_box *combo_box;
    ldComboBox_t *ld_combo_box;
    int selected_index;
    int previous_selected_index;

    (void)scene;

    if (msg.ptSender == NULL) {
        return false;
    }

    backend = (struct picoui_backend_widget *)((ldBase_t *)msg.ptSender)->pInfo;
    if (backend == NULL || backend->host_widget == NULL) {
        return false;
    }

    combo_box = (struct picoui_combo_box *)backend->host_widget;
    ld_combo_box = picoui_backend_combo_box_get_ld(backend);
    if (ld_combo_box == NULL) {
        return false;
    }

    if (msg.signal == SIGNAL_PRESS) {
        backend->open = ld_combo_box->isExpand ? 1 : 0;
        return false;
    }

    if (msg.signal != SIGNAL_CLICKED_ITEM) {
        return false;
    }

    selected_index = (int)msg.value;
    if (selected_index < 0 || selected_index >= combo_box->item_count) {
        return false;
    }

    previous_selected_index = combo_box->selected_index;
    if (combo_box->widget.visible == 0 || combo_box->widget.enabled == 0) {
        if (previous_selected_index >= 0 && previous_selected_index < combo_box->item_count) {
            ldComboBoxSetSelectItem(ld_combo_box, (uint8_t)previous_selected_index);
        }
        backend->open = ld_combo_box->isExpand ? 1 : 0;
        return false;
    }

    ldComboBoxSetSelectItem(ld_combo_box, (uint8_t)selected_index);
    combo_box->selected_index = selected_index;
    backend->value = selected_index;
    backend->open = ld_combo_box->isExpand ? 1 : 0;
    backend->last_signal = PICOUI_BACKEND_SIGNAL_VALUE_CHANGED;
    backend->dispatch_count += 1;
    (void)picoui_backend_widget_claim_focus(backend);
    if (combo_box->cb != 0) {
        combo_box->cb(combo_box, selected_index, combo_box->user_data);
    }
    return false;
}

void *picoui_backend_create_combo_box(void *parent, const char *id)
{
    struct picoui_backend_widget *widget;
    struct picoui_backend_widget *parent_widget = parent;
    struct picoui_backend_app_state *app_state;
    ldComboBox_t *ld_combo_box;
    uint16_t name_id;

    if (parent == 0 || id == 0) {
        return 0;
    }

    app_state = picoui_backend_combo_box_get_app_state(parent);
    if (app_state == NULL || app_state->ld_scene == NULL || parent_widget->ld_widget == NULL) {
        return 0;
    }

    widget = calloc(1, sizeof(*widget));
    if (widget == 0) {
        return 0;
    }

    name_id = ++app_state->next_ld_name_id;
    ld_combo_box = ldComboBox_init(app_state->ld_scene,
                                   NULL,
                                   name_id,
                                   parent_widget->ld_name_id,
                                   0,
                                   0,
                                   220,
                                   32,
                                   (arm_2d_font_t *)&ARM_2D_FONT_6x8);
    if (ld_combo_box == NULL) {
        free(widget);
        return 0;
    }

    widget->parent = parent;
    widget->id = id;
    widget->kind = PICOUI_BACKEND_WIDGET_COMBO_BOX;
    widget->theme = parent_widget->theme;
    widget->ld_widget = ld_combo_box;
    widget->ld_name_id = name_id;
    widget->value = -1;
    widget->last_signal = PICOUI_BACKEND_SIGNAL_NONE;
    if (picoui_backend_widget_attach_child(parent, widget) != 0) {
        free(widget);
        return 0;
    }
    return widget;
}

int picoui_backend_combo_box_set_items(void *backend_widget,
                                       const char *const *item_ids,
                                       const unsigned char *const *items,
                                       int item_count)
{
    struct picoui_backend_widget *widget = backend_widget;
    ldComboBox_t *ld_combo_box;
    int i;

    if (widget == NULL ||
        widget->kind != PICOUI_BACKEND_WIDGET_COMBO_BOX ||
        widget->ld_widget == NULL ||
        item_ids == NULL ||
        items == NULL ||
        item_count < 0 ||
        item_count > PICOUI_BACKEND_LIST_MAX_ITEMS) {
        return -1;
    }

    ld_combo_box = picoui_backend_combo_box_get_ld(backend_widget);
    if (ld_combo_box == NULL) {
        return -1;
    }

    ldComboBoxSetStaticItems(ld_combo_box, (uint8_t **)items, (uint8_t)item_count);
    for (i = 0; i < item_count; ++i) {
        widget->list_item_ids[i] = item_ids[i];
    }
    widget->list_item_count = item_count;
    return 0;
}

int picoui_backend_combo_box_set_text_color(void *backend_widget, unsigned int rgb)
{
    ldComboBox_t *ld_combo_box = picoui_backend_combo_box_get_ld(backend_widget);

    if (ld_combo_box == NULL) {
        return -1;
    }

    ldComboBoxSetTextColor(ld_combo_box, picoui_backend_combo_box_rgb_to_ld_color(rgb));
    return 0;
}

int picoui_backend_combo_box_set_bg_color(void *backend_widget, unsigned int rgb)
{
    ldComboBox_t *ld_combo_box = picoui_backend_combo_box_get_ld(backend_widget);

    if (ld_combo_box == NULL) {
        return -1;
    }

    ldComboBoxSetBackgroundColor(ld_combo_box, picoui_backend_combo_box_rgb_to_ld_color(rgb));
    return 0;
}

int picoui_backend_combo_box_set_frame_color(void *backend_widget, unsigned int rgb)
{
    ldComboBox_t *ld_combo_box = picoui_backend_combo_box_get_ld(backend_widget);

    if (ld_combo_box == NULL) {
        return -1;
    }

    ldComboBoxSetFrameColor(ld_combo_box, picoui_backend_combo_box_rgb_to_ld_color(rgb));
    return 0;
}

int picoui_backend_combo_box_set_select_color(void *backend_widget, unsigned int rgb)
{
    ldComboBox_t *ld_combo_box = picoui_backend_combo_box_get_ld(backend_widget);

    if (ld_combo_box == NULL) {
        return -1;
    }

    ldComboBoxSetSelectColor(ld_combo_box, picoui_backend_combo_box_rgb_to_ld_color(rgb));
    return 0;
}

int picoui_backend_combo_box_set_item_max(void *backend_widget, int item_max)
{
    struct picoui_backend_widget *widget = backend_widget;
    ldComboBox_t *ld_combo_box;

    if (widget == NULL ||
        widget->kind != PICOUI_BACKEND_WIDGET_COMBO_BOX ||
        widget->ld_widget == NULL ||
        item_max <= 0 ||
        item_max > PICOUI_BACKEND_LIST_MAX_ITEMS ||
        item_max < widget->list_item_count) {
        return -1;
    }

    ld_combo_box = picoui_backend_combo_box_get_ld(backend_widget);
    if (ld_combo_box == NULL) {
        return -1;
    }

    ldComboBoxSetItemMax(ld_combo_box, (uint8_t)item_max);
    return 0;
}

int picoui_backend_combo_box_set_dropdown_source(void *backend_widget,
                                                 struct picoui_image_source *source)
{
    ldComboBox_t *ld_combo_box = picoui_backend_combo_box_get_ld(backend_widget);

    if (ld_combo_box == NULL || source == NULL || source->img_tile == NULL) {
        return -1;
    }

    ldComboBoxSetDropdownImage(ld_combo_box, source->img_tile, source->mask_tile);
    return 0;
}

int picoui_backend_combo_box_set_selected_index(void *backend_widget, int index)
{
    struct picoui_backend_widget *widget = backend_widget;
    ldComboBox_t *ld_combo_box;

    if (widget == NULL ||
        widget->kind != PICOUI_BACKEND_WIDGET_COMBO_BOX ||
        widget->ld_widget == NULL ||
        index < 0 ||
        index >= widget->list_item_count) {
        return -1;
    }

    ld_combo_box = picoui_backend_combo_box_get_ld(backend_widget);
    if (ld_combo_box == NULL) {
        return -1;
    }

    ldComboBoxSetSelectItem(ld_combo_box, (uint8_t)index);
    widget->value = index;
    return 0;
}

int picoui_backend_combo_box_get_selected_index(void *backend_widget)
{
    ldComboBox_t *ld_combo_box = picoui_backend_combo_box_get_ld(backend_widget);

    if (ld_combo_box == NULL) {
        return -1;
    }

    if (ld_combo_box->itemCount == 0) {
        return -1;
    }
    return (int)ldComboBoxGetSelectItem(ld_combo_box);
}

const char *picoui_backend_combo_box_get_text(void *backend_widget, int index)
{
    ldComboBox_t *ld_combo_box = picoui_backend_combo_box_get_ld(backend_widget);

    if (ld_combo_box == NULL || index < 0) {
        return NULL;
    }

    return (const char *)ldComboBoxGetText(ld_combo_box, (uint8_t)index);
}

int picoui_backend_combo_box_sync_selected_index(struct picoui_combo_box *combo_box,
                                                 int *selected_index_out)
{
    struct picoui_backend_widget *backend;
    int selected_index;

    if (combo_box == NULL || combo_box->widget.backend_widget == NULL) {
        return -1;
    }

    backend = (struct picoui_backend_widget *)combo_box->widget.backend_widget;
    selected_index = picoui_backend_combo_box_get_selected_index(backend);
    if (selected_index < 0 || selected_index >= combo_box->item_count) {
        return -1;
    }

    combo_box->selected_index = selected_index;
    backend->value = selected_index;
    if (selected_index_out != NULL) {
        *selected_index_out = selected_index;
    }
    return 0;
}

int picoui_backend_combo_box_bind_host(void *backend_widget)
{
    struct picoui_backend_widget *backend = backend_widget;
    ldComboBox_t *ld_combo_box;

    if (backend == NULL) {
        return -1;
    }

    ld_combo_box = picoui_backend_combo_box_get_ld(backend_widget);
    if (ld_combo_box == NULL) {
        return -1;
    }

    if (!ldMsgConnect(ld_combo_box, SIGNAL_PRESS, picoui_backend_combo_box_native_slot)) {
        return -1;
    }
    if (!ldMsgConnect(ld_combo_box, SIGNAL_CLICKED_ITEM, picoui_backend_combo_box_native_slot)) {
        return -1;
    }
    return 0;
}

int picoui_backend_combo_box_get_open(void *backend_widget, int *is_open)
{
    ldComboBox_t *ld_combo_box = picoui_backend_combo_box_get_ld(backend_widget);

    if (ld_combo_box == NULL || is_open == NULL) {
        return -1;
    }

    *is_open = ld_combo_box->isExpand ? 1 : 0;
    return 0;
}
