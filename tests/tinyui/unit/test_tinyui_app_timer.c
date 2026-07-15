#include "core/runtime.h"
#include "core/timer.h"
#include "internal.h"
#include "tick/tick.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* ── fake clock ─────────────────────────────────────────────────────────── */

static uint32_t s_fake_now_ms;

static unsigned int fake_tick_get(void *user_data)
{
    (void)user_data;
    return (unsigned int)s_fake_now_ms;
}

static void install_fake_clock(void)
{
    struct tinyui_app *app = tinyui_runtime_internal_app_current();

    assert(app != NULL);
    s_fake_now_ms = 0U;
    assert(tinyui_tick_set_source(app, fake_tick_get, NULL) == 0);
}

static void set_now(uint32_t now_ms)
{
    s_fake_now_ms = now_ms;
}

static void process_at(uint32_t now_ms, uint32_t *next_ms)
{
    set_now(now_ms);
    assert(tinyui_process(next_ms) == TINYUI_OK);
}

/* ── allocator counters (override ld* for this TU) ─────────────────────── */

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

/* ── callbacks ──────────────────────────────────────────────────────────── */

static void count_cb(tinyui_timer_t *timer, void *user_data)
{
    unsigned int *calls = (unsigned int *)user_data;

    (void)timer;
    if (calls != NULL) {
        ++*calls;
    }
}

static void delete_self_cb(tinyui_timer_t *timer, void *user_data)
{
    unsigned int *calls = (unsigned int *)user_data;

    ++*calls;
    tinyui_timer_delete(timer);
}

static void stop_self_cb(tinyui_timer_t *timer, void *user_data)
{
    unsigned int *calls = (unsigned int *)user_data;

    ++*calls;
    assert(tinyui_timer_stop(timer) == TINYUI_OK);
}

struct create_in_cb_ctx {
    unsigned int calls;
    tinyui_timer_t *created;
    unsigned int child_calls;
};

static void child_count_cb(tinyui_timer_t *timer, void *user_data)
{
    unsigned int *calls = (unsigned int *)user_data;

    (void)timer;
    ++*calls;
}

static void create_timer_in_cb(tinyui_timer_t *timer, void *user_data)
{
    struct create_in_cb_ctx *ctx = (struct create_in_cb_ctx *)user_data;

    (void)timer;
    ++ctx->calls;
    ctx->created = tinyui_timer_create(10U, false, child_count_cb, &ctx->child_calls);
    assert(ctx->created != NULL);
    assert(tinyui_timer_start(ctx->created) == TINYUI_OK);
}

/* ── tests ──────────────────────────────────────────────────────────────── */

static void test_timer_pool_capacity_and_reuse(void)
{
    tinyui_timer_t *timers[TINYUI_TIMER_CAPACITY];
    tinyui_timer_t *extra;
    tinyui_timer_t *reused;
    unsigned int i;

    assert(tinyui_init() == TINYUI_OK);
    install_fake_clock();

    for (i = 0U; i < (unsigned int)TINYUI_TIMER_CAPACITY; ++i) {
        timers[i] = tinyui_timer_create(10U + i, true, count_cb, NULL);
        assert(timers[i] != NULL);
        assert(tinyui_last_result() == TINYUI_OK);
    }

    extra = tinyui_timer_create(99U, false, count_cb, NULL);
    assert(extra == NULL);
    assert(tinyui_last_result() == TINYUI_ERROR_CAPACITY);

    tinyui_timer_delete(timers[3]);
    reused = tinyui_timer_create(42U, false, count_cb, NULL);
    assert(reused != NULL);
    assert(tinyui_last_result() == TINYUI_OK);

    extra = tinyui_timer_create(100U, false, count_cb, NULL);
    assert(extra == NULL);
    assert(tinyui_last_result() == TINYUI_ERROR_CAPACITY);

    tinyui_deinit();
}

static void test_one_shot_and_repeat(void)
{
    tinyui_timer_t *one_shot;
    tinyui_timer_t *repeat;
    unsigned int one_calls = 0U;
    unsigned int rep_calls = 0U;
    uint32_t next_ms = 0U;

    assert(tinyui_init() == TINYUI_OK);
    install_fake_clock();

    one_shot = tinyui_timer_create(50U, false, count_cb, &one_calls);
    repeat = tinyui_timer_create(50U, true, count_cb, &rep_calls);
    assert(one_shot != NULL);
    assert(repeat != NULL);
    assert(tinyui_timer_start(one_shot) == TINYUI_OK);
    assert(tinyui_timer_start(repeat) == TINYUI_OK);

    process_at(0U, &next_ms);
    assert(one_calls == 0U);
    assert(rep_calls == 0U);
    assert(next_ms == 50U);

    process_at(50U, &next_ms);
    assert(one_calls == 1U);
    assert(rep_calls == 1U);

    process_at(100U, &next_ms);
    assert(one_calls == 1U);
    assert(rep_calls == 2U);

    tinyui_timer_delete(one_shot);
    tinyui_timer_delete(repeat);
    tinyui_deinit();
}

static void test_callback_delete_and_stop_self(void)
{
    tinyui_timer_t *del_timer;
    tinyui_timer_t *stop_timer;
    unsigned int del_calls = 0U;
    unsigned int stop_calls = 0U;
    uint32_t next_ms = 0U;

    assert(tinyui_init() == TINYUI_OK);
    install_fake_clock();

    del_timer = tinyui_timer_create(20U, true, delete_self_cb, &del_calls);
    stop_timer = tinyui_timer_create(20U, true, stop_self_cb, &stop_calls);
    assert(del_timer != NULL);
    assert(stop_timer != NULL);
    assert(tinyui_timer_start(del_timer) == TINYUI_OK);
    assert(tinyui_timer_start(stop_timer) == TINYUI_OK);

    process_at(20U, &next_ms);
    assert(del_calls == 1U);
    assert(stop_calls == 1U);

    process_at(40U, &next_ms);
    assert(del_calls == 1U);
    assert(stop_calls == 1U);

    tinyui_timer_delete(stop_timer);
    tinyui_deinit();
}

static void test_create_in_callback_not_same_round(void)
{
    tinyui_timer_t *parent;
    struct create_in_cb_ctx ctx;
    uint32_t next_ms = 0U;

    memset(&ctx, 0, sizeof(ctx));
    assert(tinyui_init() == TINYUI_OK);
    install_fake_clock();

    parent = tinyui_timer_create(10U, false, create_timer_in_cb, &ctx);
    assert(parent != NULL);
    assert(tinyui_timer_start(parent) == TINYUI_OK);

    process_at(10U, &next_ms);
    assert(ctx.calls == 1U);
    assert(ctx.created != NULL);
    assert(ctx.child_calls == 0U);
    /* 同轮新建且已到期：不触发，但 next_ms 为 0 要求立刻再 process */
    assert(next_ms == 0U);

    process_at(10U, &next_ms);
    assert(ctx.child_calls == 1U);

    tinyui_timer_delete(parent);
    tinyui_timer_delete(ctx.created);
    tinyui_deinit();
}

static void test_wraparound_deadline(void)
{
    tinyui_timer_t *timer;
    unsigned int calls = 0U;
    uint32_t next_ms = 0U;

    assert(tinyui_init() == TINYUI_OK);
    install_fake_clock();

    /* Arm at now = UINT32_MAX - 4, interval 7 → deadline wraps to 3. */
    set_now(UINT32_MAX - 4U);
    timer = tinyui_timer_create(7U, false, count_cb, &calls);
    assert(timer != NULL);
    assert(tinyui_timer_start(timer) == TINYUI_OK);

    process_at(UINT32_MAX - 4U, &next_ms);
    assert(calls == 0U);
    assert(next_ms == 7U);

    process_at(UINT32_MAX - 1U, &next_ms);
    assert(calls == 0U);
    assert(next_ms == 4U);

    process_at(3U, &next_ms);
    assert(calls == 1U);

    process_at(10U, &next_ms);
    assert(calls == 1U);

    tinyui_timer_delete(timer);
    tinyui_deinit();
}

static void test_next_ms_deadline_semantics(void)
{
    tinyui_timer_t *timer;
    tinyui_timer_t *parent;
    struct create_in_cb_ctx ctx;
    unsigned int calls = 0U;
    uint32_t next_ms = 0U;

    assert(tinyui_init() == TINYUI_OK);
    install_fake_clock();

    /* 无 deadline → UINT32_MAX */
    process_at(0U, &next_ms);
    assert(next_ms == UINT32_MAX);

    timer = tinyui_timer_create(100U, false, count_cb, &calls);
    assert(timer != NULL);
    assert(tinyui_timer_start(timer) == TINYUI_OK);

    /* 剩余毫秒 */
    process_at(0U, &next_ms);
    assert(next_ms == 100U);

    process_at(40U, &next_ms);
    assert(next_ms == 60U);
    assert(calls == 0U);

    process_at(100U, &next_ms);
    assert(calls == 1U);
    /* one-shot 触发后停止 → 无 deadline */
    assert(next_ms == UINT32_MAX);

    tinyui_timer_delete(timer);

    /* repeat 触发后从 now+interval 重排，返回剩余 */
    timer = tinyui_timer_create(25U, true, count_cb, &calls);
    assert(timer != NULL);
    calls = 0U;
    set_now(1000U);
    assert(tinyui_timer_start(timer) == TINYUI_OK);
    process_at(1025U, &next_ms);
    assert(calls == 1U);
    assert(next_ms == 25U);
    tinyui_timer_delete(timer);

    /* 回调中 create+start 的槽 born_epoch 阻塞本轮触发，但已到期 → next_ms=0 */
    memset(&ctx, 0, sizeof(ctx));
    parent = tinyui_timer_create(1U, false, create_timer_in_cb, &ctx);
    assert(parent != NULL);
    set_now(2000U);
    assert(tinyui_timer_start(parent) == TINYUI_OK);
    process_at(2001U, &next_ms);
    assert(ctx.created != NULL);
    assert(ctx.child_calls == 0U);
    assert(next_ms == 0U);

    process_at(2001U, &next_ms);
    assert(ctx.child_calls == 1U);

    tinyui_timer_delete(parent);
    tinyui_timer_delete(ctx.created);
    tinyui_deinit();
}

static void test_process_null_next_ms_and_zero_alloc(void)
{
    tinyui_timer_t *timer;
    unsigned int calls = 0U;
    uint32_t next_ms = 0U;
    struct tinyui_test_allocator_stats before;
    struct tinyui_test_allocator_stats after;

    assert(tinyui_init() == TINYUI_OK);
    install_fake_clock();

    timer = tinyui_timer_create(30U, true, count_cb, &calls);
    assert(timer != NULL);
    assert(tinyui_timer_start(timer) == TINYUI_OK);

    /* next_ms == NULL is legal (Task 3). */
    set_now(0U);
    assert(tinyui_process(NULL) == TINYUI_OK);

    set_now(30U);
    tinyui_test_allocator_reset();
    before = tinyui_test_allocator_snapshot();
    assert(tinyui_process(&next_ms) == TINYUI_OK);
    after = tinyui_test_allocator_snapshot();
    assert_zero_alloc_delta(&before, &after);
    assert(calls == 1U);

    tinyui_timer_delete(timer);
    tinyui_deinit();
}

static void test_set_interval_and_invalid_args(void)
{
    tinyui_timer_t *timer;
    unsigned int calls = 0U;
    uint32_t next_ms = 0U;

    assert(tinyui_init() == TINYUI_OK);
    install_fake_clock();

    assert(tinyui_timer_create(0U, true, count_cb, NULL) == NULL);
    assert(tinyui_last_result() == TINYUI_ERROR_INVALID_ARG);
    assert(tinyui_timer_create(10U, true, NULL, NULL) == NULL);
    assert(tinyui_last_result() == TINYUI_ERROR_INVALID_ARG);
    assert(tinyui_timer_start(NULL) == TINYUI_ERROR_INVALID_ARG);
    assert(tinyui_timer_stop(NULL) == TINYUI_ERROR_INVALID_ARG);
    assert(tinyui_timer_set_interval(NULL, 10U) == TINYUI_ERROR_INVALID_ARG);
    tinyui_timer_delete(NULL);

    timer = tinyui_timer_create(100U, false, count_cb, &calls);
    assert(timer != NULL);
    assert(tinyui_timer_set_interval(timer, 0U) == TINYUI_ERROR_INVALID_ARG);
    assert(tinyui_timer_set_interval(timer, 40U) == TINYUI_OK);
    assert(tinyui_timer_start(timer) == TINYUI_OK);

    process_at(0U, &next_ms);
    assert(next_ms == 40U);
    process_at(40U, &next_ms);
    assert(calls == 1U);

    tinyui_timer_delete(timer);
    tinyui_deinit();
}

int main(void)
{
    test_timer_pool_capacity_and_reuse();
    test_one_shot_and_repeat();
    test_callback_delete_and_stop_self();
    test_create_in_callback_not_same_round();
    test_wraparound_deadline();
    test_next_ms_deadline_semantics();
    test_process_null_next_ms_and_zero_alloc();
    test_set_interval_and_invalid_args();
    return 0;
}
