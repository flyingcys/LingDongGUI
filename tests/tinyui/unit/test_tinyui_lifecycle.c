#include <assert.h>
#include <string.h>
#include "internal.h"

static void test_registry_register_lookup_unregister(void)
{
    struct tinyui_app app;
    struct tinyui_widget a, b;
    memset(&app, 0, sizeof(app));
    memset(&a, 0, sizeof(a));
    memset(&b, 0, sizeof(b));
    a.ld_name_id = 1; b.ld_name_id = 2;

    tinyui_app_register_host(&app, &a);
    tinyui_app_register_host(&app, &b);
    assert(tinyui_app_lookup_host(&app, 1) == &a);
    assert(tinyui_app_lookup_host(&app, 2) == &b);
    assert(tinyui_app_lookup_host(&app, 99) == 0);

    tinyui_app_unregister_host(&app, &a);
    assert(tinyui_app_lookup_host(&app, 1) == 0);
    assert(tinyui_app_lookup_host(&app, 2) == &b);

    tinyui_app_unregister_host(&app, &b);
    assert(app.host_list_head == 0);
}

int main(void)
{
    test_registry_register_lookup_unregister();
    return 0;
}
