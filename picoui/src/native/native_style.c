#include "picoui/theme.h"
#include "picoui/widget.h"

#define PICOUI_NATIVE_STYLE_MAX_WIDGETS 64

struct picoui_native_style_entry {
    const struct picoui_widget *widget;
    unsigned int bg_color[PICOUI_PART_TRACK + 1][PICOUI_STATE_FOCUSED + 1];
    unsigned int text_color[PICOUI_PART_TRACK + 1][PICOUI_STATE_FOCUSED + 1];
    unsigned int border_color[PICOUI_PART_TRACK + 1][PICOUI_STATE_FOCUSED + 1];
    int has_style[PICOUI_PART_TRACK + 1][PICOUI_STATE_FOCUSED + 1];
    int radius;
    int has_radius;
    int padding;
    int has_padding;
};

static struct picoui_native_style_entry g_picoui_native_style_entries[PICOUI_NATIVE_STYLE_MAX_WIDGETS];
static int g_picoui_native_style_entry_count;

static struct picoui_native_style_entry *picoui_native_style_find_entry(const struct picoui_widget *widget)
{
    int i;

    if (widget == 0) {
        return 0;
    }

    for (i = 0; i < g_picoui_native_style_entry_count; ++i) {
        if (g_picoui_native_style_entries[i].widget == widget) {
            return &g_picoui_native_style_entries[i];
        }
    }

    return 0;
}

static struct picoui_native_style_entry *picoui_native_style_alloc_entry(const struct picoui_widget *widget)
{
    struct picoui_native_style_entry *entry;

    entry = picoui_native_style_find_entry(widget);
    if (entry != 0) {
        return entry;
    }

    if (widget == 0 || g_picoui_native_style_entry_count >= PICOUI_NATIVE_STYLE_MAX_WIDGETS) {
        return 0;
    }

    entry = &g_picoui_native_style_entries[g_picoui_native_style_entry_count++];
    *entry = (struct picoui_native_style_entry){0};
    entry->widget = widget;
    return entry;
}

void picoui_native_style_reset(void)
{
    g_picoui_native_style_entry_count = 0;
}

void picoui_native_style_set_scalar(const struct picoui_widget *widget, int radius, int padding)
{
    struct picoui_native_style_entry *entry;

    entry = picoui_native_style_alloc_entry(widget);
    if (entry == 0) {
        return;
    }

    if (radius >= 0) {
        entry->radius = radius;
        entry->has_radius = 1;
    }
    if (padding >= 0) {
        entry->padding = padding;
        entry->has_padding = 1;
    }
}

void picoui_native_style_set_colors(const struct picoui_widget *widget,
                                    enum picoui_part part,
                                    enum picoui_state state,
                                    unsigned int bg_color,
                                    unsigned int text_color,
                                    unsigned int border_color)
{
    struct picoui_native_style_entry *entry;

    if (part < PICOUI_PART_MAIN || part > PICOUI_PART_TRACK
        || state < PICOUI_STATE_DEFAULT || state > PICOUI_STATE_FOCUSED) {
        return;
    }

    entry = picoui_native_style_alloc_entry(widget);
    if (entry == 0) {
        return;
    }

    entry->bg_color[part][state] = bg_color;
    entry->text_color[part][state] = text_color;
    entry->border_color[part][state] = border_color;
    entry->has_style[part][state] = 1;
}

int picoui_native_style_get_bg_color(const struct picoui_widget *widget,
                                     enum picoui_part part,
                                     enum picoui_state state,
                                     unsigned int *rgb)
{
    struct picoui_native_style_entry *entry;

    if (rgb == 0) {
        return -1;
    }

    entry = picoui_native_style_find_entry(widget);
    if (entry == 0 || part < PICOUI_PART_MAIN || part > PICOUI_PART_TRACK
        || state < PICOUI_STATE_DEFAULT || state > PICOUI_STATE_FOCUSED
        || entry->has_style[part][state] == 0) {
        return -1;
    }

    *rgb = entry->bg_color[part][state];
    return 0;
}

int picoui_native_style_get_text_color(const struct picoui_widget *widget,
                                       enum picoui_part part,
                                       enum picoui_state state,
                                       unsigned int *rgb)
{
    struct picoui_native_style_entry *entry;

    if (rgb == 0) {
        return -1;
    }

    entry = picoui_native_style_find_entry(widget);
    if (entry == 0 || part < PICOUI_PART_MAIN || part > PICOUI_PART_TRACK
        || state < PICOUI_STATE_DEFAULT || state > PICOUI_STATE_FOCUSED
        || entry->has_style[part][state] == 0) {
        return -1;
    }

    *rgb = entry->text_color[part][state];
    return 0;
}

int picoui_native_style_get_border_color(const struct picoui_widget *widget,
                                         enum picoui_part part,
                                         enum picoui_state state,
                                         unsigned int *rgb)
{
    struct picoui_native_style_entry *entry;

    if (rgb == 0) {
        return -1;
    }

    entry = picoui_native_style_find_entry(widget);
    if (entry == 0 || part < PICOUI_PART_MAIN || part > PICOUI_PART_TRACK
        || state < PICOUI_STATE_DEFAULT || state > PICOUI_STATE_FOCUSED
        || entry->has_style[part][state] == 0) {
        return -1;
    }

    *rgb = entry->border_color[part][state];
    return 0;
}

int picoui_native_style_get_radius(const struct picoui_widget *widget, int *radius)
{
    struct picoui_native_style_entry *entry;

    if (radius == 0) {
        return -1;
    }

    entry = picoui_native_style_find_entry(widget);
    if (entry == 0 || entry->has_radius == 0) {
        return -1;
    }

    *radius = entry->radius;
    return 0;
}

int picoui_native_style_get_padding(const struct picoui_widget *widget, int *padding)
{
    struct picoui_native_style_entry *entry;

    if (padding == 0) {
        return -1;
    }

    entry = picoui_native_style_find_entry(widget);
    if (entry == 0 || entry->has_padding == 0) {
        return -1;
    }

    *padding = entry->padding;
    return 0;
}
