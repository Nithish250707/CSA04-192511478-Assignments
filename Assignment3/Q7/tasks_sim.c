/* Memory of the edge device simulated with a global-LRU page-replacement simulator.
   1 frame = 1 MiB. RAM 2048 frames, kernel + services 200 frames (fixed), no swap.
   RT control task 64 pages, locked (mlock) or not; ML task 1500 pages (model weights read
   from a file-backed mapping, swept with locality); logging task appends new pages and
   rarely rereads; optional 4th task of 500 pages with random access.
   The RT task also has a 16-page emergency-stop path (pages 64..79) that runs only once
   every 2,500 cycles - rarely used, but it must never wait for a page fault.
   A fault on an evicted page means it is re-read from flash (file-backed). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RAM 2048
#define KERNEL 200
enum { RT, ML, LOG, T4, NT };
static const char *tname[NT] = {"RT control", "ML", "logging", "4th task"};

typedef struct { int task, page; long last; int locked; } Frame;
static Frame fr[RAM];
static int nfr;
static long faults[NT], refs[NT], refaults[NT];
static unsigned char seen[NT][8192];

static void ref(int task, int page, long t, int locked)
{
    refs[task]++;
    for (int i = 0; i < nfr; i++) if (fr[i].task == task && fr[i].page == page) { fr[i].last = t; return; }
    faults[task]++;
    if (seen[task][page]) refaults[task]++; else seen[task][page] = 1;   /* re-fault = page was evicted */
    if (nfr < RAM - KERNEL) { fr[nfr++] = (Frame){task, page, t, locked}; return; }
    int v = -1;
    for (int i = 0; i < nfr; i++) if (!fr[i].locked && (v < 0 || fr[i].last < fr[v].last)) v = i;
    if (v < 0) { fprintf(stderr, "out of memory\n"); exit(1); }
    fr[v] = (Frame){task, page, t, locked};
}

static void run(const char *label, int rt_locked, int with_t4)
{
    memset(faults, 0, sizeof faults); memset(refs, 0, sizeof refs); memset(refaults, 0, sizeof refaults); memset(seen, 0, sizeof seen); nfr = 0;
    srand(11);
    int logpage = 0, mlpos = 0;
    long ticks = 20000;
    for (long t = 1; t <= ticks; t++) {
        ref(RT, (int)(t % 64), t * 10, rt_locked);                       /* control loop: its own 64 pages */
        if (t % 2500 == 0) for (int e = 64; e < 80; e++) ref(RT, e, t * 10, rt_locked);   /* emergency path */
        for (int k = 0; k < 6; k++) {                                     /* ML: sweep weights with locality */
            mlpos = (rand() % 100 < 85) ? (mlpos + 1) % 1500 : rand() % 1500;
            ref(ML, mlpos, t * 10 + 1 + k, 0);
        }
        if (t % 4 == 0) ref(LOG, logpage++, t * 10 + 7, 0);              /* logging: append a page */
        if (with_t4) for (int k = 0; k < 2; k++) ref(T4, rand() % 500, t * 10 + 8 + k, 0);
    }
    printf("%s\n", label);
    for (int i = 0; i < NT; i++) if (refs[i])
        printf("  %-11s refs %7ld  faults %6ld  (%.2f%%)  of which re-faults after eviction %6ld\n",
               tname[i], refs[i], faults[i], 100.0 * faults[i] / refs[i], refaults[i]);
    int locked = 0; for (int i = 0; i < nfr; i++) locked += fr[i].locked;
    printf("  frames: %d usable, %d locked by RT\n", RAM - KERNEL, locked);
}

int main(void)
{
    run("Scenario 1: RT + ML + logging, RT locked", 1, 0);
    run("Scenario 1b: RT + ML + logging, RT NOT locked", 0, 0);
    run("Scenario 2: + 4th task of 500 MB, RT locked", 1, 1);
    run("Scenario 3: + 4th task of 500 MB, RT NOT locked", 0, 1);
    return 0;
}
