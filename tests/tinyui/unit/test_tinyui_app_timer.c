#include "core/app.h"
#include "internal.h"

#include <assert.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

static const char *test_self_binary_path = 0;

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

static void assert_command_success(const char *command)
{
    int rc = system(command);
    if (rc == 0) {
        return;
    }

    fprintf(stderr, "command failed (%d): %s\n", rc, command);
    abort();
}

static void assert_archive_lacks_symbol(const char *archive_path, const char *symbol)
{
    char command[1024];

    snprintf(command,
             sizeof(command),
             "nm %s | awk '{print $NF}' | grep -E '^(%s|_%s)$' >/dev/null",
             archive_path,
             symbol,
             symbol);
    if (system(command) == 0) {
        fprintf(stderr, "unexpected archive symbol present: %s in %s\n", symbol, archive_path);
        abort();
    }
}

static void assert_self_binary_lacks_symbol(const char *symbol)
{
    char command[1024];

    snprintf(command,
             sizeof(command),
             "nm %s | awk '{print $NF}' | grep -E '^(%s|_%s)$' >/dev/null",
             test_self_binary_path,
             symbol,
             symbol);
    if (system(command) == 0) {
        fprintf(stderr, "unexpected self symbol present: %s in %s\n", symbol, test_self_binary_path);
        abort();
    }
}

static void assert_source_contains_text(const char *source_path, const char *needle)
{
    char command[1024];

    snprintf(command,
             sizeof(command),
             "python3 - '%s' '%s' <<'PY'\n"
             "from pathlib import Path\n"
             "import sys\n"
             "text = Path(sys.argv[1]).read_text()\n"
             "raise SystemExit(0 if sys.argv[2] in text else 1)\n"
             "PY",
             source_path,
             needle);
    if (system(command) != 0) {
        fprintf(stderr, "expected source text missing: %s in %s\n", needle, source_path);
        abort();
    }
}

static void assert_source_lacks_text(const char *source_path, const char *needle)
{
    char command[1024];

    snprintf(command,
             sizeof(command),
             "python3 - '%s' '%s' <<'PY'\n"
             "from pathlib import Path\n"
             "import sys\n"
             "text = Path(sys.argv[1]).read_text()\n"
             "raise SystemExit(1 if sys.argv[2] in text else 0)\n"
             "PY",
             source_path,
             needle);
    if (system(command) != 0) {
        fprintf(stderr, "unexpected source text present: %s in %s\n", needle, source_path);
        abort();
    }
}

static void require_condition(int condition)
{
    if (!condition) {
        abort();
    }
}

struct timer_callback_probe {
    int call_count;
    struct tinyui_app *last_app;
    struct tinyui_app_timer *last_timer;
    void *last_user_data;
};

struct timer_relink_chain_probe {
    int call_count;
    struct tinyui_app_timer *resume_timer;
};

static void timer_probe_callback(struct tinyui_app *app,
                                 struct tinyui_app_timer *timer,
                                 void *user_data)
{
    struct timer_callback_probe *probe = (struct timer_callback_probe *)user_data;

    if (probe == NULL) {
        return;
    }

    probe->call_count += 1;
    probe->last_app = app;
    probe->last_timer = timer;
    probe->last_user_data = user_data;
}

static void timer_relink_chain_callback(struct tinyui_app *app,
                                        struct tinyui_app_timer *timer,
                                        void *user_data)
{
    struct timer_relink_chain_probe *probe = (struct timer_relink_chain_probe *)user_data;

    if (probe == NULL) {
        return;
    }

    probe->call_count += 1;
    if (app != NULL) {
        app->timers = probe->resume_timer;
    }
    if (probe->resume_timer != NULL) {
        probe->resume_timer->next = NULL;
    }
    tinyui_app_timer_destroy(timer);
}

static void timer_must_not_fire_callback(struct tinyui_app *app,
                                         struct tinyui_app_timer *timer,
                                         void *user_data)
{
    (void)app;
    (void)timer;
    (void)user_data;
    abort();
}

static void test_timer_rejects_null_app(void)
{
    assert(tinyui_app_timer_create(NULL) == NULL);
}

static void test_timer_callback_contract_shape(void)
{
    struct timer_callback_probe probe = {0};
    tinyui_app_timer_cb_t callback = timer_probe_callback;

    callback(NULL, NULL, &probe);
    assert(probe.call_count == 1);
    assert(probe.last_app == NULL);
    assert(probe.last_timer == NULL);
    assert(probe.last_user_data == &probe);
}

static void test_timer_running_state_contract(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_app_timer *timer;

    assert(app != NULL);
    timer = tinyui_app_timer_create(app);
    assert(timer != NULL);

    assert(tinyui_app_timer_is_running(timer) == 0);
    require_condition(tinyui_app_timer_start(timer, 100, 1, timer_probe_callback, NULL) == 0);
    require_condition(tinyui_app_timer_is_running(timer) == 1);
    require_condition(tinyui_app_timer_stop(timer) == 0);
    require_condition(tinyui_app_timer_is_running(timer) == 0);

    tinyui_app_timer_destroy(timer);
    tinyui_app_destroy(app);
}

static void test_timer_start_rejects_invalid_arguments(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_app_timer *timer;

    assert(app != NULL);
    timer = tinyui_app_timer_create(app);
    assert(timer != NULL);

    assert(tinyui_app_timer_start(timer, 0, 1, timer_probe_callback, NULL) == -1);
    assert(tinyui_app_timer_start(timer, 100, 1, NULL, NULL) == -1);

    tinyui_app_timer_destroy(timer);
    tinyui_app_destroy(app);
}

static void test_timer_destroy_after_stop_is_safe(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_app_timer *timer;
    struct timer_callback_probe probe = {0};

    assert(app != NULL);
    timer = tinyui_app_timer_create(app);
    assert(timer != NULL);

    assert(tinyui_app_timer_start(timer, 100, 1, timer_probe_callback, &probe) == 0);
    assert(tinyui_app_timer_stop(timer) == 0);
    tinyui_app_timer_destroy(timer);
    tinyui_app_destroy(app);
}

static void test_app_destroy_cleans_residual_timers(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_app_timer *timer1;
    struct tinyui_app_timer *timer2;
    struct timer_callback_probe probe = {0};

    assert(app != NULL);
    timer1 = tinyui_app_timer_create(app);
    timer2 = tinyui_app_timer_create(app);
    assert(timer1 != NULL);
    assert(timer2 != NULL);

    require_condition(tinyui_app_timer_start(timer1, 100, 1, timer_probe_callback, &probe) == 0);
    require_condition(tinyui_app_timer_is_running(timer1) == 1);
    require_condition(tinyui_app_timer_is_running(timer2) == 0);
    require_condition(app->timers != NULL);

    tinyui_app_destroy(app);
}

static void test_repeating_timer_pump_keeps_running(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_app_timer *timer;
    struct timer_callback_probe probe = {0};

    assert(app != NULL);
    timer = tinyui_app_timer_create(app);
    assert(timer != NULL);

    require_condition(tinyui_app_timer_start(timer, 50, 1, timer_probe_callback, &probe) == 0);

    assert(probe.call_count == 0);
    tinyui_app_pump_timers(app, 1000);
    tinyui_app_pump_timers(app, 1050);
    assert(probe.call_count == 1);
    assert(tinyui_app_timer_is_running(timer) == 1);

    tinyui_app_pump_timers(app, 1100);
    assert(probe.call_count == 2);
    assert(tinyui_app_timer_is_running(timer) == 1);

    tinyui_app_timer_destroy(timer);
    tinyui_app_destroy(app);
}

static void test_one_shot_timer_pump_stops_after_fire(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_app_timer *timer;
    struct timer_callback_probe probe = {0};

    assert(app != NULL);
    timer = tinyui_app_timer_create(app);
    assert(timer != NULL);

    require_condition(tinyui_app_timer_start(timer, 50, 0, timer_probe_callback, &probe) == 0);

    assert(probe.call_count == 0);
    tinyui_app_pump_timers(app, 2000);
    tinyui_app_pump_timers(app, 2050);
    assert(probe.call_count == 1);
    assert(tinyui_app_timer_is_running(timer) == 0);

    tinyui_app_pump_timers(app, 2100);
    assert(probe.call_count == 1);
    assert(tinyui_app_timer_is_running(timer) == 0);

    tinyui_app_timer_destroy(timer);
    tinyui_app_destroy(app);
}

static void test_timer_pump_skips_detached_successor_after_callback_relink(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_app_timer *tail_timer;
    struct tinyui_app_timer *middle_timer;
    struct tinyui_app_timer *head_timer;
    struct timer_callback_probe tail_probe = {0};
    struct timer_relink_chain_probe head_probe = {0};

    assert(app != NULL);
    tail_timer = tinyui_app_timer_create(app);
    middle_timer = tinyui_app_timer_create(app);
    head_timer = tinyui_app_timer_create(app);
    assert(tail_timer != NULL);
    assert(middle_timer != NULL);
    assert(head_timer != NULL);

    head_probe.resume_timer = tail_timer;

    require_condition(tinyui_app_timer_start(tail_timer, 50, 0, timer_probe_callback, &tail_probe) == 0);
    require_condition(tinyui_app_timer_start(middle_timer, 50, 0, timer_must_not_fire_callback, NULL) == 0);
    require_condition(tinyui_app_timer_start(head_timer, 50, 0, timer_relink_chain_callback, &head_probe) == 0);

    tinyui_app_pump_timers(app, 3000);
    tinyui_app_pump_timers(app, 3050);

    assert(head_probe.call_count == 1);
    assert(tail_probe.call_count == 0);

    tinyui_app_pump_timers(app, 3100);
    assert(tail_probe.call_count == 1);

    tinyui_app_timer_destroy(middle_timer);
    tinyui_app_timer_destroy(tail_timer);
    tinyui_app_destroy(app);
}

static void test_timer_pump_backend_symbol_is_no_longer_public(void)
{
    char exists_command[4096];

    assert_command_success("test -f ../../libtinyui_backend_ldgui_porting.a");
    snprintf(exists_command, sizeof(exists_command), "test -f %s", test_self_binary_path);
    assert_command_success(exists_command);
    assert_archive_lacks_symbol("../../libtinyui_backend_ldgui_porting.a", "tinyui_backend_test_pump_timers");
    assert_self_binary_lacks_symbol("tinyui_backend_test_pump_timers");
}

static void test_app_timer_internal_seam_uses_tinyui_prefix(void)
{
    assert_source_contains_text(test_repo_path("tinyui/src/core/app.c"),
                                "static void tinyui_app_timer_unlink");
    assert_self_binary_lacks_symbol("tinyui_app_timer_unlink");
}

int main(int argc, char **argv)
{
    (void)argc;
    test_self_binary_path = (argv != NULL && argv[0] != NULL)
        ? argv[0]
        : test_repo_path("build/tests/tinyui/test_tinyui_app_timer");
    assert_self_binary_lacks_symbol("tinyui_backend_test_pump_timers");
    test_timer_rejects_null_app();
    test_timer_callback_contract_shape();
    test_timer_running_state_contract();
    test_timer_start_rejects_invalid_arguments();
    test_timer_destroy_after_stop_is_safe();
    test_app_destroy_cleans_residual_timers();
    test_repeating_timer_pump_keeps_running();
    test_one_shot_timer_pump_stops_after_fire();
    test_timer_pump_skips_detached_successor_after_callback_relink();
    test_timer_pump_backend_symbol_is_no_longer_public();
    test_app_timer_internal_seam_uses_tinyui_prefix();
    return 0;
}
