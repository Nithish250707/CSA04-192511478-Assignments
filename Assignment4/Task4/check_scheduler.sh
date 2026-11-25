#!/usr/bin/env bash
# Task 4.4 - which I/O scheduler does the disk use?
# Linux: cat /sys/block/<disk>/queue/scheduler  (e.g. "[none] mq-deadline" for NVMe)
#        lsblk -d -o NAME,ROTA,SCHED   (ROTA 0 = SSD)
if [ -d /sys/block ]; then
  for d in /sys/block/*/queue/scheduler; do echo "$d: $(cat "$d")"; done; lsblk -d -o NAME,ROTA,SCHED
else
  echo "Host $(uname -sr): there is no /sys/block on macOS and no user-selectable I/O scheduler."
  echo "The internal disk is an SSD, which is why reordering by cylinder brings nothing:"
  diskutil info disk0 | grep -E "Device / Media Name|Solid State|Protocol"
fi
