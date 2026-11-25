/* Effective access time with demand paging (memory 100 ns, page-fault service 8 ms). */
#include <stdio.h>
int main(void)
{
    const double ma = 100.0, fault = 8e6;   /* ns */
    const double rates[] = {1e-3, 1e-5, 1e-6};
    const char *txt[] = {"1 in 1,000", "1 in 100,000", "1 in 1,000,000"};
    puts("EAT = (1 - p) x 100 ns + p x 8,000,000 ns");
    for (int i = 0; i < 3; i++) {
        double p = rates[i], eat = (1 - p) * ma + p * fault;
        printf("p = %-15s EAT = %12.3f ns  (%.1fx slower than 100 ns)\n", txt[i], eat, eat / ma);
    }
    double pmax = (1.10 * ma - ma) / (fault - ma);
    printf("\n10%% slow-down: 110 >= 100 + p(8,000,000 - 100)  =>  p <= %.4e  = 1 fault in %.0f accesses\n", pmax, 1 / pmax);

    puts("\nFrame allocation example: 62 free frames, process P1 = 10 pages, P2 = 127 pages");
    printf("equal:        P1 %d, P2 %d\n", 62 / 2, 62 / 2);
    printf("proportional: P1 = 10/137 x 62 = %.2f -> %d,  P2 = 127/137 x 62 = %.2f -> %d\n",
           10.0 / 137 * 62, (int)(10.0 / 137 * 62), 127.0 / 137 * 62, (int)(127.0 / 137 * 62));
    return 0;
}
