/* tinyui/port/sdl/observe.c
 * CI / 测试基础设施 — 与生产渲染路径零耦合。
 * 包含：环境变量标志、widget 分类谓词、printf 日志、PPM capture。
 */
#include "host_internal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---- 仅 observe.c 内部使用的 static 函数 ---- */

static int tinyui_runtime_host_benchmark_log_enabled(void)
{
    static int initialized = 0;
    static int enabled = 0;

    if (!initialized) {
        const char *env = getenv("TINYUI_BENCHMARK_LOG");
        enabled = (env != NULL && env[0] != '\0' && env[0] != '0') ? 1 : 0;
        initialized = 1;
    }

    return enabled;
}

static int tinyui_runtime_host_widget_is_supported_real(const struct tinyui_backend_widget *widget)
{
    if (widget == NULL) {
        return 1;
    }

    switch (widget->kind) {
    case TINYUI_BACKEND_WIDGET_BACKGROUND:
    case TINYUI_BACKEND_WIDGET_WINDOW:
    case TINYUI_BACKEND_WIDGET_LABEL:
    case TINYUI_BACKEND_WIDGET_BUTTON:
    case TINYUI_BACKEND_WIDGET_CHECKBOX:
    case TINYUI_BACKEND_WIDGET_TEXT:
    case TINYUI_BACKEND_WIDGET_IMAGE:
    case TINYUI_BACKEND_WIDGET_SWITCH:
    case TINYUI_BACKEND_WIDGET_SLIDER:
    case TINYUI_BACKEND_WIDGET_ARC:
    case TINYUI_BACKEND_WIDGET_GAUGE:
    case TINYUI_BACKEND_WIDGET_ICON_SLIDER:
    case TINYUI_BACKEND_WIDGET_RADIAL_MENU:
    case TINYUI_BACKEND_WIDGET_PROGRESS_BAR:
    case TINYUI_BACKEND_WIDGET_QRCODE:
    case TINYUI_BACKEND_WIDGET_PROGRESS_WHEEL:
    case TINYUI_BACKEND_WIDGET_ANIMATION:
    case TINYUI_BACKEND_WIDGET_LIST:
    case TINYUI_BACKEND_WIDGET_COMBO_BOX:
    case TINYUI_BACKEND_WIDGET_SCROLL_SELECTER:
    case TINYUI_BACKEND_WIDGET_TABLE:
    case TINYUI_BACKEND_WIDGET_GRAPH:
    case TINYUI_BACKEND_WIDGET_CALENDAR:
    case TINYUI_BACKEND_WIDGET_DATE_TIME:
    case TINYUI_BACKEND_WIDGET_MESSAGE_BOX:
    case TINYUI_BACKEND_WIDGET_CLOCK:
    case TINYUI_BACKEND_WIDGET_KEYBOARD:
        return 1;
    default:
        return 0;
    }
}

static int tinyui_runtime_host_widget_is_real_mapped(const struct tinyui_backend_widget *widget)
{
    return widget != NULL &&
           widget->kind != TINYUI_BACKEND_WIDGET_WINDOW &&
           widget->kind != TINYUI_BACKEND_WIDGET_BACKGROUND &&
           tinyui_runtime_host_widget_is_supported_real(widget) &&
           widget->ld_widget != NULL;
}

static void tinyui_runtime_host_append_id(const char *id,
                                     char *buffer,
                                     size_t buffer_size,
                                     size_t *used)
{
    int written;

    if (id == NULL || id[0] == '\0') {
        return;
    }

    if (*used > 0 && *used + 1 < buffer_size) {
        buffer[*used] = ',';
        *used += 1;
        buffer[*used] = '\0';
    }

    if (*used + 1 >= buffer_size) {
        return;
    }

    written = snprintf(buffer + *used, buffer_size - *used, "%s", id);
    if (written > 0) {
        size_t advance = (size_t)written;
        if (advance >= buffer_size - *used) {
            *used = buffer_size - 1;
        } else {
            *used += advance;
        }
    }
}

static int tinyui_runtime_host_widget_needs_fallback(const struct tinyui_backend_widget *widget)
{
    return widget != NULL &&
           widget->kind != TINYUI_BACKEND_WIDGET_WINDOW &&
           widget->kind != TINYUI_BACKEND_WIDGET_BACKGROUND &&
           (!tinyui_runtime_host_widget_is_supported_real(widget) || widget->ld_widget == NULL);
}

static int tinyui_runtime_host_widget_excludes_formal_mapping(const struct tinyui_backend_widget *widget)
{
    while (widget != NULL) {
        if ((widget->runtime_evidence_flags & TINYUI_BACKEND_EVIDENCE_EXCLUDE_FORMAL_MAPPING) != 0U) {
            return 1;
        }
        if (widget->first_child != NULL && tinyui_runtime_host_widget_excludes_formal_mapping(widget->first_child)) {
            return 1;
        }
        widget = widget->next_sibling;
    }
    return 0;
}

static void tinyui_runtime_host_append_widget_ids(const struct tinyui_backend_widget *widget,
                                             int (*predicate)(const struct tinyui_backend_widget *widget),
                                             char *buffer,
                                             size_t buffer_size,
                                             size_t *used)
{
    while (widget != NULL) {
        if (predicate(widget) && widget->id != NULL && widget->id[0] != '\0') {
            tinyui_runtime_host_append_id(widget->id, buffer, buffer_size, used);
        }

        if (widget->first_child != NULL) {
            tinyui_runtime_host_append_widget_ids(widget->first_child,
                                             predicate,
                                             buffer,
                                             buffer_size,
                                             used);
        }

        widget = widget->next_sibling;
    }
}

/* ---- 外部可见函数（在 host_internal.h 中已声明）---- */

int tinyui_runtime_host_touch_log_enabled(void)
{
    static int initialized = 0;
    static int enabled = 0;

    if (!initialized) {
        const char *env = getenv("TINYUI_TOUCH_LOG");
        enabled = (env != NULL && env[0] != '\0' && env[0] != '0') ? 1 : 0;
        initialized = 1;
    }

    return enabled;
}

void tinyui_runtime_host_log_screen_create_benchmark(struct tinyui_runtime_host_state *state)
{
    double elapsed_ms;

    if (state == NULL || state->benchmark_screen_create_logged ||
        !tinyui_runtime_host_benchmark_log_enabled()) {
        return;
    }

    if (state->screen_create_end_ticks < state->screen_create_start_ticks) {
        return;
    }

    elapsed_ms = (double)(state->screen_create_end_ticks - state->screen_create_start_ticks);
    printf("TINYUI_BENCHMARK_SCREEN_OBJECT_CREATE_MS=%.3f\n", elapsed_ms);
    printf("TINYUI_BENCHMARK_SCREEN_CREATE_MS=%.3f\n", elapsed_ms);
    fflush(stdout);
    state->benchmark_screen_create_logged = 1;
}

void tinyui_runtime_host_log_first_frame_benchmark(struct tinyui_runtime_host_state *state)
{
    double elapsed_ms;

    if (state == NULL || state->benchmark_first_frame_logged ||
        !tinyui_runtime_host_benchmark_log_enabled()) {
        return;
    }

    if (state->capture_written == 0) {
        return;
    }

    elapsed_ms = (double)(SDL_GetTicks() - state->start_ticks);
    printf("TINYUI_BENCHMARK_CAPTURE_READY_MS=%.3f\n", elapsed_ms);
    printf("TINYUI_BENCHMARK_FIRST_FRAME_MS=%.3f\n", elapsed_ms);
    fflush(stdout);
    state->benchmark_first_frame_logged = 1;
}

int tinyui_runtime_host_window_has_real_layout(const struct tinyui_backend_widget *widget)
{
    ldWindow_t *ld_window;

    if (widget == NULL
        || (widget->kind != TINYUI_BACKEND_WIDGET_WINDOW
            && widget->kind != TINYUI_BACKEND_WIDGET_BACKGROUND)
        || widget->ld_widget == NULL) {
        return 0;
    }

    ld_window = (ldWindow_t *)widget->ld_widget;
    return ld_window->layoutTpye == layoutFlex || ld_window->layoutTpye == layoutGrid;
}

int tinyui_runtime_host_widget_allows_smoke_layout(const struct tinyui_backend_widget *widget)
{
    while (widget != NULL) {
        if ((widget->runtime_evidence_flags & TINYUI_BACKEND_EVIDENCE_ALLOW_SMOKE_LAYOUT) != 0U) {
            return 1;
        }
        if (widget->first_child != NULL && tinyui_runtime_host_widget_allows_smoke_layout(widget->first_child)) {
            return 1;
        }
        widget = widget->next_sibling;
    }
    return 0;
}

void tinyui_runtime_host_log_mapping_markers(struct tinyui_runtime_host_state *state,
                                               const struct tinyui_backend_widget *root)
{
    char real_ids[256] = {0};
    char fallback_ids[256] = {0};
    size_t real_used = 0;
    size_t fallback_used = 0;

    if (state == NULL || root == NULL) {
        return;
    }

    tinyui_runtime_host_append_widget_ids(root->first_child,
                                     tinyui_runtime_host_widget_is_real_mapped,
                                     real_ids,
                                     sizeof(real_ids),
                                     &real_used);
    tinyui_runtime_host_append_widget_ids(root->first_child,
                                     tinyui_runtime_host_widget_needs_fallback,
                                     fallback_ids,
                                     sizeof(fallback_ids),
                                     &fallback_used);

    if (real_used > 0 &&
        !tinyui_runtime_host_widget_excludes_formal_mapping(root->first_child) &&
        !state->static_mapping_logged) {
        printf("TINYUI_BACKEND_STATIC_MAPPING=REAL_LDGUI\n");
        printf("TINYUI_BACKEND_REAL_WIDGET_IDS=%s\n", real_ids);
        fflush(stdout);
        state->static_mapping_logged = 1;
    }

    if (tinyui_runtime_host_widget_excludes_formal_mapping(root->first_child) &&
        !state->temporary_smoke_logged) {
        printf("TINYUI_BACKEND_TEMPORARY_SMOKE_PATH=EXCLUDED_FORMAL_MAPPING\n");
        fflush(stdout);
        state->temporary_smoke_logged = 1;
    }

    if (fallback_used > 0 && !state->fallback_boundary_logged) {
        printf("TINYUI_BACKEND_INTERACTIVE_BOUNDARY=FAKE_FALLBACK\n");
        printf("TINYUI_BACKEND_FALLBACK_WIDGET_IDS=%s\n", fallback_ids);
        fflush(stdout);
        state->fallback_boundary_logged = 1;
    }
}

void tinyui_runtime_host_log_image_source_marker(const struct tinyui_backend_widget *widget)
{
    while (widget != NULL) {
        if (widget->kind == TINYUI_BACKEND_WIDGET_IMAGE && widget->id != NULL && widget->ld_widget != NULL) {
            ldImage_t *ld_image = (ldImage_t *)widget->ld_widget;

            printf("TINYUI_BACKEND_IMAGE_SOURCE=%s:img=%s,mask=%s\n",
                   widget->id,
                   ld_image->ptImgTile != NULL ? "set" : "null",
                   ld_image->ptMaskTile != NULL ? "set" : "null");
        }

        if (widget->first_child != NULL) {
            tinyui_runtime_host_log_image_source_marker(widget->first_child);
        }

        widget = widget->next_sibling;
    }
}

Uint32 tinyui_runtime_host_parse_auto_quit_ms(void)
{
    const char *value = getenv("TINYUI_DEMO_AUTO_QUIT_MS");
    char *end = NULL;
    unsigned long parsed;

    if (value == NULL || value[0] == '\0') {
        return 0U;
    }

    parsed = strtoul(value, &end, 10);
    if (end == value || (end != NULL && *end != '\0')) {
        return 0U;
    }

    if (parsed > 60000UL) {
        parsed = 60000UL;
    }
    return (Uint32)parsed;
}

int tinyui_runtime_host_write_capture(struct tinyui_runtime_host_state *state)
{
    const char *path = getenv("TINYUI_CAPTURE_FILE");
    FILE *fp;
    int x;
    int y;

    if (state == NULL || state->real_pixels == NULL || state->capture_written) {
        return 0;
    }

    if (path == NULL || path[0] == '\0') {
        return 0;
    }

    fp = fopen(path, "wb");
    if (fp == NULL) {
        return -1;
    }

    fprintf(fp, "P6\n%d %d\n255\n", state->display_width, state->display_height);
    for (y = 0; y < state->display_height; ++y) {
        for (x = 0; x < state->display_width; ++x) {
            COLOUR_INT pixel =
                state->real_pixels[(size_t)y * (size_t)state->display_width + (size_t)x];
            uint32_t rgb888 = tinyui_runtime_host_pixel_to_rgb888(pixel);
            unsigned char rgb[3];

            rgb[0] = (unsigned char)((rgb888 >> 16) & 0xFFU);
            rgb[1] = (unsigned char)((rgb888 >> 8) & 0xFFU);
            rgb[2] = (unsigned char)(rgb888 & 0xFFU);
            fwrite(rgb, 1, 3, fp);
        }
    }

    fclose(fp);
    state->capture_written = 1;
    tinyui_runtime_host_log_first_frame_benchmark(state);
    return 0;
}

void tinyui_runtime_host_log_runtime_ready(struct tinyui_app *app,
                                             struct tinyui_runtime_host_state *state)
{
    if (app == NULL || state == NULL || state->ready_logged) {
        return;
    }

    if (app->focus_owner == NULL) {
        printf("TINYUI_FOCUS_RUNTIME_READY=1\n");
        fflush(stdout);
    }
    printf("TINYUI_RUNTIME_READY\n");
    fflush(stdout);
    state->ready_logged = 1;
    state->screen_create_end_ticks = SDL_GetTicks();
    tinyui_runtime_host_log_screen_create_benchmark(state);
}

void tinyui_runtime_host_log_smoke_layout_marker(struct tinyui_runtime_host_state *state)
{
    if (state == NULL || state->smoke_layout_marker_logged) {
        return;
    }
    printf("TINYUI_SMOKE_LAYOUT_USED=%d\n", state->smoke_layout_used ? 1 : 0);
    fflush(stdout);
    state->smoke_layout_marker_logged = 1;
}
