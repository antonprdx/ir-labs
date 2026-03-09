#include "boolean_index.h"
#include <ctype.h>

static void to_lower_str(char* str) {
    for (int i = 0; str[i]; ++i) {
        if (str[i] >= 'A' && str[i] <= 'Z') {
            str[i] += 32;
        }
    }
}

static void tokenize_and_index(BooleanIndex* idx, unsigned int docId, const char* text) {
    char buffer[4096];
    int bufIdx = 0;

    for (int i = 0; text[i]; ++i) {
        unsigned char c = text[i];
        bool is_word_char = (c >= '0' && c <= '9') ||
                           (c >= 'a' && c <= 'z') ||
                           (c >= 'A' && c <= 'Z') ||
                           c == '-' || c == '_' || c > 127;

        if (is_word_char) {
            if (bufIdx < 4095) {
                buffer[bufIdx++] = c;
            }
        } else {
            if (bufIdx > 0) {
                buffer[bufIdx] = '\0';
                to_lower_str(buffer);

                PostingList* pl = hash_map_get_posting_list(&idx->index, buffer);
                if (!pl) {
                    pl = (PostingList*)malloc(sizeof(PostingList));
                    posting_list_init(pl);
                    hash_map_put_posting_list(&idx->index, buffer, pl);
                }
                posting_list_add_doc(pl, docId);

                bufIdx = 0;
            }
        }
    }

    if (bufIdx > 0) {
        buffer[bufIdx] = '\0';
        to_lower_str(buffer);

        PostingList* pl = hash_map_get_posting_list(&idx->index, buffer);
        if (!pl) {
            pl = (PostingList*)malloc(sizeof(PostingList));
            posting_list_init(pl);
            hash_map_put_posting_list(&idx->index, buffer, pl);
        }
        posting_list_add_doc(pl, docId);
    }
}

void boolean_index_init(BooleanIndex* idx) {
    if (idx) {
        hash_map_init(&idx->index);
        hash_map_init(&idx->documents);
        idx->nextDocId = 0;
    }
}

void boolean_index_free(BooleanIndex* idx) {
    if (!idx) return;

    StringVector keys = hash_map_keys(&idx->index);
    for (size_t i = 0; i < string_vector_size(&keys); i++) {
        const char* key = string_vector_get(&keys, i);
        PostingList* pl = hash_map_get_posting_list(&idx->index, key);
        if (pl) {
            posting_list_free(pl);
            free(pl);
        }
    }
    string_vector_free(&keys);

    StringVector doc_keys = hash_map_keys(&idx->documents);
    for (size_t i = 0; i < string_vector_size(&doc_keys); i++) {
        const char* key = string_vector_get(&doc_keys, i);
        String* doc = hash_map_get_string(&idx->documents, (unsigned int)atoi(key));
        if (doc) {
            string_free(doc);
            free(doc);
        }
    }
    string_vector_free(&doc_keys);

    hash_map_free(&idx->index);
    hash_map_free(&idx->documents);
}

unsigned int boolean_index_add_document(BooleanIndex* idx, const char* text) {
    if (!idx || !text) return 0;

    unsigned int docId = idx->nextDocId++;
    hash_map_put_string(&idx->documents, docId, text);
    tokenize_and_index(idx, docId, text);
    return docId;
}

PostingList* boolean_index_get_posting_list(BooleanIndex* idx, const char* term) {
    if (!idx || !term) return NULL;

    char term_lower[256];
    strncpy(term_lower, term, sizeof(term_lower) - 1);
    term_lower[sizeof(term_lower) - 1] = '\0';
    to_lower_str(term_lower);

    return hash_map_get_posting_list(&idx->index, term_lower);
}

UIntVector boolean_index_search_and(BooleanIndex* idx, const StringVector* terms) {
    UIntVector result;
    uint_vector_init(&result);

    if (!idx || !terms || string_vector_size(terms) == 0) {
        return result;
    }

    const char* first_term = string_vector_get(terms, 0);
    PostingList* plist = boolean_index_get_posting_list(idx, first_term);

    if (plist) {
        size_t doc_count;
        unsigned int* docs = posting_list_get_docs(plist, &doc_count);

        for (size_t j = 0; j < doc_count; j++) {
            uint_vector_push_back(&result, docs[j]);
        }
        free(docs);

        for (size_t i = 1; i < string_vector_size(terms); ++i) {
            const char* term = string_vector_get(terms, i);
            plist = boolean_index_get_posting_list(idx, term);

            UIntVector next;
            uint_vector_init(&next);

            if (plist) {
                for (size_t j = 0; j < uint_vector_size(&result); ++j) {
                    if (posting_list_contains(plist, uint_vector_get(&result, j))) {
                        uint_vector_push_back(&next, uint_vector_get(&result, j));
                    }
                }
            }

            uint_vector_free(&result);
            result = next;
        }
    }

    return result;
}

UIntVector boolean_index_search_or(BooleanIndex* idx, const StringVector* terms) {
    UIntVector result;
    uint_vector_init(&result);

    if (!idx || !terms) return result;

    for (size_t i = 0; i < string_vector_size(terms); ++i) {
        const char* term = string_vector_get(terms, i);
        PostingList* plist = boolean_index_get_posting_list(idx, term);

        if (plist) {
            size_t doc_count;
            unsigned int* docs = posting_list_get_docs(plist, &doc_count);

            for (size_t j = 0; j < doc_count; ++j) {
                bool found = false;
                for (size_t k = 0; k < uint_vector_size(&result); ++k) {
                    if (uint_vector_get(&result, k) == docs[j]) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    uint_vector_push_back(&result, docs[j]);
                }
            }
            free(docs);
        }
    }

    uint_vector_sort(&result);
    return result;
}

UIntVector boolean_index_search_not(BooleanIndex* idx, const String* term) {
    UIntVector result;
    uint_vector_init(&result);

    if (!idx || !term) return result;

    PostingList* plist = boolean_index_get_posting_list(idx, string_cstr(term));

    for (unsigned int i = 0; i < idx->nextDocId; ++i) {
        if (!plist || !posting_list_contains(plist, i)) {
            uint_vector_push_back(&result, i);
        }
    }

    return result;
}

const char* boolean_index_get_document_text(BooleanIndex* idx, unsigned int docId) {
    if (!idx) return NULL;

    String* doc = hash_map_get_string(&idx->documents, docId);
    return doc ? string_cstr(doc) : NULL;
}

size_t boolean_index_get_total_documents(BooleanIndex* idx) {
    return idx ? idx->nextDocId : 0;
}

size_t boolean_index_get_total_terms(BooleanIndex* idx) {
    return idx ? hash_map_size(&idx->index) : 0;
}

StringVector boolean_index_keys(BooleanIndex* idx) {
    StringVector result;
    string_vector_init(&result);

    if (idx) {
        result = hash_map_keys(&idx->index);
    }

    return result;
}

void boolean_index_save(BooleanIndex* idx, const char* filename) {
    if (!idx || !filename) return;

    FILE* f = fopen(filename, "w");
    if (!f) return;

    fprintf(f, "BOOLEAN_INDEX\n");
    fprintf(f, "%u\n", idx->nextDocId);
    fprintf(f, "%zu\n", hash_map_size(&idx->index));

    StringVector terms = hash_map_keys(&idx->index);
    for (size_t i = 0; i < string_vector_size(&terms); ++i) {
        const char* term = string_vector_get(&terms, i);
        PostingList* plist = hash_map_get_posting_list(&idx->index, term);
        if (plist) {
            fprintf(f, "%s:", term);

            size_t doc_count;
            unsigned int* docs = posting_list_get_docs(plist, &doc_count);
            for (size_t j = 0; j < doc_count; ++j) {
                fprintf(f, " %u", docs[j]);
            }
            free(docs);
            fprintf(f, "\n");
        }
    }

    string_vector_free(&terms);
    fclose(f);
}

bool boolean_index_load(BooleanIndex* idx, const char* filename) {
    if (!idx || !filename) return false;

    FILE* f = fopen(filename, "r");
    if (!f) return false;

    char header[32];
    if (fscanf(f, "%31s", header) != 1 || strcmp(header, "BOOLEAN_INDEX") != 0) {
        fclose(f);
        return false;
    }

    unsigned int docCount;
    size_t termCount;
    if (fscanf(f, "%u %zu", &docCount, &termCount) != 2) {
        fclose(f);
        return false;
    }

    idx->nextDocId = docCount;

    char line[8192];
    fgets(line, sizeof(line), f);

    for (size_t i = 0; i < termCount; ++i) {
        if (!fgets(line, sizeof(line), f)) break;

        char* colon = strchr(line, ':');
        if (!colon) continue;

        *colon = '\0';
        char* term = line;

        unsigned int docId;
        char* ptr = colon + 1;
        PostingList* pl = (PostingList*)malloc(sizeof(PostingList));
        posting_list_init(pl);

        while (sscanf(ptr, "%u", &docId) == 1) {
            posting_list_add_doc(pl, docId);
            char* space = strchr(ptr, ' ');
            if (!space) break;
            ptr = space + 1;
        }

        hash_map_put_posting_list(&idx->index, term, pl);
    }

    fclose(f);
    return true;
}
