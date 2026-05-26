#include "picoui/picoui.h"
#include "../../../src/gui/ldBase.h"
#include "internal.h"

#include <assert.h>

static int switch_toggled_count = 0;
static int switch_toggled_value = -1;
static int checkbox_toggled_count = 0;
static int checkbox_toggled_value = -1;
static int slider_value_count = 0;
static int slider_value = -1;
static int button_clicked = -1;

static void on_switch_toggle(struct picoui_widget *widget, int value, void *user_data)
{
    (void)widget;
    (void)user_data;
    switch_toggled_count++;
    switch_toggled_value = value;
}

static void on_checkbox_toggle(struct picoui_widget *widget, int value, void *user_data)
{
    (void)widget;
    (void)user_data;
    checkbox_toggled_count++;
    checkbox_toggled_value = value;
}

static void on_slider(struct picoui_widget *widget, int value, void *user_data)
{
    (void)widget;
    (void)user_data;
    slider_value_count++;
    slider_value = value;
}

static void on_button_clicked(struct picoui_widget *widget, void *user_data)
{
    (void)widget;
    button_clicked = user_data != 0 ? *(const int *)user_data : 0;
}

static void test_backend_value_changed_bridge_emits_real_queue(struct picoui_slider *slider,
                                                              struct picoui_backend_widget *slider_backend)
{
    ld_scene_t scene = {0};
    ldBase_t sender = {0};
    ldMsg_t msg = {0};

    assert(ldMsgInit(&scene.ptMsgQueue, 4) == true);
    assert(picoui_backend_widget_bind_ld_event_bridge(slider_backend, &scene, &sender) == 0);
    assert(picoui_slider_set_value(slider, 12) == 0);
    assert(slider_value_count == 2);
    assert(slider_value == 12);
    assert(xQueueDequeue(scene.ptMsgQueue, &msg, sizeof(msg)) == true);
    assert(msg.signal == SIGNAL_VALUE_CHANGED);
    assert(msg.value == 12);
    assert(msg.ptSender == &sender);
    assert(xQueueDequeue(scene.ptMsgQueue, &msg, sizeof(msg)) == false);
    ldMsgDeinit(&scene.ptMsgQueue);
}

int main(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_window *win = picoui_window_create(app, "root");
    struct picoui_switch_props sw_props = {
        .id = "wifi",
        .checked = 1,
        .on_toggled = on_switch_toggle,
        .user_data = 0,
    };
    struct picoui_checkbox_props cb_props = {
        .id = "agree",
        .text = "I agree",
        .checked = 0,
        .on_toggled = on_checkbox_toggle,
        .user_data = 0,
    };
    struct picoui_slider_props slider_props = {
        .id = "volume",
        .min_value = 10,
        .max_value = 50,
        .value = 42,
        .on_value_changed = on_slider,
        .user_data = 0,
    };
    struct picoui_switch *sw = picoui_switch_create_with_props(win, &sw_props);
    struct picoui_checkbox *cb = picoui_checkbox_create_with_props(win, &cb_props);
    struct picoui_slider *slider = picoui_slider_create_with_props(win, &slider_props);
    struct picoui_button *button = picoui_button_create(win, "ok");
    struct picoui_label *label = picoui_label_create(win, "title");
    struct picoui_text *text = picoui_text_create(win, "body");
    struct picoui_font font = {"Sans", 14};
    struct picoui_backend_widget *sw_backend;
    struct picoui_backend_widget *cb_backend;
    struct picoui_backend_widget *slider_backend;
    struct picoui_backend_widget *button_backend;
    struct picoui_backend_widget *label_backend;
    struct picoui_backend_widget *text_backend;
    int button_cookie = 7;

    assert(sw && cb && slider && button && label && text);
    sw_backend = sw->widget.backend_widget;
    cb_backend = cb->widget.backend_widget;
    slider_backend = slider->widget.backend_widget;
    button_backend = button->widget.backend_widget;
    label_backend = label->widget.backend_widget;
    text_backend = text->widget.backend_widget;
    assert(switch_toggled_count == 0);
    assert(checkbox_toggled_count == 0);
    assert(slider_value_count == 0);
    assert(picoui_switch_is_checked(sw) == 1);
    assert(picoui_checkbox_is_checked(cb) == 0);
    assert(cb->widget.text == (const char *)"I agree");
    assert(picoui_slider_get_value(slider) == 42);
    assert(sw_backend->value == 1);
    assert(sw_backend->last_signal == PICOUI_BACKEND_SIGNAL_NONE);
    assert(sw_backend->dispatch_count == 0);
    assert(cb_backend->value == 0);
    assert(cb_backend->text == (const char *)"I agree");
    assert(cb_backend->last_signal == PICOUI_BACKEND_SIGNAL_NONE);
    assert(cb_backend->dispatch_count == 0);
    assert(slider_backend->value == 42);
    assert(slider_backend->last_signal == PICOUI_BACKEND_SIGNAL_NONE);
    assert(slider_backend->dispatch_count == 0);

    assert(picoui_switch_set_checked(sw, 1) == 0);
    assert(switch_toggled_count == 0);
    assert(sw_backend->value == 1);
    assert(sw_backend->dispatch_count == 0);
    assert(picoui_switch_set_checked(sw, 0) == 0);
    assert(switch_toggled_count == 1);
    assert(switch_toggled_value == 0);
    assert(sw_backend->value == 0);
    assert(sw_backend->last_signal == PICOUI_BACKEND_SIGNAL_VALUE_CHANGED);
    assert(sw_backend->dispatch_count == 1);
    assert(picoui_switch_set_checked(sw, 0) == 0);
    assert(switch_toggled_count == 1);
    assert(sw_backend->value == 0);
    assert(sw_backend->dispatch_count == 1);

    assert(picoui_checkbox_set_checked(cb, 0) == 0);
    assert(checkbox_toggled_count == 0);
    assert(cb_backend->value == 0);
    assert(cb_backend->dispatch_count == 0);
    assert(picoui_checkbox_set_checked(cb, 1) == 0);
    assert(checkbox_toggled_count == 1);
    assert(checkbox_toggled_value == 1);
    assert(cb_backend->value == 1);
    assert(cb_backend->last_signal == PICOUI_BACKEND_SIGNAL_VALUE_CHANGED);
    assert(cb_backend->dispatch_count == 1);
    assert(picoui_checkbox_set_checked(cb, 1) == 0);
    assert(checkbox_toggled_count == 1);
    assert(cb_backend->value == 1);
    assert(cb_backend->dispatch_count == 1);
    assert(picoui_checkbox_set_text(cb, "accept terms") == 0);
    assert(cb->widget.text == (const char *)"accept terms");
    assert(cb_backend->text == (const char *)"accept terms");

    assert(picoui_slider_set_value(slider, 42) == 0);
    assert(slider_value_count == 0);
    assert(slider_backend->value == 42);
    assert(slider_backend->dispatch_count == 0);
    assert(picoui_slider_set_value(slider, 11) == 0);
    assert(slider_value_count == 1);
    assert(slider_value == 11);
    assert(slider_backend->value == 11);
    assert(slider_backend->last_signal == PICOUI_BACKEND_SIGNAL_VALUE_CHANGED);
    assert(slider_backend->dispatch_count == 1);
    assert(picoui_slider_set_value(slider, 11) == 0);
    assert(slider_value_count == 1);
    assert(slider_backend->value == 11);
    assert(slider_backend->dispatch_count == 1);
    test_backend_value_changed_bridge_emits_real_queue(slider, slider_backend);

    assert(picoui_switch_set_on_toggled(sw, on_switch_toggle, 0) == 0);
    assert(picoui_checkbox_set_on_toggled(cb, on_checkbox_toggle, 0) == 0);
    assert(picoui_slider_set_on_value_changed(slider, on_slider, 0) == 0);
    assert(picoui_button_set_on_clicked(button, on_button_clicked, &button_cookie) == 0);
    assert(button->on_clicked == on_button_clicked);
    assert(button->user_data == &button_cookie);
    button->on_clicked(&button->widget, button->user_data);
    assert(button_clicked == button_cookie);

    assert(picoui_button_set_text(button, "launch") == 0);
    assert(picoui_widget_set_style_class(&button->widget, "primary") == 0);
    assert(picoui_widget_set_user_data(&button->widget, &button_cookie) == 0);
    assert(picoui_widget_set_bg_color(&button->widget, 0x112233) == 0);
    assert(picoui_widget_set_text_color(&button->widget, 0x445566) == 0);
    assert(picoui_widget_set_border_color(&button->widget, 0x778899) == 0);
    assert(picoui_widget_set_radius(&button->widget, 8) == 0);
    assert(picoui_widget_set_padding(&button->widget, 12) == 0);
    assert(button->widget.text == (const char *)"launch");
    assert(button->widget.style_class == (const char *)"primary");
    assert(button->widget.user_data == &button_cookie);
    assert(button_backend->style_class == (const char *)"primary");
    assert(button_backend->user_data == &button_cookie);
    assert(button->widget.bg_color == 0x112233);
    assert(button->widget.text_color == 0x445566);
    assert(button->widget.border_color == 0x778899);
    assert(button->widget.radius == 8);
    assert(button->widget.padding == 12);
    assert(picoui_widget_set_radius(&button->widget, -1) == -1);
    assert(picoui_widget_set_padding(&button->widget, -1) == -1);
    assert(picoui_widget_set_text(&button->widget, 0) == -1);
    assert(picoui_checkbox_set_text(0, "x") == -1);
    assert(picoui_checkbox_set_text(cb, 0) == -1);
    assert(picoui_checkbox_create_with_props(win, 0) == 0);

    assert(picoui_label_set_text(label, "hello") == 0);
    assert(picoui_label_set_font(label, &font) == 0);
    assert(label->widget.text == (const char *)"hello");
    assert(label->widget.font == &font);
    assert(label_backend->font == &font);

    assert(picoui_text_set_text(text, "world") == 0);
    assert(picoui_text_set_font(text, &font) == 0);
    assert(text->widget.text == (const char *)"world");
    assert(text->widget.font == &font);
    assert(text_backend->font == &font);

    picoui_app_destroy(app);
    return 0;
}
