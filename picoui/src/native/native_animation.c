#include "../core/internal.h"
#include "picoui/animation.h"

#include <stdlib.h>
#include <limits.h>
#include <time.h>

struct picoui_native_animation_render_state {
    const struct picoui_animation *animation;
    struct picoui_image_source **frame_sources;
    int frame_source_count;
    struct picoui_rect dirty_rect;
    int changed_pixels;
    int rendered;
};

static struct picoui_native_animation_render_state g_picoui_native_animation_render_states[32];

static struct picoui_backend_widget *picoui_native_animation_backend(const struct picoui_animation *animation)
{
    if (animation == 0 || animation->widget.backend_widget == 0) {
        return 0;
    }

    return (struct picoui_backend_widget *)animation->widget.backend_widget;
}

static unsigned int picoui_native_animation_now_ms(void)
{
    struct timespec now;
    unsigned long long total_ms;

    if (clock_gettime(CLOCK_MONOTONIC, &now) != 0) {
        return 0U;
    }

    total_ms = (unsigned long long)now.tv_sec * 1000ULL
             + (unsigned long long)now.tv_nsec / 1000000ULL;
    return (unsigned int)(total_ms % ((unsigned long long)UINT_MAX + 1ULL));
}

static unsigned int picoui_native_animation_encode_start_tick(unsigned int tick)
{
    return tick == UINT_MAX ? UINT_MAX : tick + 1U;
}

static unsigned int picoui_native_animation_decode_start_tick(unsigned int encoded_tick)
{
    return encoded_tick == UINT_MAX ? UINT_MAX : encoded_tick - 1U;
}

static struct picoui_native_animation_render_state *picoui_native_animation_find_render_state(
    const struct picoui_animation *animation)
{
    int i;

    if (animation == 0) {
        return 0;
    }

    for (i = 0; i < (int)(sizeof(g_picoui_native_animation_render_states)
                           / sizeof(g_picoui_native_animation_render_states[0])); ++i) {
        if (g_picoui_native_animation_render_states[i].animation == animation) {
            return &g_picoui_native_animation_render_states[i];
        }
    }

    return 0;
}

static struct picoui_native_animation_render_state *picoui_native_animation_alloc_render_state(
    const struct picoui_animation *animation)
{
    struct picoui_native_animation_render_state *state;
    int i;

    state = picoui_native_animation_find_render_state(animation);
    if (state != 0) {
        return state;
    }

    if (animation == 0) {
        return 0;
    }

    for (i = 0; i < (int)(sizeof(g_picoui_native_animation_render_states)
                           / sizeof(g_picoui_native_animation_render_states[0])); ++i) {
        if (g_picoui_native_animation_render_states[i].animation == 0) {
            g_picoui_native_animation_render_states[i].animation = animation;
            g_picoui_native_animation_render_states[i].frame_sources = 0;
            g_picoui_native_animation_render_states[i].frame_source_count = 0;
            g_picoui_native_animation_render_states[i].dirty_rect.x = 0;
            g_picoui_native_animation_render_states[i].dirty_rect.y = 0;
            g_picoui_native_animation_render_states[i].dirty_rect.width = 0;
            g_picoui_native_animation_render_states[i].dirty_rect.height = 0;
            g_picoui_native_animation_render_states[i].changed_pixels = 0;
            g_picoui_native_animation_render_states[i].rendered = 0;
            return &g_picoui_native_animation_render_states[i];
        }
    }

    return 0;
}

static void picoui_native_animation_free_frame_sources(struct picoui_native_animation_render_state *state)
{
    if (state == 0) {
        return;
    }

    free(state->frame_sources);
    state->frame_sources = 0;
    state->frame_source_count = 0;
}

static struct picoui_image_source *picoui_native_animation_current_source(
    const struct picoui_animation *animation,
    const struct picoui_native_animation_render_state *state)
{
    if (animation == 0) {
        return 0;
    }

    if (state != 0 && state->frame_sources != 0
        && animation->frame_index >= 0
        && animation->frame_index < state->frame_source_count) {
        return state->frame_sources[animation->frame_index];
    }

    return animation->source;
}

void picoui_native_animation_reset_render_state(struct picoui_animation *animation)
{
    struct picoui_native_animation_render_state *state;

    if (animation == 0) {
        return;
    }

    animation->render_ready = 0;
    state = picoui_native_animation_find_render_state(animation);
    if (state != 0) {
        state->rendered = 0;
        state->dirty_rect.x = 0;
        state->dirty_rect.y = 0;
        state->dirty_rect.width = 0;
        state->dirty_rect.height = 0;
        state->changed_pixels = 0;
    }
}

int picoui_native_animation_set_frame_index(struct picoui_animation *animation, int frame_index)
{
    if (animation == 0 || frame_index < 0 || frame_index >= animation->frame_count) {
        return -1;
    }

    animation->frame_index = frame_index;
    animation->render_ready = 0;
    animation->start_ticks = picoui_native_animation_encode_start_tick(picoui_native_animation_now_ms());
    return 0;
}

static int picoui_native_animation_sync_frame(struct picoui_animation *animation)
{
    unsigned int now_ticks;
    unsigned int start_tick;
    unsigned int elapsed_ms;
    unsigned int steps;

    if (animation == 0 || animation->period_ms <= 0 || animation->frame_count <= 0) {
        return -1;
    }

    if (picoui_native_animation_backend(animation) == 0) {
        return -1;
    }

    if (animation->start_ticks == 0U) {
        animation->start_ticks = picoui_native_animation_encode_start_tick(picoui_native_animation_now_ms());
        return 0;
    }

    now_ticks = picoui_native_animation_now_ms();
    start_tick = picoui_native_animation_decode_start_tick(animation->start_ticks);
    if (now_ticks < start_tick) {
        animation->start_ticks = picoui_native_animation_encode_start_tick(now_ticks);
        return 0;
    }

    elapsed_ms = now_ticks - start_tick;
    steps = elapsed_ms / (unsigned int)animation->period_ms;
    if (steps == 0U) {
        return 0;
    }

    animation->frame_index = (animation->frame_index + (int)steps) % animation->frame_count;
    animation->start_ticks = picoui_native_animation_encode_start_tick(
        picoui_native_animation_decode_start_tick(animation->start_ticks)
        + steps * (unsigned int)animation->period_ms);
    return 0;
}

int picoui_native_animation_advance_timer(struct picoui_animation *animation, unsigned int elapsed_ms)
{
    if (animation == 0 || elapsed_ms == 0 || animation->period_ms <= 0 || animation->frame_count <= 0) {
        return -1;
    }

    animation->frame_index = (animation->frame_index + (int)(elapsed_ms / (unsigned int)animation->period_ms))
        % animation->frame_count;
    animation->render_ready = 0;
    return 0;
}

int picoui_native_animation_render(const struct picoui_animation *animation)
{
    struct picoui_native_animation_render_state *state;
    struct picoui_image_source *current_source;
    unsigned int *scratch = 0;
    size_t scratch_count;
    int i;

    if (animation == 0) {
        return -1;
    }

    state = picoui_native_animation_alloc_render_state(animation);
    if (state == 0) {
        return -1;
    }

    if (picoui_native_animation_sync_frame((struct picoui_animation *)animation) != 0) {
        return -1;
    }

    current_source = picoui_native_animation_current_source(animation, state);
    if (current_source == 0 || current_source->img_tile == 0) {
        return -1;
    }

    if (state->frame_sources != 0
        && picoui_backend_animation_set_source((struct picoui_animation *)animation, current_source) != 0) {
        return -1;
    }

    state->dirty_rect.x = 0;
    state->dirty_rect.y = 0;
    state->dirty_rect.width = 0;
    state->dirty_rect.height = 0;
    state->changed_pixels = 0;

    if (state->frame_sources != 0
        && animation->widget.width > 0
        && animation->widget.height > 0) {
        scratch_count = (size_t)animation->widget.width * (size_t)animation->widget.height;
        scratch = calloc(scratch_count, sizeof(*scratch));
        if (scratch == 0) {
            return -1;
        }

        if (picoui_native_image_render_buffer(current_source,
                                              0,
                                              scratch,
                                              animation->widget.width,
                                              animation->widget.height,
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

    (void)picoui_backend_animation_show_frame((struct picoui_animation *)animation, animation->frame_index);
    ((struct picoui_animation *)animation)->render_ready = 1;
    state->rendered = 1;
    return 0;
}

int picoui_native_animation_get_rendered_frame_index(const struct picoui_animation *animation, int *frame_index)
{
    if (animation == 0 || frame_index == 0 || animation->render_ready == 0) {
        return -1;
    }

    *frame_index = animation->frame_index;
    return 0;
}

int picoui_native_animation_bind_frame_sources(struct picoui_animation *animation,
                                               struct picoui_image_source **sources,
                                               int frame_count)
{
    struct picoui_native_animation_render_state *state;
    struct picoui_image_source **copy;
    int i;

    if (animation == 0 || sources == 0 || frame_count <= 0 || animation->widget.backend_widget == 0) {
        return -1;
    }

    for (i = 0; i < frame_count; ++i) {
        if (sources[i] == 0 || sources[i]->img_tile == 0) {
            return -1;
        }
    }

    state = picoui_native_animation_alloc_render_state(animation);
    if (state == 0) {
        return -1;
    }

    copy = calloc((size_t)frame_count, sizeof(*copy));
    if (copy == 0) {
        return -1;
    }

    for (i = 0; i < frame_count; ++i) {
        copy[i] = sources[i];
    }

    picoui_native_animation_free_frame_sources(state);
    state->frame_sources = copy;
    state->frame_source_count = frame_count;
    animation->frame_count = frame_count;
    animation->frame_index = 0;
    animation->source = sources[0];
    animation->render_ready = 0;
    return 0;
}

int picoui_native_animation_get_rendered_dirty_rect(const struct picoui_animation *animation,
                                                    struct picoui_rect *dirty_rect)
{
    struct picoui_native_animation_render_state *state;

    if (dirty_rect == 0) {
        return -1;
    }

    state = picoui_native_animation_find_render_state(animation);
    if (state == 0 || state->rendered == 0) {
        return -1;
    }

    *dirty_rect = state->dirty_rect;
    return 0;
}

int picoui_native_animation_get_rendered_changed_pixels(const struct picoui_animation *animation, int *count)
{
    struct picoui_native_animation_render_state *state;

    if (count == 0) {
        return -1;
    }

    state = picoui_native_animation_find_render_state(animation);
    if (state == 0 || state->rendered == 0) {
        return -1;
    }

    *count = state->changed_pixels;
    return 0;
}
