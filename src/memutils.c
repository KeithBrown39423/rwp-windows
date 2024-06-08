#include <stdlib.h>
#include <stdio.h>

void* xmalloc(size_t size) {
    void* p = malloc(size);
    if (p == NULL) {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }
    return p;
}

void* xrealloc(void* ptr, size_t size) {
    void* p = realloc(ptr, size);
    if (p == NULL) {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }
    return p;
}

void* xcalloc(size_t count, size_t size) {
    void* p = calloc(count, size);
    if (p == NULL) {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }
    return p;
}

