/*
 * Copyright (c) 2023-2026 flyingcys (flyingcys@gmail.com). All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * M2 Task 4: fixed event-callback pool, generation handles, sync dispatch,
 * no bubble, DELETE once, focus public API.
 */

#include "tinyui.h"
#include "internal.h"
#include "core/event.h"
#include "core/focus.h"
#include "core/runtime.h"
#include "core/result.h"
#include "integration/input.h"

#include "../../../src/gui/ldBase.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

/* ── allocator counters ──────────────────────────────────────────────────── */

struct tinyui_test_allocator_stats {
    size_t alloc_calls;
    size_t calloc_calls;
    size_t realloc_calls;
    size_t free_calls;
    size_t bytes_requested;
};

void tinyui_test_allocator_reset(void);
struct tinyui_test_allocator_stats tinyui_test_allocator_snapshot(void);
void *tinyui_test_allocator_malloc(uint32_t size);
void *tinyui_test_allocator_calloc(uint32_t num, uint32_t size);
void *tinyui_test_allocator_realloc(void *ptr, uint32_t size);
void tinyui_test_allocator_free(void *ptr);

void *ldMalloc(uint32_t size)
{
    return tinyui_test_allocator_malloc(size);
}

void *ldCalloc(uint32_t num, uint32_t size)
{
    return tinyui_test_allocator_calloc(num, size);
}

void *ldRealloc(void *ptr, uint32_t size)
{
    return tinyui_test_allocator_realloc(ptr, size);
}

void ldFree(void *ptr)
{
    tinyui_test_allocator_free(ptr);
}

static void assert_zero_alloc_delta(const struct tinyui_test_allocator_stats *before,
                                    const struct tinyui_test_allocator_stats *after)
{
    assert(after->alloc_calls == before->alloc_calls);
    assert(after->calloc_calls == before->calloc_calls);
    assert(after->realloc_calls == before->realloc_calls);
    assert(after->free_calls == before->free_calls);
    assert(after->bytes_requested == before->bytes_requested);
}

/* ── harness ─────────────────────────────────────────────────────────────── */

static tinyui_obj_t *make_screen(void)
{
    tinyui_obj_t *screen;

    tinyui_deinit();
    assert(tinyui_init() == TINYUI_OK);
    screen = tinyui_screen_create();
    assert(screen != NULL);
    assert(tinyui_screen_load(screen, TINYUI_SCREEN_TRANSITION_NONE, 0U) == TINYUI_OK);
    return screen;
}

/* ── callback fixtures ───────────────────────────────────────────────────── */

static int g_order[32];
static int g_order_count;
static int g_counts[8];
static const tinyui_event_t *g_last_event;
static tinyui_event_t g_last_event_copy;
static int g_last_event_valid;
static tinyui_obj_t *g_delete_target;
static int g_delete_count;
static int g_delete_getter_ok;
static tinyui_event_handle_t g_handle_to_remove;
static tinyui_obj_t *g_obj_for_mutation;
static int g_add_in_cb_result;
static tinyui_event_handle_t g_added_in_cb_handle;
static int g_same_round_new_cb_calls;
static int g_stop_after_delete;

static void reset_event_fixture(void)
{
    memset(g_order, 0, sizeof(g_order));
    g_order_count = 0;
    memset(g_counts, 0, sizeof(g_counts));
    g_last_event = NULL;
    memset(&g_last_event_copy, 0, sizeof(g_last_event_copy));
    g_last_event_valid = 0;
    g_delete_target = NULL;
    g_delete_count = 0;
    g_delete_getter_ok = 0;
    g_handle_to_remove = 0U;
    g_obj_for_mutation = NULL;
    g_add_in_cb_result = 0;
    g_added_in_cb_handle = 0U;
    g_same_round_new_cb_calls = 0;
    g_stop_after_delete = 0;
}

static void record_event(const tinyui_event_t *event, int tag)
{
    assert(event != NULL);
    if (g_order_count < (int)(sizeof(g_order) / sizeof(g_order[0]))) {
        g_order[g_order_count++] = tag;
    }
    if (tag >= 0 && tag < (int)(sizeof(g_counts) / sizeof(g_counts[0]))) {
        g_counts[tag] += 1;
    }
    g_last_event = event;
    g_last_event_copy = *event;
    g_last_event_valid = 1;
}

static void cb_tag0(const tinyui_event_t *event)
{
    record_event(event, 0);
}

static void cb_tag1(const tinyui_event_t *event)
{
    record_event(event, 1);
}

static void cb_tag2(const tinyui_event_t *event)
{
    record_event(event, 2);
}

static void cb_add_during_dispatch(const tinyui_event_t *event)
{
    tinyui_obj_t *target;

    record_event(event, 0);
    target = event != NULL ? event->target : g_obj_for_mutation;
    assert(target != NULL);
    g_add_in_cb_result = tinyui_obj_add_event_cb(target,
                                                 TINYUI_EVENT_MASK(TINYUI_EVENT_CLICKED),
                                                 cb_tag2,
                                                 NULL,
                                                 &g_added_in_cb_handle);
}

static void cb_same_round_new(const tinyui_event_t *event)
{
    (void)event;
    g_same_round_new_cb_calls += 1;
}

static void cb_remove_later(const tinyui_event_t *event)
{
    tinyui_obj_t *target;

    record_event(event, 0);
    target = event != NULL ? event->target : g_obj_for_mutation;
    assert(target != NULL);
    assert(g_handle_to_remove != 0U);
    assert(tinyui_obj_remove_event_cb(target, g_handle_to_remove) == TINYUI_OK);
}

static void cb_delete_target(const tinyui_event_t *event)
{
    record_event(event, 0);
    assert(event->target != NULL);
    assert(tinyui_obj_delete(event->target) == TINYUI_OK);
    g_stop_after_delete = 1;
}

static void cb_after_delete_should_not_run(const tinyui_event_t *event)
{
    record_event(event, 1);
    (void)event;
}

static void cb_on_delete(const tinyui_event_t *event)
{
    uint16_t id = 0U;

    assert(event != NULL);
    assert(event->code == TINYUI_EVENT_DELETE);
    assert(event->target == g_delete_target);
    g_delete_count += 1;

    /* DELETE: only getters allowed. */
    assert(tinyui_obj_get_id(event->target, &id) == TINYUI_OK);
    g_delete_getter_ok = 1;

    assert(tinyui_obj_add_event_cb(event->target,
                                   TINYUI_EVENT_MASK(TINYUI_EVENT_CLICKED),
                                   cb_tag0,
                                   NULL,
                                   NULL) == TINYUI_ERROR_INVALID_STATE);
    assert(tinyui_obj_delete(event->target) == TINYUI_ERROR_INVALID_STATE);
}

static void inject_clicked(tinyui_obj_t *obj)
{
    struct tinyui_widget *widget = (struct tinyui_widget *)(void *)obj;

    assert(widget != NULL);
    assert(widget->ld_widget != NULL);
    /* CLICKED is produced on SIGNAL_RELEASE for buttons (press optional). */
    assert(tinyui_runtime_internal_widget_dispatch_native_signal(widget,
                                                                  SIGNAL_PRESS,
                                                                  0) == 0);
    assert(tinyui_runtime_internal_widget_dispatch_native_signal(widget,
                                                                  SIGNAL_RELEASE,
                                                                  0) == 0);
}

static void inject_slider_value(tinyui_obj_t *slider_obj, uint64_t native_permille)
{
    struct tinyui_widget *widget = (struct tinyui_widget *)(void *)slider_obj;

    assert(widget != NULL);
    assert(tinyui_runtime_internal_widget_dispatch_native_signal(widget,
                                                                  SIGNAL_VALUE_CHANGED,
                                                                  native_permille) == 0);
}

/* ── tests ───────────────────────────────────────────────────────────────── */

static void test_event_pool_capacity_and_handle_encoding(void)
{
    tinyui_obj_t *screen = make_screen();
    tinyui_obj_t *button = tinyui_button_create(screen);
    tinyui_event_handle_t handles[TINYUI_EVENT_CB_CAPACITY];
    tinyui_event_handle_t overflow = 0xFFFFFFFFU;
    tinyui_event_handle_t old_handle;
    tinyui_event_handle_t reused_handle = 0U;
    unsigned int i;

    assert(button != NULL);
    reset_event_fixture();

    for (i = 0U; i < (unsigned int)TINYUI_EVENT_CB_CAPACITY; ++i) {
        handles[i] = 0U;
        assert(tinyui_obj_add_event_cb(button,
                                       TINYUI_EVENT_MASK(TINYUI_EVENT_CLICKED),
                                       cb_tag0,
                                       NULL,
                                       &handles[i]) == TINYUI_OK);
        assert(handles[i] != 0U);
        assert((handles[i] & UINT32_C(0xffff)) != 0U);
        assert(tinyui_last_result() == TINYUI_OK);
    }

    assert(tinyui_obj_add_event_cb(button,
                                   TINYUI_EVENT_MASK(TINYUI_EVENT_CLICKED),
                                   cb_tag1,
                                   NULL,
                                   &overflow) == TINYUI_ERROR_CAPACITY);
    assert(overflow == 0U);
    assert(tinyui_last_result() == TINYUI_ERROR_CAPACITY);

    old_handle = handles[0];
    assert(tinyui_obj_remove_event_cb(button, old_handle) == TINYUI_OK);
    assert(tinyui_obj_add_event_cb(button,
                                   TINYUI_EVENT_MASK(TINYUI_EVENT_CLICKED),
                                   cb_tag1,
                                   NULL,
                                   &reused_handle) == TINYUI_OK);
    assert(reused_handle != 0U);
    assert((reused_handle & UINT32_C(0xffff)) != 0U);
    assert(reused_handle != old_handle);
    assert(tinyui_obj_remove_event_cb(button, old_handle) == TINYUI_ERROR_INVALID_ARG);

    tinyui_deinit();
}

static void test_event_registration_order_and_mutations(void)
{
    tinyui_obj_t *screen = make_screen();
    tinyui_obj_t *button = tinyui_button_create(screen);
    tinyui_event_handle_t h0 = 0U;
    tinyui_event_handle_t h1 = 0U;
    tinyui_event_handle_t h2 = 0U;
    struct tinyui_test_allocator_stats before;
    struct tinyui_test_allocator_stats after;

    assert(button != NULL);
    reset_event_fixture();
    g_obj_for_mutation = button;

    assert(tinyui_obj_add_event_cb(button,
                                   TINYUI_EVENT_MASK(TINYUI_EVENT_CLICKED),
                                   cb_tag0,
                                   NULL,
                                   &h0) == TINYUI_OK);
    assert(tinyui_obj_add_event_cb(button,
                                   TINYUI_EVENT_MASK(TINYUI_EVENT_CLICKED),
                                   cb_tag1,
                                   NULL,
                                   &h1) == TINYUI_OK);
    assert(tinyui_obj_add_event_cb(button,
                                   TINYUI_EVENT_MASK(TINYUI_EVENT_CLICKED),
                                   cb_tag2,
                                   NULL,
                                   &h2) == TINYUI_OK);

    before = tinyui_test_allocator_snapshot();
    inject_clicked(button);
    after = tinyui_test_allocator_snapshot();
    assert_zero_alloc_delta(&before, &after);

    assert(g_order_count == 3);
    assert(g_order[0] == 0);
    assert(g_order[1] == 1);
    assert(g_order[2] == 2);
    assert(g_counts[0] == 1);
    assert(g_counts[1] == 1);
    assert(g_counts[2] == 1);

    /* remove middle handle; remaining keep registration order */
    assert(tinyui_obj_remove_event_cb(button, h1) == TINYUI_OK);
    reset_event_fixture();
    inject_clicked(button);
    assert(g_order_count == 2);
    assert(g_order[0] == 0);
    assert(g_order[1] == 2);

    /* callback 内新增不参加当前轮 */
    assert(tinyui_obj_remove_event_cb(button, h0) == TINYUI_OK);
    assert(tinyui_obj_remove_event_cb(button, h2) == TINYUI_OK);
    reset_event_fixture();
    g_obj_for_mutation = button;
    g_added_in_cb_handle = 0U;
    g_add_in_cb_result = -1;
    assert(tinyui_obj_add_event_cb(button,
                                   TINYUI_EVENT_MASK(TINYUI_EVENT_CLICKED),
                                   cb_add_during_dispatch,
                                   NULL,
                                   &h0) == TINYUI_OK);
    inject_clicked(button);
    assert(g_add_in_cb_result == TINYUI_OK);
    assert(g_added_in_cb_handle != 0U);
    assert(g_counts[0] == 1);
    assert(g_counts[2] == 0); /* newborn slot skipped this epoch */

    {
        tinyui_event_handle_t added = g_added_in_cb_handle;
        tinyui_event_handle_t adder = h0;

        reset_event_fixture();
        inject_clicked(button);
        assert(g_counts[0] == 1);
        assert(g_counts[2] == 1);

        assert(tinyui_obj_remove_event_cb(button, adder) == TINYUI_OK);
        assert(tinyui_obj_remove_event_cb(button, added) == TINYUI_OK);
        /* second-round adder may have registered yet another slot */
        if (g_added_in_cb_handle != 0U && g_added_in_cb_handle != added) {
            (void)tinyui_obj_remove_event_cb(button, g_added_in_cb_handle);
        }
    }

    /* callback 内移除后续 callback 立即生效 */
    reset_event_fixture();
    g_obj_for_mutation = button;
    assert(tinyui_obj_add_event_cb(button,
                                   TINYUI_EVENT_MASK(TINYUI_EVENT_CLICKED),
                                   cb_remove_later,
                                   NULL,
                                   &h0) == TINYUI_OK);
    assert(tinyui_obj_add_event_cb(button,
                                   TINYUI_EVENT_MASK(TINYUI_EVENT_CLICKED),
                                   cb_tag1,
                                   NULL,
                                   &h1) == TINYUI_OK);
    g_handle_to_remove = h1;
    inject_clicked(button);
    assert(g_order_count == 1);
    assert(g_order[0] == 0);
    assert(g_counts[1] == 0);
    assert(tinyui_obj_remove_event_cb(button, h0) == TINYUI_OK);

    /* 删除 target 停止后续 callback */
    reset_event_fixture();
    assert(tinyui_obj_add_event_cb(button,
                                   TINYUI_EVENT_MASK(TINYUI_EVENT_CLICKED),
                                   cb_delete_target,
                                   NULL,
                                   &h0) == TINYUI_OK);
    assert(tinyui_obj_add_event_cb(button,
                                   TINYUI_EVENT_MASK(TINYUI_EVENT_CLICKED),
                                   cb_after_delete_should_not_run,
                                   NULL,
                                   &h1) == TINYUI_OK);
    inject_clicked(button);
    assert(g_stop_after_delete == 1);
    assert(g_counts[0] == 1);
    assert(g_counts[1] == 0);

    tinyui_deinit();
}

static void test_event_delete_once_and_cleanup(void)
{
    tinyui_obj_t *screen = make_screen();
    tinyui_obj_t *button = tinyui_button_create(screen);
    tinyui_event_handle_t h_del = 0U;
    tinyui_event_handle_t h_click = 0U;

    assert(button != NULL);
    reset_event_fixture();
    g_delete_target = button;

    assert(tinyui_obj_add_event_cb(button,
                                   TINYUI_EVENT_MASK(TINYUI_EVENT_DELETE),
                                   cb_on_delete,
                                   NULL,
                                   &h_del) == TINYUI_OK);
    assert(tinyui_obj_add_event_cb(button,
                                   TINYUI_EVENT_MASK(TINYUI_EVENT_CLICKED),
                                   cb_tag0,
                                   NULL,
                                   &h_click) == TINYUI_OK);

    assert(tinyui_obj_delete(button) == TINYUI_OK);
    assert(g_delete_count == 1);
    assert(g_delete_getter_ok == 1);

    /* slots cleaned: remove with old handle fails */
    assert(tinyui_obj_remove_event_cb(button, h_del) == TINYUI_ERROR_INVALID_ARG ||
           tinyui_obj_remove_event_cb(button, h_del) == TINYUI_ERROR_INVALID_OBJECT ||
           tinyui_obj_remove_event_cb(button, h_del) == TINYUI_ERROR_INVALID_STATE);

    tinyui_deinit();
}

static void test_event_no_bubble_and_payload(void)
{
    tinyui_obj_t *screen = make_screen();
    tinyui_obj_t *parent = tinyui_button_create(screen);
    tinyui_obj_t *child = tinyui_button_create(screen);
    tinyui_obj_t *slider = tinyui_slider_create(screen);
    tinyui_event_handle_t hp = 0U;
    tinyui_event_handle_t hc = 0U;
    tinyui_event_handle_t hs = 0U;
    tinyui_event_handle_t hk = 0U;
    int parent_clicks = 0;
    int child_clicks = 0;

    assert(parent != NULL && child != NULL && slider != NULL);
    reset_event_fixture();

    assert(tinyui_obj_add_event_cb(parent,
                                   TINYUI_EVENT_MASK(TINYUI_EVENT_CLICKED),
                                   cb_tag0,
                                   &parent_clicks,
                                   &hp) == TINYUI_OK);
    assert(tinyui_obj_add_event_cb(child,
                                   TINYUI_EVENT_MASK(TINYUI_EVENT_CLICKED),
                                   cb_tag1,
                                   &child_clicks,
                                   &hc) == TINYUI_OK);

    inject_clicked(child);
    assert(g_counts[1] == 1);
    assert(g_counts[0] == 0);
    assert(g_last_event_valid == 1);
    assert(g_last_event_copy.code == TINYUI_EVENT_CLICKED);
    assert(g_last_event_copy.target == child);

    /* slider VALUE_CHANGED payload */
    reset_event_fixture();
    assert(tinyui_slider_set_range(slider, 0, 100) == 0);
    assert(tinyui_obj_add_event_cb(slider,
                                   TINYUI_EVENT_MASK(TINYUI_EVENT_VALUE_CHANGED),
                                   cb_tag0,
                                   NULL,
                                   &hs) == TINYUI_OK);
    inject_slider_value(slider, 500); /* 50% of [0,100] in permille path */
    assert(g_counts[0] == 1);
    assert(g_last_event_valid == 1);
    assert(g_last_event_copy.code == TINYUI_EVENT_VALUE_CHANGED);
    assert(g_last_event_copy.target == slider);
    /* canonical value mapped from native permille */
    assert(g_last_event_copy.data.value ==
           ((struct tinyui_widget *)(void *)slider)->value);

    /* KEY payload via public input API on focused object */
    reset_event_fixture();
    assert(tinyui_focus_set(child) == TINYUI_OK);
    assert(tinyui_obj_add_event_cb(child,
                                   TINYUI_EVENT_MASK(TINYUI_EVENT_KEY),
                                   cb_tag0,
                                   NULL,
                                   &hk) == TINYUI_OK);
    assert(tinyui_input_send_key(TINYUI_KEY_ENTER, true) == TINYUI_OK);
    assert(g_counts[0] == 1);
    assert(g_last_event_valid == 1);
    assert(g_last_event_copy.code == TINYUI_EVENT_KEY);
    assert(g_last_event_copy.target == child);
    assert(g_last_event_copy.data.key.key == (uint16_t)TINYUI_KEY_ENTER);
    assert(g_last_event_copy.data.key.pressed == true);

    tinyui_deinit();
}

static void test_focus_public_api(void)
{
    tinyui_obj_t *screen = make_screen();
    tinyui_obj_t *a = tinyui_button_create(screen);
    tinyui_obj_t *b = tinyui_button_create(screen);

    assert(a != NULL && b != NULL);

    assert(tinyui_focus_set(a) == TINYUI_OK);
    assert(tinyui_focus_current() == a);
    assert(tinyui_focus_set(b) == TINYUI_OK);
    assert(tinyui_focus_current() == b);
    assert(tinyui_focus_clear() == TINYUI_OK);
    assert(tinyui_focus_current() == NULL);
    assert(tinyui_focus_move(TINYUI_FOCUS_LEFT) == TINYUI_OK ||
           tinyui_focus_move(TINYUI_FOCUS_LEFT) == TINYUI_ERROR_BACKEND);

    tinyui_deinit();
}

static void test_legacy_internal_focus_helpers(void)
{
    tinyui_obj_t *screen = make_screen();
    tinyui_obj_t *btn_obj = tinyui_button_create(screen);
    struct tinyui_widget *btn;
    int was_focus;

    assert(btn_obj != NULL);
    btn = (struct tinyui_widget *)(void *)btn_obj;

    assert(tinyui_runtime_internal_widget_is_focus_owner(btn) == 0);
    assert(tinyui_runtime_internal_widget_claim_focus(btn) == 0);
    assert(tinyui_runtime_internal_widget_is_focus_owner(btn) == 1);
    assert(btn->has_focus == 1);
    assert(btn->focus_enter_count == 1);

    was_focus = btn->focus_enter_count;
    assert(tinyui_runtime_internal_widget_claim_focus(btn) == 0);
    assert(btn->focus_enter_count == was_focus);

    assert(tinyui_runtime_internal_widget_release_focus(btn) == 0);
    assert(tinyui_runtime_internal_widget_is_focus_owner(btn) == 0);
    assert(btn->has_focus == 0);
    assert(btn->focus_leave_count == 1);

    assert(tinyui_runtime_internal_widget_claim_focus(0) == -1);
    assert(tinyui_runtime_internal_widget_release_focus(0) == -1);
    assert(tinyui_runtime_internal_widget_is_focus_owner(0) == 0);

    assert(tinyui_runtime_internal_widget_set_visible(btn, 0) == 0);
    assert(tinyui_runtime_internal_widget_claim_focus(btn) == -1);

    tinyui_deinit();
}

int main(void)
{
    test_event_pool_capacity_and_handle_encoding();
    test_event_registration_order_and_mutations();
    test_event_delete_once_and_cleanup();
    test_event_no_bubble_and_payload();
    test_focus_public_api();
    test_legacy_internal_focus_helpers();
    return 0;
}
