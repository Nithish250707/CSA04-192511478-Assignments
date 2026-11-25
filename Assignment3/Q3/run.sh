#!/usr/bin/env bash
set -uo pipefail
cd "$(dirname "$0")"; mkdir -p bin outputs
CF="-std=c11 -Wall -Wextra -O2"
for f in segment segv seg_frag seg_tables; do gcc $CF $f.c -o bin/$f || exit 1; done
./bin/segment > outputs/01_translations.txt
{ for m in 1 2; do echo "\$ ./bin/segv $m"; ./bin/segv $m; echo "exit status: $?"; done
  echo "\$ ./bin/segv 3   (no handler installed)"; ./bin/segv 3; echo "exit status: $? (128 + signal number)"; } > outputs/02_segfault.txt 2>&1
./bin/seg_frag > outputs/03_fragmentation.txt
./bin/seg_tables > outputs/04_table_sizes.txt
{ ./bin/segment --wait > /dev/null & P=$!; sleep 1
  echo "Segments of a running process (Linux: cat /proc/$P/maps; macOS: vmmap $P)"
  vmmap $P 2>/dev/null | grep -E "^(__TEXT|__DATA|__DATA_CONST|__LINKEDIT|MALLOC_[A-Z]+ |Stack  )" | grep -E "segment|MALLOC|Stack" | head -14
  wait $P; } > outputs/05_process_segments.txt 2>&1
