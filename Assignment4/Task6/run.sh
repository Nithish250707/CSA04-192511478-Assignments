#!/usr/bin/env bash
set -uo pipefail
cd "$(dirname "$0")"; mkdir -p bin outputs
CF="-std=c11 -Wall -Wextra -O2"
gcc $CF video_sim.c -lm -o bin/video_sim && gcc $CF measure_ssd.c -o bin/measure_ssd || exit 1
./bin/measure_ssd "${TMPDIR:-/tmp}/a4t6_ssd.bin" | tee outputs/01_measured_ssd.txt
./bin/video_sim > outputs/02_video_sim_assumed.txt
MS=$(awk '/mean/{for(i=1;i<=NF;i++) if($i=="mean"){print $(i+1); exit}}' outputs/01_measured_ssd.txt)
./bin/video_sim "$MS" > outputs/03_video_sim_measured_ssd.txt
command -v fio >/dev/null || echo "fio not installed - fio_hdd_vs_ssd.fio not run" > outputs/04_fio_NOT_RUN.txt
