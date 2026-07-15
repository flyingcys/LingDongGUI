/* tests/tinyui/runtime/tinyui_sdl_observe.c
 * CI / 测试基础设施 — 仅在 ENABLE_TEST 下编入 tinyui_port_sdl。
 * 由 hal.c 经 tinyui_sdl_observe.h 的钩子宏调用;生产 port 不含这些符号。
 * 包含：环境变量标志、widget 分类谓词、printf marker、PPM capture、
 * 以及 on_setup/on_present/should_quit 观测适配。
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


/* ---- 外部可见函数（在 host_internal.h 中已声明）----
 * 注:tinyui_runtime_host_touch_log_enabled 已移至 hal.c(生产事件泵使用)。 */

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

/* Read the user-visible id of a widget. Phase C folded the backend `id` mirror
 * out of struct tinyui_widget; the id now lives on each concrete widget sub-
 * struct (e.g. struct tinyui_image) as the first member after `widget`.
 * Dispatch by kind so the read stays type-safe and does not rely on layout
 * coincidence across 28 different sub-structs. Returns NULL for window/
 * background (window carries its own id but is a container, not a real leaf)
 * and for any widget sub-struct that has no id field. */
static const char *tinyui_runtime_host_widget_id(const struct tinyui_widget *w)
{
    if (w == NULL) {
        return NULL;
    }

    switch (w->kind) {
    case TINYUI_BACKEND_WIDGET_LABEL:      return ((const struct tinyui_label *)w)->id;
    case TINYUI_BACKEND_WIDGET_BUTTON:     return ((const struct tinyui_button *)w)->id;
    case TINYUI_BACKEND_WIDGET_CHECKBOX:   return ((const struct tinyui_checkbox *)w)->id;
    case TINYUI_BACKEND_WIDGET_SWITCH:     return ((const struct tinyui_switch *)w)->id;
    case TINYUI_BACKEND_WIDGET_SLIDER:     return ((const struct tinyui_slider *)w)->id;
    case TINYUI_BACKEND_WIDGET_LINE_EDIT:  return ((const struct tinyui_line_edit *)w)->id;
    case TINYUI_BACKEND_WIDGET_ARC:        return ((const struct tinyui_arc *)w)->id;
    case TINYUI_BACKEND_WIDGET_GAUGE:      return ((const struct tinyui_gauge *)w)->id;
    case TINYUI_BACKEND_WIDGET_ICON_SLIDER:return ((const struct tinyui_icon_slider *)w)->id;
    case TINYUI_BACKEND_WIDGET_RADIAL_MENU:return ((const struct tinyui_radial_menu *)w)->id;
    case TINYUI_BACKEND_WIDGET_PROGRESS_BAR:return ((const struct tinyui_progress_bar *)w)->id;
    case TINYUI_BACKEND_WIDGET_QRCODE:     return ((const struct tinyui_qrcode *)w)->id;
    case TINYUI_BACKEND_WIDGET_PROGRESS_WHEEL:return ((const struct tinyui_progress_wheel *)w)->id;
    case TINYUI_BACKEND_WIDGET_ANIMATION:  return ((const struct tinyui_animation *)w)->id;
    case TINYUI_BACKEND_WIDGET_LIST:       return ((const struct tinyui_list *)w)->id;
    case TINYUI_BACKEND_WIDGET_MESSAGE_BOX:return ((const struct tinyui_message_box *)w)->id;
    case TINYUI_BACKEND_WIDGET_DATE_TIME:  return ((const struct tinyui_date_time *)w)->id;
    case TINYUI_BACKEND_WIDGET_CLOCK:      return ((const struct tinyui_clock *)w)->id;
    case TINYUI_BACKEND_WIDGET_TEXT:       return ((const struct tinyui_text *)w)->id;
    case TINYUI_BACKEND_WIDGET_KEYBOARD:   return ((const struct tinyui_keyboard *)w)->id;
    case TINYUI_BACKEND_WIDGET_COMBO_BOX:  return ((const struct tinyui_combo_box *)w)->id;
    case TINYUI_BACKEND_WIDGET_SCROLL_SELECTER:return ((const struct tinyui_scroll_selecter *)w)->id;
    case TINYUI_BACKEND_WIDGET_TABLE:      return ((const struct tinyui_table *)w)->id;
    case TINYUI_BACKEND_WIDGET_GRAPH:      return ((const struct tinyui_graph *)w)->id;
    case TINYUI_BACKEND_WIDGET_IMAGE:      return ((const struct tinyui_image *)w)->id;
    case TINYUI_BACKEND_WIDGET_CALENDAR:   return ((const struct tinyui_calendar *)w)->id;
    case TINYUI_BACKEND_WIDGET_CANVAS:     return ((const struct tinyui_canvas *)w)->id;
    case TINYUI_BACKEND_WIDGET_WINDOW:
    case TINYUI_BACKEND_WIDGET_BACKGROUND:
    default:
        return NULL;
    }
}

static int tinyui_runtime_host_widget_is_real_leaf(const struct tinyui_widget *w)
{
    return w != NULL
        && w->ld_widget != NULL
        && w->kind != TINYUI_BACKEND_WIDGET_WINDOW
        && w->kind != TINYUI_BACKEND_WIDGET_BACKGROUND;
}

static size_t tinyui_runtime_host_count_real_in_ld_tree(ldBase_t *node,
                                                         struct tinyui_app *app)
{
    size_t count = 0;

    while (node != NULL) {
        struct tinyui_widget *w = tinyui_runtime_internal_app_lookup_host(app, node->nameId);

        if (tinyui_runtime_host_widget_is_real_leaf(w)) {
            count++;
        }

        {
            ldBase_t *child = ldBaseGetChildList(node);
            if (child != NULL) {
                count += tinyui_runtime_host_count_real_in_ld_tree(child, app);
            }
        }

        node = ldBaseGetNextSibling(node);
    }

    return count;
}

/* Collect the comma-separated list of real widget ids from the ld subtree.
 * Writes at most `cap-1` ids and NUL-terminates; returns the number of ids
 * that were written (excluding the terminator). ids without an id field are
 * skipped. The buffer is flushed by the caller. */
static size_t tinyui_runtime_host_collect_real_widget_ids(ldBase_t *node,
                                                          char *buf,
                                                          size_t cap,
                                                          size_t *written,
                                                          struct tinyui_app *app)
{
    while (node != NULL && *written + 1 < cap) {
        struct tinyui_widget *w = tinyui_runtime_internal_app_lookup_host(app, node->nameId);

        if (tinyui_runtime_host_widget_is_real_leaf(w)) {
            const char *id = tinyui_runtime_host_widget_id(w);
            if (id != NULL && id[0] != '\0') {
                size_t id_len = strlen(id);
                size_t avail = cap - *written;
                int need_sep = (*written > 0);

                if (need_sep) {
                    if (avail < 2) {
                        break;
                    }
                    buf[*written] = ',';
                    (*written)++;
                    avail--;
                }
                if (id_len >= avail) {
                    id_len = avail - 1;
                }
                memcpy(buf + *written, id, id_len);
                *written += id_len;
            }
        }

        {
            ldBase_t *child = ldBaseGetChildList(node);
            if (child != NULL) {
                tinyui_runtime_host_collect_real_widget_ids(child, buf, cap, written, app);
            }
        }

        node = ldBaseGetNextSibling(node);
    }

    return *written;
}

void tinyui_runtime_host_log_mapping_markers(struct tinyui_runtime_host_state *state,
                                              const struct tinyui_widget *root_widget)
{
    size_t real_count = 0;

    if (state == NULL || root_widget == NULL || root_widget->ld_widget == NULL) {
        return;
    }

    {
        ldBase_t *first_child = ldBaseGetChildList((ldBase_t *)root_widget->ld_widget);
        real_count = tinyui_runtime_host_count_real_in_ld_tree(first_child, root_widget->owner);
    }

    if (real_count > 0 && !state->static_mapping_logged) {
        printf("TINYUI_BACKEND_STATIC_MAPPING=REAL_LDGUI\n");
        fflush(stdout);
        state->static_mapping_logged = 1;
    }

    if (real_count > 0 && !state->real_widget_ids_logged) {
        char buf[1024];
        size_t written = 0;
        ldBase_t *first_child = ldBaseGetChildList((ldBase_t *)root_widget->ld_widget);

        buf[0] = '\0';
        tinyui_runtime_host_collect_real_widget_ids(first_child, buf, sizeof(buf), &written,
                                                    root_widget->owner);
        buf[written] = '\0';
        printf("TINYUI_BACKEND_REAL_WIDGET_IDS=%s\n", buf);
        fflush(stdout);
        state->real_widget_ids_logged = 1;
    }
}

static void tinyui_runtime_host_log_image_source_marker_in_ld_tree(ldBase_t *node,
                                                                     struct tinyui_app *app)
{
    while (node != NULL) {
        struct tinyui_widget *widget = tinyui_runtime_internal_app_lookup_host(app, node->nameId);

        if (widget != NULL &&
            widget->kind == TINYUI_BACKEND_WIDGET_IMAGE &&
            widget->ld_widget == node) {
            const char *id = tinyui_runtime_host_widget_id(widget);
            ldImage_t *ld_image = (ldImage_t *)node;

            if (id != NULL) {
                printf("TINYUI_BACKEND_IMAGE_SOURCE=%s:img=%s,mask=%s\n",
                       id,
                       ld_image->ptImgTile != NULL ? "set" : "null",
                       ld_image->ptMaskTile != NULL ? "set" : "null");
            }
        }

        {
            ldBase_t *child = ldBaseGetChildList(node);
            if (child != NULL) {
                tinyui_runtime_host_log_image_source_marker_in_ld_tree(child, app);
            }
        }

        node = ldBaseGetNextSibling(node);
    }
}

void tinyui_runtime_host_log_image_source_marker(const struct tinyui_widget *widget)
{
    struct tinyui_runtime_host_state *state;

    if (widget == NULL || widget->ld_widget == NULL || widget->owner == NULL) {
        return;
    }

    state = (struct tinyui_runtime_host_state *)widget->owner->runtime_state;
    if (state != NULL && state->image_source_logged) {
        return;
    }

    tinyui_runtime_host_log_image_source_marker_in_ld_tree((ldBase_t *)widget->ld_widget,
                                                            widget->owner);
    if (state != NULL) {
        state->image_source_logged = 1;
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
    const char *min_frames_env = getenv("TINYUI_CAPTURE_MIN_FRAMES");
    unsigned long min_frames = 1UL;
    char *end = NULL;
    FILE *fp;
    int x;
    int y;

    if (state == NULL || state->real_pixels == NULL || state->capture_written) {
        return 0;
    }

    if (min_frames_env != NULL && min_frames_env[0] != '\0') {
        unsigned long parsed = strtoul(min_frames_env, &end, 10);
        if (end != min_frames_env && (end == NULL || *end == '\0') && parsed > 0UL) {
            if (parsed > 60000UL) {
                parsed = 60000UL;
            }
            min_frames = parsed;
        }
    }

    if (state->rendered_frames < (Uint32)min_frames) {
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

/* ── ENABLE_TEST 观测适配层 ────────────────────────────────────────────────
 * 仅在 ENABLE_TEST 下编入 tinyui_port_sdl,由 hal.c 的 SDL 驱动经
 * tinyui_sdl_observe.h 宏调用。把旧 step.c 的 CI marker/capture/auto-quit 编排
 * 集中到测试侧,生产 port(ENABLE_TEST=OFF)不含任何这些符号。
 * ──────────────────────────────────────────────────────────────────────── */

static int tinyui_runtime_host_is_v23_scenario(const char *scenario)
{
    if (scenario == NULL || scenario[0] == '\0') {
        return 0;
    }
    return strncmp(scenario, "v23_", 4) == 0;
}

static int tinyui_runtime_host_script_enabled(void)
{
    const char *scenario = getenv("TINYUI_SCENARIO");
    const char *script = getenv("TINYUI_SCRIPT_EVENTS");

    if (script != NULL && script[0] != '\0' && script[0] != '0') {
        return 1;
    }
    if (tinyui_runtime_host_is_v23_scenario(scenario)) {
        return 1;
    }
    return 0;
}

static void tinyui_runtime_host_script_begin(struct tinyui_runtime_host_state *state)
{
    if (state == NULL || state->event_trace_header_logged) {
        return;
    }
    printf("TINYUI_EVENT_TRACE_BEGIN\n");
    fflush(stdout);
    state->event_trace_header_logged = 1;
}

static void tinyui_runtime_host_script_end(struct tinyui_runtime_host_state *state)
{
    if (state == NULL) {
        return;
    }
    printf("TINYUI_EVENT_TRACE_END\n");
    fflush(stdout);
    state->script_done = 1;
}

/* Deterministic L5-E pointer script for v23_core_vertical.
 * Coordinates match tinyui/demo/v23_core_vertical fixed layout. */
static void tinyui_runtime_host_run_v23_core_vertical_script(
    struct tinyui_app *app,
    struct tinyui_runtime_host_state *state)
{
    switch (state->script_step) {
    case 0: /* button press center (24+80, 72+20) */
        (void)tinyui_runtime_bridge_commit_pointer_event(
            app, state->display_width, state->display_height, 104, 92, 1);
        break;
    case 1: /* button release → CLICKED */
        (void)tinyui_runtime_bridge_commit_pointer_event(
            app, state->display_width, state->display_height, 104, 92, 0);
        break;
    case 2: /* checkbox press near box (24+12, 128+16) */
        (void)tinyui_runtime_bridge_commit_pointer_event(
            app, state->display_width, state->display_height, 36, 144, 1);
        break;
    case 3: /* checkbox release → VALUE_CHANGED:1 */
        (void)tinyui_runtime_bridge_commit_pointer_event(
            app, state->display_width, state->display_height, 36, 144, 0);
        break;
    case 4: /* slider press for value 75 (W=280,I=10,half=5 → x=232) */
        (void)tinyui_runtime_bridge_commit_pointer_event(
            app, state->display_width, state->display_height, 232, 190, 1);
        break;
    case 5: /* slider hold/motion same point */
        (void)tinyui_runtime_bridge_commit_pointer_event(
            app, state->display_width, state->display_height, 232, 190, 1);
        break;
    case 6: /* slider release → VALUE_CHANGED:75 */
        (void)tinyui_runtime_bridge_commit_pointer_event(
            app, state->display_width, state->display_height, 232, 190, 0);
        break;
    case 7:
        tinyui_runtime_host_script_end(state);
        break;
    default:
        state->script_done = 1;
        break;
    }
}

/* switch center ≈ (24+48, 24+18) */
static void tinyui_runtime_host_run_v23_value_instruments_script(
    struct tinyui_app *app,
    struct tinyui_runtime_host_state *state)
{
    switch (state->script_step) {
    case 0:
        (void)tinyui_runtime_bridge_commit_pointer_event(
            app, state->display_width, state->display_height, 72, 42, 1);
        break;
    case 1:
        (void)tinyui_runtime_bridge_commit_pointer_event(
            app, state->display_width, state->display_height, 72, 42, 0);
        break;
    case 2:
        tinyui_runtime_host_script_end(state);
        break;
    default:
        state->script_done = 1;
        break;
    }
}

/* list item "Beta" ≈ (16+40, 16+28+14) */
static void tinyui_runtime_host_run_v23_selection_collection_script(
    struct tinyui_app *app,
    struct tinyui_runtime_host_state *state)
{
    switch (state->script_step) {
    case 0:
        (void)tinyui_runtime_bridge_commit_pointer_event(
            app, state->display_width, state->display_height, 56, 58, 1);
        break;
    case 1:
        (void)tinyui_runtime_bridge_commit_pointer_event(
            app, state->display_width, state->display_height, 56, 58, 0);
        break;
    case 2:
        tinyui_runtime_host_script_end(state);
        break;
    default:
        state->script_done = 1;
        break;
    }
}

/* line_edit focus then qwerty 'q' key (480x320 LD keyboard layout). */
static void tinyui_runtime_host_run_v23_input_data_script(
    struct tinyui_app *app,
    struct tinyui_runtime_host_state *state)
{
    switch (state->script_step) {
    case 0: /* focus line_edit */
        (void)tinyui_runtime_bridge_commit_pointer_event(
            app, state->display_width, state->display_height, 80, 64, 1);
        break;
    case 1:
        (void)tinyui_runtime_bridge_commit_pointer_event(
            app, state->display_width, state->display_height, 80, 64, 0);
        break;
    case 2: /* 'q' key center ≈ (7+21, 165+16) on 480x320 */
        (void)tinyui_runtime_bridge_commit_pointer_event(
            app, state->display_width, state->display_height, 28, 181, 1);
        break;
    case 3:
        (void)tinyui_runtime_bridge_commit_pointer_event(
            app, state->display_width, state->display_height, 28, 181, 0);
        break;
    case 4:
        tinyui_runtime_host_script_end(state);
        break;
    default:
        state->script_done = 1;
        break;
    }
}

/* message_box at (16,160) layout 260x140; confirm button near bottom-center. */
static void tinyui_runtime_host_run_v23_media_composite_script(
    struct tinyui_app *app,
    struct tinyui_runtime_host_state *state)
{
    switch (state->script_step) {
    case 0:
        (void)tinyui_runtime_bridge_commit_pointer_event(
            app, state->display_width, state->display_height, 146, 275, 1);
        break;
    case 1:
        (void)tinyui_runtime_bridge_commit_pointer_event(
            app, state->display_width, state->display_height, 146, 275, 0);
        break;
    case 2:
        tinyui_runtime_host_script_end(state);
        break;
    default:
        state->script_done = 1;
        break;
    }
}

/* theme/layout/resource: no operable user event required; close after frames. */
static void tinyui_runtime_host_run_v23_theme_layout_resource_script(
    struct tinyui_app *app,
    struct tinyui_runtime_host_state *state)
{
    (void)app;
    switch (state->script_step) {
    case 0:
        tinyui_runtime_host_script_end(state);
        break;
    default:
        state->script_done = 1;
        break;
    }
}

static void tinyui_runtime_host_run_scenario_script(
    struct tinyui_app *app,
    struct tinyui_runtime_host_state *state)
{
    const char *scenario;

    if (app == NULL || state == NULL || state->script_done) {
        return;
    }
    if (!state->ready_logged || state->rendered_frames < 2U) {
        return;
    }

    tinyui_runtime_host_script_begin(state);
    scenario = getenv("TINYUI_SCENARIO");
    if (scenario == NULL) {
        scenario = "v23_core_vertical";
    }

    if (strcmp(scenario, "v23_core_vertical") == 0) {
        tinyui_runtime_host_run_v23_core_vertical_script(app, state);
    } else if (strcmp(scenario, "v23_value_instruments") == 0) {
        tinyui_runtime_host_run_v23_value_instruments_script(app, state);
    } else if (strcmp(scenario, "v23_selection_collection") == 0) {
        tinyui_runtime_host_run_v23_selection_collection_script(app, state);
    } else if (strcmp(scenario, "v23_input_data") == 0) {
        tinyui_runtime_host_run_v23_input_data_script(app, state);
    } else if (strcmp(scenario, "v23_media_composite") == 0) {
        tinyui_runtime_host_run_v23_media_composite_script(app, state);
    } else if (strcmp(scenario, "v23_theme_layout_resource") == 0) {
        tinyui_runtime_host_run_v23_theme_layout_resource_script(app, state);
    } else {
        /* Unknown scripted scenario: end without injecting fake events. */
        tinyui_runtime_host_script_end(state);
        return;
    }

    state->script_step += 1;
}

/* 安装期(window_create):建立 image_source 标记所需的 app→state 链接,解析
 * auto-quit 时限。新架构下 state 是 hal.c 文件静态,log_image_source_marker 仍
 * 从 widget->owner->runtime_state 取 state,故在此赋值(生产不走此路径)。 */
void tinyui_sdl_observe_on_setup(struct tinyui_app *app,
                                 struct tinyui_runtime_host_state *state)
{
    if (app == NULL || state == NULL) {
        return;
    }
    app->runtime_state = state;
    state->auto_quit_ms = tinyui_runtime_host_parse_auto_quit_ms();
    state->script_enabled = tinyui_runtime_host_script_enabled();
    state->script_step = 0;
    state->script_done = 0;
    state->event_trace_header_logged = 0;
}

/* 每帧呈现后:发 runtime-ready / 映射 / smoke / image-source marker(各自一次),
 * 计帧并写 PPM 截图。等价旧 step.c 的 prepare(ready)+render(markers+capture)。 */
void tinyui_sdl_observe_on_present(struct tinyui_app *app,
                                   struct tinyui_runtime_host_state *state)
{
    struct tinyui_window *window;

    if (app == NULL || state == NULL) {
        return;
    }

    tinyui_runtime_host_log_runtime_ready(app, state);

    window = app->root_window;
    if (window != NULL && window->widget.ld_widget != NULL) {
        tinyui_runtime_host_log_mapping_markers(state, &window->widget);
        if (app->ld_scene != NULL && state->real_pixels != NULL) {
            state->smoke_layout_used = 0;
            tinyui_runtime_host_log_smoke_layout_marker(state);
            tinyui_runtime_host_log_image_source_marker(&window->widget);
            state->rendered_frames += 1U;
        }
    }

    if (state->script_enabled) {
        tinyui_runtime_host_run_scenario_script(app, state);
        /* Wait for scripted interactions before freezing capture. */
        if (!state->script_done) {
            return;
        }
    }

    (void)tinyui_runtime_host_write_capture(state);
}

/* auto-quit(TINYUI_DEMO_AUTO_QUIT_MS):headless 冒烟/CI 用,到时请求退出。 */
int tinyui_sdl_observe_should_quit(struct tinyui_app *app,
                                   struct tinyui_runtime_host_state *state)
{
    if (app == NULL || state == NULL) {
        return 0;
    }
    /* Scripted scenarios may finish early once capture is frozen. */
    if (state->script_enabled && state->script_done && state->capture_written) {
        return 1;
    }
    if (state->auto_quit_ms > 0 &&
        tinyui_tick_get(app) - state->start_ticks >= state->auto_quit_ms) {
        return 1;
    }
    return 0;
}
