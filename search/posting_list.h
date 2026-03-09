#ifndef POSTING_LIST_H
#define POSTING_LIST_H

#include <stdlib.h>
#include <stdbool.h>

typedef struct {
    unsigned int* docs;
    size_t size;
    size_t capacity;
} PostingList;

void posting_list_init(PostingList* pl);

void posting_list_free(PostingList* pl);

void posting_list_add_doc(PostingList* pl, unsigned int docId);

unsigned int* posting_list_get_docs(const PostingList* pl, size_t* out_size);

size_t posting_list_size(const PostingList* pl);

bool posting_list_contains(const PostingList* pl, unsigned int docId);

PostingList* posting_list_intersect(const PostingList* a, const PostingList* b);

PostingList* posting_list_unite(const PostingList* a, const PostingList* b);

PostingList* posting_list_difference(const PostingList* a, const PostingList* b);

void posting_list_sort(PostingList* pl);

#endif
