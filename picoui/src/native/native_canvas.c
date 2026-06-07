#include "../core/internal.h"

#include <stdlib.h>

#define PICOUI_NATIVE_CANVAS_MAX 32
#define PICOUI_NATIVE_CANVAS_COMMAND_DRAW_CIRCLE 5

struct picoui_native_canvas_state {
    const struct picoui_canvas *canvas;
    int circle_mask[PICOUI_CANVAS_MAX_COMMANDS];
    int rendered_command_count;
    int rendered_command_kinds[PICOUI_CANVAS_MAX_COMMANDS];
    struct picoui_rect dirty_rect;
    int changed_pixels;
    int render_ready;
};

static struct picoui_native_canvas_state s_canvas_states[PICOUI_NATIVE_CANVAS_MAX];

int picoui_native_canvas_render_buffer(const struct picoui_canvas *canvas,
                                       unsigned int *buffer,
                                       int width,
                                       int height,
                                       struct picoui_rect *dirty_rect);

static struct picoui_native_canvas_state *picoui_native_canvas_state_get(const struct picoui_canvas *canvas,
                                                                         int create)
{
    struct picoui_native_canvas_state *free_slot = 0;
    int index;

    if (canvas == 0) {
        return 0;
    }

    for (index = 0; index < PICOUI_NATIVE_CANVAS_MAX; ++index) {
        if (s_canvas_states[index].canvas == canvas) {
            return &s_canvas_states[index];
        }
        if (free_slot == 0 && s_canvas_states[index].canvas == 0) {
            free_slot = &s_canvas_states[index];
        }
    }

    if (!create || free_slot == 0) {
        return 0;
    }

    free_slot->canvas = canvas;
    free_slot->rendered_command_count = 0;
    free_slot->dirty_rect.x = 0;
    free_slot->dirty_rect.y = 0;
    free_slot->dirty_rect.width = 0;
    free_slot->dirty_rect.height = 0;
    free_slot->changed_pixels = 0;
    free_slot->render_ready = 0;
    return free_slot;
}

static int picoui_native_canvas_sync_rendered(const struct picoui_canvas *canvas,
                                              struct picoui_native_canvas_state *state)
{
    int index;

    if (canvas == 0 || state == 0 || canvas->command_count < 0 || canvas->command_count > PICOUI_CANVAS_MAX_COMMANDS) {
        return -1;
    }

    state->rendered_command_count = canvas->command_count;
    for (index = 0; index < canvas->command_count; ++index) {
        state->rendered_command_kinds[index] = state->circle_mask[index]
            ? PICOUI_NATIVE_CANVAS_COMMAND_DRAW_CIRCLE
            : canvas->commands[index].kind;
    }
    state->render_ready = 1;
    return 0;
}

int picoui_native_canvas_reset_render_state(struct picoui_canvas *canvas)
{
    struct picoui_native_canvas_state *state = picoui_native_canvas_state_get(canvas, 1);

    if (state == 0) {
        return -1;
    }

    state->render_ready = 0;
    return 0;
}

int picoui_native_canvas_clear_shadow(struct picoui_canvas *canvas)
{
    struct picoui_native_canvas_state *state = picoui_native_canvas_state_get(canvas, 1);
    int index;

    if (state == 0) {
        return -1;
    }

    for (index = 0; index < PICOUI_CANVAS_MAX_COMMANDS; ++index) {
        state->circle_mask[index] = 0;
        state->rendered_command_kinds[index] = 0;
    }
    state->rendered_command_count = 0;
    state->dirty_rect.x = 0;
    state->dirty_rect.y = 0;
    state->dirty_rect.width = 0;
    state->dirty_rect.height = 0;
    state->changed_pixels = 0;
    state->render_ready = 0;
    return 0;
}

int picoui_native_canvas_mark_circle_command(struct picoui_canvas *canvas, int command_index)
{
    struct picoui_native_canvas_state *state = picoui_native_canvas_state_get(canvas, 1);

    if (state == 0 || command_index < 0 || command_index >= PICOUI_CANVAS_MAX_COMMANDS) {
        return -1;
    }

    state->circle_mask[command_index] = 1;
    state->render_ready = 0;
    return 0;
}

int picoui_native_canvas_render(const struct picoui_canvas *canvas)
{
    struct picoui_native_canvas_state *state;
    unsigned int *scratch;
    size_t scratch_count;
    int i;

    if (canvas == 0) {
        return -1;
    }

    state = picoui_native_canvas_state_get(canvas, 1);
    if (state == 0) {
        return -1;
    }

    state->dirty_rect.x = 0;
    state->dirty_rect.y = 0;
    state->dirty_rect.width = 0;
    state->dirty_rect.height = 0;
    state->changed_pixels = 0;

    if (canvas->widget.width > 0 && canvas->widget.height > 0) {
        scratch_count = (size_t)canvas->widget.width * (size_t)canvas->widget.height;
        scratch = calloc(scratch_count, sizeof(*scratch));
        if (scratch == 0) {
            return -1;
        }

        if (picoui_native_canvas_render_buffer(canvas,
                                               scratch,
                                               canvas->widget.width,
                                               canvas->widget.height,
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

    return picoui_native_canvas_sync_rendered(canvas, state);
}

int picoui_native_canvas_get_rendered_command_count(const struct picoui_canvas *canvas, int *count)
{
    struct picoui_native_canvas_state *state = picoui_native_canvas_state_get(canvas, 1);

    if (count == 0 || state == 0) {
        return -1;
    }

    if (state->render_ready == 0) {
        return -1;
    }

    *count = state->rendered_command_count;
    return 0;
}

int picoui_native_canvas_get_rendered_command_kind(const struct picoui_canvas *canvas, int index, int *kind)
{
    struct picoui_native_canvas_state *state = picoui_native_canvas_state_get(canvas, 1);

    if (kind == 0 || state == 0) {
        return -1;
    }

    if (state->render_ready == 0) {
        return -1;
    }

    if (index < 0 || index >= state->rendered_command_count) {
        return -1;
    }

    *kind = state->rendered_command_kinds[index];
    return 0;
}

int picoui_native_canvas_get_rendered_dirty_rect(const struct picoui_canvas *canvas,
                                                 struct picoui_rect *dirty_rect)
{
    struct picoui_native_canvas_state *state = picoui_native_canvas_state_get(canvas, 1);

    if (dirty_rect == 0 || state == 0 || state->render_ready == 0) {
        return -1;
    }

    *dirty_rect = state->dirty_rect;
    return 0;
}

int picoui_native_canvas_get_rendered_changed_pixels(const struct picoui_canvas *canvas, int *count)
{
    struct picoui_native_canvas_state *state = picoui_native_canvas_state_get(canvas, 1);

    if (count == 0 || state == 0 || state->render_ready == 0) {
        return -1;
    }

    *count = state->changed_pixels;
    return 0;
}
