/* Task 2.3 / 2.4 / alternative: disk reads to reach the last block of a 4 GB file,
   maximum file size of a classic inode, free-space bit vector and FAT size for a 4 TB disk. */
#include <stdio.h>
int main(void)
{
    const unsigned long long B = 4096, file = 4ULL << 30, disk = 4ULL << 40, ptr = 4;
    const unsigned long long nblk = file / B, ppb = B / ptr;        /* pointers per block */
    printf("4 GB file / 4 KB blocks = %llu blocks; pointers per index block = %llu\n", nblk, ppb);
    unsigned long long last = nblk - 1;
    printf("Reads to reach the last block (block %llu):\n", last);
    printf("  contiguous : 1 (address = start + %llu)\n", last);
    printf("  linked     : %llu (follow the chain from block 0)\n", nblk);
    printf("  FAT        : 1 data read + %llu in-memory FAT look-ups (if the FAT is cached)\n", last);
    unsigned long long d = 12, s = d + ppb, dd = s + ppb * ppb;
    const char *where = last < d ? "direct" : last < s ? "single-indirect" : last < dd ? "double-indirect" : "triple-indirect";
    int reads = last < d ? 1 : last < s ? 2 : last < dd ? 3 : 4;
    printf("  inode      : block %llu is in the %s range [%llu, %llu) -> %d reads (+1 if the inode is not cached)\n",
           last, where, s, dd, reads);
    unsigned long long maxblk = 12 + ppb + ppb * ppb + ppb * ppb * ppb;
    printf("Maximum file size: (12 + %llu + %llu^2 + %llu^3) x 4 KB = %llu blocks = %llu bytes = %.3f TiB\n",
           ppb, ppb, ppb, maxblk, maxblk * B, (double)(maxblk * B) / (1ULL << 40));
    unsigned long long dblk = disk / B;
    printf("4 TB disk = %llu blocks\n", dblk);
    printf("  bit vector : %llu bits = %llu bytes = %llu MiB\n", dblk, dblk / 8, dblk / 8 >> 20);
    printf("  FAT (4 B per block): %llu bytes = %llu GiB\n", dblk * 4, dblk * 4 >> 30);
    printf("  linked free list: 0 extra bytes (pointers live in the free blocks), O(n) to find a run\n");
    return 0;
}
