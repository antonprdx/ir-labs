#include "posting_list.h"
#include <string.h>

static int compare_unsigned_int(const void* a, const void* b) {
    unsigned int ia = *(const unsigned int*)a;
    unsigned int ib = *(const unsigned int*)b;
    return (ia > ib) - (ia < ib);
}

void posting_list_init(PostingList* pl) {
    if (pl) {
        pl->docs = NULL;
        pl->size = 0;
        pl->capacity = 0;
    }
}

void posting_list_free(PostingList* pl) {
    if (pl && pl->docs) {
        free(pl->docs);
        pl->docs = NULL;
        pl->size = 0;
        pl->capacity = 0;
    }
}

void posting_list_add_doc(PostingList* pl, unsigned int docId) {
    if (!pl) return;

    for (size_t i = 0; i < pl->size; i++) {
        if (pl->docs[i] == docId) {
            return;
        }
    }

    if (pl->size >= pl->capacity) {
        pl->capacity = pl->capacity == 0 ? 4 : pl->capacity * 2;
        pl->docs = (unsigned int*)realloc(pl->docs, pl->capacity * sizeof(unsigned int));
        if (!pl->docs) {
            pl->capacity = 0;
            pl->size = 0;
            return;
        }
    }

    pl->docs[pl->size++] = docId;

    posting_list_sort(pl);
}

unsigned int* posting_list_get_docs(const PostingList* pl, size_t* out_size) {
    if (!pl || !out_size) return NULL;

    *out_size = pl->size;
    if (pl->size == 0) return NULL;

    unsigned int* copy = (unsigned int*)malloc(pl->size * sizeof(unsigned int));
    if (copy) {
        memcpy(copy, pl->docs, pl->size * sizeof(unsigned int));
    }
    return copy;
}

size_t posting_list_size(const PostingList* pl) {
    return pl ? pl->size : 0;
}

bool posting_list_contains(const PostingList* pl, unsigned int docId) {
    if (!pl || pl->size == 0) return false;

    int left = 0;
    int right = pl->size - 1;

    while (left <= right) {
        int mid = left + (right - left) / 2;
        if (pl->docs[mid] == docId) {
            return true;
        }
        if (pl->docs[mid] < docId) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    return false;
}

PostingList* posting_list_intersect(const PostingList* a, const PostingList* b) {
    if (!a || !b) return NULL;

    PostingList* result = (PostingList*)malloc(sizeof(PostingList));
    if (!result) return NULL;

    posting_list_init(result);

    size_t i = 0, j = 0;
    while (i < a->size && j < b->size) {
        if (a->docs[i] < b->docs[j]) {
            i++;
        } else if (a->docs[i] > b->docs[j]) {
            j++;
        } else {
            posting_list_add_doc(result, a->docs[i]);
            i++;
            j++;
        }
    }

    return result;
}

PostingList* posting_list_unite(const PostingList* a, const PostingList* b) {
    if (!a || !b) return NULL;

    PostingList* result = (PostingList*)malloc(sizeof(PostingList));
    if (!result) return NULL;

    posting_list_init(result);

    size_t i = 0, j = 0;
    while (i < a->size && j < b->size) {
        if (a->docs[i] < b->docs[j]) {
            posting_list_add_doc(result, a->docs[i]);
            i++;
        } else if (a->docs[i] > b->docs[j]) {
            posting_list_add_doc(result, b->docs[j]);
            j++;
        } else {
            posting_list_add_doc(result, a->docs[i]);
            i++;
            j++;
        }
    }

    while (i < a->size) {
        posting_list_add_doc(result, a->docs[i]);
        i++;
    }

    while (j < b->size) {
        posting_list_add_doc(result, b->docs[j]);
        j++;
    }

    return result;
}

PostingList* posting_list_difference(const PostingList* a, const PostingList* b) {
    if (!a || !b) return NULL;

    PostingList* result = (PostingList*)malloc(sizeof(PostingList));
    if (!result) return NULL;

    posting_list_init(result);

    for (size_t i = 0; i < a->size; i++) {
        if (!posting_list_contains(b, a->docs[i])) {
            posting_list_add_doc(result, a->docs[i]);
        }
    }

    return result;
}

void posting_list_sort(PostingList* pl) {
    if (pl && pl->size > 1) {
        qsort(pl->docs, pl->size, sizeof(unsigned int), compare_unsigned_int);
    }
}
