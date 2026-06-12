#ifndef PICOUI_TEST_SUPPORT_H
#define PICOUI_TEST_SUPPORT_H

#include "picoui/picoui.h"

int picoui_test_support_stub(void);
const char *picoui_test_repo_path_from_file(const char *file, const char *relative_path);
int picoui_test_source_contains(const char *path, const char *needle);
int picoui_test_source_has_function_definition(const char *path, const char *symbol);
int picoui_test_source_lacks_function_definition(const char *path, const char *symbol);

struct picoui_image_test_dispose_snapshot {
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

struct picoui_qrcode_test_dispose_snapshot {
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

void picoui_backend_image_test_reset_state(void);
struct picoui_image *picoui_backend_image_test_create_with_props_fail_before_size(
    struct picoui_window *parent,
    const struct picoui_image_props *props);
int picoui_backend_image_test_take_last_dispose_snapshot(
    struct picoui_image_test_dispose_snapshot *snapshot);

void picoui_backend_qrcode_test_reset_state(void);
struct picoui_qrcode *picoui_backend_qrcode_test_create_with_props_fail_before_text(
    struct picoui_widget *parent,
    const struct picoui_qrcode_props *props);
int picoui_backend_qrcode_test_take_last_dispose_snapshot(
    struct picoui_qrcode_test_dispose_snapshot *snapshot);

#endif
