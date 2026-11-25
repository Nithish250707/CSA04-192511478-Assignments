# Hand calculation with PAGE_SIZE = 2048 (done before running paging_2048.c)

Page table: page 0→5, 1→2, 2→–, 3→7, 4→0, 5→–, 6→3, 7→–. Valid logical range is 0 … 8×2048−1 = 16383.

| Logical | p = a / 2048 | d = a % 2048 | Frame | Result |
|---|---|---|---|---|
| 0    | 0 | 0    | 5 | 5×2048 + 0 = **10240** |
| 1500 | 0 | 1500 | 5 | 10240 + 1500 = **11740** |
| 3100 | 1 | 1052 | 2 | 4096 + 1052 = **5148** |
| 6200 | 3 | 56   | 7 | 14336 + 56 = **14392** |
| 9000 | 4 | 808  | 0 | 0 + 808 = **808** |
| 2049 | 1 | 1    | 2 | 4096 + 1 = **4097** |

With 2 KB pages, 9000 is no longer invalid (the address space doubles to 16 KB) and 2049 now falls in page 1, which is in memory, so there is no page fault.
Program output: `outputs/02_paging_2048.txt`. All six results match the program.

Note: the first hand attempt had 6200 → offset 8 (an arithmetic slip: 3 × 2048 = 6144, so 6200 − 6144 = 56). Comparing with the program caught it, and it is corrected above.
