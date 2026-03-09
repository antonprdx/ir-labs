#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "boolean_index.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <index_file>\n", argv[0]);
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

    printf("=== Index Statistics ===\n");
    printf("Total documents: %zu\n", boolean_index_get_total_documents(&index));
    printf("Total unique terms: %zu\n", boolean_index_get_total_terms(&index));
    printf("\n");

    if (boolean_index_get_total_terms(&index) > 0) {
        printf("=== Term Statistics ===\n");

        size_t total_posting_size = 0;
        size_t max_posting_size = 0;
        size_t min_posting_size = boolean_index_get_total_documents(&index) + 1;

        printf("\nMost frequent terms (top 20):\n");
        printf("Rank | Term | Document Count\n");
        printf("-----|------|----------------\n");

        int rank = 1;
        StringVector terms = boolean_index_keys(&index);

        for (size_t i = 0; i < string_vector_size(&terms) && rank <= 20; ++i) {
            const char* term_str = string_vector_get(&terms, i);

            String term;
            string_init_copy(&term, term_str);

            PostingList* plist = boolean_index_get_posting_list(&index, string_cstr(&term));
            if (plist) {
                size_t count = posting_list_size(plist);
                total_posting_size += count;
                if (count > max_posting_size) max_posting_size = count;
                if (count < min_posting_size) min_posting_size = count;

                printf("%4d | %s | %lu\n", rank++, term_str, count);
            }

            string_free(&term);
        }

        printf("\n=== Index Characteristics ===\n");
        size_t total_terms = boolean_index_get_total_terms(&index);
        printf("Average posting list size: %.2f\n",
               total_terms > 0 ? (double)total_posting_size / total_terms : 0.0);
        printf("Max posting list size: %zu\n", max_posting_size);
        printf("Min posting list size: %zu\n",
               min_posting_size > boolean_index_get_total_documents(&index) ? 0 : min_posting_size);

        string_vector_free(&terms);
    }

    boolean_index_free(&index);
    return 0;
}
