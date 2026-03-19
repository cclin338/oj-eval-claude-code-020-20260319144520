#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_RANK 16
#define NULL ((void *)0)
#define OK 0
#define EINVAL 22
#define ENOSPC 28
#define IS_ERR_VALUE(x) ((x) >= (unsigned long)-4095)
static inline void *ERR_PTR(long error) { return (void *)error; }
static inline long PTR_ERR(const void *ptr) { return (long)ptr; }
static inline long IS_ERR(const void *ptr) { return IS_ERR_VALUE((unsigned long)ptr); }

// Include the actual implementation
#include "buddy.c"

int main() {
    void *pool = malloc(128 * 1024 * 1024);
    printf("Pool base: %p\n", pool);
    printf("MAXRANK0PAGE: %d\n", 128 * 1024 / 4);

    init_page(pool, 32768);

    printf("\nAfter init:\n");
    for (int i = 1; i <= MAX_RANK; i++) {
        printf("Free list rank %d: %s\n", i, free_lists[i] ? "has blocks" : "empty");
    }

    printf("\nFirst allocation:\n");
    void *p = alloc_pages(1);
    printf("Result: %p\n", p);

    printf("\nAfter first alloc:\n");
    for (int i = 1; i <= MAX_RANK; i++) {
        printf("Free list rank %d: %s\n", i, free_lists[i] ? "has blocks" : "empty");
    }

    return 0;
}