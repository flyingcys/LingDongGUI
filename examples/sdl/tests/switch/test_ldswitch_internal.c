#include <assert.h>

#include "ldSwitchInternal.h"

static void test_horizontal_metrics_reserve_knob_padding(void)
{
    ldSwitchAxisMetrics_t metrics = ldSwitchResolveAxisMetrics(44, 24, 2, true);

    assert(metrics.knobSize == 20);
    assert(metrics.trackLength == 20);
}

static void test_vertical_metrics_reserve_knob_padding(void)
{
    ldSwitchAxisMetrics_t metrics = ldSwitchResolveAxisMetrics(24, 44, 2, false);

    assert(metrics.knobSize == 20);
    assert(metrics.trackLength == 20);
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
    test_horizontal_metrics_reserve_knob_padding();
    test_vertical_metrics_reserve_knob_padding();
    test_anim_progress_reaches_target();
    test_knob_offset_matches_progress();
    test_anim_progress_supports_reverse_direction();
    return 0;
}
