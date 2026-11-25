/* Trigger real memory faults and report what the OS delivered.
   mode 1: write to a string literal (read-only page)   mode 2: NULL dereference
   mode 3: same as 2 but with no handler (default action: the process is killed) */
#define _DARWIN_C_SOURCE
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void handler(int sig, siginfo_t *si, void *ctx)
{
    (void)ctx;
    char buf[160];
    int n = snprintf(buf, sizeof buf, "caught signal %d (%s), faulting address %p, si_code %d\n",
                     sig, sig == SIGSEGV ? "SIGSEGV" : sig == SIGBUS ? "SIGBUS" : "?", si->si_addr, si->si_code);
    write(1, buf, (size_t)n);
    _exit(128 + sig);
}

int main(int argc, char **argv)
{
    int mode = argc > 1 ? atoi(argv[1]) : 1;
    if (mode != 3) {
        struct sigaction sa;
        memset(&sa, 0, sizeof sa);
        sa.sa_sigaction = handler; sa.sa_flags = SA_SIGINFO;
        sigaction(SIGSEGV, &sa, NULL); sigaction(SIGBUS, &sa, NULL);
    }
    volatile char *p;
    if (mode == 1) { p = (volatile char *)"read-only literal"; printf("writing to read-only literal at %p\n", (void *)p); fflush(stdout); p[0] = 'X'; }
    else           { p = NULL; printf("dereferencing NULL\n"); fflush(stdout); p[0] = 'X'; }
    puts("not reached");
    return 0;
}
