#include <assert.h>

#include "uiWidgetSwipeLogic.h"

static void test_wrap_index_loops_from_first_to_last(void)
{
    assert(uiWidgetSwipeWrapIndex(0, -1, 23) == 22);
}

static void test_wrap_index_loops_from_last_to_first(void)
{
    assert(uiWidgetSwipeWrapIndex(22, 1, 23) == 0);
}

static void test_short_drag_does_not_flip_page(void)
{
    uiWidgetSwipeGestureState_t state = {0};

    uiWidgetSwipeGestureBegin(&state, 240, 120);
    uiWidgetSwipeGestureTrack(&state, 280, 126);
    assert(uiWidgetSwipeGestureEnd(&state, 280, 126, 96) == UI_WIDGET_SWIPE_NONE);
}

static void test_left_drag_enters_next_page(void)
{
    uiWidgetSwipeGestureState_t state = {0};

    uiWidgetSwipeGestureBegin(&state, 320, 120);
    uiWidgetSwipeGestureTrack(&state, 180, 122);
    assert(uiWidgetSwipeGestureEnd(&state, 180, 122, 96) == UI_WIDGET_SWIPE_NEXT);
}

static void test_right_drag_enters_previous_page(void)
{
    uiWidgetSwipeGestureState_t state = {0};

    uiWidgetSwipeGestureBegin(&state, 140, 120);
    uiWidgetSwipeGestureTrack(&state, 280, 122);
    assert(uiWidgetSwipeGestureEnd(&state, 280, 122, 96) == UI_WIDGET_SWIPE_PREV);
}

static void test_vertical_drag_is_rejected(void)
{
    assert(uiWidgetSwipeIsHorizontalIntent(-120, 20, 96) == true);
    assert(uiWidgetSwipeIsHorizontalIntent(-40, 120, 96) == false);
}

int main(void)
{
    test_wrap_index_loops_from_first_to_last();
    test_wrap_index_loops_from_last_to_first();
    test_short_drag_does_not_flip_page();
    test_left_drag_enters_next_page();
    test_right_drag_enters_previous_page();
    test_vertical_drag_is_rejected();
    return 0;
}
