/* Copy-on-write after fork(): the child shares the parent's pages until it writes them. */
#define _DARWIN_C_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#ifdef __APPLE__
#include <mach/mach.h>
static double footprint_mib(void)
{
    task_vm_info_data_t vi; mach_msg_type_number_t cnt = TASK_VM_INFO_COUNT;
    if (task_info(mach_task_self(), TASK_VM_INFO, (task_info_t)&vi, &cnt) != KERN_SUCCESS) return -1;
    return vi.phys_footprint / 1048576.0;
}
#else
static double footprint_mib(void) { return -1; }   /* on Linux read /proc/self/smaps_rollup (Private_Dirty) */
#endif
static long minflt(void) { struct rusage r; getrusage(RUSAGE_SELF, &r); return r.ru_minflt; }
static double now(void) { struct timespec t; clock_gettime(CLOCK_MONOTONIC, &t); return t.tv_sec + t.tv_nsec / 1e9; }

int main(void)
{
    setvbuf(stdout, NULL, _IONBF, 0);   /* unbuffered: the child exits with _exit() */
    const size_t n = 128u << 20; long ps = sysconf(_SC_PAGESIZE);
    volatile char *buf = malloc(n);
    if (!buf) return 1;
    for (size_t i = 0; i < n; i += (size_t)ps) buf[i] = 1;            /* parent owns 128 MiB */
    printf("parent: 128 MiB touched, private footprint %.1f MiB, page size %ld\n", footprint_mib(), ps);
    double t0 = now(); pid_t pid = fork(); double t1 = now();
    if (pid == 0) {
        printf("child : fork() returned after %.3f ms (no 128 MiB copy)\n", (t1 - t0) * 1e3);
        long f0 = minflt(); double m0 = footprint_mib();
        long sum = 0; for (size_t i = 0; i < n; i += (size_t)ps) sum += buf[i];   /* read only */
        long f1 = minflt(); double m1 = footprint_mib();
        printf("child : after READING all pages  : +%ld minor faults, footprint %.1f -> %.1f MiB (sum %ld)\n", f1 - f0, m0, m1, sum);
        for (size_t i = 0; i < n / 2; i += (size_t)ps) buf[i] = 2;           /* write half */
        long f2 = minflt(); double m2 = footprint_mib();
        printf("child : after WRITING 64 MiB     : +%ld minor faults (%zu pages written), footprint %.1f MiB\n",
               f2 - f1, n / 2 / (size_t)ps, m2);
        _exit(0);
    }
    waitpid(pid, NULL, 0);
    printf("parent: buf[0] is still %d (child's write went to its own copy)\n", buf[0]);
    return 0;
}
