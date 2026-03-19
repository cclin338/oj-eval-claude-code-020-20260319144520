#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "buddy.h"

int main() {
    int ret, i;
    void *p, *pages[10];

    p = malloc(128 * 1024 * 1024);
    ret = init_page(p, 32768);
    printf("init_page returned: %d\n", ret);

    // Allocate 4 pages
    for (i = 0; i < 4; i++) {
        pages[i] = alloc_pages(1);
        printf("alloc %d: %p\n", i, pages[i]);
    }

    printf("\nFreeing page 0 (should be buddy with page 1)\n");
    ret = return_pages(pages[0]);
    printf("return_pages(0) = %d\n", ret);
    printf("query_page_counts(1) = %d\n", query_page_counts(1));

    printf("\nFreeing page 1 (should merge with page 0 to make rank 2 block)\n");
    ret = return_pages(pages[1]);
    printf("return_pages(1) = %d\n", ret);
    printf("query_page_counts(1) = %d\n", query_page_counts(1));
    printf("query_page_counts(2) = %d\n", query_page_counts(2));

    printf("\nFreeing page 2\n");
    ret = return_pages(pages[2]);
    printf("return_pages(2) = %d\n", ret);
    printf("query_page_counts(1) = %d\n", query_page_counts(1));

    printf("\nFreeing page 3 (should merge with page 2)\n");
    ret = return_pages(pages[3]);
    printf("return_pages(3) = %d\n", ret);
    printf("query_page_counts(1) = %d\n", query_page_counts(1));
    printf("query_page_counts(2) = %d\n", query_page_counts(2));
    printf("query_page_counts(3) = %d\n", query_page_counts(3));

    return 0;
}