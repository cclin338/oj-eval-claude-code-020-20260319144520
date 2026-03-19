#include <stdio.h>
#include <stdlib.h>
#include "buddy.h"

int main() {
    void *p;
    int ret;

    p = malloc(128 * 1024 * 1024);
    ret = init_page(p, 32768);
    printf("init_page: %d\n", ret);

    void *r = alloc_pages(1);
    printf("First alloc: %p\n", r);
    printf("Expected: %p\n", p);
    printf("Match: %s\n", r == p ? "YES" : "NO");

    // Check return_pages with NULL
    int err = return_pages(NULL);
    printf("return_pages(NULL): %d\n", err);

    return 0;
}