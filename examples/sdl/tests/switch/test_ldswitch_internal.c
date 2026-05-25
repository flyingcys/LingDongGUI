#include <assert.h>
#include <stddef.h>

#include "ldSwitchInternal.h"

static void test_image_layer_requires_img_and_mask(void)
{
    int img;
    int mask;

    assert(ldSwitchLayerUsesImage(&img, NULL) == false);
    assert(ldSwitchLayerUsesImage(NULL, &mask) == false);
    assert(ldSwitchLayerUsesImage(&img, &mask) == true);
    assert(ldSwitchLayerUsesImage(NULL, NULL) == false);
}

static void test_horizontal_metrics_reserve_knob_padding(void)
{
    ldSwitchAxisMetrics_t metrics = ldSwitchResolveAxisMetrics(44, 24, 2, true);

    assert(metrics.knobSize == 24);
    assert(metrics.trackLength == 20);
}

static void test_vertical_metrics_reserve_knob_padding(void)
{
    ldSwitchAxisMetrics_t metrics = ldSwitchResolveAxisMetrics(24, 44, 2, false);

    assert(metrics.knobSize == 24);
    assert(metrics.trackLength == 20);
}

static void test_auto_direction_uses_longer_axis(void)
{
    assert(ldSwitchResolveIsHorizontal(44, 24, LD_SWITCH_DIRECTION_AUTO) == true);
    assert(ldSwitchResolveIsHorizontal(24, 44, LD_SWITCH_DIRECTION_AUTO) == false);
    assert(ldSwitchResolveIsHorizontal(24, 24, LD_SWITCH_DIRECTION_AUTO) == true);
}

static void test_forced_direction_overrides_shape(void)
{
    assert(ldSwitchResolveIsHorizontal(20, 60, LD_SWITCH_DIRECTION_HORIZONTAL) == true);
    assert(ldSwitchResolveIsHorizontal(60, 20, LD_SWITCH_DIRECTION_VERTICAL) == false);
}

static void test_anim_progress_reaches_target(void)
{
    ldSwitchAnimState_t anim = {
        .start = 0,
        .target = 1000,
        .current = 0,
        .elapsedMs = 0,
        .durationMs = 150,
        .running = true,
    };
    uint16_t progress = 0;

    assert(ldSwitchAdvanceAnimation(&anim, 75, &progress) == true);
    assert(progress > 0 && progress < 1000);
    assert(ldSwitchAdvanceAnimation(&anim, 75, &progress) == false);
    assert(progress == 1000);
}

static void test_knob_offset_matches_progress(void)
{
    ldSwitchAxisMetrics_t metrics = {
        .trackStart = 2,
        .trackLength = 20,
        .knobSize = 20,
    };

    assert(ldSwitchResolveKnobOffset(&metrics, 0) == 0);
    assert(ldSwitchResolveKnobOffset(&metrics, 500) == 10);
    assert(ldSwitchResolveKnobOffset(&metrics, 1000) == 20);
}

static void test_horizontal_indicator_and_knob_are_continuous(void)
{
    ldSwitchGeometry_t start = ldSwitchResolveGeometry(44, 24, 2, LD_SWITCH_DIRECTION_HORIZONTAL, 0);
    ldSwitchGeometry_t middle = ldSwitchResolveGeometry(44, 24, 2, LD_SWITCH_DIRECTION_HORIZONTAL, 500);
    ldSwitchGeometry_t end = ldSwitchResolveGeometry(44, 24, 2, LD_SWITCH_DIRECTION_HORIZONTAL, 1000);

    assert(start.isHorizontal == true);
    assert(start.knob.iX == 0);
    assert(start.knob.iHeight > start.track.iHeight);
    assert(start.knob.iY == 0);
    assert(start.indicator.iX == 2);
    assert(start.indicator.iY == 2);
    assert(start.indicator.iHeight == 20);
    assert(start.indicator.iWidth == 0);
    assert(middle.knob.iX == 10);
    assert(end.knob.iX == 20);
    assert(middle.indicator.iWidth == 20);
    assert(end.indicator.iWidth == 40);
    assert(start.knob.iX < middle.knob.iX);
    assert(middle.knob.iX < end.knob.iX);
    assert(start.indicator.iWidth < middle.indicator.iWidth);
    assert(middle.indicator.iWidth < end.indicator.iWidth);
}

static void test_vertical_indicator_and_knob_are_continuous(void)
{
    ldSwitchGeometry_t start = ldSwitchResolveGeometry(24, 44, 2, LD_SWITCH_DIRECTION_VERTICAL, 0);
    ldSwitchGeometry_t middle = ldSwitchResolveGeometry(24, 44, 2, LD_SWITCH_DIRECTION_VERTICAL, 500);
    ldSwitchGeometry_t end = ldSwitchResolveGeometry(24, 44, 2, LD_SWITCH_DIRECTION_VERTICAL, 1000);

    assert(start.isHorizontal == false);
    assert(start.knob.iY == 20);
    assert(start.knob.iWidth > start.track.iWidth);
    assert(start.knob.iX == 0);
    assert(start.indicator.iX == 2);
    assert(start.indicator.iWidth == 20);
    assert(start.indicator.iY == 44);
    assert(start.indicator.iHeight == 0);
    assert(middle.indicator.iY == 22);
    assert(middle.knob.iY == 10);
    assert(middle.indicator.iHeight == 20);
    assert(end.knob.iY == 0);
    assert(end.indicator.iY == 2);
    assert(end.indicator.iHeight == 40);
    assert(start.knob.iY > middle.knob.iY);
    assert(middle.knob.iY > end.knob.iY);
    assert(start.indicator.iHeight < middle.indicator.iHeight);
    assert(middle.indicator.iHeight < end.indicator.iHeight);
}

static void test_anim_progress_supports_reverse_direction(void)
{
    ldSwitchAnimState_t anim = {
        .start = 1000,
        .target = 0,
        .current = 1000,
        .elapsedMs = 0,
        .durationMs = 150,
        .running = true,
    };
    uint16_t progress = 0;

    assert(ldSwitchAdvanceAnimation(&anim, 150, &progress) == false);
    assert(progress == 0);
}

int main(void)
{
    test_image_layer_requires_img_and_mask();
    test_horizontal_metrics_reserve_knob_padding();
    test_vertical_metrics_reserve_knob_padding();
    test_auto_direction_uses_longer_axis();
    test_forced_direction_overrides_shape();
    test_anim_progress_reaches_target();
    test_knob_offset_matches_progress();
    test_horizontal_indicator_and_knob_are_continuous();
    test_vertical_indicator_and_knob_are_continuous();
    test_anim_progress_supports_reverse_direction();
    return 0;
}
