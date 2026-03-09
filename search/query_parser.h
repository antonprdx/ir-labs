#ifndef QUERY_PARSER_H
#define QUERY_PARSER_H

#include "vector.h"
#include "string.h"

typedef enum {
    OP_AND,
    OP_OR,
    OP_NOT,
    OP_TERM,
    OP_LPAREN,
    OP_RPAREN
} QueryOperator;

typedef struct {
    QueryOperator op;
    String term;
} QueryToken;

typedef struct {
    QueryToken* data;
    size_t size;
    size_t capacity;
} QueryTokenVector;

typedef struct {
    const char* query;
    size_t pos;
} QueryParser;

void query_parser_init(QueryParser* parser, const char* query);

QueryTokenVector query_parser_parse(QueryParser* parser);

void query_token_vector_free(QueryTokenVector* tokens);

size_t query_token_vector_size(const QueryTokenVector* tokens);

const QueryToken* query_token_vector_get(const QueryTokenVector* tokens, size_t index);

QueryToken query_token_clone(const QueryToken* token);

void query_token_free(QueryToken* token);

#endif
