/* Same disk state as alloc.c, but the new file needs 14 blocks: 21 blocks are free, the longest
   free run is 13, so contiguous allocation fails while indexed allocation succeeds. */
#include <stdio.h>
#include <string.h>
#define BLOCKS 32
static char disk[BLOCKS];
static int contiguous(char f, int n)
{ int run = 0; for (int b = 0; b < BLOCKS; b++) { run = disk[b] == '.' ? run + 1 : 0;
    if (run == n) { memset(disk + b - n + 1, f, (size_t)n); return b - n + 1; } } return -1; }
static int indexed(char f, int n)
{ int idx = -1, got = 0, blk[BLOCKS];
  for (int b = 0; b < BLOCKS && got < n; b++) if (disk[b] == '.') { if (idx < 0) idx = b; else blk[got++] = b; }
  if (idx < 0 || got < n) return -1; disk[idx] = (char)(f + 32); for (int i = 0; i < n; i++) disk[blk[i]] = f; return idx; }
int main(void)
{
    memset(disk, '.', BLOCKS);
    contiguous('A', 6); contiguous('B', 8); contiguous('C', 5); contiguous('D', 7);
    for (int b = 0; b < BLOCKS; b++) if (disk[b] == 'B' || disk[b] == 'D') disk[b] = '.';
    int free = 0, run = 0, longest = 0;
    for (int b = 0; b < BLOCKS; b++) { if (disk[b] == '.') { free++; if (++run > longest) longest = run; } else run = 0; }
    printf("%.32s  free %d, longest free run %d\n", disk, free, longest);
    printf("F (14 blocks) contiguous: start %d\n", contiguous('F', 14));
    printf("F (14 blocks) indexed: index block %d\n%.32s\n", indexed('F', 14), disk);
    return 0;
}
