#ifndef BOOLEAN_SEARCHER_H
#define BOOLEAN_SEARCHER_H

#include "boolean_index.h"
#include "query_parser.h"
#include "vector.h"
#include "string.h"

typedef struct {
    BooleanIndex* index;
} BooleanSearcher;

void boolean_searcher_init(BooleanSearcher* searcher, BooleanIndex* idx);

UIntVector boolean_searcher_search(BooleanSearcher* searcher, const char* query_str);

UIntVector evaluate_tokens(BooleanSearcher* searcher, QueryTokenVector* tokens, size_t* pos);

#endif
