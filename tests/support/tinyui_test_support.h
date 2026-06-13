#ifndef TINYUI_TEST_SUPPORT_H
#define TINYUI_TEST_SUPPORT_H

#include "tinyui.h"

int tinyui_test_support_stub(void);
const char *tinyui_test_repo_path_from_file(const char *file, const char *relative_path);
int tinyui_test_source_contains(const char *path, const char *needle);
int tinyui_test_source_has_function_definition(const char *path, const char *symbol);
int tinyui_test_source_lacks_function_definition(const char *path, const char *symbol);

struct tinyui_image_test_dispose_snapshot {
    int kind;
    int cleanup_complete;
    int cleanup_incomplete;
    int detach_result;
    int unbind_result;
    int detached;
    int owner_cleared;
    int root_cleared;
    int parent_cleared;
    int next_sibling_cleared;
    int host_cleared;
    int event_bridge_cleared;
    int ld_pinfo_cleared;
};

struct tinyui_qrcode_test_dispose_snapshot {
    int kind;
    int cleanup_complete;
    int cleanup_incomplete;
    int detach_result;
    int unbind_result;
    int detached;
    int owner_cleared;
    int root_cleared;
    int parent_cleared;
    int next_sibling_cleared;
    int host_cleared;
    int event_bridge_cleared;
    int ld_pinfo_cleared;
};

void tinyui_backend_image_test_reset_state(void);
struct tinyui_image *tinyui_backend_image_test_create_with_props_fail_before_size(
    struct tinyui_window *parent,
    const struct tinyui_image_props *props);
int tinyui_backend_image_test_take_last_dispose_snapshot(
    struct tinyui_image_test_dispose_snapshot *snapshot);

void tinyui_backend_qrcode_test_reset_state(void);
struct tinyui_qrcode *tinyui_backend_qrcode_test_create_with_props_fail_before_text(
    struct tinyui_widget *parent,
    const struct tinyui_qrcode_props *props);
int tinyui_backend_qrcode_test_take_last_dispose_snapshot(
    struct tinyui_qrcode_test_dispose_snapshot *snapshot);

#endif
