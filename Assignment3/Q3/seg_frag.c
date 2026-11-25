/* External fragmentation under segmentation: load 20 segments of random size into
   10,000 units of memory (first fit), remove 10 of them, then try to load new segments. */
#include <stdio.h>
#include <stdlib.h>
#define MEM 10000
static int owner[MEM];   /* 0 = free, else segment id */

static int load(int id, int size)
{
    for (int s = 0, run = 0; s < MEM; s++) {
        run = owner[s] ? 0 : run + 1;
        if (run == size) { for (int i = s - size + 1; i <= s; i++) owner[i] = id; return s - size + 1; }
    }
    return -1;
}
static void unload(int id) { for (int i = 0; i < MEM; i++) if (owner[i] == id) owner[i] = 0; }
static void report(const char *when)
{
    int free = 0, holes = 0, largest = 0, run = 0;
    for (int i = 0; i < MEM; i++) {
        if (!owner[i]) { free++; run++; if (run == 1) holes++; if (run > largest) largest = run; } else run = 0;
    }
    printf("%-34s free %5d  holes %2d  largest hole %5d  external fragmentation %.1f%%\n",
           when, free, holes, largest, free ? 100.0 * (free - largest) / free : 0.0);
}
int main(void)
{
    srand(7);
    int size[21];
    for (int id = 1; id <= 20; id++) {
        size[id] = 100 + rand() % 900;   /* 100 .. 999 units */
        if (load(id, size[id]) < 0) printf("segment %d (%d) did not fit\n", id, size[id]);
    }
    report("after loading 20 segments:");
    for (int id = 2; id <= 20; id += 2) unload(id);   /* remove every second segment */
    report("after removing 10 segments:");
    int tried = 0, failed = 0;
    for (int id = 21; id <= 40; id++) {
        int s = 600 + rand() % 400; tried++;
        if (load(id, s) < 0) { failed++; printf("  new segment %d of %d units does not fit\n", id, s); }
    }
    printf("new segments tried %d, failed %d\n", tried, failed);
    report("after loading new segments:");
    return 0;
}
