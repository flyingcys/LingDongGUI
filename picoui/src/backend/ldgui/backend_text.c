#include "backend.h"
#include "internal.h"
#include "ldText.h"
#include "picoui/widget.h"

#include <stdlib.h>
#include <string.h>

extern const arm_2d_a1_font_t ARM_2D_FONT_6x8;
extern const arm_2d_a1_font_t ARM_2D_FONT_16x24;

struct picoui_backend_text_box_prefix_view {
    text_box_cfg_t tCFG;
};

static int picoui_backend_text_fail_next_set_font = 0;

static struct picoui_backend_app_state *picoui_backend_text_get_app_state(void *parent)
{
    struct picoui_backend_widget *parent_widget = parent;

    if (parent_widget == NULL || parent_widget->owner == NULL || parent_widget->owner->backend_app == NULL) {
        return NULL;
    }
    return (struct picoui_backend_app_state *)parent_widget->owner->backend_app;
}

static int picoui_backend_text_apply_consumed_font(ldText_t *ld_text, arm_2d_font_t *font)
{
    if (ld_text == NULL || font == NULL) {
        return -1;
    }

    return ldTextSetConsumedFont(ld_text, font);
}

static arm_2d_font_t *picoui_backend_text_default_font(void)
{
    return (arm_2d_font_t *)&ARM_2D_FONT_6x8;
}

static arm_2d_font_t *picoui_backend_text_resolve_font(const struct picoui_font *font)
{
    if (font == NULL || font->family == NULL || font->size <= 0) {
        return picoui_backend_text_default_font();
    }

    if (strcmp(font->family, "Sans") == 0 && font->size >= 20) {
        return (arm_2d_font_t *)&ARM_2D_FONT_16x24;
    }

    return picoui_backend_text_default_font();
}

void picoui_backend_text_test_fail_next_set_font(void)
{
    picoui_backend_text_fail_next_set_font = 1;
}

int picoui_backend_text_set_font(void *backend_widget, const void *font)
{
    struct picoui_backend_widget *widget = backend_widget;
    const struct picoui_font *picoui_font = (const struct picoui_font *)font;
    ldText_t *ld_text;
    arm_2d_font_t *resolved_font;

    if (widget == NULL || widget->kind != PICOUI_BACKEND_WIDGET_TEXT || widget->ld_widget == NULL) {
        return -1;
    }

    ld_text = (ldText_t *)widget->ld_widget;

    if (picoui_backend_text_fail_next_set_font != 0) {
        picoui_backend_text_fail_next_set_font = 0;
        return -1;
    }

    resolved_font = picoui_backend_text_resolve_font(picoui_font);
    if (resolved_font == NULL) {
        return -1;
    }

    if (font != NULL && ldTextSetFont(ld_text, resolved_font) != 0) {
        return -1;
    }

    if (font == NULL && picoui_backend_text_apply_consumed_font(ld_text, resolved_font) != 0) {
        return -1;
    }

    widget->font = font;
    return 0;
}

void *picoui_backend_create_text(void *parent, const char *id)
{
    struct picoui_backend_widget *widget;
    struct picoui_backend_widget *parent_widget = parent;
    struct picoui_backend_app_state *app_state;
    ldText_t *ld_text;
    uint16_t name_id;

    if (parent == 0 || id == 0) {
        return 0;
    }

    app_state = picoui_backend_text_get_app_state(parent);
    if (app_state == NULL || app_state->ld_scene == NULL || parent_widget->ld_widget == NULL) {
        return 0;
    }

    widget = calloc(1, sizeof(*widget));
    if (widget == 0) {
        return 0;
    }

    name_id = ++app_state->next_ld_name_id;
    ld_text = ldText_init(app_state->ld_scene,
                          NULL,
                          name_id,
                          parent_widget->ld_name_id,
                          0,
                          0,
                          220,
                          48,
                          NULL,
                          TEXT_BOX_LINE_ALIGN_LEFT,
                          false);
    if (ld_text == NULL) {
        free(widget);
        return 0;
    }

    widget->parent = parent;
    widget->id = id;
    widget->kind = PICOUI_BACKEND_WIDGET_TEXT;
    widget->theme = ((struct picoui_backend_widget *)parent)->theme;
    widget->ld_widget = ld_text;
    widget->ld_name_id = name_id;
    if (picoui_backend_widget_attach_child(parent, widget) != 0) {
        free(widget);
        return 0;
    }
    return widget;
}
