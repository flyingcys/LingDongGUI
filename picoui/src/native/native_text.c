#include "../backend/ldgui/backend.h"
#include "../core/internal.h"

#include <stdlib.h>

#define PICOUI_NATIVE_TEXT_RENDER_MAX 32

struct picoui_native_text_render_state {
    const struct picoui_text *text;
    const char *value;
    const void *font;
    int wrap_width;
    struct picoui_rect dirty_rect;
    int changed_pixels;
    int rendered;
};

static struct picoui_native_text_render_state
    g_picoui_native_text_render_states[PICOUI_NATIVE_TEXT_RENDER_MAX];
static int g_picoui_native_text_fail_next_set_text;

static struct picoui_native_text_render_state *picoui_native_text_find_render_state(
    const struct picoui_text *text)
{
    int i;

    if (text == 0) {
        return 0;
    }

    for (i = 0; i < PICOUI_NATIVE_TEXT_RENDER_MAX; ++i) {
        if (g_picoui_native_text_render_states[i].text == text) {
            return &g_picoui_native_text_render_states[i];
        }
    }

    return 0;
}

static struct picoui_native_text_render_state *picoui_native_text_alloc_render_state(
    const struct picoui_text *text)
{
    struct picoui_native_text_render_state *state;
    int i;

    state = picoui_native_text_find_render_state(text);
    if (state != 0) {
        return state;
    }

    if (text == 0) {
        return 0;
    }

    for (i = 0; i < PICOUI_NATIVE_TEXT_RENDER_MAX; ++i) {
        if (g_picoui_native_text_render_states[i].text == 0) {
            g_picoui_native_text_render_states[i].text = text;
            g_picoui_native_text_render_states[i].value = 0;
            g_picoui_native_text_render_states[i].font = 0;
            g_picoui_native_text_render_states[i].wrap_width = 0;
            g_picoui_native_text_render_states[i].dirty_rect.x = 0;
            g_picoui_native_text_render_states[i].dirty_rect.y = 0;
            g_picoui_native_text_render_states[i].dirty_rect.width = 0;
            g_picoui_native_text_render_states[i].dirty_rect.height = 0;
            g_picoui_native_text_render_states[i].changed_pixels = 0;
            g_picoui_native_text_render_states[i].rendered = 0;
            return &g_picoui_native_text_render_states[i];
        }
    }

    return 0;
}

int picoui_native_text_set_text(struct picoui_text *text, const char *value)
{
    struct picoui_backend_widget *backend;

    if (text == 0 || value == 0 || text->widget.backend_widget == 0) {
        return -1;
    }

    backend = (struct picoui_backend_widget *)text->widget.backend_widget;
    if (backend->kind != PICOUI_BACKEND_WIDGET_TEXT) {
        return -1;
    }

    if (g_picoui_native_text_fail_next_set_text != 0) {
        g_picoui_native_text_fail_next_set_text = 0;
        return -1;
    }

    backend->text = value;
    backend->data_model_epoch++;
    backend->last_data_source = PICOUI_BACKEND_DATA_SOURCE_SETTER;
    return 0;
}

void picoui_native_text_test_fail_next_set_text(void)
{
    g_picoui_native_text_fail_next_set_text = 1;
}

int picoui_native_text_render(const struct picoui_backend_widget *backend)
{
    const struct picoui_text *text;
    struct picoui_native_font native_font;
    struct picoui_native_text_render_state *state;
    unsigned int *scratch = 0;
    size_t scratch_count;
    int i;

    if (backend == 0 || backend->kind != PICOUI_BACKEND_WIDGET_TEXT
        || backend->host_widget == 0) {
        return -1;
    }

    text = (const struct picoui_text *)backend->host_widget;
    state = picoui_native_text_alloc_render_state(text);
    if (state == 0) {
        return -1;
    }

    native_font = picoui_font_resolve_native(text->widget.font);
    state->value = text->widget.text;
    state->font = native_font.font;
    state->wrap_width = text->widget.width;
    state->changed_pixels = 0;
    state->dirty_rect.x = 0;
    state->dirty_rect.y = 0;
    state->dirty_rect.width = 0;
    state->dirty_rect.height = 0;

    if (state->value == 0 || state->value[0] == '\0') {
        state->rendered = 1;
        return 0;
    }

    if (state->value != 0 && text->widget.width > 0 && text->widget.height > 0) {
        scratch_count = (size_t)text->widget.width * (size_t)text->widget.height;
        scratch = calloc(scratch_count, sizeof(*scratch));
        if (scratch == 0) {
            return -1;
        }

        if (picoui_native_font_render_text(native_font,
                                           state->value,
                                           scratch,
                                           text->widget.width,
                                           text->widget.height,
                                           0x00FFFFFFU,
                                           &state->dirty_rect)
            != 0) {
            free(scratch);
            return -1;
        }

        for (i = 0; i < (int)scratch_count; ++i) {
            if (scratch[i] != 0U) {
                state->changed_pixels++;
            }
        }

        free(scratch);
    }

    state->rendered = 1;
    return 0;
}

int picoui_native_text_get_rendered_text(const struct picoui_text *text, const char **value)
{
    struct picoui_native_text_render_state *state;

    if (value == 0) {
        return -1;
    }

    state = picoui_native_text_find_render_state(text);
    if (state == 0 || state->rendered == 0) {
        return -1;
    }

    *value = state->value;
    return 0;
}

int picoui_native_text_get_rendered_wrap_width(const struct picoui_text *text, int *wrap_width)
{
    struct picoui_native_text_render_state *state;

    if (wrap_width == 0) {
        return -1;
    }

    state = picoui_native_text_find_render_state(text);
    if (state == 0 || state->rendered == 0) {
        return -1;
    }

    *wrap_width = state->wrap_width;
    return 0;
}

int picoui_native_text_get_rendered_dirty_rect(const struct picoui_text *text,
                                               struct picoui_rect *dirty_rect)
{
    struct picoui_native_text_render_state *state;

    if (dirty_rect == 0) {
        return -1;
    }

    state = picoui_native_text_find_render_state(text);
    if (state == 0 || state->rendered == 0) {
        return -1;
    }

    *dirty_rect = state->dirty_rect;
    return 0;
}

int picoui_native_text_get_rendered_changed_pixels(const struct picoui_text *text, int *count)
{
    struct picoui_native_text_render_state *state;

    if (count == 0) {
        return -1;
    }

    state = picoui_native_text_find_render_state(text);
    if (state == 0 || state->rendered == 0) {
        return -1;
    }

    *count = state->changed_pixels;
    return 0;
}
