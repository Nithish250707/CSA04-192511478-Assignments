/* Task 5.3 evidence - buffering. Write 16 MiB as 4,096 writes of 4 KiB:
   (a) write() + fsync() after every block (no buffering, every block forced to the device)
   (b) write() only - the kernel buffers the blocks and writes them back later, one fsync at end
   (c) stdio fwrite() - user-space buffer (fewer system calls) + kernel buffer, one fsync at end
   Note: on macOS fsync() hands data to the drive; F_FULLFSYNC would also flush the drive cache. */
#define _DARWIN_C_SOURCE
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#define BS 4096
#define NB 4096
static double now(void) { struct timespec t; clock_gettime(CLOCK_MONOTONIC, &t); return t.tv_sec + t.tv_nsec / 1e9; }
int main(int argc, char **argv)
{
    const char *path = argc > 1 ? argv[1] : "buffer_test.bin";
    char blk[BS]; memset(blk, 'x', BS);
    double t;
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0600);
    t = now(); for (int i = 0; i < NB; i++) { if (write(fd, blk, BS) != BS) return 1; fsync(fd); } t = now() - t; close(fd);
    printf("(a) write + fsync every 4 KiB : %8.1f ms  (%6.0f MiB/s, %d syscalls + %d fsyncs)\n", t * 1e3, 16 / t, NB, NB);
    fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0600);
    t = now(); for (int i = 0; i < NB; i++) if (write(fd, blk, BS) != BS) return 1; fsync(fd); t = now() - t; close(fd);
    printf("(b) write, kernel buffering   : %8.1f ms  (%6.0f MiB/s, %d syscalls + 1 fsync)\n", t * 1e3, 16 / t, NB);
    FILE *f = fopen(path, "wb"); setvbuf(f, NULL, _IOFBF, 1 << 20);
    t = now(); for (int i = 0; i < NB; i++) fwrite(blk, 1, BS, f); fflush(f); fsync(fileno(f)); t = now() - t; fclose(f);
    printf("(c) fwrite, 1 MiB stdio buffer: %8.1f ms  (%6.0f MiB/s, ~16 write syscalls + 1 fsync)\n", t * 1e3, 16 / t);
    unlink(path);
    return 0;
}
