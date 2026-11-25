# Assignment 3 (CO3): Memory Management

| | |
|---|---|
| **Name** | Nithish G |
| **Register number** | 192511478 |
| **Course** | CSA0421 – Operating Systems |

## Environment

All outputs were produced on **macOS, not Linux**. Where the brief names a Linux-only tool, the macOS equivalent was used, and each output file shows the command that actually ran.

| Item | Value |
|---|---|
| Machine | MacBook Air, Apple Silicon (arm64), 10 cores, 16 GiB RAM, internal SSD |
| OS | macOS 27.0.1 (Darwin 27.0.0), **page size 16 KB** |
| Compiler | Apple clang 21.0.0 (`gcc` on macOS is clang) |
| Compile command | `gcc -std=c11 -Wall -Wextra -O2 <file>.c -o bin/<file>` (add `-lm` for `paging_calc` and `raid_calc`) |
| Plots | Python 3.13 + matplotlib (`plot.py` in Q4 and Q5) |

| Linux tool in the brief | Used here instead |
|---|---|
| `ldd` | `otool -L` |
| `/proc/self/maps`, `/proc/<pid>/maps` | `dladdr()` in the program, plus `vmmap <pid>` |
| `/usr/bin/time -v` | `/usr/bin/time -l` (minor faults are reported as "page reclaims") |
| `lsblk`, `fdisk -l`, `swapon` | `diskutil list`, `sysctl vm.swapusage` |
| `O_DIRECT` | `F_NOCACHE` |

## How to run

Every question folder has a `run.sh` that compiles the programs into `bin/` and writes every result to `outputs/`:

```bash
cd Assignment3/Q1 && ./run.sh     # likewise for Q2 ... Q7
```

## Results

### Q1 – Linking, loading and address binding
Files: `bank.c`, `bankcrypt_v1.c`, `bankcrypt_v2.c`, `dlopen_demo.c`. Outputs: `outputs/01`–`08`.
- **Fully static linking (`-static`) is not supported on macOS.** The linker error (`crt0.o not found`) is recorded. The "static" build links the crypto library statically from `libbankcrypt.a`, while libSystem stays dynamic.
- Sizes are 50,568 B (static library) vs 50,544 B (dynamic) plus a 33,424 B `.dylib`. `otool -L` lists `@rpath/libbankcrypt.dylib` only for the dynamic build.
- Start-up over 500 runs: 1.672 ms static vs 1.742 ms dynamic. Max RSS: 1,703,936 B vs 1,736,704 B.
- Replacing the `.dylib` with v1.1 changed the dynamic program's output without rebuilding it (checksum unchanged). The static program still runs v1.0.
- In two running copies, `libsystem_c` text sits at the **same address** in both (the shared cache, which is shared pages). Each copy's own text is marked `SM=COW`.
- With the library missing, `bank_dynamic` is killed by dyld ("Library not loaded", exit 134), `bank_static` still runs, and `dlopen_demo` reports the error and keeps running (exit 2).

### Q2 – Paging
- **Complete the Code:** `paging.c` output is **identical** to the expected output (`outputs/01_paging_diff.txt`).
- PAGE_SIZE 2048: the hand calculation is in `hand_calc_2048.md`. My first attempt had one arithmetic slip, which comparing with the program caught. The corrected values match the program: 10240, 11740, 5148, 14392, 808, 4097.
- Boundary tests: 1023→6143 and 1024→2048; 7168 is a page fault (page 7 not in memory); 8192 and above are invalid.
- Page-table size: 32-bit is 4 MiB. 48-bit with a flat table would be 512 GiB, while 4 levels for 1 GiB used need only 515 tables (2,060 KiB).
- Effective access time (100 ns memory, 20 ns TLB, which are assumed values): 200 ns without a TLB; 140 / 125 / 121 ns at 80 / 95 / 99% hit rates.
- Internal fragmentation for 100 random processes: 1 KB pages waste 0.012%, 4 KB 0.050%, 2 MB **26.1%**.
- Real system: `getconf PAGESIZE` = **16384**. Touching 64 MiB caused 4,373 minor faults (4,096 pages plus start-up).

### Q3 – Segmentation
- 8 addresses were translated by hand (`hand_translations.md`) and all match the program: 649, 2310, trap, 1727, trap, 818, trap, 1327.
- A real fault: writing to a read-only literal gave **SIGBUS** (macOS/arm64 reports it this way), and NULL gave **SIGSEGV**. Exit status was 139 with no handler.
- 20 random segments in 10,000 units: after removing 10, 4,820 units were free but the largest hole was 894, so **81.5% external fragmentation**. 17 of 20 new segments did not fit.
- Table sizes for a 75 MB process: flat paging 4 MB, two-level 92 KB, segmentation 32 B, segmented paging 75 KB.

### Q4 – Virtual memory and demand paging
- Effective access time: 8,099.9 ns at 1 fault in 1,000, 180.0 ns at 1 in 100,000 and 108.0 ns at 1 in 1,000,000. A 10% slow-down allows at most **1 fault in 799,990 accesses**.
- Sequential vs random first touch: both cause exactly one fault per page (4,096 / 16,384 / 32,768 faults for 64 / 256 / 512 MiB). Times are about 0.7 µs per page; the first run of each size is an outlier.
- Copy-on-write: `fork()` of a 128 MiB process took 2.5 ms. Writing 64 MiB in the child caused **4,096 faults, one per page written**. The parent's data was unchanged.
- Faults vs frames are plotted in `outputs/04_faults_vs_frames.png`.
- **Not executed:** run time beyond free RAM. This Mac has little free RAM and a 97% full disk, so swapping was unsafe here. The Linux memory-limit command is in `run.sh`.

### Q5 – Page-replacement techniques
- **Complete the Code:** `replace.c` (LRU TODO) output is **identical** to the expected output: 15/12/9, 10/8/8, and Belady 9 → 10.
- The LRU hand trace (`hand_trace_lru.md`, 12 faults) matches the program trace step by step.
- 100,000 references, 8 frames, locality workload: FIFO 17,349, LRU 10,086, CLOCK 12,733 and OPT 7,675 faults. On random references, FIFO, LRU and CLOCK are within 0.2% of each other. Plots are in `outputs/04_plot_*.png`.
- Running time per reference: FIFO 3.6–4.7 ns, LRU 5.3–5.8 ns, CLOCK 5.9–6.1 ns. OPT is 5–6 ns, but only because it uses a precomputed next-use array, which needs the future reference string and so can't run in a real OS.

### Q6 – Disk management
- This Mac has a GPT disk with APFS containers and **swap files** on the APFS VM volume, not a swap partition. 0 MB of swap was in use.
- 6 × 4 TB usable capacity: RAID 0 24 TB, RAID 1/10 12 TB, RAID 5 20 TB, RAID 6 16 TB.
- Loss during an 11.1 h rebuild (2% annual failure rate, which is assumed): RAID 1/10 2.6e-5, RAID 5 1.3e-4, RAID 6 6.6e-9. A RAID 5 rebuild reads 20 TB, giving a **79.8%** chance of an unrecoverable read error on consumer disks (14.8% on enterprise disks).
- Measured parity CPU cost: RAID 6 (P+Q) is **1.6×** RAID 5 (P).
- SSD read with the cache bypassed was about **3,000 MiB/s**; from the page cache it was about 18,800 MiB/s. **No hard disk is available**, so HDD speed was not measured.

### Q7 – Memory-management plan for the edge device
- Control loop over 16 MiB, 5 runs each:
  - **unlocked:** 1,024 page faults, worst-case delay **12–66 µs**
  - **`mlock` + pre-touch:** 0 faults, worst case **≤ 84 ns**
- One unlocked run also had a 23 µs delay with no fault (preemption by the scheduler). Locking memory does not remove that, so it is the remaining limitation.
- Global-LRU simulation (2 GB RAM, no swap): with the real-time task unlocked, its rarely used emergency-stop path re-faulted all 16 pages on every use (112 re-faults); locked, 0. Adding a 500 MB fourth task raises ML faults from 2.2% to 17.7% of references.

## Screenshots

To be added by the student.
