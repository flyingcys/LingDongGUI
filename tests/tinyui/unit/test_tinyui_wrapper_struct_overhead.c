#include "internal.h"

#include <stdio.h>

int main(void)
{
    printf("TINYUI_BENCHMARK_WIDGET_WRAPPER_STRUCT_BYTES=%zu\n", sizeof(struct tinyui_widget));
    printf("TINYUI_BENCHMARK_SWITCH_WRAPPER_STRUCT_DELTA_BYTES=%zu\n",
           sizeof(struct tinyui_switch) - sizeof(struct tinyui_widget));
    printf("TINYUI_BENCHMARK_BACKEND_WIDGET_STRUCT_BYTES=%zu\n",
           sizeof(struct tinyui_backend_widget));
    return 0;
}
