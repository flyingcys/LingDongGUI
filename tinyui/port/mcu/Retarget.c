/* Retarget.c — ARM MCU 工具链 I/O syscall 桩
 * 仅在 MCU 工具链下编译有效，Apple/Linux/Windows 通过宏排除。
 */

#if !defined(__APPLE__) && !defined(__linux__) && !defined(_WIN32)

#include <stdint.h>

int _write(int fd, const char *buf, int count)
{
    (void)fd;
    (void)buf;
    return count;
}

int _read(int fd, char *buf, int count)
{
    (void)fd;
    (void)buf;
    (void)count;
    return 0;
}

int _close(int fd)
{
    (void)fd;
    return -1;
}

int _fstat(int fd, void *st)
{
    (void)fd;
    (void)st;
    return 0;
}

int _isatty(int fd)
{
    (void)fd;
    return 1;
}

int _lseek(int fd, int offset, int whence)
{
    (void)fd;
    (void)offset;
    (void)whence;
    return 0;
}

__attribute__((weak))
void initialise_monitor_handles(void) {}

#endif /* !__APPLE__ && !__linux__ && !_WIN32 */
