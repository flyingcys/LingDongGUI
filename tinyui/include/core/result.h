#ifndef TINYUI_RESULT_H
#define TINYUI_RESULT_H

typedef enum tinyui_result {
    TINYUI_OK = 0,
    TINYUI_ERROR_INVALID_ARG,
    TINYUI_ERROR_INVALID_OBJECT,
    TINYUI_ERROR_INVALID_STATE,
    TINYUI_ERROR_NOT_SUPPORTED,
    TINYUI_ERROR_OUT_OF_RANGE,
    TINYUI_ERROR_NO_MEMORY,
    TINYUI_ERROR_CAPACITY,
    TINYUI_ERROR_BACKEND,
} tinyui_result_t;

tinyui_result_t tinyui_last_result(void);

/* Declared always so link probes and diagnostics consumers share one symbol. */
const char *tinyui_last_error_message(void);

#endif
