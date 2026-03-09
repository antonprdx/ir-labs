#ifndef VECTOR_H
#define VECTOR_H

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct {
    unsigned int* data;
    size_t size;
    size_t capacity;
} UIntVector;

typedef struct {
    char** data;
    size_t size;
    size_t capacity;
} StringVector;

void uint_vector_init(UIntVector* vec);
void uint_vector_free(UIntVector* vec);
void uint_vector_push_back(UIntVector* vec, unsigned int value);
size_t uint_vector_size(const UIntVector* vec);
unsigned int uint_vector_get(const UIntVector* vec, size_t index);
void uint_vector_set(UIntVector* vec, size_t index, unsigned int value);
bool uint_vector_empty(const UIntVector* vec);
void uint_vector_clear(UIntVector* vec);
void uint_vector_sort(UIntVector* vec);
UIntVector uint_vector_clone(const UIntVector* vec);
int uint_vector_find(const UIntVector* vec, unsigned int value);

void string_vector_init(StringVector* vec);
void string_vector_free(StringVector* vec);
void string_vector_push_back(StringVector* vec, const char* str);
size_t string_vector_size(const StringVector* vec);
const char* string_vector_get(const StringVector* vec, size_t index);
char* string_vector_get_mutable(StringVector* vec, size_t index);
bool string_vector_empty(const StringVector* vec);
void string_vector_clear(StringVector* vec);
int string_vector_find(const StringVector* vec, const char* str);

#endif
