# Hand translation of 8 (segment, offset) addresses (done before running `segment`)

| # | (s, d) | Access | Check d < limit | Result |
|---|---|---|---|---|
| 1 | (0, 430) | read  | 430 < 600 ✓ | 219 + 430 = **649** |
| 2 | (1, 10)  | write | 10 < 14 ✓, data is rw | 2300 + 10 = **2310** |
| 3 | (2, 500) | read  | 500 ≥ 100 ✗ | **trap** |
| 4 | (3, 400) | write | 400 < 580 ✓, stack is rw | 1327 + 400 = **1727** |
| 5 | (4, 112) | read  | 112 ≥ 96 ✗ | **trap** |
| 6 | (0, 599) | read  | 599 < 600 ✓ (last byte) | 219 + 599 = **818** |
| 7 | (1, 14)  | read  | 14 ≥ 14 ✗ (one past the end) | **trap** |
| 8 | (3, 0)   | write | 0 < 580 ✓ | 1327 + 0 = **1327** |

Program output: `outputs/01_translations.txt`. All 8 match.
