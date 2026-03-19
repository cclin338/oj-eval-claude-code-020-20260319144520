
#include "buddy.c"
#include <stdio.h>
#include <stdlib.h>

int main() {
    void *pool = malloc(128 * 1024 * 1024);
    init_page(pool, 32768);
    
    void *p = alloc_pages(1);
    printf("First allocation: %p\nExpected: %p\n", p, pool);
    if (p != pool) {
        printf("ERROR: First allocation not at start of pool!\n");
        return 1;
    }
    
    printf("SUCCESS\n");
    return 0;
}

