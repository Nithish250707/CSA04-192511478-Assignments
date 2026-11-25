#!/usr/bin/env bash
# Linux equivalents for the inspection step: lsblk; sudo fdisk -l; swapon --show; free -h
set -uo pipefail
cd "$(dirname "$0")"; mkdir -p bin outputs
CF="-std=c11 -Wall -Wextra -O2"
gcc $CF raid_calc.c -lm -o bin/raid_calc && gcc $CF parity_bench.c -o bin/parity_bench && gcc $CF read_speed.c -o bin/read_speed || exit 1
{ echo "Host: $(sw_vers -productName) $(sw_vers -productVersion) - Linux lsblk/fdisk/swapon do not exist here; macOS equivalents:"
  echo "\$ diskutil list"; diskutil list
  echo "\$ df -h /"; df -h /
  echo "\$ sysctl vm.swapusage   (macOS uses swap FILES on the APFS VM volume, not a swap partition)"; sysctl vm.swapusage
  echo "\$ ls -lh /System/Volumes/VM"; ls -lh /System/Volumes/VM 2>&1
  echo "\$ diskutil info disk0 | grep -E 'Device / Media|Solid State|Protocol|Disk Size|Block Size'"
  diskutil info disk0 | grep -E "Device / Media Name|Solid State|Protocol|Disk Size|Device Block Size"
} > outputs/01_partitions_and_swap.txt 2>&1
./bin/raid_calc > outputs/02_raid_calculations.txt
./bin/parity_bench > outputs/03_parity_cost.txt
F="${TMPDIR:-/tmp}/a3q6_readtest.bin"
{ echo "Internal SSD (APFS, $(diskutil info disk0 | awk -F: '/Device \/ Media Name/{gsub(/^ +/,"",$2);print $2}')):"
  ./bin/read_speed "$F"
  echo "Hard disk: not available on this machine (MacBook Air has only an SSD) - not measured."; } > outputs/04_read_speed.txt 2>&1
rm -f "$F"
