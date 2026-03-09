#ifndef HASH_MAP_H
#define HASH_MAP_H

#include "vector.h"
#include "string.h"
#include "posting_list.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct {
    char* key;
    void* value;
} HashMapPair;

typedef struct {
    HashMapPair* items;
    size_t size;
    size_t capacity;
} HashMap;

void hash_map_init(HashMap* map);
void hash_map_free(HashMap* map);

void* hash_map_get(HashMap* map, const char* key);
void hash_map_put(HashMap* map, const char* key, void* value);
bool hash_map_contains(HashMap* map, const char* key);
StringVector hash_map_keys(HashMap* map);
size_t hash_map_size(HashMap* map);
bool hash_map_empty(HashMap* map);
void hash_map_clear(HashMap* map);

PostingList* hash_map_get_posting_list(HashMap* map, const char* key);
void hash_map_put_posting_list(HashMap* map, const char* key, PostingList* value);

String* hash_map_get_string(HashMap* map, unsigned int key);
void hash_map_put_string(HashMap* map, unsigned int key, const char* value);

#endif
