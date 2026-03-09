#include "string.h"
#include <ctype.h>

void string_init(String* str) {
    if (str) {
        str->data = NULL;
        str->len = 0;
        str->capacity = 0;
    }
}

void string_init_copy(String* dest, const char* src) {
    if (!dest) return;

    if (src) {
        size_t len = strlen(src);
        dest->data = (char*)malloc(len + 1);
        if (dest->data) {
            dest->capacity = len + 1;
            dest->len = len;
            strcpy(dest->data, src);
        } else {
            dest->len = 0;
            dest->capacity = 0;
        }
    } else {
        string_init(dest);
    }
}

void string_init_from(String* dest, const String* src) {
    if (!dest) return;

    if (src && src->data) {
        string_init_copy(dest, src->data);
    } else {
        string_init(dest);
    }
}

void string_free(String* str) {
    if (str && str->data) {
        free(str->data);
        str->data = NULL;
        str->len = 0;
        str->capacity = 0;
    }
}

void string_assign(String* str, const char* src) {
    if (!str || !src) return;

    size_t len = strlen(src);
    if (str->capacity <= len) {
        char* new_data = (char*)realloc(str->data, len + 1);
        if (new_data) {
            str->data = new_data;
            str->capacity = len + 1;
        } else {
            return;
        }
    }
    strcpy(str->data, src);
    str->len = len;
}

void string_assign_string(String* dest, const String* src) {
    if (dest && src) {
        string_assign(dest, src->data);
    }
}

const char* string_cstr(const String* str) {
    return (str && str->data) ? str->data : "";
}

size_t string_length(const String* str) {
    return str ? str->len : 0;
}

bool string_empty(const String* str) {
    return str ? str->len == 0 : true;
}

bool string_equals(const String* a, const String* b) {
    if (!a || !b) return false;
    if (a->len != b->len) return false;
    if (!a->data || !b->data) return false;
    return strcmp(a->data, b->data) == 0;
}

bool string_equals_cstr(const String* str, const char* cstr) {
    if (!str || !cstr) return false;
    if (!str->data) return false;
    return strcmp(str->data, cstr) == 0;
}

int string_compare(const String* a, const String* b) {
    if (!a || !b) return 0;
    if (!a->data && !b->data) return 0;
    if (!a->data) return -1;
    if (!b->data) return 1;
    return strcmp(a->data, b->data);
}

int string_compare_cstr(const String* str, const char* cstr) {
    if (!str || !cstr) return 0;
    if (!str->data && !cstr) return 0;
    if (!str->data) return -1;
    if (!cstr) return 1;
    return strcmp(str->data, cstr);
}

void string_to_lower(String* str) {
    if (str && str->data) {
        for (size_t i = 0; i < str->len; i++) {
            str->data[i] = tolower(str->data[i]);
        }
    }
}

void string_to_lower_buf(char* str) {
    if (str) {
        for (size_t i = 0; str[i]; i++) {
            str[i] = tolower(str[i]);
        }
    }
}

void string_clear(String* str) {
    if (str) {
        str->len = 0;
        if (str->data) {
            str->data[0] = '\0';
        }
    }
}
