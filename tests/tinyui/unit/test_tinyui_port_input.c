#include "app.h"
#include "indev.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

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

static void test_input_internal_helper_no_longer_uses_picoui_prefix(void)
{
    assert_source_lacks_static_definition(
        "/Users/cys/embedded/LingDongGUI/tinyui/src/indev/indev.c",
        "picoui_input_key_is_valid");
}

static void test_pointer_defaults_and_round_trip(void)
{
    struct picoui_app *app = picoui_app_create();
    int x = 1;
    int y = 1;
    int pressed = 1;

    assert(app != NULL);
    assert(picoui_input_get_pointer(app, &x, &y, &pressed) == 0);
    assert(x == 0);
    assert(y == 0);
    assert(pressed == 0);

    assert(picoui_input_push_pointer(app, 12, 34, 1) == 0);
    x = 0;
    y = 0;
    pressed = 0;
    assert(picoui_input_get_pointer(app, &x, &y, &pressed) == 0);
    assert(x == 12);
    assert(y == 34);
    assert(pressed == 1);

    picoui_app_destroy(app);
}

static void test_key_defaults_and_round_trip(void)
{
    struct picoui_app *app = picoui_app_create();
    enum picoui_input_key key = PICOUI_INPUT_KEY_ENTER;
    int pressed = 1;

    assert(app != NULL);
    assert(picoui_input_get_key(app, &key, &pressed) == 0);
    assert(key == PICOUI_INPUT_KEY_NONE);
    assert(pressed == 0);

    assert(picoui_input_push_key(app, PICOUI_INPUT_KEY_LEFT, 1) == 0);
    key = PICOUI_INPUT_KEY_NONE;
    pressed = 0;
    assert(picoui_input_get_key(app, &key, &pressed) == 0);
    assert(key == PICOUI_INPUT_KEY_LEFT);
    assert(pressed == 1);

    picoui_app_destroy(app);
}

static void test_input_rejects_invalid_arguments(void)
{
    struct picoui_app *app = picoui_app_create();
    int x = 0;
    int y = 0;
    int pressed = 0;
    enum picoui_input_key key = PICOUI_INPUT_KEY_NONE;

    assert(app != NULL);
    assert(picoui_input_push_pointer(NULL, 1, 2, 1) == -1);
    assert(picoui_input_get_pointer(NULL, &x, &y, &pressed) == -1);
    assert(picoui_input_get_pointer(app, NULL, &y, &pressed) == -1);
    assert(picoui_input_push_key(NULL, PICOUI_INPUT_KEY_LEFT, 1) == -1);
    assert(picoui_input_push_key(app, (enum picoui_input_key)999, 1) == -1);
    assert(picoui_input_get_key(NULL, &key, &pressed) == -1);
    assert(picoui_input_get_key(app, NULL, &pressed) == -1);

    picoui_app_destroy(app);
}

int main(void)
{
    test_input_internal_helper_no_longer_uses_picoui_prefix();
    test_pointer_defaults_and_round_trip();
    test_key_defaults_and_round_trip();
    test_input_rejects_invalid_arguments();
    return 0;
}
