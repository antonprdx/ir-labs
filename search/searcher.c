#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "boolean_searcher.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <index_file> [query]\n", argv[0]);
        fprintf(stderr, "Examples:\n");
        fprintf(stderr, "  %s index.bin \"word1 and word2\"\n", argv[0]);
        fprintf(stderr, "  %s index.bin \"word1 or word2\"\n", argv[0]);
        fprintf(stderr, "  %s index.bin \"-word1 word2\"\n", argv[0]);
        return 1;
    }

    const char* index_file = argv[1];

    BooleanIndex index;
    boolean_index_init(&index);

    if (!boolean_index_load(&index, index_file)) {
        fprintf(stderr, "Error loading index from: %s\n", index_file);
        boolean_index_free(&index);
        return 1;
    }

    BooleanSearcher searcher;
    boolean_searcher_init(&searcher, &index);

    if (argc >= 3) {
        const char* query = argv[2];
        UIntVector results = boolean_searcher_search(&searcher, query);

        printf("Query: %s\n", query);
        printf("Found documents: %zu\n", uint_vector_size(&results));

        for (size_t i = 0; i < uint_vector_size(&results); ++i) {
            printf("Doc %u\n", uint_vector_get(&results, i));
        }

        uint_vector_free(&results);
    } else {
        char query[2048];
        printf("Boolean Search Engine (type 'quit' to exit)\n");

        while (1) {
            printf("query> ");
            if (!fgets(query, sizeof(query), stdin)) break;

            size_t len = strlen(query);
            if (len > 0 && query[len - 1] == '\n') {
                query[len - 1] = '\0';
            }

            if (strcmp(query, "quit") == 0 || strcmp(query, "exit") == 0) {
                break;
            }

            if (strlen(query) == 0) {
                continue;
            }

            UIntVector results = boolean_searcher_search(&searcher, query);
            printf("Found: %zu documents\n", uint_vector_size(&results));

            for (size_t i = 0; i < uint_vector_size(&results) && i < 10; ++i) {
                const char* doc_text = boolean_index_get_document_text(&index,
                                                                       uint_vector_get(&results, i));
                printf("  Doc %u: %s\n", uint_vector_get(&results, i),
                       doc_text ? doc_text : "[no text]");
            }

            if (uint_vector_size(&results) > 10) {
                printf("  ... and %zu more\n", uint_vector_size(&results) - 10);
            }

            uint_vector_free(&results);
        }
    }

    boolean_index_free(&index);
    return 0;
}
