/*
 * Stub implementations of the virtual display mutex hooks.
 * The SDL demo runs single-threaded, so these are no-ops.
 * Providing strong definitions here prevents LTO from leaving
 * the weak symbols in tinyui_core unresolved across archives.
 */
void VT_enter_global_mutex(void) {}
void VT_leave_global_mutex(void) {}
