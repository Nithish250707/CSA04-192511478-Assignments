/* Real device measurement for Task 6: random 2 MiB reads (the read needed to start playback)
   from a 512 MiB file with the page cache bypassed (F_NOCACHE / O_DIRECT). Usage: measure_ssd <file> */
#define _DARWIN_C_SOURCE
#define _GNU_SOURCE
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#define MiB (1u << 20)
static double now(void) { struct timespec t; clock_gettime(CLOCK_MONOTONIC, &t); return t.tv_sec + t.tv_nsec / 1e9; }
static int cmpd(const void *a, const void *b) { double x = *(const double *)a, y = *(const double *)b; return (x > y) - (x < y); }
static int open_nocache(const char *p, int fl)
{
#ifdef O_DIRECT
    fl |= O_DIRECT;
#endif
    int fd = open(p, fl, 0600);
#ifdef F_NOCACHE
    if (fd >= 0) fcntl(fd, F_NOCACHE, 1);
#endif
    return fd;
}
int main(int argc, char **argv)
{
    if (argc < 2) return 1;
    const size_t fsz = 512u * MiB, rd = 2 * MiB; char *buf; if (posix_memalign((void **)&buf, 4096, 4 * MiB)) return 1;
    int fd = open_nocache(argv[1], O_WRONLY | O_CREAT | O_TRUNC);
    for (size_t i = 0; i < 4 * MiB; i++) buf[i] = (char)(i * 31u);
    for (size_t off = 0; off < fsz; off += 4 * MiB) if (write(fd, buf, 4 * MiB) != (ssize_t)(4 * MiB)) return 1;
    fsync(fd); close(fd);
    fd = open_nocache(argv[1], O_RDONLY);
    const int n = 200; double lat[200]; srand(4);
    double t0 = now();
    for (int i = 0; i < n; i++) {
        off_t off = (off_t)(rand() % (int)((fsz - rd) / 4096)) * 4096;
        double s = now(); if (pread(fd, buf, rd, off) != (ssize_t)rd) return 1; lat[i] = (now() - s) * 1e3;
    }
    double tot = now() - t0; close(fd); unlink(argv[1]);
    qsort(lat, n, sizeof lat[0], cmpd); double mean = 0; for (int i = 0; i < n; i++) mean += lat[i] / n;
    printf("this Mac's SSD, %d random 2 MiB uncached reads: mean %.3f ms, p95 %.3f ms, max %.3f ms, %.0f MiB/s\n",
           n, mean, lat[n * 95 / 100], lat[n - 1], n * 2.0 / tot);
    return 0;
}
