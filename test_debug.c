#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "buddy.h"

int main() {
    int i, ret, pgIdx, currentRank, returnPages;
    void *p, *q;

    p = malloc(128 * 1024 * 1024);
    ret = init_page(p, 32768);
    printf("init_page returned: %d\n", ret);

    // Allocate all pages
    for (pgIdx = 0; pgIdx < 32768; pgIdx++) {
        void *r = alloc_pages(1);
        if (pgIdx < 5) printf("alloc %d: %p\n", pgIdx, r);
    }

    printf("\nPhase 8A simulation:\n");
    q = p;
    currentRank = 1;
    while (currentRank <= 2) {  // Just test first two iterations
        returnPages = (currentRank <= 1) ? 1 : (1 << (currentRank - 2));
        printf("currentRank=%d, returnPages=%d\n", currentRank, returnPages);

        for (int i = 0; i < returnPages; i++, q += 4096) {
            ret = return_pages(q);
            printf("  freed page at %p, ret=%d\n", q, ret);
        }

        printf("  query_page_counts(%d) = %d\n", currentRank, query_page_counts(currentRank));

        currentRank++;
    }

    return 0;
}