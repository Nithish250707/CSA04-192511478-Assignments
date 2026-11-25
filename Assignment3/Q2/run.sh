#!/usr/bin/env bash
set -uo pipefail
cd "$(dirname "$0")"; mkdir -p bin outputs
CF="-std=c11 -Wall -Wextra -O2"
for f in paging paging_2048 paging_boundaries contig_frag touch64; do gcc $CF $f.c -o bin/$f || exit 1; done
gcc $CF paging_calc.c -lm -o bin/paging_calc || exit 1
./bin/paging > outputs/01_paging.txt
{ diff expected_output.txt outputs/01_paging.txt && echo "IDENTICAL to expected output (diff exit 0)"; } > outputs/01_paging_diff.txt
./bin/paging_2048 > outputs/02_paging_2048.txt
./bin/paging_boundaries > outputs/03_boundaries.txt
./bin/paging_calc > outputs/04_calculations.txt
./bin/contig_frag > outputs/05_external_fragmentation.txt
{ echo "\$ getconf PAGESIZE   (Linux x86-64 normally prints 4096)"; getconf PAGESIZE
  echo "\$ sysctl hw.pagesize vm.pagesize"; sysctl hw.pagesize vm.pagesize 2>/dev/null
  echo; echo "\$ /usr/bin/time -l ./bin/touch64   (Linux: /usr/bin/time -v; 'page reclaims' = minor faults, 'page faults' = major faults)"
  /usr/bin/time -l ./bin/touch64 2>&1 | grep -E "touched|maximum resident|page reclaims|page faults"
  echo "expected minor faults if every page faults once: 64 MiB / $(getconf PAGESIZE) B = $(( (64<<20) / $(getconf PAGESIZE) ))"
} > outputs/06_real_system.txt 2>&1
