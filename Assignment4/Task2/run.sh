#!/usr/bin/env bash
set -uo pipefail
cd "$(dirname "$0")"; mkdir -p bin outputs
CF="-std=c11 -Wall -Wextra -O2"
for f in alloc calc fragmentation_demo; do gcc $CF $f.c -o bin/$f || exit 1; done
./bin/alloc > outputs/01_alloc.txt
{ diff expected_output.txt outputs/01_alloc.txt && echo "IDENTICAL to expected output (diff exit 0)"; } > outputs/01_alloc_diff.txt
./bin/fragmentation_demo > outputs/02_fragmentation_14_blocks.txt
./bin/calc > outputs/03_calc.txt
