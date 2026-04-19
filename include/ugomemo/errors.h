#ifndef UGOMEMO_ERRORS_H_
#define UGOMEMO_ERRORS_H_

#include <ugomemo/types.h>

// Overall error codes, directly correspond to exit codes
#define UGOMEMO_OK 0
#define UGOMEMO_INPUT_ERROR 1
#define UGOMEMO_INTERNAL_ERROR 2
#define UGOMEMO_MEMORY_ERROR 3

#define UGOMEMO_SIGNATURE_ERROR_1 11
#define UGOMEMO_SIGNATURE_ERROR_2 12

// Severity levels for context-level error tracking
typedef enum {
    UGOMEMO_SEVERITY_NOTICE,
    UGOMEMO_SEVERITY_WARNING,
    UGOMEMO_SEVERITY_ERROR
} ugomemo_severity;

typedef struct ugomemo_error {
    ugomemo_severity severity;
    char *message;
    struct ugomemo_error *next;
} ugomemo_error;

typedef struct {
    ugomemo_error *head;
    ugomemo_error *tail;
    u32 count;
} ugomemo_error_list;

// Context-level error macros, requires ctx->errors to be a ugomemo_error_list
#define CTX_ERROR(ctx, msg)   ugomemo_error_add(&(ctx)->errors, UGOMEMO_SEVERITY_ERROR, msg)
#define CTX_WARNING(ctx, msg) ugomemo_error_add(&(ctx)->errors, UGOMEMO_SEVERITY_WARNING, msg)
#define CTX_NOTICE(ctx, msg)  ugomemo_error_add(&(ctx)->errors, UGOMEMO_SEVERITY_NOTICE, msg)

void ugomemo_error_add(ugomemo_error_list *list, ugomemo_severity severity, const char *message);
void ugomemo_error_list_free(ugomemo_error_list *list);

// FFI-friendly iteration
u32 ugomemo_error_count(const ugomemo_error_list *list);
const char *ugomemo_error_get(const ugomemo_error_list *list, u32 index, int *severity);

#endif
