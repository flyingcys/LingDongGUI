#include "internal.h"
#include "picoui/image.h"

#include <stdlib.h>

struct picoui_image *picoui_image_create(struct picoui_window *parent, const char *id)
{
    struct picoui_image *image;

    if (parent == 0 || id == 0) {
        return 0;
    }

    image = calloc(1, sizeof(*image));
    if (image == 0) {
        return 0;
    }

    image->widget.backend_widget = picoui_backend_create_image(parent->widget.backend_widget, id);
    if (image->widget.backend_widget == 0) {
        free(image);
        return 0;
    }

    image->id = id;
    image->widget.visible = 1;
    image->widget.enabled = 1;
    return image;
}

int picoui_image_set_source(struct picoui_image *image, struct picoui_image_source *source)
{
    if (image == 0 || source == 0) {
        return -1;
    }

    image->source = source;
    return picoui_backend_set_image_source(image->widget.backend_widget, source);
}
