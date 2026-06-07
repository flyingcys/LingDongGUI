#include "../backend/ldgui/backend.h"
#include "../core/internal.h"

#include <stdlib.h>

struct picoui_gauge_ext {
    struct picoui_gauge gauge;
    int min_value;
    int max_value;
    int value;
    int tick_count;
    int tick_step;
    int render_ready;
    int rendered_min_value;
    int rendered_max_value;
    int rendered_value;
    int rendered_tick_count;
    int rendered_tick_step;
    float rendered_needle_angle;
};

static struct picoui_gauge_ext *picoui_gauge_ext_from_gauge(struct picoui_gauge *gauge)
{
    if (gauge == 0) {
        return 0;
    }

    return (struct picoui_gauge_ext *)gauge;
}

static const struct picoui_gauge_ext *picoui_gauge_ext_from_gauge_const(const struct picoui_gauge *gauge)
{
    if (gauge == 0) {
        return 0;
    }

    return (const struct picoui_gauge_ext *)gauge;
}

int picoui_native_gauge_init_state(struct picoui_gauge *gauge)
{
    struct picoui_gauge_ext *ext;

    ext = picoui_gauge_ext_from_gauge(gauge);
    if (ext == 0) {
        return -1;
    }

    ext->min_value = 0;
    ext->max_value = 100;
    ext->value = 0;
    ext->tick_count = 11;
    ext->tick_step = 10;
    ext->render_ready = 0;
    ext->rendered_min_value = 0;
    ext->rendered_max_value = 0;
    ext->rendered_value = 0;
    ext->rendered_tick_count = 0;
    ext->rendered_tick_step = 0;
    ext->rendered_needle_angle = 0.0f;
    return 0;
}

void picoui_native_gauge_reset_render_state(struct picoui_gauge *gauge)
{
    struct picoui_gauge_ext *ext;

    ext = picoui_gauge_ext_from_gauge(gauge);
    if (ext == 0) {
        return;
    }

    ext->render_ready = 0;
    ext->rendered_min_value = 0;
    ext->rendered_max_value = 0;
    ext->rendered_value = 0;
    ext->rendered_tick_count = 0;
    ext->rendered_tick_step = 0;
    ext->rendered_needle_angle = 0.0f;
}

int picoui_native_gauge_set_state(struct picoui_gauge *gauge,
                                  int min_value,
                                  int max_value,
                                  int value,
                                  int tick_count)
{
    struct picoui_gauge_ext *ext;

    ext = picoui_gauge_ext_from_gauge(gauge);
    if (ext == 0 || min_value > max_value || tick_count <= 0) {
        return -1;
    }

    ext->min_value = min_value;
    ext->max_value = max_value;
    ext->value = value;
    ext->tick_count = tick_count;
    ext->tick_step = (max_value - min_value) / tick_count;
    if (ext->tick_step <= 0) {
        ext->tick_step = 1;
    }
    return 0;
}

int picoui_native_gauge_get_state(const struct picoui_gauge *gauge,
                                  int *min_value,
                                  int *max_value,
                                  int *value,
                                  int *tick_count,
                                  int *tick_step)
{
    const struct picoui_gauge_ext *ext;

    if (min_value == 0 || max_value == 0 || value == 0 || tick_count == 0 || tick_step == 0) {
        return -1;
    }

    ext = picoui_gauge_ext_from_gauge_const(gauge);
    if (ext == 0) {
        return -1;
    }

    *min_value = ext->min_value;
    *max_value = ext->max_value;
    *value = ext->value;
    *tick_count = ext->tick_count;
    *tick_step = ext->tick_step;
    return 0;
}

int picoui_native_gauge_render(const struct picoui_backend_widget *backend)
{
    struct picoui_gauge *gauge;
    struct picoui_gauge_ext *ext;

    if (backend == 0 || backend->kind != PICOUI_BACKEND_WIDGET_GAUGE
        || backend->host_widget == 0) {
        return -1;
    }

    gauge = (struct picoui_gauge *)backend->host_widget;
    ext = picoui_gauge_ext_from_gauge(gauge);
    if (ext == 0) {
        return -1;
    }

    ext->rendered_min_value = ext->min_value;
    ext->rendered_max_value = ext->max_value;
    ext->rendered_value = ext->value;
    ext->rendered_tick_count = ext->tick_count;
    ext->rendered_tick_step = ext->tick_step;
    ext->rendered_needle_angle = gauge->angle;
    ext->render_ready = 1;
    return 0;
}

int picoui_native_gauge_get_rendered_state(const struct picoui_gauge *gauge,
                                           int *min_value,
                                           int *max_value,
                                           int *value,
                                           int *tick_count,
                                           int *tick_step,
                                           float *needle_angle)
{
    const struct picoui_gauge_ext *ext;

    if (min_value == 0 || max_value == 0 || value == 0
        || tick_count == 0 || tick_step == 0 || needle_angle == 0) {
        return -1;
    }

    ext = picoui_gauge_ext_from_gauge_const(gauge);
    if (ext == 0 || ext->render_ready == 0) {
        return -1;
    }

    *min_value = ext->rendered_min_value;
    *max_value = ext->rendered_max_value;
    *value = ext->rendered_value;
    *tick_count = ext->rendered_tick_count;
    *tick_step = ext->rendered_tick_step;
    *needle_angle = ext->rendered_needle_angle;
    return 0;
}
