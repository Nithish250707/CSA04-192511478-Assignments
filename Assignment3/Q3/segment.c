/* Segment-table translation with base, limit and protection. Use --wait to keep the
   process alive so its real segments can be listed (vmmap / /proc/<pid>/maps). */
#include <stdio.h>
#include <string.h>
#include <unistd.h>

struct seg { const char *name; unsigned base, limit; const char *prot; };
static const struct seg table[] = {
    {"code",   219, 600, "r-x"},
    {"data",  2300,  14, "rw-"},
    {"heap",    90, 100, "rw-"},
    {"stack", 1327, 580, "rw-"},
    {"shared",1952,  96, "r--"},
};
#define NSEG (sizeof table / sizeof table[0])

static void translate(unsigned s, unsigned off, char access)
{
    printf("(%u, %4u) %c : ", s, off, access);
    if (s >= NSEG) { printf("TRAP - no such segment\n"); return; }
    const struct seg *g = &table[s];
    if (off >= g->limit) { printf("TRAP - offset %u >= limit %u of %s (segmentation fault)\n", off, g->limit, g->name); return; }
    if (access == 'w' && g->prot[1] != 'w') { printf("TRAP - write to %s segment (%s): protection fault\n", g->name, g->prot); return; }
    printf("%-6s base %4u + %4u = physical %u\n", g->name, g->base, off, g->base + off);
}

int main(int argc, char **argv)
{
    puts("Segment table: seg name base limit prot");
    for (unsigned i = 0; i < NSEG; i++) printf("  %u %-6s %4u %4u %s\n", i, table[i].name, table[i].base, table[i].limit, table[i].prot);
    puts("Translations (r = read, w = write):");
    unsigned s[] = {0, 1, 2, 3, 4, 0, 1, 3}, o[] = {430, 10, 500, 400, 112, 599, 14, 0};
    char a[]     = {'r','w','r','w','r','r','r','w'};
    for (int i = 0; i < 8; i++) translate(s[i], o[i], a[i]);
    puts("Extra checks:");
    translate(0, 10, 'w');   /* write into code */
    translate(5, 0, 'r');    /* segment number out of range */
    if (argc > 1 && strcmp(argv[1], "--wait") == 0) { fflush(stdout); sleep(4); }
    return 0;
}
