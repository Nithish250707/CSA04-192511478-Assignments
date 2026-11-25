/* Addresses at the start and end of a page, and beyond the last page (PAGE_SIZE 1024, 8 pages). */
#include <stdio.h>
#define PAGE_SIZE 1024
#define PAGES 8
int page_table[PAGES] = {5, 2, -1, 7, 0, -1, 3, -1};
int main(void)
{
    unsigned t[] = {0, 1023, 1024, 2047, 3072, 4095, 6144, 7167, 7168, 8191, 8192, 8193};
    for (unsigned i = 0; i < sizeof t / sizeof t[0]; i++) {
        unsigned a = t[i], p = a / PAGE_SIZE, d = a % PAGE_SIZE;
        printf("%5u -> page %u offset %4u : ", a, p, d);
        if (p >= PAGES) printf("invalid address (beyond last page 7)\n");
        else if (page_table[p] == -1) printf("page fault\n");
        else printf("physical %u\n", (unsigned)page_table[p] * PAGE_SIZE + d);
    }
    return 0;
}
