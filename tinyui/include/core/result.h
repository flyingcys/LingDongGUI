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

#if defined(TINYUI_ENABLE_DIAGNOSTICS) && TINYUI_ENABLE_DIAGNOSTICS
const char *tinyui_last_error_message(void);
#endif

#endif
