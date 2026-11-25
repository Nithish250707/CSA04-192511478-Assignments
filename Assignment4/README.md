# Assignment 4 (CO4): Optimising File Storage and Disk Access

| | |
|---|---|
| **Name** | Nithish G |
| **Register number** | 192511478 |
| **Course** | CSA0421 – Operating Systems |

## Environment

All outputs were produced on **macOS, not Linux**. Linux-only commands are kept in the scripts, and the output shows which branch actually ran.

| Item | Value |
|---|---|
| Machine | MacBook Air, Apple Silicon (arm64), 16 GiB RAM, internal APFS SSD (no hard disk) |
| OS | macOS 27.0.1 (Darwin 27.0.0) |
| Compiler | Apple clang 21.0.0 (`gcc` on macOS is clang) |
| Compile command | `gcc -std=c11 -Wall -Wextra -O2 <file>.c -o bin/<file>` (add `-lm` for `sim`, `replication` and `video_sim`). All programs compile with no warnings. |

## How to run

```bash
cd Assignment4/Task1 && ./build_tree.sh     # Task 1
cd Assignment4/Task2 && ./run.sh            # Tasks 2, 4, 5 and 6 likewise
cd Assignment4/Task3 && ./inspect_fs.sh     # Task 3
```

Every result is saved in the task's `outputs/` folder.

## Results

### Task 1 – File-system design review
`build_tree.sh` builds the improved organisation in a scratch folder:
- private homes with mode 700 and a 2770 (setgid) project folder;
- an ACL giving a lecturer group read-only access (`chmod +a` on macOS, `setfacl` on Linux), with the built-in `admin` group standing in for lecturers;
- a symlink from a student's home to the shared project.

Listing a flat directory of 12,000 entries took 3.270 ms, against 0.012 ms for a student's own directory. Lookup by exact name stays fast because APFS uses B-trees, but names clash.

### Task 2 – File-allocation method
- **Complete the Code:** `alloc.c` output is **identical** to the expected output.
- E (10 blocks) does fit contiguously at block 19, as the expected output itself shows. The longest free run is 13 of 21 free blocks, so the external-fragmentation case is a file of 14 or more blocks. `fragmentation_demo` shows a 14-block file failing contiguously and succeeding with indexed allocation.
- `calc.c`: reaching the last block of a 4 GB file takes 1 read (contiguous), 1,048,576 reads (linked) or 3 reads (inode, double-indirect). The maximum inode file size is 4.004 TiB. For a 4 TB disk, the bit vector is 128 MiB and a FAT is 4 GiB.

### Task 3 – Modern file systems
`inspect_fs.sh` ran the APFS branch:
- inode numbers, 4096-byte allocation block, 70.9 M free inodes;
- no local snapshots present;
- **copy-on-write evidence:** copying a 256 MiB file took 163 ms and used 256 MiB, while an APFS clone (`cp -c`) took 37.5 ms and used no extra space.

`dumpe2fs`/`tune2fs` are in the Linux branch and were **not run**, because there is no ext4 here.

### Task 4 – Disk scheduling
- **Complete the Code:** `sstf.c` output is **identical** to the expected output.
- Given queue (`sim`): FCFS 640, SSTF 236, SCAN 236, C-SCAN 382 (including the 199-cylinder jump), LOOK 208, C-LOOK 322 (including the return jump).
- 10,000 random requests:
  - Heavy load: FCFS average wait was 3,792 ms (the queue grows without bound), while SSTF and LOOK stay at about 42–43 cylinders per request.
  - The fairness cost shows in maximum wait: SSTF's is 74.6 ms against LOOK's 38.8 ms.
- **Starvation:** the request at 183 waited **5,057 ms under SSTF**, against 15.3 ms under SCAN and 2.2 ms under LOOK.
- Scheduler check: macOS has no `/sys/block` and no selectable I/O scheduler, and the disk is an SSD. The Linux command is in `check_scheduler.sh`.

### Task 5 – Distributed storage and device management
- Buffering, 16 MiB written in 4 KiB writes (3 repeated runs):
  - `fsync` after every block: 97–128 ms
  - kernel buffering: 32–53 ms
  - 1 MiB stdio buffer: about 3.5 ms
- On macOS, `fsync` does not flush the drive cache (`F_FULLFSYNC` would), so these figures measure buffering cost, not durability.
- One-year failure simulation (12 nodes, MTBF 1 year, 24 h repair):
  - 1 copy: 99.83% block availability, with some data unavailable for 168 h;
  - 2 or 3 copies: 100%;
  - a single NFS server: everything unavailable for 72 h.
- Eventual consistency with a 50 ms mean replication lag: 16.5% of reads return stale data.

### Task 6 – Video-streaming storage design
- Measured on this SSD: a random 2 MiB uncached read takes **0.688 ms** on average (2,905 MiB/s).
- Library 2,304 GB; 2 copies plus 25% free space = 5,761 GB raw. The top 400 videos (464 GB) get 80.3% of views.
- Simulation (100 starts/s, 200,000 starts):

| | Design A: 4 HDDs | Design B: SSD (top 400) + 4 HDDs |
|---|---|---|
| Mean start latency | 55.3 ms | 9.5 ms (5.9 ms with the measured SSD) |
| p95 start latency | 142.6 ms | 25.5 ms |
| Busiest HDD | 81% | 13% |
| Devices for 2,000 streams (≤ 80% load) | 16 HDDs | 3 SSDs + 3 HDDs |

- HDD figures are typical datasheet values (8 ms seek, 7,200 rpm, 150 MB/s), because no hard disk is available. `fio_hdd_vs_ssd.fio` is **not run**, because `fio` is not installed.

## Differences from the report PDF

These numbers come from the programs in this repo. Where the report's simulation numbers differ, they came from earlier versions of the programs that are not in the repo. Update the report from these outputs:

| Item | Report | This repo (measured) |
|---|---|---|
| Task 4.2, e.g. heavy-load FCFS average wait | 4,320.8 ms | 3,792.07 ms (full table in `Task4/outputs/02_sim.txt`) |
| Task 4.3 starvation, SSTF wait for 183 | 5,059 ms | 5,057.0 ms |
| Task 4.3 starvation, SCAN / LOOK | 9.9 / 2.6 ms | 15.3 / 2.2 ms |
| Task 6, Design A mean start latency | 70.6 ms | 55.3 ms |
| Task 6, Design A devices | 13 HDDs | 16 HDDs (at ≤ 80% load) |
| Task 6, Design B mean start latency | 10.7 ms | 9.5 ms |

## Screenshots

To be added by the student.
