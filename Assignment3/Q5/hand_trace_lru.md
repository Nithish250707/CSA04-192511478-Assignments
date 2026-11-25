# Hand calculation: LRU, 3 frames, reference string 7 0 1 2 0 3 0 4 2 3 0 3 2 1 2 0 1 7 0 1

"Evict" names the page whose last use is oldest at that moment.

| # | Ref | Result | Frames after | Evicted (last used at) |
|---|---|---|---|---|
| 1 | 7 | fault | 7 | – |
| 2 | 0 | fault | 7 0 | – |
| 3 | 1 | fault | 7 0 1 | – |
| 4 | 2 | fault | 2 0 1 | 7 (ref 1) |
| 5 | 0 | hit | 2 0 1 | |
| 6 | 3 | fault | 2 0 3 | 1 (ref 3) |
| 7 | 0 | hit | 2 0 3 | |
| 8 | 4 | fault | 4 0 3 | 2 (ref 4) |
| 9 | 2 | fault | 4 0 2 | 3 (ref 6) |
| 10 | 3 | fault | 4 3 2 | 0 (ref 7) |
| 11 | 0 | fault | 0 3 2 | 4 (ref 8) |
| 12 | 3 | hit | 0 3 2 | |
| 13 | 2 | hit | 0 3 2 | |
| 14 | 1 | fault | 1 3 2 | 0 (ref 11) |
| 15 | 2 | hit | 1 3 2 | |
| 16 | 0 | fault | 1 0 2 | 3 (ref 12) |
| 17 | 1 | hit | 1 0 2 | |
| 18 | 7 | fault | 1 0 7 | 2 (ref 15) |
| 19 | 0 | hit | 1 0 7 | |
| 20 | 1 | hit | 1 0 7 | |

**Total: 12 faults.** This matches `outputs/01_replace.txt` (LRU 12) and the step-by-step program trace in `outputs/02_lru_trace.txt`.
