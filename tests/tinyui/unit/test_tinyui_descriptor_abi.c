#include "internal.h"
#include "resource/font.h"
#include "resource/image_source.h"

#include <stdio.h>
#include <stdint.h>

/* arm_2d_tile_t is the private tile storage target for image/mask descriptors. */
#include "arm_2d.h"

static int assert_size_le(const char *name, size_t actual, size_t limit)
{
    if (actual > limit) {
        fprintf(stderr,
                "%s size regression: expected <= %zu, got %zu\n",
                name,
                limit,
                actual);
        return 1;
    }
    return 0;
}

static int assert_size_ge(const char *name, size_t actual, size_t limit)
{
    if (actual < limit) {
        fprintf(stderr,
                "%s size regression: expected >= %zu, got %zu\n",
                name,
                limit,
                actual);
        return 1;
    }
    return 0;
}

int main(void)
{
    int failed = 0;
    size_t image_source_bytes = sizeof(struct tinyui_image_source);
    size_t font_bytes = sizeof(struct tinyui_font);
    size_t image_private_bytes = sizeof(((struct tinyui_image_source *)0)->_image_private);
    size_t mask_private_bytes = sizeof(((struct tinyui_image_source *)0)->_mask_private);

    printf("TINYUI_DESC_IMAGE_SOURCE_BYTES=%zu\n", image_source_bytes);
    printf("TINYUI_DESC_FONT_BYTES=%zu\n", font_bytes);
    printf("TINYUI_DESC_IMAGE_PRIVATE_BYTES=%zu\n", image_private_bytes);
    printf("TINYUI_DESC_MASK_PRIVATE_BYTES=%zu\n", mask_private_bytes);
    printf("TINYUI_DESC_ARM_2D_TILE_BYTES=%zu\n", sizeof(arm_2d_tile_t));

    /* Private tile storage must hold one arm_2d_tile_t on every host ABI. */
    failed |= assert_size_ge("image private storage",
                             image_private_bytes,
                             sizeof(arm_2d_tile_t));
    failed |= assert_size_ge("mask private storage",
                             mask_private_bytes,
                             sizeof(arm_2d_tile_t));
    failed |= assert_size_ge("image private storage",
                             image_private_bytes,
                             6U * sizeof(uintptr_t));
    failed |= assert_size_ge("mask private storage",
                             mask_private_bytes,
                             6U * sizeof(uintptr_t));

    /*
     * 32-bit hard descriptor budgets are only enforceable on a 32-bit ABI.
     * Host 64-bit sizes are printed for diagnostics only.
     */
#if defined(UINTPTR_MAX) && defined(UINT32_MAX) && (UINTPTR_MAX == UINT32_MAX)
    failed |= assert_size_le("32-bit image source descriptor",
                             image_source_bytes,
                             80U);
    failed |= assert_size_le("32-bit font descriptor", font_bytes, 16U);
    printf("TINYUI_ABI32_DESC_BUDGET_OK image=%zu font=%zu\n",
           image_source_bytes,
           font_bytes);
#else
    printf("TINYUI_ABI32_DESC_BUDGET_HOST_ONLY image=%zu font=%zu (not 32-bit ABI)\n",
           image_source_bytes,
           font_bytes);
#endif

    return failed;
}
