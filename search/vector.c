#include "vector.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void uint_vector_init(UIntVector* vec) {
    if (vec) {
        vec->data = NULL;
        vec->size = 0;
        vec->capacity = 0;
    }
}

void uint_vector_free(UIntVector* vec) {
    if (vec && vec->data) {
        free(vec->data);
        vec->data = NULL;
        vec->size = 0;
        vec->capacity = 0;
    }
}

void uint_vector_push_back(UIntVector* vec, unsigned int value) {
    if (!vec) return;

    if (vec->size >= vec->capacity) {
        vec->capacity = vec->capacity == 0 ? 16 : vec->capacity * 2;
        vec->data = (unsigned int*)realloc(vec->data, vec->capacity * sizeof(unsigned int));
        if (!vec->data) {
            vec->capacity = 0;
            vec->size = 0;
            return;
        }
    }
    vec->data[vec->size++] = value;
}

size_t uint_vector_size(const UIntVector* vec) {
    return vec ? vec->size : 0;
}

unsigned int uint_vector_get(const UIntVector* vec, size_t index) {
    if (vec && index < vec->size) {
        return vec->data[index];
    }
    return 0;
}

void uint_vector_set(UIntVector* vec, size_t index, unsigned int value) {
    if (vec && index < vec->size) {
        vec->data[index] = value;
    }
}

bool uint_vector_empty(const UIntVector* vec) {
    return vec ? vec->size == 0 : true;
}

void uint_vector_clear(UIntVector* vec) {
    if (vec) {
        vec->size = 0;
    }
}

static int compare_uint(const void* a, const void* b) {
    unsigned int ia = *(const unsigned int*)a;
    unsigned int ib = *(const unsigned int*)b;
    return (ia > ib) - (ia < ib);
}

void uint_vector_sort(UIntVector* vec) {
    if (vec && vec->size > 1) {
        qsort(vec->data, vec->size, sizeof(unsigned int), compare_uint);
    }
}

UIntVector uint_vector_clone(const UIntVector* vec) {
    UIntVector result;
    uint_vector_init(&result);

    if (vec) {
        for (size_t i = 0; i < vec->size; i++) {
            uint_vector_push_back(&result, vec->data[i]);
        }
    }

    return result;
}

int uint_vector_find(const UIntVector* vec, unsigned int value) {
    if (!vec) return -1;

    for (size_t i = 0; i < vec->size; i++) {
        if (vec->data[i] == value) {
            return i;
        }
    }
    return -1;
}


void string_vector_init(StringVector* vec) {
    if (vec) {
        vec->data = NULL;
        vec->size = 0;
        vec->capacity = 0;
    }
}

void string_vector_free(StringVector* vec) {
    if (vec && vec->data) {
        for (size_t i = 0; i < vec->size; i++) {
            if (vec->data[i]) {
                free(vec->data[i]);
            }
        }
        free(vec->data);
        vec->data = NULL;
        vec->size = 0;
        vec->capacity = 0;
    }
}

static char* string_duplicate(const char* src) {
    if (!src) return NULL;

    size_t len = strlen(src);
    char* dst = (char*)malloc(len + 1);
    if (dst) {
        strcpy(dst, src);
    }
    return dst;
}

void string_vector_push_back(StringVector* vec, const char* str) {
    if (!vec || !str) return;

    if (vec->size >= vec->capacity) {
        vec->capacity = vec->capacity == 0 ? 16 : vec->capacity * 2;
        vec->data = (char**)realloc(vec->data, vec->capacity * sizeof(char*));
        if (!vec->data) {
            vec->capacity = 0;
            vec->size = 0;
            return;
        }
    }

    vec->data[vec->size] = string_duplicate(str);
    if (vec->data[vec->size]) {
        vec->size++;
    }
}

size_t string_vector_size(const StringVector* vec) {
    return vec ? vec->size : 0;
}

const char* string_vector_get(const StringVector* vec, size_t index) {
    if (vec && index < vec->size) {
        return vec->data[index];
    }
    return NULL;
}

char* string_vector_get_mutable(StringVector* vec, size_t index) {
    if (vec && index < vec->size) {
        return vec->data[index];
    }
    return NULL;
}

bool string_vector_empty(const StringVector* vec) {
    return vec ? vec->size == 0 : true;
}

void string_vector_clear(StringVector* vec) {
    if (vec) {
        for (size_t i = 0; i < vec->size; i++) {
            if (vec->data[i]) {
                free(vec->data[i]);
                vec->data[i] = NULL;
            }
        }
        vec->size = 0;
    }
}

int string_vector_find(const StringVector* vec, const char* str) {
    if (!vec || !str) return -1;

    for (size_t i = 0; i < vec->size; i++) {
        if (vec->data[i] && strcmp(vec->data[i], str) == 0) {
            return i;
        }
    }
    return -1;
}
