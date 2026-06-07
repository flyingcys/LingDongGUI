#include "picoui/picoui.h"
#include "internal.h"
#include "backend.h"

#include <assert.h>
#include <string.h>

int picoui_native_date_time_format_text(const struct picoui_date_time *dt,
                                        char *buffer,
                                        unsigned long buffer_size);

int main(void)
{
    struct picoui_display *display;
    struct picoui_screen *screen;
    struct picoui_window *window;
    struct picoui_date_time *date_time;
    struct picoui_date_time *date_time_second;
    struct picoui_backend_widget *backend;
    void *saved_ld_widget;
    char formatted[64];
    char formatted_second[64];

    assert(picoui_init() == 0);

    display = picoui_display_create(320, 480);
    assert(display != 0);
    assert(picoui_display_set_default(display) == 0);

    screen = picoui_screen_active();
    assert(screen != 0);

    window = picoui_window_create_root(screen, "root");
    assert(window != 0);

    date_time = picoui_date_time_create((struct picoui_widget *)window, "date_time");
    assert(date_time != 0);

    assert(picoui_date_time_set_use_system_time(date_time, 0) == 0);
    assert(picoui_date_time_set_format(date_time, "yyyy/mm/dd hh:nn:ss") == 0);
    assert(picoui_date_time_set_date(date_time, 2026, 6, 5) == 0);
    assert(picoui_date_time_set_time(date_time, 7, 8, 9) == 0);

    assert(picoui_date_time_get_format(date_time) != 0);
    assert(strcmp(picoui_date_time_get_format(date_time), "yyyy/mm/dd hh:nn:ss") == 0);
    assert(picoui_native_date_time_format_text(date_time, formatted, sizeof(formatted)) == 0);
    assert(strcmp(formatted, "2026/06/05 07:08:09") == 0);

    date_time_second = picoui_date_time_create((struct picoui_widget *)window, "date_time_second");
    assert(date_time_second != 0);
    assert(picoui_date_time_set_use_system_time(date_time_second, 0) == 0);
    assert(picoui_date_time_set_format(date_time_second, "hh:nn:ss") == 0);
    assert(picoui_date_time_set_date(date_time_second, 2026, 6, 6) == 0);
    assert(picoui_date_time_set_time(date_time_second, 10, 11, 12) == 0);
    assert(picoui_date_time_get_format(date_time_second) != 0);
    assert(strcmp(picoui_date_time_get_format(date_time_second), "hh:nn:ss") == 0);
    assert(picoui_native_date_time_format_text(date_time_second,
                                               formatted_second,
                                               sizeof(formatted_second)) == 0);
    assert(strcmp(formatted_second, "10:11:12") == 0);

    backend = (struct picoui_backend_widget *)date_time->widget.backend_widget;
    assert(backend != 0);
    saved_ld_widget = backend->ld_widget;
    assert(saved_ld_widget != 0);

    backend->ld_widget = 0;
    assert(picoui_date_time_set_format(date_time, "hh:nn:ss") == -1);
    assert(picoui_date_time_get_format(date_time) != 0);
    assert(strcmp(picoui_date_time_get_format(date_time), "yyyy/mm/dd hh:nn:ss") == 0);
    assert(picoui_native_date_time_format_text(date_time, formatted, sizeof(formatted)) == 0);
    assert(strcmp(formatted, "2026/06/05 07:08:09") == 0);
    backend->ld_widget = saved_ld_widget;

    assert(picoui_native_date_time_format_text(0, formatted, sizeof(formatted)) == -1);
    assert(picoui_native_date_time_format_text(date_time, 0, sizeof(formatted)) == -1);
    assert(picoui_native_date_time_format_text(date_time, formatted, 0) == -1);

    picoui_deinit();
    return 0;
}
