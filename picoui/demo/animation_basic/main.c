#include "picoui/picoui.h"
#include "picoui/image.h"

extern const unsigned char c_tileQuaterArcGRAY8;

static struct picoui_image_source s_animation_source = {
    .img_tile = (void *)&c_tileQuaterArcGRAY8,
    .mask_tile = 0,
};

static void make_ui(struct picoui_window *win)
{
    struct picoui_label *title;
    struct picoui_animation *animation;
    struct picoui_animation_props props = {
        .id = "animation",
        .width = 62,
        .height = 31,
        .period_ms = 120,
        .source = &s_animation_source,
    };

    title = picoui_label_create(win, "title");
    animation = picoui_animation_create_with_props((struct picoui_widget *)win, &props);

    picoui_label_set_text(title, "Animation");
    picoui_widget_set_pos((struct picoui_widget *)title, 32, 32);
    picoui_widget_set_pos((struct picoui_widget *)animation, 160, 120);
    (void)picoui_animation_show_frame(animation, 0);
}

static int run_demo(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_window *win;

    if (app == 0) {
        return 1;
    }

    win = picoui_window_create(app, "root");
    if (win == 0) {
        picoui_app_destroy(app);
        return 1;
    }

    make_ui(win);
    if (picoui_app_run(app, win) != 0) {
        picoui_app_destroy(app);
        return 1;
    }
    picoui_app_destroy(app);
    return 0;
}

int main(void)
{
    return run_demo();
}
