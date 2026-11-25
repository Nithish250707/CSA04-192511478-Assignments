/* Measured CPU cost of the parity calculation for one full stripe of 4 data chunks x 256 KiB:
   RAID 5 = P (XOR), RAID 6 = P (XOR) + Q (Reed-Solomon over GF(2^8), byte-wise). */
#define _POSIX_C_SOURCE 199309L
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define D 4
#define CH (256 * 1024)
static uint8_t data[D][CH], P[CH], Q[CH];
static double now(void) { struct timespec t; clock_gettime(CLOCK_MONOTONIC, &t); return t.tv_sec + t.tv_nsec / 1e9; }
static uint8_t gmul2(uint8_t x) { return (uint8_t)((x << 1) ^ ((x & 0x80) ? 0x1d : 0)); }
static void raid5(void) { for (int i = 0; i < CH; i++) { uint8_t p = 0; for (int d = 0; d < D; d++) p ^= data[d][i]; P[i] = p; } }
static void raid6(void)
{ for (int i = 0; i < CH; i++) { uint8_t p = 0, q = 0;
    for (int d = D - 1; d >= 0; d--) { p ^= data[d][i]; q = gmul2(q) ^ data[d][i]; }   /* Horner: Q = sum g^d * D_d */
    P[i] = p; Q[i] = q; } }
int main(void)
{
    srand(1); for (int d = 0; d < D; d++) for (int i = 0; i < CH; i++) data[d][i] = (uint8_t)rand();
    const int reps = 200; double b5 = 1e9, b6 = 1e9;
    for (int k = 0; k < 5; k++) {
        double t = now(); for (int r = 0; r < reps; r++) raid5(); t = (now() - t) / reps; if (t < b5) b5 = t;
        t = now(); for (int r = 0; r < reps; r++) raid6(); t = (now() - t) / reps; if (t < b6) b6 = t;
    }
    double mb = D * CH / 1048576.0;
    printf("stripe = %d x %d KiB = %.0f MiB of data (best of 5 x %d runs, plain C at -O2; clang auto-vectorises the loops)\n", D, CH / 1024, mb, reps);
    printf("RAID 5 parity (P)     : %7.3f ms per stripe  = %6.0f MiB/s\n", b5 * 1e3, mb / b5);
    printf("RAID 6 parity (P + Q) : %7.3f ms per stripe  = %6.0f MiB/s  (%.2fx RAID 5)\n", b6 * 1e3, mb / b6, b6 / b5);
    printf("check bytes: P[0]=%u Q[0]=%u\n", P[0], Q[0]);
    return 0;
}
