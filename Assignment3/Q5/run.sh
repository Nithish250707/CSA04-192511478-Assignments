#!/usr/bin/env bash
set -uo pipefail
cd "$(dirname "$0")"; mkdir -p bin outputs
CF="-std=c11 -Wall -Wextra -O2"
gcc $CF replace.c -o bin/replace && gcc $CF experiments.c -o bin/experiments || exit 1
./bin/replace > outputs/01_replace.txt
{ diff expected_output.txt outputs/01_replace.txt && echo "IDENTICAL to expected output (diff exit 0)"; } > outputs/01_replace_diff.txt
./bin/experiments --trace > outputs/02_lru_trace.txt
./bin/experiments --csv > outputs/03_faults_3_to_10_frames.csv
./bin/experiments > outputs/05_running_time.txt
python3 plot.py > /dev/null
