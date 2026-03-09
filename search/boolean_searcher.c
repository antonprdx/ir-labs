#include "boolean_searcher.h"
#include <string.h>

static UIntVector parse_primary(BooleanSearcher* searcher, QueryTokenVector* tokens, size_t* pos);
static UIntVector parse_and(BooleanSearcher* searcher, QueryTokenVector* tokens, size_t* pos);
static UIntVector parse_or(BooleanSearcher* searcher, QueryTokenVector* tokens, size_t* pos);

void boolean_searcher_init(BooleanSearcher* searcher, BooleanIndex* idx) {
    if (searcher) {
        searcher->index = idx;
    }
}

UIntVector evaluate_tokens(BooleanSearcher* searcher, QueryTokenVector* tokens, size_t* pos) {
    return parse_or(searcher, tokens, pos);
}

static UIntVector parse_or(BooleanSearcher* searcher, QueryTokenVector* tokens, size_t* pos) {
    UIntVector result = parse_and(searcher, tokens, pos);

    while (*pos < query_token_vector_size(tokens)) {
        const QueryToken* token = query_token_vector_get(tokens, *pos);
        if (token->op != OP_OR) break;

        (*pos)++;
        UIntVector right = parse_and(searcher, tokens, pos);

        UIntVector new_result = uint_vector_clone(&result);

        for (size_t i = 0; i < uint_vector_size(&right); i++) {
            unsigned int val = uint_vector_get(&right, i);
            bool found = false;

            for (size_t j = 0; j < uint_vector_size(&new_result); j++) {
                if (uint_vector_get(&new_result, j) == val) {
                    found = true;
                    break;
                }
            }

            if (!found) {
                uint_vector_push_back(&new_result, val);
            }
        }

        uint_vector_sort(&new_result);
        uint_vector_free(&result);
        uint_vector_free(&right);
        result = new_result;
    }

    return result;
}

static UIntVector parse_and(BooleanSearcher* searcher, QueryTokenVector* tokens, size_t* pos) {
    UIntVector result = parse_primary(searcher, tokens, pos);

    while (*pos < query_token_vector_size(tokens)) {
        const QueryToken* token = query_token_vector_get(tokens, *pos);
        if (token->op != OP_AND) break;

        (*pos)++;
        UIntVector right = parse_primary(searcher, tokens, pos);

        UIntVector new_result;
        uint_vector_init(&new_result);

        uint_vector_sort(&result);
        uint_vector_sort(&right);

        size_t i = 0, j = 0;
        while (i < uint_vector_size(&result) && j < uint_vector_size(&right)) {
            unsigned int l = uint_vector_get(&result, i);
            unsigned int r = uint_vector_get(&right, j);

            if (l < r) {
                i++;
            } else if (l > r) {
                j++;
            } else {
                uint_vector_push_back(&new_result, l);
                i++;
                j++;
            }
        }

        uint_vector_free(&result);
        uint_vector_free(&right);
        result = new_result;
    }

    return result;
}

static UIntVector parse_primary(BooleanSearcher* searcher, QueryTokenVector* tokens, size_t* pos) {
    UIntVector result;
    uint_vector_init(&result);

    if (*pos >= query_token_vector_size(tokens)) {
        return result;
    }

    const QueryToken* token = query_token_vector_get(tokens, *pos);

    if (token->op == OP_LPAREN) {
        (*pos)++;
        result = parse_or(searcher, tokens, pos);
        if (*pos < query_token_vector_size(tokens)) {
            const QueryToken* next = query_token_vector_get(tokens, *pos);
            if (next->op == OP_RPAREN) {
                (*pos)++;
            }
        }
    } else if (token->op == OP_NOT) {
        (*pos)++;
        UIntVector operand = parse_primary(searcher, tokens, pos);

        for (unsigned int i = 0; i < boolean_index_get_total_documents(searcher->index); i++) {
            bool found = false;
            for (size_t j = 0; j < uint_vector_size(&operand); j++) {
                if (i == uint_vector_get(&operand, j)) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                uint_vector_push_back(&result, i);
            }
        }

        uint_vector_free(&operand);
    } else if (token->op == OP_TERM) {
        PostingList* plist = boolean_index_get_posting_list(searcher->index, string_cstr(&token->term));
        if (plist) {
            size_t doc_count;
            unsigned int* docs = posting_list_get_docs(plist, &doc_count);
            for (size_t i = 0; i < doc_count; i++) {
                uint_vector_push_back(&result, docs[i]);
            }
            free(docs);
        }
        (*pos)++;
    } else {
        (*pos)++;
    }

    return result;
}

UIntVector boolean_searcher_search(BooleanSearcher* searcher, const char* query_str) {
    UIntVector empty_result;
    uint_vector_init(&empty_result);

    if (!searcher || !searcher->index || !query_str) {
        return empty_result;
    }

    QueryParser parser;
    query_parser_init(&parser, query_str);
    QueryTokenVector tokens = query_parser_parse(&parser);

    if (query_token_vector_size(&tokens) == 0) {
        query_token_vector_free(&tokens);
        return empty_result;
    }

    size_t pos = 0;
    UIntVector result = evaluate_tokens(searcher, &tokens, &pos);

    query_token_vector_free(&tokens);
    return result;
}
