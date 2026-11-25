/* Page-table sizes, effective access time with a TLB, and internal fragmentation. */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

static double mib(double b) { return b / (1024.0 * 1024.0); }

int main(void)
{
    puts("== Page-table size, 4 KB pages (single-level table)");
    double e32 = pow(2, 32 - 12), e48 = pow(2, 48 - 12);
    printf("32-bit: 2^20 = %.0f entries x 4 B = %.0f B = %.0f MiB per process\n", e32, e32 * 4, mib(e32 * 4));
    printf("48-bit: 2^36 = %.0f entries x 8 B = %.0f B = %.0f GiB per process (impossible -> multi-level)\n",
           e48, e48 * 8, e48 * 8 / pow(1024, 3));
    puts("48-bit, 4-level (x86-64): each table = 512 entries x 8 B = 4 KB.");
    puts("A process using 1 GiB contiguous: 2^18 pages / 512 per page table = 512 PTs,");
    printf("  plus 1 PD + 1 PDPT + 1 PML4 = %d tables = %d KiB (instead of 512 GiB flat)\n", 515, 515 * 4);

    puts("\n== Effective memory-access time (memory 100 ns, TLB lookup 20 ns - assumed values)");
    double m = 100, t = 20;
    printf("No TLB: 2 x %.0f = %.0f ns\n", m, 2 * m);
    double ratios[] = {0.80, 0.95, 0.99};
    for (int i = 0; i < 3; i++) {
        double a = ratios[i], eat = a * (t + m) + (1 - a) * (t + 2 * m);
        printf("TLB hit %2.0f%%: %.2f*(%g+%g) + %.2f*(%g+%g) = %.1f ns (%.0f%% slower than 100 ns)\n",
               a * 100, a, t, m, 1 - a, t, 2 * m, eat, (eat / m - 1) * 100);
    }

    puts("\n== Internal fragmentation: 100 processes, sizes uniform 1 KB .. 8 MB, seed 42");
    srand(42);
    long sizes[100];
    long total = 0;
    for (int i = 0; i < 100; i++) { sizes[i] = 1024 + (long)((double)rand() / RAND_MAX * (8L * 1024 * 1024 - 1024)); total += sizes[i]; }
    printf("total requested: %ld B (%.1f MiB)\n", total, mib(total));
    long ps[] = {1024, 4096, 2L * 1024 * 1024};
    const char *name[] = {"1 KB", "4 KB", "2 MB"};
    printf("%-6s %12s %14s %10s %16s\n", "page", "pages used", "wasted bytes", "waste %", "avg waste/proc");
    for (int k = 0; k < 3; k++) {
        long pages = 0, waste = 0;
        for (int i = 0; i < 100; i++) {
            long n = (sizes[i] + ps[k] - 1) / ps[k];
            pages += n; waste += n * ps[k] - sizes[i];
        }
        printf("%-6s %12ld %14ld %9.3f%% %14.0f B\n", name[k], pages, waste, 100.0 * waste / total, waste / 100.0);
    }
    puts("(expected average waste is about half a page per process: 512 B, 2 KB, 1 MB)");
    return 0;
}
