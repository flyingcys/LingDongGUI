/*
 * TinyUI animation unit tests — M3 Task 5 L3/L4 harness.
 */

#include "tinyui.h"
#include "../../../src/gui/ldAnimation.h"
#include "../../../src/gui/ldBase.h"
#include "internal.h"
#include "resource/image_source.h"
#include "widgets/animation.h"

#include <assert.h>
#include <string.h>

static void bind_sheet(tinyui_image_source_t *source, arm_2d_tile_t *tile, int w, int h)
{
    memset(source, 0, sizeof(*source));
    source->kind = TINYUI_IMAGE_SOURCE_RGB565_MEMORY;
    tile->tRegion.tSize.iWidth = (int16_t)w;
    tile->tRegion.tSize.iHeight = (int16_t)h;
    memcpy(source->_image_private, tile, sizeof(*tile));
}

static void test_animation_create_default(tinyui_obj_t *root)
{
    tinyui_obj_t *anim = tinyui_animation_create(root);
    struct tinyui_widget *backend;
    ldAnimation_t *ld_anim;

    assert(anim != 0);
    backend = (struct tinyui_widget *)(void *)anim;
    assert(backend->kind == TINYUI_BACKEND_WIDGET_ANIMATION);
    assert(backend->ld_widget != 0);
    ld_anim = (ldAnimation_t *)backend->ld_widget;
    assert(((ldBase_t *)ld_anim)->widgetType == widgetTypeAnimation);
    assert(ld_anim->periodMs == 1);
}

static void test_animation_source_period_frame(tinyui_obj_t *root)
{
    arm_2d_tile_t sheet = {0};
    tinyui_image_source_t source;
    tinyui_animation_props_t props;
    tinyui_obj_t *anim;
    ldAnimation_t *ld_anim;
    struct tinyui_animation *host;

    bind_sheet(&source, &sheet, 40, 20);
    memset(&props, 0, sizeof(props));
    props.fields = TINYUI_ANIMATION_FIELD_SOURCE | TINYUI_ANIMATION_FIELD_WIDTH
                   | TINYUI_ANIMATION_FIELD_HEIGHT | TINYUI_ANIMATION_FIELD_PERIOD_MS;
    props.source = &source;
    props.width = 20;
    props.height = 10;
    props.period_ms = 120;

    anim = tinyui_animation_create_with_props(root, &props);
    assert(anim != 0);
    host = (struct tinyui_animation *)(void *)anim;
    ld_anim = (ldAnimation_t *)host->widget.ld_widget;
    assert(ld_anim != 0);
    assert(ld_anim->ptImgTile == tinyui_image_source_get_image_tile(&source));
    assert(ld_anim->periodMs == 120);
    assert(host->width == 20);
    assert(host->height == 10);
    assert(host->period_ms == 120);
    assert(host->source == &source);

    assert(tinyui_animation_set_period_ms(anim, 200) == 0);
    assert(ld_anim->periodMs == 200);

    assert(tinyui_animation_show_frame(anim, 0) == 0);
    assert(ld_anim->showRegion.tLocation.iX == 0);
    assert(ld_anim->showRegion.tLocation.iY == 0);
    assert(ld_anim->showRegion.tSize.iWidth == 20);
    assert(ld_anim->showRegion.tSize.iHeight == 10);

    assert(tinyui_animation_show_frame(anim, 1) == 0);
    assert(ld_anim->showRegion.tLocation.iX == 20);
    assert(ld_anim->showRegion.tLocation.iY == 0);

    assert(tinyui_animation_show_frame(anim, 2) == 0);
    assert(ld_anim->showRegion.tLocation.iX == 0);
    assert(ld_anim->showRegion.tLocation.iY == 10);

    assert(tinyui_animation_show_frame(anim, 99) == -1);
}

static void test_animation_rejects_invalid(tinyui_obj_t *root)
{
    tinyui_obj_t *anim = tinyui_animation_create(root);
    tinyui_image_source_t empty;

    assert(anim != 0);
    memset(&empty, 0, sizeof(empty));
    assert(tinyui_animation_create(0) == 0);
    assert(tinyui_animation_set_source(0, 0) == -1);
    assert(tinyui_animation_set_source(anim, &empty) == -1);
    assert(tinyui_animation_set_period_ms(anim, 0) == -1);
    assert(tinyui_animation_show_frame(anim, -1) == -1);
}

int main(void)
{
    tinyui_obj_t *root;

    tinyui_deinit();
    assert(tinyui_init() == TINYUI_OK);
    root = tinyui_screen_create();
    assert(root != 0);

    test_animation_create_default(root);
    test_animation_source_period_frame(root);
    test_animation_rejects_invalid(root);

    tinyui_deinit();
    return 0;
}
