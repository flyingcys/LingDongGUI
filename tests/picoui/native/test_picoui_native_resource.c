#include "picoui/resource.h"

#include <assert.h>
#include <fcntl.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>

static void write_temp_file(const char *path, const unsigned char *bytes, size_t size)
{
    int fd;
    ssize_t written;

    fd = open(path, O_CREAT | O_TRUNC | O_WRONLY, 0600);
    assert(fd >= 0);
    written = write(fd, bytes, size);
    assert(written == (ssize_t)size);
    assert(close(fd) == 0);
}

int main(void)
{
    static const unsigned char memory_bytes[] = {0x50, 0x49, 0x43, 0x4FU};
    static const unsigned char file_bytes[] = {'P', '5', '-', 'A', '\n'};
    struct picoui_resource *memory_resource;
    struct picoui_resource *file_resource;
    char temp_path[] = "/tmp/picoui-resource-XXXXXX";
    int temp_fd;

    assert(picoui_resource_create_from_memory(0, sizeof(memory_bytes)) == 0);
    assert(picoui_resource_create_from_memory(memory_bytes, 0U) == 0);
    assert(picoui_resource_create_from_file(0) == 0);
    assert(picoui_resource_get_type(0) == PICOUI_RESOURCE_TYPE_NONE);
    assert(picoui_resource_get_size(0) == 0U);
    assert(picoui_resource_get_refcount(0) == 0U);

    memory_resource = picoui_resource_create_from_memory(memory_bytes, sizeof(memory_bytes));
    assert(memory_resource != 0);
    assert(picoui_resource_get_type(memory_resource) == PICOUI_RESOURCE_TYPE_CONST_MEMORY);
    assert(picoui_resource_get_size(memory_resource) == sizeof(memory_bytes));
    assert(picoui_resource_get_refcount(memory_resource) == 1U);
    assert(picoui_resource_ref(memory_resource) == memory_resource);
    assert(picoui_resource_get_refcount(memory_resource) == 2U);
    picoui_resource_unref(memory_resource);
    assert(picoui_resource_get_refcount(memory_resource) == 1U);

    temp_fd = mkstemp(temp_path);
    assert(temp_fd >= 0);
    assert(close(temp_fd) == 0);
    write_temp_file(temp_path, file_bytes, sizeof(file_bytes));
    file_resource = picoui_resource_create_from_file(temp_path);
    assert(unlink(temp_path) == 0);
    assert(file_resource != 0);
    assert(picoui_resource_get_type(file_resource) == PICOUI_RESOURCE_TYPE_FILE);
    assert(picoui_resource_get_size(file_resource) == sizeof(file_bytes));
    assert(picoui_resource_get_refcount(file_resource) == 1U);
    assert(picoui_resource_ref(file_resource) == file_resource);
    assert(picoui_resource_get_refcount(file_resource) == 2U);
    picoui_resource_unref(file_resource);
    assert(picoui_resource_get_refcount(file_resource) == 1U);

    picoui_resource_unref(file_resource);
    picoui_resource_unref(memory_resource);
    return 0;
}
