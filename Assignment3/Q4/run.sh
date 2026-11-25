#!/usr/bin/env bash
# Memory beyond free RAM is NOT tested here (host has ~0 free RAM and a 97%-full disk, so it
# would swap onto a nearly full disk). On a Linux machine run, for example:
#   for m in 256 512 1024 2048; do systemd-run --user --scope -p MemoryMax=512M ./bin/touch $m seq; done
set -uo pipefail
cd "$(dirname "$0")"; mkdir -p bin outputs
CF="-std=c11 -Wall -Wextra -O2"
for f in vm_calc touch cow faults_vs_frames; do gcc $CF $f.c -o bin/$f || exit 1; done
./bin/vm_calc > outputs/01_eat_calculations.txt
{ echo "3 rounds per size, order alternated to remove first-run bias"
  for m in 64 256 512; do for k in 1 2 3; do
    if [ $k -eq 2 ]; then ./bin/touch $m rand; ./bin/touch $m seq; else ./bin/touch $m seq; ./bin/touch $m rand; fi
  done; done; } > outputs/02_touch_seq_vs_random.txt
./bin/cow > outputs/03_copy_on_write.txt
./bin/faults_vs_frames > outputs/04_faults_vs_frames.csv
python3 plot.py > /dev/null
