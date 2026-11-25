/* Sequential read speed of a file with the page cache bypassed (F_NOCACHE on macOS, O_DIRECT on
   Linux), compared with a second, cached read. Usage: read_speed <file> */
#define _DARWIN_C_SOURCE
#define _GNU_SOURCE
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
static double now(void) { struct timespec t; clock_gettime(CLOCK_MONOTONIC, &t); return t.tv_sec + t.tv_nsec / 1e9; }
static double readall(const char *path, int nocache)
{
    int fl = O_RDONLY;
#ifdef O_DIRECT
    if (nocache) fl |= O_DIRECT;
#endif
    int fd = open(path, fl); if (fd < 0) { perror("open"); exit(1); }
#ifdef F_NOCACHE
    if (nocache) fcntl(fd, F_NOCACHE, 1);
#endif
    size_t bs = 4u << 20; void *buf; if (posix_memalign(&buf, 4096, bs)) exit(1);
    double t = now(); long long tot = 0; ssize_t n;
    while ((n = read(fd, buf, bs)) > 0) tot += n;
    t = now() - t; close(fd); free(buf);
    printf("  %-28s %lld MiB in %.3f s = %.0f MiB/s\n", nocache ? "uncached (F_NOCACHE/O_DIRECT)" : "cached (page cache)", tot >> 20, t, (tot >> 20) / t);
    return t;
}
/* Create the test file with caching disabled so its pages are NOT left in the page cache;
   otherwise the "uncached" read would just be served from RAM. */
static void create_uncached(const char *path, size_t mib)
{
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0600); if (fd < 0) { perror("create"); exit(1); }
#ifdef F_NOCACHE
    fcntl(fd, F_NOCACHE, 1);
#endif
    size_t bs = 4u << 20; char *buf; if (posix_memalign((void **)&buf, 4096, bs)) exit(1);
    for (size_t i = 0; i < bs; i++) buf[i] = (char)(i * 2654435761u >> 24);
    for (size_t k = 0; k < mib / 4; k++) { buf[0] = (char)k; if (write(fd, buf, bs) != (ssize_t)bs) { perror("write"); exit(1); } }
    fsync(fd); close(fd); free(buf);
}
int main(int argc, char **argv)
{
    if (argc < 2) { fprintf(stderr, "usage: %s file\n", argv[0]); return 1; }
    create_uncached(argv[1], 512);
    printf("file %s (512 MiB, written with caching disabled)\n", argv[1]);
    readall(argv[1], 1); readall(argv[1], 1);
    readall(argv[1], 0); readall(argv[1], 0);
    return 0;
}
