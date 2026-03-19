#include "buddy.h"
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#define NULL ((void *)0)
#define PAGE_SIZE 4096
#define MAX_RANK 16

// Structure to track page information
typedef struct page_info {
    int rank;           // Current rank of the page/block
    int is_free;        // 1 if free, 0 if allocated
    struct page_info *prev;  // Previous page in the same free list
    struct page_info *next;  // Next page in the same free list
} page_info_t;

// Global variables
static void *base_addr = NULL;      // Base address of the page pool
static int total_pages = 0;         // Total number of pages
static page_info_t *page_metadata = NULL;  // Array of page metadata

// Free lists for each rank
static page_info_t *free_lists[MAX_RANK + 1];

// Helper functions
static int get_page_index(void *p) {
    if (p < base_addr || p >= (base_addr + total_pages * PAGE_SIZE)) {
        return -1;
    }
    return ((char *)p - (char *)base_addr) / PAGE_SIZE;
}

static void *get_page_address(int index) {
    if (index < 0 || index >= total_pages) {
        return NULL;
    }
    return (char *)base_addr + index * PAGE_SIZE;
}

static int get_buddy_index(int page_index, int rank) {
    int buddy_index;
    int block_size = 1 << (rank - 1);

    if (page_index % (2 * block_size) == 0) {
        buddy_index = page_index + block_size;
    } else {
        buddy_index = page_index - block_size;
    }

    return buddy_index;
}

static void add_to_free_list(page_info_t *page, int rank) {
    page->rank = rank;
    page->is_free = 1;
    page->prev = NULL;
    page->next = free_lists[rank];

    if (free_lists[rank] != NULL) {
        free_lists[rank]->prev = page;
    }
    free_lists[rank] = page;
}

static void remove_from_free_list(page_info_t *page, int rank) {
    if (page->prev != NULL) {
        page->prev->next = page->next;
    } else {
        free_lists[rank] = page->next;
    }

    if (page->next != NULL) {
        page->next->prev = page->prev;
    }

    page->is_free = 0;
    page->prev = page->next = NULL;
}

int init_page(void *p, int pgcount) {
    int i;

    if (p == NULL || pgcount <= 0) {
        return -EINVAL;
    }

    base_addr = p;
    total_pages = pgcount;

    // Allocate metadata for all pages
    page_metadata = (page_info_t *)malloc(pgcount * sizeof(page_info_t));
    if (page_metadata == NULL) {
        return -EINVAL;
    }

    // Initialize all free lists
    for (i = 1; i <= MAX_RANK; i++) {
        free_lists[i] = NULL;
    }

    // Clear all page metadata
    memset(page_metadata, 0, pgcount * sizeof(page_info_t));

    // Add all pages as one large free block of maximum rank
    page_metadata[0].rank = MAX_RANK;
    page_metadata[0].is_free = 1;
    page_metadata[0].prev = NULL;
    page_metadata[0].next = NULL;
    free_lists[MAX_RANK] = &page_metadata[0];

    return OK;
}

void *alloc_pages(int rank) {
    int i, target_rank;
    page_info_t *page;
    int block_size, buddy_block_size;
    int page_index, buddy_index;

    if (rank < 1 || rank > MAX_RANK) {
        return ERR_PTR(-EINVAL);
    }

    // Find the smallest rank that has free pages >= requested rank
    target_rank = rank;
    while (target_rank <= MAX_RANK && free_lists[target_rank] == NULL) {
        target_rank++;
    }

    if (target_rank > MAX_RANK) {
        return ERR_PTR(-ENOSPC);
    }

    // Split blocks until we reach the target rank
    while (target_rank > rank) {
        page = free_lists[target_rank];
        remove_from_free_list(page, target_rank);

        // Split the block
        block_size = 1 << (target_rank - 1);
        page_index = page - page_metadata;
        buddy_index = page_index + (block_size / 2);

        // Add first half to lower rank
        page->rank = target_rank - 1;
        add_to_free_list(page, target_rank - 1);

        // Add second half (buddy) to lower rank
        page_metadata[buddy_index].rank = target_rank - 1;
        add_to_free_list(&page_metadata[buddy_index], target_rank - 1);

        target_rank--;
    }

    // Now allocate the block of exact rank
    page = free_lists[rank];
    if (page == NULL) {
        return ERR_PTR(-ENOSPC);
    }

    remove_from_free_list(page, rank);

    return get_page_address(page - page_metadata);
}

int return_pages(void *p) {
    int page_index, buddy_index, rank;
    page_info_t *page, *buddy;
    int block_size;
    int merged;

    if (p == NULL) {
        return -EINVAL;
    }

    page_index = get_page_index(p);
    if (page_index < 0) {
        return -EINVAL;
    }

    page = &page_metadata[page_index];
    rank = page->rank;

    if (page->is_free) {
        return -EINVAL;  // Already free
    }

    // Try to merge with buddy
    while (rank < MAX_RANK) {
        block_size = 1 << (rank - 1);
        buddy_index = get_buddy_index(page_index, rank);

        if (buddy_index < 0 || buddy_index >= total_pages) {
            break;
        }

        buddy = &page_metadata[buddy_index];

        // Check if buddy is free and has the same rank
        if (!buddy->is_free || buddy->rank != rank) {
            break;
        }

        // Remove buddy from free list
        remove_from_free_list(buddy, rank);

        // Merge - keep the page with smaller index
        if (page_index < buddy_index) {
            page->rank = rank + 1;
        } else {
            buddy->rank = rank + 1;
            page = buddy;
            page_index = buddy_index;
        }

        rank++;
    }

    // Add to free list
    add_to_free_list(page, rank);

    return OK;
}

int query_ranks(void *p) {
    int page_index;
    page_info_t *page;

    if (p == NULL) {
        return -EINVAL;
    }

    page_index = get_page_index(p);
    if (page_index < 0) {
        return -EINVAL;
    }

    page = &page_metadata[page_index];
    return page->rank;
}

int query_page_counts(int rank) {
    int count = 0;
    page_info_t *page;

    if (rank < 1 || rank > MAX_RANK) {
        return -EINVAL;
    }

    // Count the number of free blocks at this rank level
    page = free_lists[rank];
    while (page != NULL) {
        count++;
        page = page->next;
    }

    return count;
}
