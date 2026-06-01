#include "backend.h"
#include "internal.h"
#include "ldScrollSelecter.h"

#include <stdlib.h>

extern const arm_2d_a1_font_t ARM_2D_FONT_6x8;

static ldColor picoui_backend_scroll_selecter_rgb_to_ld_color(unsigned int rgb)
{
    unsigned int red = (rgb >> 16) & 0xFFU;
    unsigned int green = (rgb >> 8) & 0xFFU;
    unsigned int blue = rgb & 0xFFU;

    return (ldColor)(((red >> 3) << 11) | ((green >> 2) << 5) | (blue >> 3));
}

static struct picoui_backend_app_state *picoui_backend_scroll_selecter_get_app_state(void *parent)
{
    struct picoui_backend_widget *parent_widget = parent;

    if (parent_widget == NULL || parent_widget->owner == NULL || parent_widget->owner->backend_app == NULL) {
        return NULL;
    }
    return (struct picoui_backend_app_state *)parent_widget->owner->backend_app;
}

static ldScrollSelecter_t *picoui_backend_scroll_selecter_get_ld(void *backend_widget)
{
    struct picoui_backend_widget *widget = backend_widget;

    if (widget == NULL || widget->ld_widget == NULL) {
        return NULL;
    }

    return (ldScrollSelecter_t *)widget->ld_widget;
}

void *picoui_backend_create_scroll_selecter(void *parent, const char *id)
{
    struct picoui_backend_widget *widget;
    struct picoui_backend_widget *parent_widget = parent;
    struct picoui_backend_app_state *app_state;
    ldScrollSelecter_t *ld_scroll_selecter;
    uint16_t name_id;

    if (parent == 0 || id == 0) {
        return 0;
    }

    app_state = picoui_backend_scroll_selecter_get_app_state(parent);
    if (app_state == NULL || app_state->ld_scene == NULL || parent_widget->ld_widget == NULL) {
        return 0;
    }

    widget = calloc(1, sizeof(*widget));
    if (widget == 0) {
        return 0;
    }

    name_id = ++app_state->next_ld_name_id;
    ld_scroll_selecter = ldScrollSelecter_init(app_state->ld_scene,
                                               NULL,
                                               name_id,
                                               parent_widget->ld_name_id,
                                               0,
                                               0,
                                               180,
                                               72,
                                               (arm_2d_font_t *)&ARM_2D_FONT_6x8);
    if (ld_scroll_selecter == NULL) {
        free(widget);
        return 0;
    }

    widget->parent = parent;
    widget->id = id;
    widget->kind = PICOUI_BACKEND_WIDGET_SCROLL_SELECTER;
    widget->theme = parent_widget->theme;
    widget->ld_widget = ld_scroll_selecter;
    widget->ld_name_id = name_id;
    widget->value = -1;
    widget->last_signal = PICOUI_BACKEND_SIGNAL_NONE;
    if (picoui_backend_widget_attach_child(parent, widget) != 0) {
        free(widget);
        return 0;
    }
    return widget;
}

int picoui_backend_scroll_selecter_set_items(void *backend_widget,
                                             const char *const *item_ids,
                                             const unsigned char *const *items,
                                             int item_count)
{
    struct picoui_backend_widget *widget = backend_widget;
    ldScrollSelecter_t *ld_scroll_selecter;
    int i;

    if (widget == NULL ||
        widget->kind != PICOUI_BACKEND_WIDGET_SCROLL_SELECTER ||
        widget->ld_widget == NULL ||
        item_ids == NULL ||
        items == NULL ||
        item_count < 0 ||
        item_count > PICOUI_BACKEND_LIST_MAX_ITEMS) {
        return -1;
    }

    ld_scroll_selecter = picoui_backend_scroll_selecter_get_ld(backend_widget);
    if (ld_scroll_selecter == NULL) {
        return -1;
    }

    ldScrollSelecterSetItems(ld_scroll_selecter, (const uint8_t **)items, (uint8_t)item_count);
    for (i = 0; i < item_count; ++i) {
        widget->list_item_ids[i] = item_ids[i];
    }
    widget->list_item_count = item_count;
    return 0;
}

int picoui_backend_scroll_selecter_set_text_color(void *backend_widget, unsigned int rgb)
{
    ldScrollSelecter_t *ld_scroll_selecter = picoui_backend_scroll_selecter_get_ld(backend_widget);

    if (ld_scroll_selecter == NULL) {
        return -1;
    }

    ldScrollSelecterSetTextColor(ld_scroll_selecter, picoui_backend_scroll_selecter_rgb_to_ld_color(rgb));
    return 0;
}

int picoui_backend_scroll_selecter_set_bg_color(void *backend_widget, unsigned int rgb)
{
    ldScrollSelecter_t *ld_scroll_selecter = picoui_backend_scroll_selecter_get_ld(backend_widget);

    if (ld_scroll_selecter == NULL) {
        return -1;
    }

    ldScrollSelecterSetBackgroundColor(ld_scroll_selecter, picoui_backend_scroll_selecter_rgb_to_ld_color(rgb));
    return 0;
}

int picoui_backend_scroll_selecter_set_indicator_color(void *backend_widget, unsigned int rgb)
{
    ldScrollSelecter_t *ld_scroll_selecter = picoui_backend_scroll_selecter_get_ld(backend_widget);

    if (ld_scroll_selecter == NULL) {
        return -1;
    }

    ldScrollSelecterSetIndicatorColor(ld_scroll_selecter, picoui_backend_scroll_selecter_rgb_to_ld_color(rgb));
    return 0;
}

int picoui_backend_scroll_selecter_set_bg_source(void *backend_widget,
                                                 struct picoui_image_source *source)
{
    ldScrollSelecter_t *ld_scroll_selecter = picoui_backend_scroll_selecter_get_ld(backend_widget);

    if (ld_scroll_selecter == NULL || source == NULL || source->img_tile == NULL) {
        return -1;
    }

    ldScrollSelecterSetBackgroundImage(ld_scroll_selecter, source->img_tile, source->mask_tile);
    return 0;
}

int picoui_backend_scroll_selecter_set_indicator_source(void *backend_widget,
                                                        struct picoui_image_source *source)
{
    ldScrollSelecter_t *ld_scroll_selecter = picoui_backend_scroll_selecter_get_ld(backend_widget);

    if (ld_scroll_selecter == NULL || source == NULL || source->img_tile == NULL) {
        return -1;
    }

    ldScrollSelecterSetIndicatorImage(ld_scroll_selecter, source->img_tile, source->mask_tile);
    return 0;
}

int picoui_backend_scroll_selecter_set_transparent(void *backend_widget, int transparent)
{
    ldScrollSelecter_t *ld_scroll_selecter = picoui_backend_scroll_selecter_get_ld(backend_widget);

    if (ld_scroll_selecter == NULL) {
        return -1;
    }

    ldScrollSelecterSetTransparent(ld_scroll_selecter, transparent != 0);
    return 0;
}

int picoui_backend_scroll_selecter_set_speed(void *backend_widget, int speed)
{
    ldScrollSelecter_t *ld_scroll_selecter = picoui_backend_scroll_selecter_get_ld(backend_widget);

    if (ld_scroll_selecter == NULL || speed <= 0) {
        return -1;
    }

    ldScrollSelecterSetSpeed(ld_scroll_selecter, (uint8_t)speed);
    return 0;
}

int picoui_backend_scroll_selecter_set_select_text(void *backend_widget, const char *text)
{
    ldScrollSelecter_t *ld_scroll_selecter = picoui_backend_scroll_selecter_get_ld(backend_widget);

    if (ld_scroll_selecter == NULL || text == NULL) {
        return -1;
    }

    ldScrollSelecterSetSelectText(ld_scroll_selecter, (uint8_t *)text);
    return 0;
}

int picoui_backend_scroll_selecter_set_selected_index(void *backend_widget, int index)
{
    struct picoui_backend_widget *widget = backend_widget;
    ldScrollSelecter_t *ld_scroll_selecter;

    if (widget == NULL ||
        widget->kind != PICOUI_BACKEND_WIDGET_SCROLL_SELECTER ||
        widget->ld_widget == NULL ||
        index < 0 ||
        index >= widget->list_item_count) {
        return -1;
    }

    ld_scroll_selecter = picoui_backend_scroll_selecter_get_ld(backend_widget);
    if (ld_scroll_selecter == NULL) {
        return -1;
    }

    ldScrollSelecterSetSelectItemNum(ld_scroll_selecter, (int8_t)index);
    widget->value = index;
    return 0;
}

int picoui_backend_scroll_selecter_get_selected_index(void *backend_widget)
{
    ldScrollSelecter_t *ld_scroll_selecter = picoui_backend_scroll_selecter_get_ld(backend_widget);

    if (ld_scroll_selecter == NULL || ld_scroll_selecter->itemCount == 0) {
        return -1;
    }

    return (int)ldScrollSelecterGetSelectItemNum(ld_scroll_selecter);
}

const char *picoui_backend_scroll_selecter_get_selected_text(void *backend_widget)
{
    ldScrollSelecter_t *ld_scroll_selecter = picoui_backend_scroll_selecter_get_ld(backend_widget);

    if (ld_scroll_selecter == NULL) {
        return NULL;
    }

    return (const char *)ldScrollSelecterGetSelectText(ld_scroll_selecter);
}

int picoui_backend_scroll_selecter_sync_selected_index(struct picoui_scroll_selecter *scroll_selecter,
                                                       int *selected_index_out)
{
    struct picoui_backend_widget *backend;
    int selected_index;

    if (scroll_selecter == NULL || scroll_selecter->widget.backend_widget == NULL) {
        return -1;
    }

    backend = (struct picoui_backend_widget *)scroll_selecter->widget.backend_widget;
    selected_index = picoui_backend_scroll_selecter_get_selected_index(backend);
    if (selected_index < 0 || selected_index >= scroll_selecter->item_count) {
        return -1;
    }

    scroll_selecter->selected_index = selected_index;
    backend->value = selected_index;
    if (selected_index_out != NULL) {
        *selected_index_out = selected_index;
    }
    return 0;
}

int picoui_backend_scroll_selecter_set_edit_mode(void *backend_widget, int is_edit)
{
    ldScrollSelecter_t *ld_scroll_selecter = picoui_backend_scroll_selecter_get_ld(backend_widget);

    if (ld_scroll_selecter == NULL) {
        return -1;
    }

    ldScrollSelecterSetEditMode(ld_scroll_selecter, is_edit != 0);
    return 0;
}

int picoui_backend_scroll_selecter_get_edit_mode(void *backend_widget, int *is_edit)
{
    ldScrollSelecter_t *ld_scroll_selecter = picoui_backend_scroll_selecter_get_ld(backend_widget);

    if (ld_scroll_selecter == NULL || is_edit == NULL) {
        return -1;
    }

    *is_edit = ld_scroll_selecter->isEdit ? 1 : 0;
    return 0;
}
