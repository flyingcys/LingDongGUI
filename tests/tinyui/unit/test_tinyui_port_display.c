#include "app.h"
#include "display.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *test_repo_path(const char *relative_path)
{
    static char path[2048];
    char base[2048];
    char *tests_dir;

    snprintf(base, sizeof(base), "%s", __FILE__);
    tests_dir = strstr(base, "tests/tinyui/unit/");
    assert(tests_dir != 0);
    *tests_dir = '\0';
    snprintf(path, sizeof(path), "%s%s", base, relative_path);
    return path;
}

static void assert_source_lacks_static_definition(const char *path, const char *symbol_name)
{
    char command[1024];

    snprintf(command,
             sizeof(command),
             "rg -n \"^[[:space:]]*static[[:space:]].*%s[[:space:]]*(=|\\()\" %s >/dev/null",
             symbol_name,
             path);
    if (system(command) == 0) {
        fprintf(stderr, "unexpected old static helper still present: %s in %s\n", symbol_name, path);
        abort();
    }
}

static void test_display_internal_helpers_no_longer_use_tinyui_prefix(void)
{
    const char *source = test_repo_path("tinyui/src/display/display.c");

    assert(source != 0);
}

static void test_default_display_config(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_display_config config = {0};

    assert(app != NULL);
    assert(tinyui_display_get_config(app, &config) == 0);
    assert(config.width == 480);
    assert(config.height == 320);
    assert(config.color_format == TINYUI_COLOR_FORMAT_RGB565);
    assert(config.buffer_height == 0);
    assert(config.user_data == NULL);

    tinyui_app_destroy(app);
}

static void test_display_config_round_trip(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_display_config config = {
        .width = 320,
        .height = 240,
        .color_format = TINYUI_COLOR_FORMAT_ARGB8888,
        .buffer_height = 32,
        .user_data = (void *)(uintptr_t)0x1234,
    };
    struct tinyui_display_config readback = {0};

    assert(app != NULL);
    assert(tinyui_display_set_config(app, &config) == 0);
    assert(tinyui_display_get_config(app, &readback) == 0);
    assert(readback.width == 320);
    assert(readback.height == 240);
    assert(readback.color_format == TINYUI_COLOR_FORMAT_ARGB8888);
    assert(readback.buffer_height == 32);
    assert(readback.user_data == (void *)(uintptr_t)0x1234);

    tinyui_app_destroy(app);
}

static void test_display_rejects_invalid_config(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_display_config config = {
        .width = 0,
        .height = 240,
        .color_format = TINYUI_COLOR_FORMAT_RGB565,
        .buffer_height = 0,
        .user_data = NULL,
    };

    assert(app != NULL);
    assert(tinyui_display_set_config(app, NULL) == -1);
    assert(tinyui_display_get_config(NULL, &config) == -1);
    assert(tinyui_display_get_config(app, NULL) == -1);
    assert(tinyui_display_set_config(app, &config) == -1);

    config.width = 320;
    config.height = -1;
    assert(tinyui_display_set_config(app, &config) == -1);

    tinyui_app_destroy(app);
}

int main(void)
{
    test_display_internal_helpers_no_longer_use_tinyui_prefix();
    test_default_display_config();
    test_display_config_round_trip();
    test_display_rejects_invalid_config();
    return 0;
}
