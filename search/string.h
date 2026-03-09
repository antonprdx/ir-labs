#ifndef STRING_H
#define STRING_H

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct {
    char* data;
    size_t len;
    size_t capacity;
} String;

void string_init(String* str);
void string_init_copy(String* dest, const char* src);
void string_init_from(String* dest, const String* src);
void string_free(String* str);

void string_assign(String* str, const char* src);
void string_assign_string(String* dest, const String* src);

const char* string_cstr(const String* str);
size_t string_length(const String* str);
bool string_empty(const String* str);

bool string_equals(const String* a, const String* b);
bool string_equals_cstr(const String* str, const char* cstr);
int string_compare(const String* a, const String* b);
int string_compare_cstr(const String* str, const char* cstr);

void string_to_lower(String* str);
void string_to_lower_buf(char* str);

void string_clear(String* str);

#endif
