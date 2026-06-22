#include "app.h"
#include "indev.h"

#include <assert.h>
#include <stddef.h>
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

static void test_input_internal_helper_no_longer_uses_tinyui_prefix(void)
{
    assert(test_repo_path("tinyui/src/indev/indev.c") != 0);
}

static void test_pointer_defaults_and_round_trip(void)
{
    struct tinyui_app *app = tinyui_app_create();
    int x = 1;
    int y = 1;
    int pressed = 1;

    assert(app != NULL);
    assert(tinyui_input_get_pointer(app, &x, &y, &pressed) == 0);
    assert(x == 0);
    assert(y == 0);
    assert(pressed == 0);

    assert(tinyui_input_push_pointer(app, 12, 34, 1) == 0);
    x = 0;
    y = 0;
    pressed = 0;
    assert(tinyui_input_get_pointer(app, &x, &y, &pressed) == 0);
    assert(x == 12);
    assert(y == 34);
    assert(pressed == 1);

    tinyui_app_destroy(app);
}

static void test_key_defaults_and_round_trip(void)
{
    struct tinyui_app *app = tinyui_app_create();
    enum tinyui_input_key key = TINYUI_INPUT_KEY_ENTER;
    int pressed = 1;

    assert(app != NULL);
    assert(tinyui_input_get_key(app, &key, &pressed) == 0);
    assert(key == TINYUI_INPUT_KEY_NONE);
    assert(pressed == 0);

    assert(tinyui_input_push_key(app, TINYUI_INPUT_KEY_LEFT, 1) == 0);
    key = TINYUI_INPUT_KEY_NONE;
    pressed = 0;
    assert(tinyui_input_get_key(app, &key, &pressed) == 0);
    assert(key == TINYUI_INPUT_KEY_LEFT);
    assert(pressed == 1);

    tinyui_app_destroy(app);
}

static void test_input_rejects_invalid_arguments(void)
{
    struct tinyui_app *app = tinyui_app_create();
    int x = 0;
    int y = 0;
    int pressed = 0;
    enum tinyui_input_key key = TINYUI_INPUT_KEY_NONE;

    assert(app != NULL);
    assert(tinyui_input_push_pointer(NULL, 1, 2, 1) == -1);
    assert(tinyui_input_get_pointer(NULL, &x, &y, &pressed) == -1);
    assert(tinyui_input_get_pointer(app, NULL, &y, &pressed) == -1);
    assert(tinyui_input_push_key(NULL, TINYUI_INPUT_KEY_LEFT, 1) == -1);
    assert(tinyui_input_push_key(app, (enum tinyui_input_key)999, 1) == -1);
    assert(tinyui_input_get_key(NULL, &key, &pressed) == -1);
    assert(tinyui_input_get_key(app, NULL, &pressed) == -1);

    tinyui_app_destroy(app);
}

int main(void)
{
    test_input_internal_helper_no_longer_uses_tinyui_prefix();
    test_pointer_defaults_and_round_trip();
    test_key_defaults_and_round_trip();
    test_input_rejects_invalid_arguments();
    return 0;
}
