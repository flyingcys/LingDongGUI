#include "internal.h"

#include <stdio.h>

int main(void)
{
    printf("PICOUI_BENCHMARK_WIDGET_WRAPPER_STRUCT_BYTES=%zu\n", sizeof(struct picoui_widget));
    printf("PICOUI_BENCHMARK_SWITCH_WRAPPER_STRUCT_DELTA_BYTES=%zu\n",
           sizeof(struct picoui_switch) - sizeof(struct picoui_widget));
    printf("PICOUI_BENCHMARK_BACKEND_WIDGET_STRUCT_BYTES=%zu\n",
           sizeof(struct picoui_backend_widget));
    return 0;
}
