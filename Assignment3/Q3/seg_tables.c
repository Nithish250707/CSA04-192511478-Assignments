/* Size of the address-translation tables for one process using 2 MB code, 8 MB data,
   64 MB heap and 1 MB stack in a 32-bit address space (4 KB pages, 4-byte entries). */
#include <stdio.h>
int main(void)
{
    long seg_mb[] = {2, 8, 64, 1}, used = 0;
    for (int i = 0; i < 4; i++) used += seg_mb[i];
    long flat = (1L << 20) * 4;                       /* one entry per page of 4 GB */
    long segtab = 4 * 8;                              /* base + limit (+prot) = 8 B each */
    long segpaged = segtab;
    for (int i = 0; i < 4; i++) segpaged += (seg_mb[i] * 1024 / 4) * 4;
    long two_level = 4096 + 4096L * ((used * 1024 / 4 + 1023) / 1024 + 3);   /* dir + ~used PTs */
    printf("process uses %ld MB in 4 segments\n", used);
    printf("pure paging, single-level table  : %8ld B (%ld KB) - covers the whole 4 GB space\n", flat, flat / 1024);
    printf("pure paging, two-level table     : %8ld B (%ld KB) - only tables for used regions\n", two_level, two_level / 1024);
    printf("pure segmentation                : %8ld B - 4 entries of base+limit\n", segtab);
    printf("segmentation with paging         : %8ld B (%ld KB) - one page table per segment\n", segpaged, segpaged / 1024);
    return 0;
}
