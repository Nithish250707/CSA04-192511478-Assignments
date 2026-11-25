/* External fragmentation under contiguous allocation (first fit), 64 units of memory. */
#include <stdio.h>
#include <string.h>
#define MEM 64
static char mem[MEM + 1];
static int alloc(char id, int n)
{
    for (int s = 0, run = 0; s < MEM; s++) {
        run = mem[s] == '.' ? run + 1 : 0;
        if (run == n) { memset(mem + s - n + 1, id, n); return s - n + 1; }
    }
    return -1;
}
static void release(char id) { for (int i = 0; i < MEM; i++) if (mem[i] == id) mem[i] = '.'; }
static void show(const char *what)
{
    int free = 0, largest = 0, run = 0;
    for (int i = 0; i < MEM; i++) { if (mem[i] == '.') { free++; if (++run > largest) largest = run; } else run = 0; }
    printf("%-26s |%s|  free %2d, largest hole %2d\n", what, mem, free, largest);
}
int main(void)
{
    memset(mem, '.', MEM);
    const char ids[] = "ABCDEFGH"; int sz[] = {10, 6, 9, 5, 12, 7, 8, 7};
    for (int i = 0; i < 8; i++) alloc(ids[i], sz[i]);
    show("8 programs loaded");
    release('B'); release('D'); release('F'); release('H');
    show("B, D, F, H finish");
    printf("Load program X needing 20 units: start = %d\n", alloc('X', 20));
    puts("-> 25 units are free in total, but no single hole is 20 long: external fragmentation.");
    puts("   With paging, X would get any 20 free frames; no contiguous hole is needed.");
    return 0;
}
