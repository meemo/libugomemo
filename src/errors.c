#include <stdlib.h>
#include <string.h>

#include <ugomemo/errors.h>

void ugomemo_error_add(ugomemo_error_list *list, ugomemo_severity severity, const char *message) {
    ugomemo_error *err = (ugomemo_error *) calloc(1, sizeof(ugomemo_error));
    if (err == NULL) return;

    err->severity = severity;
    err->message = strdup(message);
    if (err->message == NULL) {
        free(err);
        return;
    }
    err->next = NULL;

    if (list->tail != NULL) {
        list->tail->next = err;
    } else {
        list->head = err;
    }
    list->tail = err;
    list->count++;
}

void ugomemo_error_list_free(ugomemo_error_list *list) {
    ugomemo_error *current = list->head;
    ugomemo_error *next;

    while (current != NULL) {
        next = current->next;
        free(current->message);
        free(current);
        current = next;
    }

    list->head = NULL;
    list->tail = NULL;
    list->count = 0;
}

u32 ugomemo_error_count(const ugomemo_error_list *list) {
    return list ? list->count : 0;
}

const char *ugomemo_error_get(const ugomemo_error_list *list, u32 index, int *severity) {
    if (!list) return NULL;
    ugomemo_error *e = list->head;
    for (u32 i = 0; i < index && e != NULL; i++)
        e = e->next;
    if (!e) return NULL;
    if (severity) *severity = (int)e->severity;
    return e->message;
}
