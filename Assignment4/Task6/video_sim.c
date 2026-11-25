/* Task 6 - storage design for the video-streaming start-up.
   Workload (assumed): 2,000 videos of 300 MB-2 GB, Zipf(1) popularity, 5 Mbit/s streams,
   peak 100 playback starts per second (Poisson), each start reads 2 MiB at a random minute.
   Devices (assumed, typical datasheet figures): HDD seek 8 ms + half rotation 4.17 ms (7,200 rpm),
   150 MB/s transfer; SSD 0.1 ms + 500 MB/s. Optional argument: measured SSD ms per 2 MiB read.
   Design A: 4 HDDs, each video on one HDD (video id mod 4).
   Design B: 1 SSD holding the 400 most popular videos + the same 4 HDDs for the rest.
   Discrete-event simulation, FIFO queue per device, 200,000 playback starts, seed 1. */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#define V 2000
#define STARTS 200000
static double u(void) { return ((double)rand() + 1.0) / ((double)RAND_MAX + 2.0); }
static int cmpd(const void *a, const void *b) { double x = *(const double *)a, y = *(const double *)b; return (x > y) - (x < y); }

static double hdd_ms(double mb) { return 8.0 + 4.17 + mb / 150.0 * 1000; }
static double ssd_fixed = -1;
static double ssd_ms(double mb) { return ssd_fixed > 0 ? ssd_fixed * mb / 2.0 : 0.1 + mb / 500.0 * 1000; }

static void design(const char *name, int ssd_top, const double *cdf, double rate_per_s)
{
    srand(1);
    double free_at[5] = {0}, busy[5] = {0}; double *lat = malloc(sizeof *lat * STARTS);
    long hits = 0; double t = 0, ssd_lat = 0, hdd_lat = 0; long nh = 0, ns = 0;
    for (int i = 0; i < STARTS; i++) {
        t += -log(u()) / rate_per_s * 1000.0;                          /* ms */
        double x = u(); int lo = 0, hi = V - 1;                          /* Zipf rank by inverse CDF */
        while (lo < hi) { int mid = (lo + hi) / 2; if (cdf[mid] < x) lo = mid + 1; else hi = mid; }
        int rank = lo, dev; double svc;
        if (rank < ssd_top) { dev = 4; svc = ssd_ms(2.0); hits++; } else { dev = rank % 4; svc = hdd_ms(2.0); }
        double start = t > free_at[dev] ? t : free_at[dev];
        free_at[dev] = start + svc; busy[dev] += svc;
        lat[i] = free_at[dev] - t;
        if (dev == 4) { ssd_lat += lat[i]; ns++; } else { hdd_lat += lat[i]; nh++; }
    }
    double mean = 0; for (int i = 0; i < STARTS; i++) mean += lat[i] / STARTS;
    qsort(lat, STARTS, sizeof *lat, cmpd);
    printf("%s\n", name);
    if (ssd_top) printf("  SSD hit ratio %.1f%% | SSD utilisation %.0f%%, mean start latency on SSD %.1f ms\n",
                        100.0 * hits / STARTS, 100 * busy[4] / t, ns ? ssd_lat / ns : 0);
    printf("  HDD utilisation %.0f%% %.0f%% %.0f%% %.0f%%, mean start latency on HDD %.1f ms\n",
           100 * busy[0] / t, 100 * busy[1] / t, 100 * busy[2] / t, 100 * busy[3] / t, nh ? hdd_lat / nh : 0);
    printf("  ALL starts: mean %.1f ms, p95 %.1f ms, p99 %.1f ms, max %.1f ms\n",
           mean, lat[STARTS * 95 / 100], lat[STARTS * 99 / 100], lat[STARTS - 1]);
    free(lat);
}

int main(int argc, char **argv)
{
    if (argc > 1) ssd_fixed = atof(argv[1]);
    srand(1);
    double size_gb[V], total = 0;
    for (int i = 0; i < V; i++) { size_gb[i] = 0.3 + 1.7 * u(); total += size_gb[i]; }
    double cdf[V], h = 0; for (int i = 0; i < V; i++) h += 1.0 / (i + 1);
    double c = 0; for (int i = 0; i < V; i++) { c += 1.0 / (i + 1) / h; cdf[i] = c; }
    double top400 = 0; for (int i = 0; i < 400; i++) top400 += size_gb[i];

    puts("== Capacity and throughput");
    printf("library %.0f GB (avg %.2f GB/video); 2 copies = %.0f GB; +25%% free space = %.0f GB raw\n",
           total, total / V, 2 * total, 2.5 * total);
    printf("SSD tier for the top 400 videos: %.0f GB; they receive %.1f%% of views (Zipf 1)\n", top400, 100 * cdf[399]);
    double streams = 2000, mbps = 5, need = streams * mbps / 8;    /* MB/s */
    double hdd_stream = 4.0 / ((8 + 4.17) / 1000 + 4.0 / 150);       /* 4 MB chunk + one seek each */
    printf("2,000 streams x 5 Mbit/s = %.0f MB/s; one HDD streaming 4 MB chunks with a seek each = %.1f MB/s (%.0f streams)\n",
           need, hdd_stream, hdd_stream * 8 / mbps);
    printf("Design A devices at <= 80%% load: %.0f HDDs\n", ceil(need / (hdd_stream * 0.8)));
    printf("Design B devices at <= 80%% load: %.0f SSDs (%.0f MB/s of hits) + %.0f HDDs (%.0f MB/s of misses)\n",
           ceil(need * cdf[399] / (500 * 0.8)), need * cdf[399], ceil(need * (1 - cdf[399]) / (hdd_stream * 0.8)), need * (1 - cdf[399]));

    puts("\n== Disk reads to start playing at minute m of a 1 GB video (4 KB blocks, 5 Mbit/s)");
    for (int m = 0; m <= 25; m += 5) {
        long blk = (long)(m * 60 * mbps / 8 * 1e6 / 4096);
        int ind = blk < 12 ? 1 : blk < 12 + 1024 ? 2 : blk < 12 + 1024 + 1048576 ? 3 : 4;
        printf("  minute %2d -> block %7ld: classic inode %d reads (+1 inode); extents 1 extent-tree read + 1 data read\n", m, blk, ind);
    }

    printf("\n== Simulation: 100 starts/s, 2 MiB per start%s\n", ssd_fixed > 0 ? " (SSD time = measured value)" : " (assumed device figures)");
    design("Design A: 4 HDDs", 0, cdf, 100);
    design("Design B: SSD cache (top 400) + 4 HDDs", 400, cdf, 100);
    return 0;
}
