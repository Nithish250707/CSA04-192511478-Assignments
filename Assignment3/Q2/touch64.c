/* Touch 64 MiB (one write per 1 KiB) so the page-fault count can be read with /usr/bin/time.
   The volatile pointer stops the compiler from optimising the writes away. */
#include <stdio.h>
#include <stdlib.h>
int main(void)
{
    size_t n = 64u << 20;
    volatile char *p = malloc(n);
    if (!p) return 1;
    for (size_t i = 0; i < n; i += 1024) p[i] = 1;
    printf("touched %zu bytes\n", n);
    free((void *)p);
    return 0;
}
