#include "picoui/picoui.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldCanvas.h"
#include "internal.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *repo_root_from_file(const char *file)
{
    static char root[1024];
    const char *suffix = "/tests/tinyui/unit/test_tinyui_canvas.c";
    const char *hit = strstr(file, suffix);
    size_t len;

    assert(hit != 0);
    len = (size_t)(hit - file);
    assert(len < sizeof(root));
    memcpy(root, file, len);
    root[len] = '\0';
    return root;
}

static char *read_file_text(const char *path)
{
    FILE *fp = fopen(path, "rb");
    long size;
    char *buf;

    assert(fp != 0);
    assert(fseek(fp, 0, SEEK_END) == 0);
    size = ftell(fp);
    assert(size >= 0);
    assert(fseek(fp, 0, SEEK_SET) == 0);

    buf = (char *)malloc((size_t)size + 1U);
    assert(buf != 0);
    assert(fread(buf, 1U, (size_t)size, fp) == (size_t)size);
    buf[size] = '\0';
    assert(fclose(fp) == 0);
    return buf;
}

static bool source_has_function_definition(const char *source, const char *name)
{
    char needle[256];
    assert((size_t)snprintf(needle, sizeof(needle), "static %s(", name) < sizeof(needle)
           || (size_t)snprintf(needle, sizeof(needle), "static int %s(", name) < sizeof(needle));
    return strstr(source, needle) != 0;
}

static void assert_source_has_function_definition(const char *source, const char *name)
{
    char needle[256];
    int written = snprintf(needle, sizeof(needle), "static %s(", name);
    if (written > 0 && (size_t)written < sizeof(needle) && strstr(source, needle) != 0) {
        return;
    }
    written = snprintf(needle, sizeof(needle), "static int %s(", name);
    if (written > 0 && (size_t)written < sizeof(needle) && strstr(source, needle) != 0) {
        return;
    }
    written = snprintf(needle, sizeof(needle), "static ldColor %s(", name);
    if (written > 0 && (size_t)written < sizeof(needle) && strstr(source, needle) != 0) {
        return;
    }
    written = snprintf(needle, sizeof(needle), "static arm_2d_align_t %s(", name);
    assert(written > 0 && (size_t)written < sizeof(needle) && strstr(source, needle) != 0);
}

static void assert_source_lacks_function_definition(const char *source, const char *name)
{
    char needle[256];
    int written = snprintf(needle, sizeof(needle), "static %s(", name);
    assert(!(written > 0 && (size_t)written < sizeof(needle) && strstr(source, needle) != 0));
    written = snprintf(needle, sizeof(needle), "static int %s(", name);
    assert(!(written > 0 && (size_t)written < sizeof(needle) && strstr(source, needle) != 0));
    written = snprintf(needle, sizeof(needle), "static ldColor %s(", name);
    assert(!(written > 0 && (size_t)written < sizeof(needle) && strstr(source, needle) != 0));
    written = snprintf(needle, sizeof(needle), "static arm_2d_align_t %s(", name);
    assert(!(written > 0 && (size_t)written < sizeof(needle) && strstr(source, needle) != 0));
}

static unsigned int encode_ld_color(unsigned int rgb)
{
    return (unsigned int)__RGB((rgb >> 16) & 0xFFU, (rgb >> 8) & 0xFFU, rgb & 0xFFU);
}

int main(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_window *win = picoui_window_create(app, "root");
    struct picoui_canvas *canvas = picoui_canvas_create(win, "canvas");
    char mutable_text[] = "canvas";
    arm_2d_tile_t image_tile = {0};
    arm_2d_tile_t mask_tile = {0};
    struct picoui_image_source image_source = {
        .img_tile = &image_tile,
        .mask_tile = &mask_tile,
    };
    const struct picoui_backend_widget *backend;
    const struct picoui_backend_widget *parent_backend;
    const ldCanvas_t *ld_canvas;
    const char *repo_root = repo_root_from_file(__FILE__);
    char canvas_source_path[1200];
    char *canvas_source;
    int command_count = 0;

    assert(app != 0);
    assert(win != 0);
    assert(canvas != 0);
    assert((size_t)snprintf(canvas_source_path,
                            sizeof(canvas_source_path),
                            "%s/tinyui/src/widgets/canvas.c",
                            repo_root) < sizeof(canvas_source_path));
    canvas_source = read_file_text(canvas_source_path);
    assert_source_lacks_function_definition(canvas_source, "picoui_canvas_rgb_to_ld");
    assert_source_lacks_function_definition(canvas_source, "picoui_canvas_align_to_ld");
    assert_source_lacks_function_definition(canvas_source, "picoui_canvas_push_native");
    assert_source_lacks_function_definition(canvas_source, "picoui_canvas_clear_native");
    assert_source_lacks_function_definition(canvas_source, "picoui_canvas_is_valid");
    assert_source_lacks_function_definition(canvas_source, "picoui_canvas_push");
    assert_source_has_function_definition(canvas_source, "tinyui_canvas_rgb_to_ld");
    assert_source_has_function_definition(canvas_source, "tinyui_canvas_align_to_ld");
    assert_source_has_function_definition(canvas_source, "tinyui_canvas_push_native");
    assert_source_has_function_definition(canvas_source, "tinyui_canvas_clear_native");
    assert_source_has_function_definition(canvas_source, "tinyui_canvas_is_valid");
    assert_source_has_function_definition(canvas_source, "tinyui_canvas_push");
    assert(picoui_widget_set_size((struct picoui_widget *)canvas, 120, 80) == 0);

    assert(picoui_canvas_fill_rect(canvas, 1, 2, 30, 40, 0x112233U, 200) == 0);
    assert(picoui_canvas_draw_line(canvas, 0, 0, 20, 10, 3, 0x445566U, 255, 32) == 0);
    assert(picoui_canvas_draw_image(canvas, 5, 6, 24, 18, &image_source, 0x778899U, 180) == 0);
    assert(picoui_canvas_draw_image_scaled(canvas, 7, 8, 32, 20, &image_source, 0.5f, 210) == 0);
    assert(picoui_canvas_draw_text(canvas,
                                   9,
                                   10,
                                   50,
                                   16,
                                   mutable_text,
                                   PICOUI_ALIGN_CENTER,
                                   0xAABBCCU,
                                   255) == 0);

    assert(picoui_canvas_get_command_count(canvas, &command_count) == 0);
    assert(command_count == 5);

    backend = canvas->widget.backend_widget;
    assert(backend != 0);
    parent_backend = win->widget.backend_widget;
    assert(parent_backend != 0);
    assert(backend->parent == parent_backend);
    assert(backend->root == parent_backend->root);
    assert(backend->owner == parent_backend->owner);
    assert(backend->host_widget == &canvas->widget);
    ld_canvas = (const ldCanvas_t *)backend->ld_widget;
    assert(ld_canvas != 0);
    assert(ld_canvas->use_as__ldBase_t.widgetType == widgetTypeCanvas);
    assert(ld_canvas->use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iWidth == 120);
    assert(ld_canvas->use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iHeight == 80);
    assert(ld_canvas->commandCount == 5);
    assert(ld_canvas->commands[0].kind == ldCanvasCommandFillRect);
    assert(ld_canvas->commands[0].region.tLocation.iX == 1);
    assert(ld_canvas->commands[0].region.tLocation.iY == 2);
    assert(ld_canvas->commands[0].region.tSize.iWidth == 30);
    assert(ld_canvas->commands[0].region.tSize.iHeight == 40);
    assert(ld_canvas->commands[0].color0 == encode_ld_color(0x112233U));
    assert(ld_canvas->commands[0].opacity0 == 200);
    assert(ld_canvas->commands[1].kind == ldCanvasCommandDrawLine);
    assert(ld_canvas->commands[1].x1 == 20);
    assert(ld_canvas->commands[1].y1 == 10);
    assert(ld_canvas->commands[1].lineSize == 3);
    assert(ld_canvas->commands[1].color0 == encode_ld_color(0x445566U));
    assert(ld_canvas->commands[1].opacity0 == 255);
    assert(ld_canvas->commands[1].opacity1 == 32);
    assert(ld_canvas->commands[2].kind == ldCanvasCommandDrawImage);
    assert(ld_canvas->commands[2].ptImgTile == &image_tile);
    assert(ld_canvas->commands[2].ptMaskTile == &mask_tile);
    assert(ld_canvas->commands[2].color0 == encode_ld_color(0x778899U));
    assert(ld_canvas->commands[2].opacity0 == 180);
    assert(ld_canvas->commands[3].kind == ldCanvasCommandDrawImageScale);
    assert(ld_canvas->commands[3].ptImgTile == &image_tile);
    assert(ld_canvas->commands[3].ptMaskTile == &mask_tile);
    assert(ld_canvas->commands[3].scale == 0.5f);
    assert(ld_canvas->commands[3].opacity0 == 210);
    assert(ld_canvas->commands[4].kind == ldCanvasCommandDrawText);
    assert(ld_canvas->commands[4].align == ARM_2D_ALIGN_CENTRE);
    assert(ld_canvas->commands[4].color0 == encode_ld_color(0xAABBCCU));
    assert(ld_canvas->commands[4].opacity0 == 255);
    assert(strcmp((const char *)ld_canvas->commands[4].pStr, "canvas") == 0);

    mutable_text[0] = 'X';
    assert(picoui_canvas_fill_rect(canvas, 11, 12, 13, 14, 0x010203U, 99) == 0);
    assert(ld_canvas->commandCount == 6);
    assert(strcmp((const char *)ld_canvas->commands[4].pStr, "canvas") == 0);
    assert(ld_canvas->commands[5].kind == ldCanvasCommandFillRect);
    assert(ld_canvas->commands[5].region.tLocation.iX == 11);
    assert(ld_canvas->commands[5].region.tLocation.iY == 12);
    assert(ld_canvas->commands[5].region.tSize.iWidth == 13);
    assert(ld_canvas->commands[5].region.tSize.iHeight == 14);
    assert(ld_canvas->commands[5].color0 == encode_ld_color(0x010203U));
    assert(ld_canvas->commands[5].opacity0 == 99);

    assert(picoui_canvas_clear(canvas) == 0);
    assert(picoui_canvas_get_command_count(canvas, &command_count) == 0);
    assert(command_count == 0);
    assert(ld_canvas->commandCount == 0);
    assert(ld_canvas->commands[4].pStr == 0);

    assert(picoui_canvas_fill_rect(0, 0, 0, 1, 1, 0, 255) == -1);
    assert(picoui_canvas_draw_line(0, 0, 0, 1, 1, 1, 0, 255, 255) == -1);
    assert(picoui_canvas_draw_image(canvas, 0, 0, 10, 10, 0, 0, 255) == -1);
    assert(picoui_canvas_draw_image_scaled(canvas, 0, 0, 10, 10, 0, 1.0f, 255) == -1);
    assert(picoui_canvas_draw_text(canvas, 0, 0, 10, 10, 0, PICOUI_ALIGN_START, 0, 255) == -1);
    assert(picoui_canvas_get_command_count(canvas, 0) == -1);

    picoui_app_destroy(app);
    free(canvas_source);
    return 0;
}
