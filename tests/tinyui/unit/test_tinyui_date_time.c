/*
 * TinyUI date_time unit tests — M3 Task 5 L3/L4 harness.
 */

#include "tinyui.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldDateTime.h"
#include "internal.h"
#include "widgets/date_time.h"

#include <assert.h>
#include <string.h>

static unsigned int test_rgb_to_ld_color(unsigned int rgb)
{
    return (unsigned int)__RGB((rgb >> 16) & 0xFFU, (rgb >> 8) & 0xFFU, rgb & 0xFFU);
}

static void test_date_time_create_and_ld_mapping(tinyui_obj_t *root)
{
    tinyui_obj_t *dt = tinyui_date_time_create(root);
    struct tinyui_widget *backend;
    ldDateTime_t *ld_dt;

    assert(dt != 0);
    backend = (struct tinyui_widget *)(void *)dt;
    assert(backend->kind == TINYUI_BACKEND_WIDGET_DATE_TIME);
    ld_dt = (ldDateTime_t *)backend->ld_widget;
    assert(ld_dt != 0);
    assert(((ldBase_t *)ld_dt)->widgetType == widgetTypeDateTime);
}

static void test_date_time_explicit_values_and_system_time(tinyui_obj_t *root)
{
    tinyui_obj_t *dt = tinyui_date_time_create(root);
    ldDateTime_t *ld_dt;
    int y = 0, m = 0, d = 0, h = 0, mi = 0, s = 0;

    assert(dt != 0);
    ld_dt = (ldDateTime_t *)((struct tinyui_widget *)(void *)dt)->ld_widget;
    assert(ld_dt != 0);

    assert(tinyui_date_time_set_format(dt, "YYYY-MM-DD") == 0);
    assert(strcmp((const char *)ld_dt->formatStr, "YYYY-MM-DD") == 0);
    assert(strcmp(tinyui_date_time_get_format(dt), "YYYY-MM-DD") == 0);

    assert(tinyui_date_time_set_date(dt, 2026, 7, 15) == 0);
    assert(ld_dt->year == 2026);
    assert(ld_dt->month == 7);
    assert(ld_dt->day == 15);
    assert(ld_dt->isAutoSysTime == 0);
    assert(tinyui_date_time_get_date(dt, &y, &m, &d) == 0);
    assert(y == 2026 && m == 7 && d == 15);

    assert(tinyui_date_time_set_time(dt, 13, 45, 9) == 0);
    assert(ld_dt->hour == 13 && ld_dt->minute == 45 && ld_dt->second == 9);
    assert(tinyui_date_time_get_time(dt, &h, &mi, &s) == 0);
    assert(h == 13 && mi == 45 && s == 9);

    assert(tinyui_date_time_set_text_color(dt, 0x112233U) == 0);
    assert(ld_dt->textColor == (ldColor)test_rgb_to_ld_color(0x112233U));
    assert(tinyui_date_time_set_background_color(dt, 0xAABBCCU) == 0);
    assert(ld_dt->bgColor == (ldColor)test_rgb_to_ld_color(0xAABBCCU));

    assert(tinyui_date_time_set_align(dt, TINYUI_ALIGN_CENTER) == 0);
    assert(tinyui_date_time_set_transparent(dt, 1) == 0);
    assert(ld_dt->isTransparent == 1);
    assert(tinyui_date_time_get_transparent(dt) == 1);

    assert(tinyui_date_time_set_use_system_time(dt, 1) == 0);
    assert(ld_dt->isAutoSysTime == 1);
    assert(tinyui_date_time_get_use_system_time(dt) == 1);
    assert(tinyui_date_time_set_use_system_time(dt, 0) == 0);
    assert(ld_dt->isAutoSysTime == 0);
}

static void test_date_time_rejects_invalid(tinyui_obj_t *root)
{
    tinyui_obj_t *dt = tinyui_date_time_create(root);
    assert(dt != 0);
    assert(tinyui_date_time_create(0) == 0);
    assert(tinyui_date_time_set_format(0, "x") == -1);
    assert(tinyui_date_time_set_date(dt, 2026, 13, 1) == -1);
    assert(tinyui_date_time_set_time(dt, 25, 0, 0) == -1);
}

int main(void)
{
    tinyui_obj_t *root;

    tinyui_deinit();
    assert(tinyui_init() == TINYUI_OK);
    root = tinyui_screen_create();
    assert(root != 0);

    test_date_time_create_and_ld_mapping(root);
    test_date_time_explicit_values_and_system_time(root);
    test_date_time_rejects_invalid(root);

    tinyui_deinit();
    return 0;
}
