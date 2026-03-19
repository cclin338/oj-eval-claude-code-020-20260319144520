#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_RANK 16
#define PAGE_SIZE 4096
#define NULL ((void *)0)
#define OK 0

// Include the actual implementation
#define static // Remove static for debugging
#define inline // Remove inline for debugging
#include "buddy.c"

void *base_addr_global;

int init_page(void *p, int pgcount);
void *alloc_pages(int rank);
int return_pages(void *p);
int query_ranks(void *p);
int query_page_counts(int rank);

int main() {
    void *pool = malloc(128 * 1024 * 1024);
    base_addr_global = pool;
    printf("Pool allocated at: %p\n", pool);

    init_page(pool, 32768);

    printf("\nFirst few allocations:\n");
    for (int i = 0; i < 5; i++) {
        void *p = alloc_pages(1);
        printf("  alloc %d: %p (offset: %ld)\n", i, p, (char*)p - (char*)pool);
    }

    return 0;
}