#!/usr/bin/env bash
# Task 3.4 - inspect a real file system: inodes, block size, free space.
# Linux (ext4): stat, stat -f, df -i, df -h, ls -i, sudo dumpe2fs -h / tune2fs -l.
# macOS (APFS): BSD stat, df -i, df -h, ls -i, diskutil info, APFS snapshots, and a
# copy-on-write clone (cp -c) as evidence of CoW. Each command is printed before its output.
set -uo pipefail
run() { echo "\$ $*"; "$@"; echo; }
D="${TMPDIR:-/tmp}/a4t3"; mkdir -p "$D"; echo "hello inode" > "$D/sample.txt"
echo "Host: $(uname -sr)"; echo
if [ "$(uname)" = Linux ]; then
  run stat "$D/sample.txt"
  run stat -f "$D"
  run df -i "$D"
  run df -h "$D"
  run ls -i "$D/sample.txt"
  DEV=$(df "$D" | awk 'NR==2{print $1}')
  echo "\$ sudo dumpe2fs -h $DEV | grep -E 'Block size|Inode count|Free blocks|Free inodes|Journal|features'"
  sudo dumpe2fs -h "$DEV" 2>/dev/null | grep -E "Block size|Inode count|Inode size|Free blocks|Free inodes|Journal|features"
else
  run stat -x "$D/sample.txt"
  echo "\$ stat -f 'inode %i  size %z B  blocks %b x 512 B  optimal I/O block %k B' $D/sample.txt"
  stat -f 'inode %i  size %z B  blocks %b x 512 B  optimal I/O block %k B' "$D/sample.txt"; echo
  run df -i "$D"
  run df -h "$D"
  run ls -i "$D/sample.txt"
  echo "\$ diskutil info / | grep -E 'File System Personality|Block Size|Container Total|Container Free|Volume Free|Encrypted'"
  diskutil info / | grep -E "File System Personality|Block Size|Container Total Space|Container Free Space|Volume Free Space|FileVault|Encrypted"; echo
  echo "\$ tmutil listlocalsnapshots /   (APFS snapshots)"; tmutil listlocalsnapshots / 2>&1; echo
  echo "== Copy-on-write evidence: full copy vs APFS clone of a 256 MiB file"
  dd if=/dev/zero of="$D/big.bin" bs=1m count=256 2>/dev/null; sync
  free0=$(df -k "$D" | awk 'NR==2{print $4}')
  s=$(python3 -c 'import time;print(time.perf_counter())'); cp "$D/big.bin" "$D/copy.bin"; sync; e=$(python3 -c 'import time;print(time.perf_counter())')
  free1=$(df -k "$D" | awk 'NR==2{print $4}')
  s2=$(python3 -c 'import time;print(time.perf_counter())'); cp -c "$D/big.bin" "$D/clone.bin"; sync; e2=$(python3 -c 'import time;print(time.perf_counter())')
  free2=$(df -k "$D" | awk 'NR==2{print $4}')
  python3 -c "print('cp    (full copy): %.1f ms, free space change %+d MiB' % (($e-$s)*1e3, ($free1-$free0)//1024))"
  python3 -c "print('cp -c (CoW clone): %.1f ms, free space change %+d MiB' % (($e2-$s2)*1e3, ($free2-$free1)//1024))"
  echo "(df free space also moves with other activity on the disk; the clone shares blocks until either file is written)"
  ls -li "$D"/big.bin "$D"/copy.bin "$D"/clone.bin
fi
rm -rf "$D"
