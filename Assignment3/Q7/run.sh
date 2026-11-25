#!/usr/bin/env bash
set -uo pipefail
cd "$(dirname "$0")"; mkdir -p bin outputs
CF="-std=c11 -Wall -Wextra -O2"
gcc $CF mlock_rt.c -o bin/mlock_rt && gcc $CF tasks_sim.c -o bin/tasks_sim || exit 1
{ echo "5 runs each, alternating (host: macOS $(sw_vers -productVersion), page size $(getconf PAGESIZE))"
  for k in 1 2 3 4 5; do ./bin/mlock_rt unlocked; ./bin/mlock_rt locked; done; } > outputs/01_mlock_latency.txt 2>&1
./bin/tasks_sim > outputs/02_tasks_simulation.txt 2>&1
