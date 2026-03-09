#ifndef BOOLEAN_INDEX_H
#define BOOLEAN_INDEX_H

#include "hashmap.h"
#include "string.h"
#include "posting_list.h"
#include "vector.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

typedef struct {
    HashMap index;
    HashMap documents;
    unsigned int nextDocId;
} BooleanIndex;

void boolean_index_init(BooleanIndex* idx);
void boolean_index_free(BooleanIndex* idx);

unsigned int boolean_index_add_document(BooleanIndex* idx, const char* text);

PostingList* boolean_index_get_posting_list(BooleanIndex* idx, const char* term);

UIntVector boolean_index_search_and(BooleanIndex* idx, const StringVector* terms);
UIntVector boolean_index_search_or(BooleanIndex* idx, const StringVector* terms);
UIntVector boolean_index_search_not(BooleanIndex* idx, const String* term);

const char* boolean_index_get_document_text(BooleanIndex* idx, unsigned int docId);

size_t boolean_index_get_total_documents(BooleanIndex* idx);
size_t boolean_index_get_total_terms(BooleanIndex* idx);
StringVector boolean_index_keys(BooleanIndex* idx);

void boolean_index_save(BooleanIndex* idx, const char* filename);
bool boolean_index_load(BooleanIndex* idx, const char* filename);

#endif
