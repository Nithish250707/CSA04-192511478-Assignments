/* Task 5.2 / alternative - replication and consistency, simulated.
   A. 12 storage nodes for one year (hourly steps). Each node fails with MTBF 1 year (on
      average) and comes back after a 24 h repair; disks are NOT lost permanently, so this
      measures AVAILABILITY. 1,000 data blocks; each block is placed on k distinct nodes.
      A block is unavailable in an hour when all k nodes holding it are down.
      k = 1 also models one big NFS server holding everything (1 node).
   B. Eventual consistency: a replica applies each write after a random lag (exponential,
      mean L ms). Reads go to a random replica at a random time 0-200 ms after the write.
      Report the fraction of reads that still return the old value. */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#define NODES 12
#define BLOCKS 1000
#define HOURS (24 * 365)
static double u(void) { return (double)rand() / ((double)RAND_MAX + 1.0); }
int main(void)
{
    puts("== A. Availability over one year (MTBF 8,760 h per node, 24 h repair)");
    for (int k = 1; k <= 3; k++) {
        srand(99);
        int place[BLOCKS][3];
        for (int b = 0; b < BLOCKS; b++) for (int j = 0; j < k; j++) {
            int n, dup; do { n = rand() % NODES; dup = 0; for (int x = 0; x < j; x++) if (place[b][x] == n) dup = 1; } while (dup);
            place[b][j] = n; }
        int down_until[NODES] = {0}; long block_hours_down = 0; int hours_any_down = 0;
        for (int h = 0; h < HOURS; h++) {
            for (int n = 0; n < NODES; n++) if (down_until[n] <= h && u() < 1.0 / 8760) down_until[n] = h + 24;
            int any = 0;
            for (int b = 0; b < BLOCKS; b++) { int alive = 0; for (int j = 0; j < k; j++) if (down_until[place[b][j]] <= h) alive = 1;
                if (!alive) { block_hours_down++; any = 1; } }
            hours_any_down += any;
        }
        printf("k = %d copies: storage %dx | hours with some data unavailable %4d | block availability %.6f%%\n",
               k, k, hours_any_down, 100.0 * (1 - (double)block_hours_down / ((double)BLOCKS * HOURS)));
    }
    srand(5); int nfs_down = 0, until = 0;
    for (int h = 0; h < HOURS; h++) { if (until <= h && u() < 1.0 / 8760) until = h + 24; if (until > h) nfs_down++; }
    printf("single NFS server (1 node holds all data): storage 1x | hours everything unavailable %d\n", nfs_down);

    puts("\n== B. Eventual consistency: stale reads vs replication lag (3 replicas, 1 updated synchronously)");
    srand(3);
    const double lags[] = {1, 10, 50, 200};
    for (int i = 0; i < 4; i++) {
        long stale = 0, reads = 100000;
        for (long r = 0; r < reads; r++) {
            double t = u() * 200.0; int replica = rand() % 3;
            if (replica == 0) continue;                              /* the primary is always fresh */
            double lag = -lags[i] * log(1 - u());
            if (t < lag) stale++;
        }
        printf("mean lag %5.0f ms: %5.2f%% of reads return the old value (strong consistency: 0%%, but writes wait for all replicas)\n",
               lags[i], 100.0 * stale / reads);
    }
    return 0;
}
