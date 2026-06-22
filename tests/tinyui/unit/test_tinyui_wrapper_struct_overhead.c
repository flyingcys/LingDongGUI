#include "internal.h"

#include <stdio.h>

int main(void)
{
    printf("TINYUI_BENCHMARK_WIDGET_WRAPPER_STRUCT_BYTES=%zu\n", sizeof(struct tinyui_widget));
    printf("TINYUI_BENCHMARK_SWITCH_WRAPPER_STRUCT_DELTA_BYTES=%zu\n",
           sizeof(struct tinyui_switch) - sizeof(struct tinyui_widget));
    /* C3-T4: the backend_widget_struct_bytes metric is gone — the
     * struct tinyui_backend_widget type has been deleted. */
    return 0;
}
