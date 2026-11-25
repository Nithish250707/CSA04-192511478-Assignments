/* Disk-scheduling simulator: FCFS, SSTF, SCAN, C-SCAN, LOOK, C-LOOK.
   Part 1: the given queue (head 53, cylinders 0-199) - order and total head movement.
   Part 2: 10,000 random requests, Poisson arrivals (mean gap 10 / 5 / 3.5 ms = light / medium /
           heavy), service time = 1 ms + 0.05 ms per cylinder moved, seed 42. Runs to a disk
           end (SCAN), and returns (C-SCAN, C-LOOK) count as head movement and take time.
           wait = time from arrival until the request's seek starts.
   Part 3: starvation - one request at 183 plus a request near cylinder 60 every 1 ms for 5 s. */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define MAXCYL 199
enum { FCFS, SSTF, SCAN, CSCAN, LOOK, CLOOK, NALG };
static const char *NAME[NALG] = {"FCFS", "SSTF", "SCAN", "C-SCAN", "LOOK", "C-LOOK"};

typedef struct { double arr; int cyl; } Req;
typedef struct { double avg_move, avg_wait, max_wait, total_move; double wait_of_first; } Stats;

static Stats simulate(int alg, const Req *r, int n, int head, int dir)
{
    char *done = calloc((size_t)n, 1); double *wait = calloc((size_t)n, sizeof *wait);
    double t = 0, total = 0; int served = 0, arrived = 0, fcfs_next = 0;
    while (served < n) {
        while (arrived < n && r[arrived].arr <= t) arrived++;
        if (arrived == served && served < n) {          /* nothing pending: idle until next arrival */
            int any = 0; for (int i = 0; i < arrived; i++) if (!done[i]) { any = 1; break; }
            if (!any) { t = r[arrived].arr; continue; }
        }
        int best = -1;
        if (alg == FCFS) { while (fcfs_next < arrived && done[fcfs_next]) fcfs_next++; best = fcfs_next < arrived ? fcfs_next : -1; }
        else if (alg == SSTF) {
            for (int i = 0; i < arrived; i++) if (!done[i] && (best < 0 || abs(r[i].cyl - head) < abs(r[best].cyl - head))) best = i;
        } else {
            int d = (alg == CSCAN || alg == CLOOK) ? 1 : dir;
            for (int i = 0; i < arrived; i++) {
                if (done[i]) continue;
                int dist = (r[i].cyl - head) * d;
                if (dist >= 0 && (best < 0 || dist < (r[best].cyl - head) * d)) best = i;
            }
            if (best < 0) {                                 /* nothing ahead in this direction */
                int lo = -1; for (int i = 0; i < arrived; i++) if (!done[i] && (lo < 0 || r[i].cyl < r[lo].cyl)) lo = i;
                if (lo < 0) { t = r[arrived].arr; continue; }
                int move;
                if (alg == SCAN)       { int end = dir > 0 ? MAXCYL : 0; move = abs(end - head); head = end; dir = -dir; }
                else if (alg == LOOK)  { move = 0; dir = -dir; }
                else if (alg == CSCAN) { move = (MAXCYL - head) + MAXCYL; head = 0; }
                else                   { move = head - r[lo].cyl; head = r[lo].cyl; }          /* C-LOOK */
                total += move; t += 0.05 * move;
                continue;
            }
        }
        if (best < 0) { t = r[arrived].arr; continue; }
        wait[best] = t - r[best].arr;
        int move = abs(r[best].cyl - head);
        total += move; t += 1.0 + 0.05 * move; head = r[best].cyl; done[best] = 1; served++;
    }
    Stats s = {total / n, 0, 0, total, wait[0]};
    for (int i = 0; i < n; i++) { s.avg_wait += wait[i] / n; if (wait[i] > s.max_wait) s.max_wait = wait[i]; }
    free(done); free(wait);
    return s;
}

/* Part 1 helper: static queue, all requests present at t = 0, print the service order */
static void static_queue(int alg)
{
    const int q[] = {98, 183, 37, 122, 14, 124, 65, 67}, n = 8; int head = 53, dir = -1;  /* SCAN/LOOK go towards 0 first */
    int done[8] = {0}, total = 0, fi = 0;
    printf("%-7s %d", NAME[alg], head);
    for (int served = 0; served < n;) {
        int best = -1;
        if (alg == FCFS) best = fi++;
        else if (alg == SSTF) { for (int i = 0; i < n; i++) if (!done[i] && (best < 0 || abs(q[i] - head) < abs(q[best] - head))) best = i; }
        else {
            int d = (alg == CSCAN || alg == CLOOK) ? 1 : dir;
            for (int i = 0; i < n; i++) if (!done[i] && (q[i] - head) * d >= 0 && (best < 0 || (q[i] - head) * d < (q[best] - head) * d)) best = i;
            if (best < 0) {
                if (alg == SCAN) { int end = dir > 0 ? MAXCYL : 0; total += abs(end - head); printf(" (%d)", end); head = end; dir = -dir; }
                else if (alg == LOOK) dir = -dir;
                else if (alg == CSCAN) { total += (MAXCYL - head) + MAXCYL; printf(" (%d, jump to 0)", MAXCYL); head = 0; }
                else { int lo = -1; for (int i = 0; i < n; i++) if (!done[i] && (lo < 0 || q[i] < q[lo])) lo = i;
                       total += head - q[lo]; printf(" (jump)"); head = q[lo]; }
                continue;
            }
        }
        printf(" %d", q[best]); total += abs(q[best] - head); head = q[best]; done[best] = 1; served++;
    }
    printf("  -> %d cylinders%s\n", total, alg == CSCAN ? " (incl. 199-cylinder jump)" : alg == CLOOK ? " (incl. the return jump)" : "");
}

static double expo(double mean) { return -mean * log(1.0 - (double)rand() / ((double)RAND_MAX + 1.0)); }

int main(void)
{
    puts("== Part 1: head 53, queue 98 183 37 122 14 124 65 67 (SCAN/LOOK move towards 0 first; C-SCAN/C-LOOK move up)");
    for (int a = 0; a < NALG; a++) static_queue(a);

    puts("\n== Part 2: 10,000 random requests, seed 42");
    const double gaps[3] = {10.0, 5.0, 3.5}; const char *load[3] = {"light", "medium", "heavy"};
    const int n = 10000; Req *r = malloc(sizeof *r * n);
    printf("%-7s %-7s %14s %14s %14s\n", "load", "alg", "avg move/req", "avg wait ms", "max wait ms");
    for (int l = 0; l < 3; l++) {
        srand(42); double t = 0;
        for (int i = 0; i < n; i++) { t += expo(gaps[l]); r[i].arr = t; r[i].cyl = rand() % (MAXCYL + 1); }
        for (int a = 0; a < NALG; a++) {
            Stats s = simulate(a, r, n, 53, 1);
            printf("%-7s %-7s %14.2f %14.2f %14.2f\n", load[l], NAME[a], s.avg_move, s.avg_wait, s.max_wait);
        }
    }

    puts("\n== Part 3: starvation - request at 183 (t = 0) + one request at cylinder 55..65 every 1 ms for 5,000 ms");
    puts("   head starts at 60 moving towards 0, so SCAN/LOOK must finish their downward sweep first");
    const int m = 5001; Req *q = malloc(sizeof *q * m);
    q[0] = (Req){0.0, 183}; srand(7);
    for (int i = 1; i < m; i++) q[i] = (Req){(double)(i - 1), 55 + rand() % 11};
    for (int a = 0; a < NALG; a++) {
        Stats s = simulate(a, q, m, 60, -1);
        printf("%-7s wait of the request at 183: %9.1f ms   (avg wait of all %8.1f ms)\n", NAME[a], s.wait_of_first, s.avg_wait);
    }
    free(r); free(q);
    return 0;
}
