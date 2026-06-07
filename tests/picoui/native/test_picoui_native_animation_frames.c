#include "picoui/picoui.h"
#include "../../../examples/common/Arm-2D/Library/Include/arm_2d_types.h"

#include <assert.h>

int picoui_native_animation_bind_frame_sources(struct picoui_animation *animation,
                                               struct picoui_image_source **sources,
                                               int frame_count);
int picoui_native_animation_advance_timer(struct picoui_animation *animation, unsigned int elapsed_ms);
int picoui_native_animation_render(const struct picoui_animation *animation);
int picoui_native_animation_get_rendered_frame_index(const struct picoui_animation *animation, int *frame_index);
int picoui_native_animation_get_rendered_dirty_rect(const struct picoui_animation *animation,
                                                    struct picoui_rect *dirty_rect);
int picoui_native_animation_get_rendered_changed_pixels(const struct picoui_animation *animation, int *count);

static unsigned int s_frame0_pixels[16 * 16];
static unsigned int s_frame1_pixels[16 * 16];
static unsigned int s_frame2_pixels[16 * 16];

static arm_2d_tile_t s_frame0_tile = {
    .tInfo = {
        .bIsRoot = true,
        .bHasEnforcedColour = true,
        .tColourInfo = {
            .chScheme = ARM_2D_COLOUR_32BIT,
        },
    },
    .tRegion = {
        .tSize = {
            .iWidth = 16,
            .iHeight = 16,
        },
    },
    .pwBuffer = s_frame0_pixels,
};

static arm_2d_tile_t s_frame1_tile = {
    .tInfo = {
        .bIsRoot = true,
        .bHasEnforcedColour = true,
        .tColourInfo = {
            .chScheme = ARM_2D_COLOUR_32BIT,
        },
    },
    .tRegion = {
        .tSize = {
            .iWidth = 16,
            .iHeight = 16,
        },
    },
    .pwBuffer = s_frame1_pixels,
};

static arm_2d_tile_t s_frame2_tile = {
    .tInfo = {
        .bIsRoot = true,
        .bHasEnforcedColour = true,
        .tColourInfo = {
            .chScheme = ARM_2D_COLOUR_32BIT,
        },
    },
    .tRegion = {
        .tSize = {
            .iWidth = 16,
            .iHeight = 16,
        },
    },
    .pwBuffer = s_frame2_pixels,
};

static void fill_frame(unsigned int *pixels, unsigned int color)
{
    int i;

    for (i = 0; i < 16 * 16; ++i) {
        pixels[i] = color;
    }
}

static void test_animation_frame_sources_advance_and_mark_dirty(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_window *window;
    struct picoui_animation *animation;
    struct picoui_image_source frame0 = {.img_tile = &s_frame0_tile};
    struct picoui_image_source frame1 = {.img_tile = &s_frame1_tile};
    struct picoui_image_source frame2 = {.img_tile = &s_frame2_tile};
    struct picoui_image_source *sources[3] = {&frame0, &frame1, &frame2};
    struct picoui_animation_props props = {
        .id = "anim_frames",
        .width = 16,
        .height = 16,
        .period_ms = 40,
        .source = &frame0,
    };
    struct picoui_rect dirty_rect = {-1, -1, -1, -1};
    int frame_index = -1;
    int changed_pixels = -1;

    fill_frame(s_frame0_pixels, 0xFF112233U);
    fill_frame(s_frame1_pixels, 0xFF445566U);
    fill_frame(s_frame2_pixels, 0xFF778899U);

    assert(app != 0);
    window = picoui_window_create(app, "animation_frames_root");
    assert(window != 0);
    animation = picoui_animation_create_with_props((struct picoui_widget *)window, &props);
    assert(animation != 0);

    assert(picoui_native_animation_bind_frame_sources(animation, sources, 3) == 0);
    assert(picoui_native_animation_get_rendered_frame_index(animation, &frame_index) == -1);
    assert(picoui_native_animation_advance_timer(animation, 80) == 0);
    assert(picoui_native_animation_render(animation) == 0);
    assert(picoui_native_animation_get_rendered_frame_index(animation, &frame_index) == 0);
    assert(frame_index == 2);
    assert(picoui_native_animation_get_rendered_dirty_rect(animation, &dirty_rect) == 0);
    assert(dirty_rect.width > 0);
    assert(dirty_rect.height > 0);
    assert(picoui_native_animation_get_rendered_changed_pixels(animation, &changed_pixels) == 0);
    assert(changed_pixels > 0);

    picoui_app_destroy(app);
}

static void test_animation_frame_sources_reject_invalid_inputs(void)
{
    assert(picoui_native_animation_bind_frame_sources(0, 0, 0) == -1);
}

int main(void)
{
    test_animation_frame_sources_advance_and_mark_dirty();
    test_animation_frame_sources_reject_invalid_inputs();
    return 0;
}
