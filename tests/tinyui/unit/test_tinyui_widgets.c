/*
 * TinyUI unit tests — M2 Task 2 object/text focus harness.
 *
 * Historical parity suite pre-dates M1 1-arg create / resource value types and is
 * temporarily reduced to the Task 2 text-copy contract so TDD can proceed on the
 * common text ownership path. Broader widget parity re-entry is M2 Task 5+.
 */

#include "tinyui.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldLabel.h"
#include "internal.h"
#include "widgets/label.h"
#include "widgets/window.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

static int g_v23_fail_next_alloc = 0;
static size_t g_v23_live_blocks = 0;

void *ldMalloc(uint32_t size)
{
    void *ptr;

    if (g_v23_fail_next_alloc) {
        g_v23_fail_next_alloc = 0;
        return 0;
    }
    ptr = malloc((size_t)size);
    if (ptr != 0) {
        g_v23_live_blocks += 1U;
    }
    return ptr;
}

void *ldCalloc(uint32_t num, uint32_t size)
{
    void *ptr;

    if (g_v23_fail_next_alloc) {
        g_v23_fail_next_alloc = 0;
        return 0;
    }
    ptr = calloc((size_t)num, (size_t)size);
    if (ptr != 0) {
        g_v23_live_blocks += 1U;
    }
    return ptr;
}

void *ldRealloc(void *ptr, uint32_t size)
{
    if (ptr == 0) {
        return ldMalloc(size);
    }
    if (g_v23_fail_next_alloc) {
        g_v23_fail_next_alloc = 0;
        return 0;
    }
    return realloc(ptr, (size_t)size);
}

void ldFree(void *ptr)
{
    if (ptr == 0) {
        return;
    }
    if (g_v23_live_blocks > 0U) {
        g_v23_live_blocks -= 1U;
    }
    free(ptr);
}

/* M2 Task 2: text setter copies storage; OOM fail-closed; delete frees copy. */
static void test_v23_text_copy_and_oom_atomicity(void)
{
    tinyui_obj_t *root;
    tinyui_obj_t *label_obj;
    struct tinyui_label *label;
    ldLabel_t *ld_label;
    char buf[32];
    size_t live_after_create;
    size_t live_after_set;
    size_t live_after_replace;
    size_t live_after_delete;
    const char *got;

    tinyui_deinit();
    assert(tinyui_init() == TINYUI_OK);
    root = tinyui_screen_create();
    assert(root != 0);
    label_obj = tinyui_label_create(root);
    assert(label_obj != 0);
    label = (struct tinyui_label *)(void *)label_obj;
    ld_label = (ldLabel_t *)label->widget.ld_widget;
    assert(ld_label != 0);

    live_after_create = g_v23_live_blocks;

    memcpy(buf, "copy-me", 8);
    assert(tinyui_label_set_text(label_obj, buf) == 0);
    got = tinyui_label_get_text(label_obj);
    assert(got != 0);
    assert(strcmp(got, "copy-me") == 0);
    assert(got != buf);
    assert(ldLabelGetText(ld_label) != 0);
    assert(strcmp((const char *)ldLabelGetText(ld_label), "copy-me") == 0);

    memcpy(buf, "mutated!", 9);
    assert(strcmp(tinyui_label_get_text(label_obj), "copy-me") == 0);
    assert(strcmp((const char *)ldLabelGetText(ld_label), "copy-me") == 0);

    live_after_set = g_v23_live_blocks;
    assert(live_after_set > live_after_create);

    assert(tinyui_label_set_text(label_obj, "old-text") == 0);
    live_after_replace = g_v23_live_blocks;

    g_v23_fail_next_alloc = 1;
    assert(tinyui_label_set_text(label_obj, "new-text") != 0);
    assert(g_v23_fail_next_alloc == 0);
    assert(strcmp(tinyui_label_get_text(label_obj), "old-text") == 0);
    assert(strcmp((const char *)ldLabelGetText(ld_label), "old-text") == 0);
    assert(g_v23_live_blocks == live_after_replace);

    assert(tinyui_label_set_text(label_obj, "final-text") == 0);
    assert(strcmp(tinyui_label_get_text(label_obj), "final-text") == 0);

    assert(tinyui_obj_delete(label_obj) == TINYUI_OK);
    live_after_delete = g_v23_live_blocks;
    assert(live_after_delete <= live_after_create);

    tinyui_deinit();
    assert(g_v23_live_blocks == 0U);
}

int main(void)
{
    test_v23_text_copy_and_oom_atomicity();
    return 0;
}
