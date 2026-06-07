#ifndef PICOUI_RESOURCE_H
#define PICOUI_RESOURCE_H

#include <stddef.h>

struct picoui_resource;

enum picoui_resource_type {
    PICOUI_RESOURCE_TYPE_NONE = 0,
    PICOUI_RESOURCE_TYPE_CONST_MEMORY,
    PICOUI_RESOURCE_TYPE_FILE,
};

struct picoui_resource *picoui_resource_create_from_memory(const void *data, size_t size);
struct picoui_resource *picoui_resource_create_from_file(const char *path);
struct picoui_resource *picoui_resource_ref(struct picoui_resource *resource);
void picoui_resource_unref(struct picoui_resource *resource);
enum picoui_resource_type picoui_resource_get_type(const struct picoui_resource *resource);
size_t picoui_resource_get_size(const struct picoui_resource *resource);
unsigned int picoui_resource_get_refcount(const struct picoui_resource *resource);

#endif
