#include "../core/internal.h"

#include <stdio.h>
#include <string.h>

#define PICOUI_NATIVE_DATE_TIME_FORMAT_MAX 64

static int picoui_native_date_time_append_text(char *buffer,
                                               unsigned long buffer_size,
                                               unsigned long *used,
                                               const char *text,
                                               unsigned long text_len)
{
    if (buffer == 0 || used == 0 || text == 0 || *used >= buffer_size) {
        return -1;
    }

    if (text_len >= buffer_size - *used) {
        return -1;
    }

    memcpy(buffer + *used, text, text_len);
    *used += text_len;
    buffer[*used] = '\0';
    return 0;
}

static int picoui_native_date_time_append_number(char *buffer,
                                                 unsigned long buffer_size,
                                                 unsigned long *used,
                                                 int value,
                                                 int width)
{
    char temp[16];
    int written;

    written = snprintf(temp, sizeof(temp), "%0*d", width, value);
    if (written <= 0) {
        return -1;
    }

    return picoui_native_date_time_append_text(buffer,
                                               buffer_size,
                                               used,
                                               temp,
                                               (unsigned long)written);
}

const char *picoui_native_date_time_get_format_state(const struct picoui_date_time *dt)
{
    if (dt == 0 || dt->format_storage[0] == '\0') {
        return 0;
    }

    return dt->format_storage;
}

const char *picoui_native_date_time_set_format_state(struct picoui_date_time *dt, const char *format)
{
    size_t length;

    if (dt == 0 || format == 0) {
        return 0;
    }

    length = strlen(format);
    if (length == 0 || length >= PICOUI_NATIVE_DATE_TIME_FORMAT_MAX) {
        return 0;
    }

    memcpy(dt->format_storage, format, length + 1);
    return dt->format_storage;
}

int picoui_native_date_time_format_text(const struct picoui_date_time *dt,
                                        char *buffer,
                                        unsigned long buffer_size)
{
    const char *format;
    unsigned long used = 0;

    if (dt == 0 || buffer == 0 || buffer_size == 0) {
        return -1;
    }

    format = picoui_native_date_time_get_format_state(dt);
    if (format == 0) {
        return -1;
    }

    buffer[0] = '\0';
    while (*format != '\0') {
        if (strncmp(format, "yyyy", 4) == 0) {
            if (picoui_native_date_time_append_number(buffer,
                                                      buffer_size,
                                                      &used,
                                                      dt->year,
                                                      4) != 0) {
                return -1;
            }
            format += 4;
        } else if (strncmp(format, "mm", 2) == 0) {
            if (picoui_native_date_time_append_number(buffer,
                                                      buffer_size,
                                                      &used,
                                                      dt->month,
                                                      2) != 0) {
                return -1;
            }
            format += 2;
        } else if (strncmp(format, "dd", 2) == 0) {
            if (picoui_native_date_time_append_number(buffer,
                                                      buffer_size,
                                                      &used,
                                                      dt->day,
                                                      2) != 0) {
                return -1;
            }
            format += 2;
        } else if (strncmp(format, "hh", 2) == 0) {
            if (picoui_native_date_time_append_number(buffer,
                                                      buffer_size,
                                                      &used,
                                                      dt->hour,
                                                      2) != 0) {
                return -1;
            }
            format += 2;
        } else if (strncmp(format, "nn", 2) == 0) {
            if (picoui_native_date_time_append_number(buffer,
                                                      buffer_size,
                                                      &used,
                                                      dt->minute,
                                                      2) != 0) {
                return -1;
            }
            format += 2;
        } else if (strncmp(format, "ss", 2) == 0) {
            if (picoui_native_date_time_append_number(buffer,
                                                      buffer_size,
                                                      &used,
                                                      dt->second,
                                                      2) != 0) {
                return -1;
            }
            format += 2;
        } else {
            if (picoui_native_date_time_append_text(buffer,
                                                    buffer_size,
                                                    &used,
                                                    format,
                                                    1) != 0) {
                return -1;
            }
            format++;
        }
    }

    return 0;
}
