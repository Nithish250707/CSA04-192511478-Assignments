#!/usr/bin/env bash
set -uo pipefail
cd "$(dirname "$0")"; mkdir -p bin outputs
CF="-std=c11 -Wall -Wextra -O2"
gcc $CF buffering.c -o bin/buffering && gcc $CF replication.c -lm -o bin/replication || exit 1
./bin/buffering "${TMPDIR:-/tmp}/a4t5_buffer.bin" > outputs/01_buffering.txt
./bin/replication > outputs/02_replication.txt
