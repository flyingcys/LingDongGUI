#include "../backend/ldgui/backend.h"
#include "../core/internal.h"
#include "../../../src/gui/ldQRCode.h"
#include "../../../examples/common/Arm-2D/examples/common/controls/qrcode_box/qrcodegen.h"

#include <stdlib.h>

#define PICOUI_NATIVE_QRCODE_RENDER_READY (1u << 25)
#define PICOUI_NATIVE_QRCODE_RENDER_MAX 32

struct picoui_native_qrcode_render_state {
    const struct picoui_qrcode *qrcode;
    const char *text;
    int ecc;
    int module_count;
    struct picoui_rect dirty_rect;
    int changed_pixels;
    int rendered;
};

static struct picoui_native_qrcode_render_state
    g_picoui_native_qrcode_render_states[PICOUI_NATIVE_QRCODE_RENDER_MAX];

static struct picoui_backend_widget *picoui_native_qrcode_backend(const struct picoui_qrcode *qrcode)
{
    if (qrcode == 0 || qrcode->widget.backend_widget == 0) {
        return 0;
    }

    return (struct picoui_backend_widget *)qrcode->widget.backend_widget;
}

int picoui_backend_qrcode_set_text(struct picoui_qrcode *qrcode, const char *text);
int picoui_backend_qrcode_set_ecc(void *backend_widget, int ecc);
int picoui_native_qrcode_render_buffer(const struct picoui_qrcode *qrcode,
                                       unsigned int *buffer,
                                       int width,
                                       int height,
                                       struct picoui_rect *dirty_rect);

static struct picoui_native_qrcode_render_state *picoui_native_qrcode_find_render_state(
    const struct picoui_qrcode *qrcode)
{
    int i;

    if (qrcode == 0) {
        return 0;
    }

    for (i = 0; i < PICOUI_NATIVE_QRCODE_RENDER_MAX; ++i) {
        if (g_picoui_native_qrcode_render_states[i].qrcode == qrcode) {
            return &g_picoui_native_qrcode_render_states[i];
        }
    }

    return 0;
}

static struct picoui_native_qrcode_render_state *picoui_native_qrcode_alloc_render_state(
    const struct picoui_qrcode *qrcode)
{
    struct picoui_native_qrcode_render_state *state;
    int i;

    state = picoui_native_qrcode_find_render_state(qrcode);
    if (state != 0) {
        return state;
    }

    if (qrcode == 0) {
        return 0;
    }

    for (i = 0; i < PICOUI_NATIVE_QRCODE_RENDER_MAX; ++i) {
        if (g_picoui_native_qrcode_render_states[i].qrcode == 0) {
            g_picoui_native_qrcode_render_states[i].qrcode = qrcode;
            g_picoui_native_qrcode_render_states[i].text = 0;
            g_picoui_native_qrcode_render_states[i].ecc = 0;
            g_picoui_native_qrcode_render_states[i].module_count = 0;
            g_picoui_native_qrcode_render_states[i].dirty_rect.x = 0;
            g_picoui_native_qrcode_render_states[i].dirty_rect.y = 0;
            g_picoui_native_qrcode_render_states[i].dirty_rect.width = 0;
            g_picoui_native_qrcode_render_states[i].dirty_rect.height = 0;
            g_picoui_native_qrcode_render_states[i].changed_pixels = 0;
            g_picoui_native_qrcode_render_states[i].rendered = 0;
            return &g_picoui_native_qrcode_render_states[i];
        }
    }

    return 0;
}

static int picoui_native_qrcode_module_count_from_encode(const struct picoui_qrcode *qrcode)
{
    uint8_t *qr0;
    uint8_t *temp_buffer;
    int version;
    int size = -1;
    int rc = -1;

    if (qrcode == 0 || qrcode->text == 0) {
        return -1;
    }

    version = qrcode->max_version;
    if (version <= 0) {
        version = 1;
    }
    if (version > 40) {
        version = 40;
    }

    qr0 = (uint8_t *)calloc(1, qrcodegen_BUFFER_LEN_FOR_VERSION(version));
    temp_buffer = (uint8_t *)calloc(1, qrcodegen_BUFFER_LEN_FOR_VERSION(version));
    if (qr0 == 0 || temp_buffer == 0) {
        goto done;
    }

    if (!qrcodegen_encodeText(qrcode->text,
                              temp_buffer,
                              qr0,
                              (enum qrcodegen_Ecc)qrcode->ecc,
                              version,
                              version,
                              qrcodegen_Mask_AUTO,
                              true)) {
        goto done;
    }

    size = qrcodegen_getSize(qr0);
    if (size > 0) {
        rc = size;
    }

done:
    free(qr0);
    free(temp_buffer);
    return rc;
}

void picoui_native_qrcode_reset_render_state(struct picoui_qrcode *qrcode)
{
    struct picoui_backend_widget *backend = picoui_native_qrcode_backend(qrcode);

    if (backend == 0) {
        return;
    }

    backend->runtime_evidence_flags &= ~PICOUI_NATIVE_QRCODE_RENDER_READY;
}

int picoui_native_qrcode_render(const struct picoui_backend_widget *backend)
{
    struct picoui_qrcode *qrcode;
    struct picoui_native_qrcode_render_state *state;
    unsigned int *scratch = 0;
    size_t scratch_count;
    int module_count;
    int i;

    if (backend == 0 || backend->kind != PICOUI_BACKEND_WIDGET_QRCODE || backend->host_widget == 0) {
        return -1;
    }

    qrcode = (struct picoui_qrcode *)backend->host_widget;
    state = picoui_native_qrcode_alloc_render_state(qrcode);
    if (state == 0) {
        return -1;
    }
    if (qrcode->text == 0) {
        return -1;
    }

    if (picoui_backend_qrcode_set_text(qrcode, qrcode->text) != 0
        || picoui_backend_qrcode_set_ecc(qrcode->widget.backend_widget, qrcode->ecc) != 0) {
        return -1;
    }

    module_count = picoui_native_qrcode_module_count_from_encode(qrcode);
    if (module_count <= 0) {
        return -1;
    }

    state->text = qrcode->text;
    state->ecc = qrcode->ecc;
    state->module_count = module_count;
    state->dirty_rect.x = 0;
    state->dirty_rect.y = 0;
    state->dirty_rect.width = 0;
    state->dirty_rect.height = 0;
    state->changed_pixels = 0;

    if (qrcode->widget.width > 0 && qrcode->widget.height > 0) {
        scratch_count = (size_t)qrcode->widget.width * (size_t)qrcode->widget.height;
        scratch = calloc(scratch_count, sizeof(*scratch));
        if (scratch == 0) {
            return -1;
        }

        if (picoui_native_qrcode_render_buffer(qrcode,
                                               scratch,
                                               qrcode->widget.width,
                                               qrcode->widget.height,
                                               &state->dirty_rect)
            != 0) {
            free(scratch);
            return -1;
        }

        for (i = 0; i < (int)scratch_count; ++i) {
            if (scratch[i] != 0U) {
                state->changed_pixels++;
            }
        }

        free(scratch);
    }

    ((struct picoui_backend_widget *)backend)->runtime_evidence_flags |=
        PICOUI_NATIVE_QRCODE_RENDER_READY;
    state->rendered = 1;
    return 0;
}

int picoui_native_qrcode_get_rendered_state(const struct picoui_qrcode *qrcode,
                                            const char **text,
                                            int *ecc,
                                            int *module_count)
{
    const struct picoui_backend_widget *backend = picoui_native_qrcode_backend(qrcode);

    if (text == 0 || ecc == 0 || module_count == 0) {
        return -1;
    }

    if (qrcode == 0 || backend == 0
        || (backend->runtime_evidence_flags & PICOUI_NATIVE_QRCODE_RENDER_READY) == 0) {
        return -1;
    }

    *text = qrcode->text;
    *ecc = qrcode->ecc;
    *module_count = picoui_native_qrcode_module_count_from_encode(qrcode);
    return *module_count > 0 ? 0 : -1;
}

int picoui_native_qrcode_get_rendered_dirty_rect(const struct picoui_qrcode *qrcode,
                                                 struct picoui_rect *dirty_rect)
{
    struct picoui_native_qrcode_render_state *state;

    if (dirty_rect == 0) {
        return -1;
    }

    state = picoui_native_qrcode_find_render_state(qrcode);
    if (state == 0 || state->rendered == 0) {
        return -1;
    }

    *dirty_rect = state->dirty_rect;
    return 0;
}

int picoui_native_qrcode_get_rendered_changed_pixels(const struct picoui_qrcode *qrcode, int *count)
{
    struct picoui_native_qrcode_render_state *state;

    if (count == 0) {
        return -1;
    }

    state = picoui_native_qrcode_find_render_state(qrcode);
    if (state == 0 || state->rendered == 0) {
        return -1;
    }

    *count = state->changed_pixels;
    return 0;
}
