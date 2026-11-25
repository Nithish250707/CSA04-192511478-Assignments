/* FIFO, LRU, OPT and CLOCK (second chance) on strong-locality and random reference strings,
   3..10 frames, plus running time for 100,000 references. Also prints an LRU trace (--trace).
   OPT uses a precomputed next-use array so it is O(n * frames) instead of O(n^2 * frames). */
#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAXF 16
enum { NREF = 100000, PAGES = 50 };

static int find(const int f[], int n, int p) { for (int i = 0; i < n; i++) if (f[i] == p) return i; return -1; }

static long fifo(const int *r, int len, int fr)
{ int f[MAXF], used = 0, next = 0; long faults = 0;
  for (int i = 0; i < len; i++) { if (find(f, used, r[i]) >= 0) continue; faults++;
    if (used < fr) f[used++] = r[i]; else { f[next] = r[i]; next = (next + 1) % fr; } }
  return faults; }

static long lru(const int *r, int len, int fr, int trace)
{ int f[MAXF], last[MAXF], used = 0; long faults = 0;
  for (int i = 0; i < len; i++) {
    int k = find(f, used, r[i]);
    if (k >= 0) last[k] = i;
    else { faults++; int v;
      if (used < fr) v = used++; else { v = 0; for (int j = 1; j < fr; j++) if (last[j] < last[v]) v = j; }
      f[v] = r[i]; last[v] = i; }
    if (trace) { printf("%2d: ref %d %s  frames [", i + 1, r[i], k >= 0 ? "hit  " : "FAULT");
      for (int j = 0; j < used; j++) printf("%s%d", j ? " " : "", f[j]); printf("]\n"); } }
  return faults; }

static long opt(const int *r, int len, int fr, int *nextuse)
{ int f[MAXF], nx[MAXF], used = 0; long faults = 0; int lastpos[PAGES + 64];
  for (int p = 0; p < PAGES + 64; p++) lastpos[p] = len;
  for (int i = len - 1; i >= 0; i--) { nextuse[i] = lastpos[r[i]]; lastpos[r[i]] = i; }
  for (int i = 0; i < len; i++) {
    int k = find(f, used, r[i]);
    if (k >= 0) { nx[k] = nextuse[i]; continue; }
    faults++;
    if (used < fr) { f[used] = r[i]; nx[used++] = nextuse[i]; continue; }
    int v = 0; for (int j = 1; j < fr; j++) if (nx[j] > nx[v]) v = j;
    f[v] = r[i]; nx[v] = nextuse[i]; }
  return faults; }

static long clock_alg(const int *r, int len, int fr)
{ int f[MAXF], ref[MAXF], used = 0, hand = 0; long faults = 0;
  for (int i = 0; i < len; i++) {
    int k = find(f, used, r[i]);
    if (k >= 0) { ref[k] = 1; continue; }              /* hardware would just set this bit */
    faults++;
    if (used < fr) { f[used] = r[i]; ref[used++] = 1; continue; }
    while (ref[hand]) { ref[hand] = 0; hand = (hand + 1) % fr; }   /* second chance */
    f[hand] = r[i]; ref[hand] = 1; hand = (hand + 1) % fr; }
  return faults; }

static void gen_locality(int *r, int len)   /* 90% of refs inside a 5-page working set that moves every 500 refs */
{ for (int i = 0; i < len; i++) { int base = (i / 500 * 7) % (PAGES - 5);
    r[i] = (rand() % 100 < 90) ? base + rand() % 5 : rand() % PAGES; } }
static void gen_random(int *r, int len) { for (int i = 0; i < len; i++) r[i] = rand() % PAGES; }

static double now(void) { struct timespec t; clock_gettime(CLOCK_MONOTONIC, &t); return t.tv_sec + t.tv_nsec / 1e9; }

int main(int argc, char **argv)
{
    if (argc > 1 && strcmp(argv[1], "--trace") == 0) {
        int r[] = {7, 0, 1, 2, 0, 3, 0, 4, 2, 3, 0, 3, 2, 1, 2, 0, 1, 7, 0, 1};
        printf("LRU, 3 frames: %ld faults\n", lru(r, 20, 3, 1));
        return 0;
    }
    static int loc[NREF], rnd[NREF], nextuse[NREF];
    srand(2026); gen_locality(loc, NREF); gen_random(rnd, NREF);
    if (argc > 1 && strcmp(argv[1], "--csv") == 0) {
        puts("workload,frames,FIFO,LRU,OPT,CLOCK");
        for (int w = 0; w < 2; w++) { int *r = w ? rnd : loc;
            for (int fr = 3; fr <= 10; fr++)
                printf("%s,%d,%ld,%ld,%ld,%ld\n", w ? "random" : "locality", fr, fifo(r, NREF, fr),
                       lru(r, NREF, fr, 0), opt(r, NREF, fr, nextuse), clock_alg(r, NREF, fr)); }
        return 0;
    }
    printf("Running time for %d references (locality string, 4 and 8 frames), best of 5:\n", NREF);
    const char *nm[] = {"FIFO", "LRU", "OPT", "CLOCK"};
    for (int fr = 4; fr <= 8; fr += 4) for (int a = 0; a < 4; a++) {
        double best = 1e9; long faults = 0;
        for (int rep = 0; rep < 5; rep++) { double t0 = now();
            faults = a == 0 ? fifo(loc, NREF, fr) : a == 1 ? lru(loc, NREF, fr, 0) : a == 2 ? opt(loc, NREF, fr, nextuse) : clock_alg(loc, NREF, fr);
            double t = now() - t0; if (t < best) best = t; }
        printf("  %d frames %-5s %7ld faults  %7.3f ms  (%.1f ns/reference)\n", fr, nm[a], faults, best * 1e3, best * 1e9 / NREF);
    }
    return 0;
}
