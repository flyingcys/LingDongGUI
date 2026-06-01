#include "backend.h"
#include "internal.h"
#include "../../../../src/gui/ldBase.h"

static unsigned int g_picoui_backend_next_data_model_identity = 1;

int picoui_native_nav_dir_to_ld(enum picoui_native_nav_dir dir)
{
    switch (dir) {
    case PICOUI_NATIVE_NAV_LEFT:
        return NAV_LEFT;
    case PICOUI_NATIVE_NAV_RIGHT:
        return NAV_RIGHT;
    case PICOUI_NATIVE_NAV_UP:
        return NAV_UP;
    case PICOUI_NATIVE_NAV_DOWN:
    default:
        return NAV_DOWN;
    }
}

void picoui_backend_widget_init_data_model(struct picoui_backend_widget *backend)
{
    if (backend == 0) {
        return;
    }

    switch (backend->kind) {
    case PICOUI_BACKEND_WIDGET_CHECKBOX:
    case PICOUI_BACKEND_WIDGET_SWITCH:
    case PICOUI_BACKEND_WIDGET_SLIDER:
    case PICOUI_BACKEND_WIDGET_LIST:
    case PICOUI_BACKEND_WIDGET_COMBO_BOX:
    case PICOUI_BACKEND_WIDGET_SCROLL_SELECTER:
        backend->data_truth_policy = PICOUI_BACKEND_DATA_TRUTH_BACKEND_VALUE;
        backend->data_model_identity = g_picoui_backend_next_data_model_identity++;
        if (backend->data_model_identity == 0) {
            backend->data_model_identity = g_picoui_backend_next_data_model_identity++;
        }
        break;
    default:
        backend->data_truth_policy = PICOUI_BACKEND_DATA_TRUTH_NOT_APPLICABLE;
        backend->data_model_identity = 0;
        break;
    }
}

int picoui_backend_widget_claim_focus(void *backend_widget)
{
    struct picoui_backend_widget *backend;

    if (backend_widget == 0) {
        return -1;
    }

    backend = (struct picoui_backend_widget *)backend_widget;
    if (backend->host_widget == 0) {
        return -1;
    }

    return picoui_widget_claim_focus(backend->host_widget);
}

int picoui_backend_widget_release_focus(void *backend_widget)
{
    struct picoui_backend_widget *backend;

    if (backend_widget == 0) {
        return -1;
    }

    backend = (struct picoui_backend_widget *)backend_widget;
    if (backend->host_widget == 0) {
        return -1;
    }

    return picoui_widget_release_focus(backend->host_widget);
}
