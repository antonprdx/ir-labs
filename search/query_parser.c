#include "query_parser.h"
#include <ctype.h>
#include <stdio.h>

static void skip_spaces(QueryParser* parser) {
    while (parser->query[parser->pos] && isspace(parser->query[parser->pos])) {
        parser->pos++;
    }
}

static QueryToken parse_next_token(QueryParser* parser) {
    QueryToken token;
    token.op = OP_TERM;
    string_init(&token.term);

    skip_spaces(parser);

    if (!parser->query[parser->pos]) {
        return token;
    }

    char c = parser->query[parser->pos];

    if (c == '(') {
        token.op = OP_LPAREN;
        string_assign(&token.term, "(");
        parser->pos++;
    } else if (c == ')') {
        token.op = OP_RPAREN;
        string_assign(&token.term, ")");
        parser->pos++;
    } else if (c == '-') {
        token.op = OP_NOT;
        parser->pos++;
        skip_spaces(parser);

        char term_buf[256];
        int i = 0;
        while (parser->query[parser->pos] && !isspace(parser->query[parser->pos]) &&
               parser->query[parser->pos] != '(' && parser->query[parser->pos] != ')' && i < 255) {
            term_buf[i++] = parser->query[parser->pos++];
        }
        term_buf[i] = '\0';
        string_to_lower_buf(term_buf);
        string_assign(&token.term, term_buf);
    } else {
        char word[256];
        int i = 0;
        while (parser->query[parser->pos] && !isspace(parser->query[parser->pos]) &&
               parser->query[parser->pos] != '(' && parser->query[parser->pos] != ')' && i < 255) {
            word[i++] = parser->query[parser->pos++];
        }
        word[i] = '\0';
        string_to_lower_buf(word);

        if (strcmp(word, "and") == 0) {
            token.op = OP_AND;
            string_assign(&token.term, "AND");
        } else if (strcmp(word, "or") == 0) {
            token.op = OP_OR;
            string_assign(&token.term, "OR");
        } else if (strcmp(word, "not") == 0) {
            token.op = OP_NOT;
            string_assign(&token.term, "NOT");
        } else {
            token.op = OP_TERM;
            string_assign(&token.term, word);
        }
    }

    return token;
}

void query_parser_init(QueryParser* parser, const char* query) {
    if (parser) {
        parser->query = query;
        parser->pos = 0;
    }
}

QueryTokenVector query_parser_parse(QueryParser* parser) {
    QueryTokenVector tokens;
    tokens.data = NULL;
    tokens.size = 0;
    tokens.capacity = 0;

    if (!parser) return tokens;

    while (parser->query[parser->pos]) {
        QueryToken token = parse_next_token(parser);
        if (token.op == OP_TERM && string_empty(&token.term)) {
            string_free(&token.term);
            break;
        }

        if (tokens.size >= tokens.capacity) {
            tokens.capacity = tokens.capacity == 0 ? 8 : tokens.capacity * 2;
            tokens.data = (QueryToken*)realloc(tokens.data, tokens.capacity * sizeof(QueryToken));
        }

        if (tokens.data) {
            tokens.data[tokens.size] = token;
            tokens.size++;
        } else {
            string_free(&token.term);
        }
    }

    return tokens;
}

void query_token_vector_free(QueryTokenVector* tokens) {
    if (tokens && tokens->data) {
        for (size_t i = 0; i < tokens->size; i++) {
            string_free(&tokens->data[i].term);
        }
        free(tokens->data);
        tokens->data = NULL;
        tokens->size = 0;
        tokens->capacity = 0;
    }
}

size_t query_token_vector_size(const QueryTokenVector* tokens) {
    return tokens ? tokens->size : 0;
}

const QueryToken* query_token_vector_get(const QueryTokenVector* tokens, size_t index) {
    if (tokens && index < tokens->size) {
        return &tokens->data[index];
    }
    return NULL;
}

QueryToken query_token_clone(const QueryToken* token) {
    QueryToken clone;
    clone.op = token->op;
    string_init_from(&clone.term, &token->term);
    return clone;
}

void query_token_free(QueryToken* token) {
    if (token) {
        string_free(&token->term);
    }
}
