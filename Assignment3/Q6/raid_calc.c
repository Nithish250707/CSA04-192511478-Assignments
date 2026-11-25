/* Usable capacity of 6 x 4 TB at each RAID level, I/Os per small write, failures survived,
   and the probability of losing data during a rebuild.
   Assumptions (stated, not measured): annual failure rate 2% per disk; rebuild of a 4 TB disk
   at 100 MB/s = 11.1 h; unrecoverable read error rate 1 per 1e14 bits (consumer) / 1e15 (enterprise). */
#include <math.h>
#include <stdio.h>
int main(void)
{
    const int n = 6; const double tb = 4.0;
    puts("Level    usable   efficiency  survives            I/Os per small write");
    printf("RAID 0  %4.0f TB   %5.1f%%     0 failures          1\n", n * tb, 100.0);
    printf("RAID 1  %4.0f TB   %5.1f%%     1 per mirror pair   2 (3 pairs of mirrors)\n", n * tb / 2, 50.0);
    printf("RAID 5  %4.0f TB   %5.1f%%     1 failure           4 (read data+parity, write data+parity)\n", (n - 1) * tb, 100.0 * (n - 1) / n);
    printf("RAID 6  %4.0f TB   %5.1f%%     any 2 failures      6 (read data+P+Q, write data+P+Q)\n", (n - 2) * tb, 100.0 * (n - 2) / n);
    printf("RAID 10 %4.0f TB   %5.1f%%     1 per pair (up to 3) 2\n", n * tb / 2, 50.0);

    const double afr = 0.02, rebuild_h = 4e12 / 100e6 / 3600.0;
    double p = 1 - pow(1 - afr, rebuild_h / 8760.0);         /* one disk fails during the rebuild */
    printf("\nRebuild window %.1f h; P(a given disk fails during it) = %.3e\n", rebuild_h, p);
    double r1 = p;                                           /* only the mirror partner matters */
    double r5 = 1 - pow(1 - p, n - 1);                       /* any of the 5 survivors */
    double r6 = 0; for (int k = 2; k <= n - 1; k++) {        /* 2+ of 5 survivors */
        double c = 1; for (int i = 0; i < k; i++) c = c * (n - 1 - i) / (i + 1);
        r6 += c * pow(p, k) * pow(1 - p, n - 1 - k); }
    puts("P(data loss | one disk has already failed), disk failures only:");
    printf("  RAID 1 / RAID 10 : %.3e\n  RAID 5           : %.3e\n  RAID 6           : %.3e\n", r1, r5, r6);
    double first = 1 - pow(1 - afr, n);
    printf("P(at least one of 6 disks fails in a year) = %.2f%%  ->  yearly loss: RAID 5 %.4f%%, RAID 10 %.4f%%, RAID 6 %.7f%%\n",
           first * 100, first * r5 * 100, first * r1 * 100, first * r6 * 100);

    double bits5 = (n - 1) * tb * 1e12 * 8, bits1 = tb * 1e12 * 8;
    puts("Unrecoverable read errors while rebuilding (all surviving data must be read):");
    for (int e = 14; e <= 15; e++) {
        double ure = pow(10, -e);
        printf("  URE 1e-%d: RAID 5 reads %.0f TB -> P(>=1 URE) = %.1f%%;  RAID 1/10 reads 4 TB -> %.1f%%\n",
               e, (n - 1) * tb, 100 * (1 - exp(-bits5 * ure)), 100 * (1 - exp(-bits1 * ure)));
    }
    return 0;
}
