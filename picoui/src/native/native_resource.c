#include "picoui/resource.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct picoui_resource {
    enum picoui_resource_type type;
    unsigned int refcount;
    size_t size;
    void *data;
    char *path;
};

static char *picoui_resource_strdup(const char *value)
{
    size_t size;
    char *copy;

    if (value == 0) {
        return 0;
    }

    size = strlen(value) + 1U;
    copy = malloc(size);
    if (copy == 0) {
        return 0;
    }

    memcpy(copy, value, size);
    return copy;
}

static struct picoui_resource *picoui_resource_alloc(enum picoui_resource_type type, size_t size)
{
    struct picoui_resource *resource;

    resource = calloc(1, sizeof(*resource));
    if (resource == 0) {
        return 0;
    }

    resource->type = type;
    resource->refcount = 1U;
    resource->size = size;
    return resource;
}

struct picoui_resource *picoui_resource_create_from_memory(const void *data, size_t size)
{
    struct picoui_resource *resource;

    if (data == 0 || size == 0U) {
        return 0;
    }

    resource = picoui_resource_alloc(PICOUI_RESOURCE_TYPE_CONST_MEMORY, size);
    if (resource == 0) {
        return 0;
    }

    resource->data = malloc(size);
    if (resource->data == 0) {
        free(resource);
        return 0;
    }

    memcpy(resource->data, data, size);
    return resource;
}

struct picoui_resource *picoui_resource_create_from_file(const char *path)
{
    struct picoui_resource *resource;
    FILE *fp;
    long file_size;
    size_t read_size;

    if (path == 0 || path[0] == '\0') {
        return 0;
    }

    fp = fopen(path, "rb");
    if (fp == 0) {
        return 0;
    }
    if (fseek(fp, 0L, SEEK_END) != 0) {
        fclose(fp);
        return 0;
    }

    file_size = ftell(fp);
    if (file_size <= 0L || fseek(fp, 0L, SEEK_SET) != 0) {
        fclose(fp);
        return 0;
    }

    resource = picoui_resource_alloc(PICOUI_RESOURCE_TYPE_FILE, (size_t)file_size);
    if (resource == 0) {
        fclose(fp);
        return 0;
    }

    resource->data = malloc(resource->size);
    resource->path = picoui_resource_strdup(path);
    if (resource->data == 0 || resource->path == 0) {
        free(resource->path);
        free(resource->data);
        free(resource);
        fclose(fp);
        return 0;
    }

    read_size = fread(resource->data, 1U, resource->size, fp);
    fclose(fp);
    if (read_size != resource->size) {
        free(resource->path);
        free(resource->data);
        free(resource);
        return 0;
    }

    return resource;
}

struct picoui_resource *picoui_resource_ref(struct picoui_resource *resource)
{
    if (resource == 0) {
        return 0;
    }

    resource->refcount += 1U;
    return resource;
}

void picoui_resource_unref(struct picoui_resource *resource)
{
    if (resource == 0) {
        return;
    }

    if (resource->refcount > 1U) {
        resource->refcount -= 1U;
        return;
    }

    free(resource->path);
    free(resource->data);
    free(resource);
}

enum picoui_resource_type picoui_resource_get_type(const struct picoui_resource *resource)
{
    if (resource == 0) {
        return PICOUI_RESOURCE_TYPE_NONE;
    }

    return resource->type;
}

size_t picoui_resource_get_size(const struct picoui_resource *resource)
{
    if (resource == 0) {
        return 0U;
    }

    return resource->size;
}

unsigned int picoui_resource_get_refcount(const struct picoui_resource *resource)
{
    if (resource == 0) {
        return 0U;
    }

    return resource->refcount;
}
