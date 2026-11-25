#!/usr/bin/env bash
# B1.1 Observe the system: processes, memory, disk and network.
# Uses the Linux tools named in the assignment (ps, top, free, vmstat, iostat, df, ss).
# When a Linux tool does not exist (e.g. on macOS), the nearest equivalent is used and
# the command that actually ran is printed, so the output never pretends to be Linux.
set -uo pipefail

have() { command -v "$1" >/dev/null 2>&1; }
section() { printf '\n===== %s =====\n' "$1"; }
run() { echo "\$ $*"; "$@"; }

section "Host"
run uname -a
if have lsb_release; then run lsb_release -a; elif [ -r /etc/os-release ]; then run cat /etc/os-release; elif have sw_vers; then run sw_vers; fi
run uptime

section "Processes (top 10 by CPU)"
if ps --version >/dev/null 2>&1; then          # procps (Linux)
  echo "\$ ps -eo pid,user,%cpu,%mem,rss,stat,comm --sort=-%cpu | head -11"
  ps -eo pid,user,%cpu,%mem,rss,stat,comm --sort=-%cpu | head -11
else                                            # BSD ps (macOS)
  echo "\$ ps -Ao pid,user,%cpu,%mem,rss,stat,comm -r | head -11"
  ps -Ao pid,user,%cpu,%mem,rss,stat,comm -r | head -11
fi

section "top snapshot"
if top -v 2>&1 | grep -qi procps; then
  echo "\$ top -b -n 1 | head -15"; top -b -n 1 | head -15
else
  echo "\$ top -l 2 -n 10 -o cpu -stats pid,command,cpu,mem,state | tail -22"; top -l 2 -n 10 -o cpu -stats pid,command,cpu,mem,state | tail -22
fi

section "Memory"
if have free; then run free -h
else
  echo "free: not available on this OS; using sysctl hw.memsize + vm_stat"
  run sysctl hw.memsize
  run vm_stat
  run sysctl vm.swapusage
fi

section "vmstat (3 samples, 1 s apart)"
if have vmstat; then run vmstat 1 3
else echo "vmstat: not available on this OS; see vm_stat above"; fi

section "iostat (3 samples, 1 s apart)"
if have iostat; then
  if iostat -V >/dev/null 2>&1; then run iostat -xz 1 3     # sysstat (Linux)
  else run iostat -c 3 -w 1; fi                           # BSD iostat (macOS)
else echo "iostat: not installed (Linux: apt install sysstat)"; fi

section "Disk usage"
run df -h

section "Listening sockets"
if have ss; then run ss -tuln
else
  echo "ss: not available on this OS; using netstat"
  echo "\$ netstat -an -p tcp | grep LISTEN"; netstat -an -p tcp | grep LISTEN
fi
