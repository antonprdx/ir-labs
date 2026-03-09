#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include "boolean_index.h"

static char* read_file(const char* filepath, size_t* out_size) {
    FILE* f = fopen(filepath, "rb");
    if (!f) return NULL;

    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (file_size <= 0) {
        fclose(f);
        return NULL;
    }

    char* buffer = (char*)malloc(file_size + 1);
    if (!buffer) {
        fclose(f);
        return NULL;
    }

    size_t bytes_read = fread(buffer, 1, file_size, f);
    buffer[bytes_read] = '\0';

    fclose(f);

    if (out_size) {
        *out_size = bytes_read;
    }

    return buffer;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <corpus_dir> <index_file>\n", argv[0]);
        return 1;
    }

    const char* corpus_dir = argv[1];
    const char* index_file = argv[2];

    BooleanIndex index;
    boolean_index_init(&index);

    DIR* dir = opendir(corpus_dir);
    if (!dir) {
        fprintf(stderr, "Error opening directory: %s\n", corpus_dir);
        boolean_index_free(&index);
        return 1;
    }

    struct dirent* entry;
    unsigned int doc_count = 0;

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char filepath[512];
        snprintf(filepath, sizeof(filepath), "%s/%s", corpus_dir, entry->d_name);

        struct stat path_stat;
        if (stat(filepath, &path_stat) != 0 || !S_ISREG(path_stat.st_mode)) {
            continue;
        }

        size_t file_size;
        char* content = read_file(filepath, &file_size);
        if (!content) {
            fprintf(stderr, "Warning: Could not read file: %s\n", filepath);
            continue;
        }

        boolean_index_add_document(&index, content);
        doc_count++;
        free(content);

        if (doc_count % 100 == 0) {
            fprintf(stderr, "Indexed %u documents\n", doc_count);
        }
    }

    closedir(dir);

    boolean_index_save(&index, index_file);

    printf("Total documents: %u\n", doc_count);
    printf("Total terms: %zu\n", boolean_index_get_total_terms(&index));
    printf("Index saved to: %s\n", index_file);

    boolean_index_free(&index);
    return 0;
}
