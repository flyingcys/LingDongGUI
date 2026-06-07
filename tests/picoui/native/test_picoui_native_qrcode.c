#include "picoui/picoui.h"

#include <assert.h>
#include <string.h>

int picoui_native_event_bind_root(struct picoui_screen *screen, struct picoui_window *root_window);
int picoui_native_qrcode_get_rendered_state(const struct picoui_qrcode *qrcode,
                                            const char **text,
                                            int *ecc,
                                            int *module_count);
int picoui_native_qrcode_get_rendered_dirty_rect(const struct picoui_qrcode *qrcode,
                                                 struct picoui_rect *dirty_rect);
int picoui_native_qrcode_get_rendered_changed_pixels(const struct picoui_qrcode *qrcode, int *count);

int main(void)
{
    struct picoui_display *display;
    struct picoui_screen *screen;
    struct picoui_window *window;
    struct picoui_qrcode *qrcode;
    const char *rendered_text = 0;
    int rendered_ecc = -1;
    int rendered_module_count = -1;
    int changed_pixels = -1;
    struct picoui_rect dirty_rect = {-1, -1, -1, -1};
    int rc;

    assert(picoui_init() == 0);

    display = picoui_display_create(320, 480);
    assert(display != 0);
    assert(picoui_display_set_default(display) == 0);

    screen = picoui_screen_active();
    assert(screen != 0);

    window = picoui_window_create_root(screen, "root");
    assert(window != 0);

    qrcode = picoui_qrcode_create((struct picoui_widget *)window, "qr");
    assert(qrcode != 0);
    assert(picoui_widget_set_size((struct picoui_widget *)qrcode, 128, 128) == 0);
    assert(picoui_qrcode_set_text(qrcode, "https://example.local/p4-l") == 0);
    assert(strcmp(picoui_qrcode_get_text(qrcode), "https://example.local/p4-l") == 0);
    assert(picoui_qrcode_set_ecc(qrcode, 3) == 0);

    assert(picoui_native_event_bind_root(screen, window) == 0);
    assert(picoui_native_qrcode_get_rendered_state(qrcode,
                                                   &rendered_text,
                                                   &rendered_ecc,
                                                   &rendered_module_count) == -1);

    rc = picoui_timer_handler();
    assert(rc == -1);
    assert(picoui_native_qrcode_get_rendered_state(qrcode,
                                                   &rendered_text,
                                                   &rendered_ecc,
                                                   &rendered_module_count) == -1);

    assert(picoui_qrcode_set_max_version(qrcode, 10) == 0);

    rc = picoui_timer_handler();
    assert(rc == 0 || rc == 1);

    assert(picoui_native_qrcode_get_rendered_state(qrcode,
                                                   &rendered_text,
                                                   &rendered_ecc,
                                                   &rendered_module_count) == 0);
    assert(rendered_text != 0);
    assert(strcmp(rendered_text, "https://example.local/p4-l") == 0);
    assert(rendered_ecc == 3);
    /* module_count is only available after native render proves qrcodegen encode succeeds. */
    assert(rendered_module_count > 0);
    assert(picoui_native_qrcode_get_rendered_dirty_rect(qrcode, &dirty_rect) == 0);
    assert(dirty_rect.width > 0);
    assert(dirty_rect.height > 0);
    assert(picoui_native_qrcode_get_rendered_changed_pixels(qrcode, &changed_pixels) == 0);
    assert(changed_pixels > 0);

    picoui_deinit();
    return 0;
}
