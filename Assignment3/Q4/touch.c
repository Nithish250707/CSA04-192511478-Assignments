/* Touch every page of an N-MiB buffer once, in sequential or random order, and count the
   page faults (getrusage ru_minflt / ru_majflt) and the time. Usage: touch <MiB> <seq|rand> */
#define _DARWIN_C_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <time.h>
#include <unistd.h>

static double now(void) { struct timespec t; clock_gettime(CLOCK_MONOTONIC, &t); return t.tv_sec + t.tv_nsec / 1e9; }

int main(int argc, char **argv)
{
    size_t mib = argc > 1 ? strtoul(argv[1], NULL, 10) : 256;
    int rnd = argc > 2 && strcmp(argv[2], "rand") == 0;
    long ps = sysconf(_SC_PAGESIZE);
    size_t n = mib << 20, pages = n / (size_t)ps;
    size_t *order = malloc(pages * sizeof *order);
    volatile char *buf = malloc(n);
    if (!order || !buf) { perror("malloc"); return 1; }
    for (size_t i = 0; i < pages; i++) order[i] = i;
    if (rnd) { srand(1); for (size_t i = pages - 1; i > 0; i--) { size_t j = (size_t)rand() % (i + 1), t = order[i]; order[i] = order[j]; order[j] = t; } }

    struct rusage a, b, c;
    getrusage(RUSAGE_SELF, &a); double t0 = now();
    for (size_t i = 0; i < pages; i++) buf[order[i] * (size_t)ps] = 1;          /* first touch */
    double t1 = now(); getrusage(RUSAGE_SELF, &b);
    for (size_t i = 0; i < pages; i++) buf[order[i] * (size_t)ps] += 1;         /* second touch */
    double t2 = now(); getrusage(RUSAGE_SELF, &c);
    printf("%5zu MiB %-4s pages %7zu | 1st pass: minor %7ld major %ld  %8.2f ms (%5.0f ns/page) | 2nd pass: minor %ld  %7.2f ms\n",
           mib, rnd ? "rand" : "seq", pages, b.ru_minflt - a.ru_minflt, b.ru_majflt - a.ru_majflt,
           (t1 - t0) * 1e3, (t1 - t0) * 1e9 / pages, c.ru_minflt - b.ru_minflt, (t2 - t1) * 1e3);
    return 0;
}
