#include "ldgui_test_support.h"

#include "arm_2d.h"

static int64_t g_ld_test_timestamp;

bool arm_2d_op_wait_async(arm_2d_op_core_t *ptOP)
{
    (void)ptOP;
    return true;
}

void VT_enter_global_mutex(void)
{
}

void VT_leave_global_mutex(void)
{
}

int64_t arm_2d_helper_get_system_timestamp(void)
{
    return g_ld_test_timestamp;
}

uint32_t arm_2d_helper_get_reference_clock_frequency(void)
{
    return 1000000u;
}

void ld_test_set_system_timestamp(int64_t timestamp)
{
    g_ld_test_timestamp = timestamp;
}

void ldGuiUpdateScene(void)
{
}
