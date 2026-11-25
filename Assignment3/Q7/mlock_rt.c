/* Worst-case delay of a time-critical control loop, with and without locked memory.
   Each control cycle reads/writes the next page of its 16 MiB working buffer (state tables).
   unlocked: buffer is fresh malloc memory, so first use of each page takes a page fault.
   locked  : mlock() + pre-touch before the loop, so no page fault can happen inside it.
   Usage: mlock_rt <locked|unlocked> */
#define _DARWIN_C_SOURCE
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <time.h>
#include <unistd.h>

#ifdef CLOCK_UPTIME_RAW                     /* macOS: CLOCK_MONOTONIC has only 1 us resolution */
#define RT_CLOCK CLOCK_UPTIME_RAW
#else
#define RT_CLOCK CLOCK_MONOTONIC
#endif
static long long ns(void) { struct timespec t; clock_gettime(RT_CLOCK, &t); return t.tv_sec * 1000000000LL + t.tv_nsec; }
static int cmp(const void *a, const void *b) { long long x = *(const long long *)a, y = *(const long long *)b; return (x > y) - (x < y); }

int main(int argc, char **argv)
{
    int lock = argc > 1 && strcmp(argv[1], "locked") == 0;
    const size_t n = 16u << 20; const long ps = sysconf(_SC_PAGESIZE); const size_t pages = n / (size_t)ps;
    const int cycles = 3 * (int)pages;            /* 3 passes over the buffer */
    volatile char *buf = malloc(n);
    long long *lat = malloc(sizeof *lat * (size_t)cycles);
    if (!buf || !lat) return 1;
    memset(lat, 0, sizeof *lat * (size_t)cycles);              /* fault in the result array before timing */
    if (lock) {
        if (mlock((const void *)buf, n) != 0) { printf("mlock failed: %s\n", strerror(errno)); return 2; }
        for (size_t i = 0; i < n; i += (size_t)ps) buf[i] = 0;    /* pre-fault every page */
    }
    struct rusage r0, r1; getrusage(RUSAGE_SELF, &r0);
    for (int c = 0; c < cycles; c++) {
        size_t off = (size_t)(c % (int)pages) * (size_t)ps;
        long long t0 = ns();
        buf[off] += 1; buf[off + 64] += buf[off];                 /* the "control computation" */
        lat[c] = ns() - t0;
    }
    getrusage(RUSAGE_SELF, &r1);
    long long *s = malloc(sizeof *s * (size_t)cycles); memcpy(s, lat, sizeof *s * (size_t)cycles);
    qsort(s, (size_t)cycles, sizeof *s, cmp);
    long long first_pass_max = 0; for (size_t c = 0; c < pages; c++) if (lat[c] > first_pass_max) first_pass_max = lat[c];
    long long later_max = 0; for (int c = (int)pages; c < cycles; c++) if (lat[c] > later_max) later_max = lat[c];
    printf("%-8s cycles %d | page faults in loop %3ld | median %4lld ns  p99 %6lld ns  p99.9 %6lld ns  worst %7lld ns"
           " | worst 1st pass %7lld ns, worst later passes %6lld ns\n",
           lock ? "locked" : "unlocked", cycles, r1.ru_minflt - r0.ru_minflt, s[cycles / 2], s[cycles * 99 / 100],
           s[cycles * 999 / 1000], s[cycles - 1], first_pass_max, later_max);
    if (lock) munlock((const void *)buf, n);
    return 0;
}
