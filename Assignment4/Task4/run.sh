#!/usr/bin/env bash
set -uo pipefail
cd "$(dirname "$0")"; mkdir -p bin outputs
CF="-std=c11 -Wall -Wextra -O2"
gcc $CF sstf.c -o bin/sstf && gcc $CF sim.c -lm -o bin/sim || exit 1
./bin/sstf > outputs/01_sstf.txt
{ diff expected_output.txt outputs/01_sstf.txt && echo "IDENTICAL to expected output (diff exit 0)"; } > outputs/01_sstf_diff.txt
./bin/sim > outputs/02_sim.txt
bash check_scheduler.sh > outputs/03_scheduler.txt 2>&1
