#include "hashmap.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char* string_duplicate(const char* src) {
    if (!src) return NULL;

    size_t len = strlen(src);
    char* dst = (char*)malloc(len + 1);
    if (dst) {
        strcpy(dst, src);
    }
    return dst;
}

static unsigned int hash_string(const char* str) {
    unsigned int hash = 0;
    while (*str) {
        hash = hash * 31 + *str++;
    }
    return hash;
}

void hash_map_init(HashMap* map) {
    if (map) {
        map->items = NULL;
        map->size = 0;
        map->capacity = 0;
    }
}

void hash_map_free(HashMap* map) {
    if (map && map->items) {
        for (size_t i = 0; i < map->size; i++) {
            if (map->items[i].key) {
                free(map->items[i].key);
            }
        }
        free(map->items);
        map->items = NULL;
        map->size = 0;
        map->capacity = 0;
    }
}

static int hash_map_find_index(HashMap* map, const char* key) {
    if (!map || !key) return -1;

    for (size_t i = 0; i < map->size; i++) {
        if (map->items[i].key && strcmp(map->items[i].key, key) == 0) {
            return i;
        }
    }
    return -1;
}

void* hash_map_get(HashMap* map, const char* key) {
    if (!map || !key) return NULL;

    int idx = hash_map_find_index(map, key);
    return (idx != -1) ? map->items[idx].value : NULL;
}

void hash_map_put(HashMap* map, const char* key, void* value) {
    if (!map || !key) return;

    int idx = hash_map_find_index(map, key);
    if (idx != -1) {
        map->items[idx].value = value;
        return;
    }

    if (map->size >= map->capacity) {
        map->capacity = map->capacity == 0 ? 8 : map->capacity * 2;
        map->items = (HashMapPair*)realloc(map->items, map->capacity * sizeof(HashMapPair));
        if (!map->items) {
            map->capacity = 0;
            map->size = 0;
            return;
        }
    }

    map->items[map->size].key = string_duplicate(key);
    map->items[map->size].value = value;
    map->size++;
}

bool hash_map_contains(HashMap* map, const char* key) {
    return hash_map_find_index(map, key) != -1;
}

StringVector hash_map_keys(HashMap* map) {
    StringVector result;
    string_vector_init(&result);

    if (map) {
        for (size_t i = 0; i < map->size; i++) {
            if (map->items[i].key) {
                string_vector_push_back(&result, map->items[i].key);
            }
        }
    }

    return result;
}

size_t hash_map_size(HashMap* map) {
    return map ? map->size : 0;
}

bool hash_map_empty(HashMap* map) {
    return map ? map->size == 0 : true;
}

void hash_map_clear(HashMap* map) {
    if (map) {
        for (size_t i = 0; i < map->size; i++) {
            if (map->items[i].key) {
                free(map->items[i].key);
                map->items[i].key = NULL;
            }
        }
        map->size = 0;
    }
}

PostingList* hash_map_get_posting_list(HashMap* map, const char* key) {
    return (PostingList*)hash_map_get(map, key);
}

void hash_map_put_posting_list(HashMap* map, const char* key, PostingList* value) {
    hash_map_put(map, key, value);
}

String* hash_map_get_string(HashMap* map, unsigned int key) {
    char key_str[32];
    snprintf(key_str, sizeof(key_str), "%u", key);
    return (String*)hash_map_get(map, key_str);
}

void hash_map_put_string(HashMap* map, unsigned int key, const char* value) {
    char key_str[32];
    snprintf(key_str, sizeof(key_str), "%u", key);

    String* str_val = (String*)malloc(sizeof(String));
    if (str_val) {
        string_init_copy(str_val, value);
        hash_map_put(map, key_str, str_val);
    }
}
