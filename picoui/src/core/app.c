#include "internal.h"
#include "picoui/app.h"

#include <stdlib.h>

struct picoui_app *picoui_app_create(void)
{
    return calloc(1, sizeof(struct picoui_app));
}

void picoui_app_destroy(struct picoui_app *app)
{
    free(app);
}
