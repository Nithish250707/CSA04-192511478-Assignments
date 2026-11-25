#!/usr/bin/env bash
# B4.2 helper – how many CPU-seconds does the portal spend per request?
# Needed because the B3 rps was measured with no CPU limit, while a Kubernetes pod is
# limited to 500m CPU. Measures CPU time of gunicorn (master + 2 workers) across one ab run.
set -euo pipefail
cd "$(dirname "$0")"
APP=$PWD/../Task_B2/app; VENV=$PWD/../Task_B3/.venv; PORT=8040; DB=$TMPDIR/b4_portal.db; N=20000
cpu_s() { # summed CPU time (user+sys) of a process tree, in seconds; ps TIME is [[dd-]hh:]mm:ss.ss
  local pids; pids="$1 $(pgrep -P "$1" | tr '\n' ' ')"
  ps -o time= -p "${pids// /,}" | awk '{n=split($1,a,":"); s=0; for(i=1;i<=n;i++) s=s*60+a[i]; t+=s} END {printf "%.2f", t}'
}
rm -f "$DB"
( cd "$APP" && SQLITE_PATH=$DB exec "$VENV/bin/gunicorn" --workers 2 --bind 127.0.0.1:$PORT app:app ) >/dev/null 2>&1 &
G=$!
until curl -sf -o /dev/null http://127.0.0.1:$PORT/health; do python3 -c 'import time; time.sleep(0.05)'; done
curl -s -o /dev/null http://127.0.0.1:$PORT/result/20260001
c0=$(cpu_s $G)
ab -q -n $N -c 50 http://127.0.0.1:$PORT/result/20260001 > outputs/ab_cpu_run.txt 2>&1
c1=$(cpu_s $G)
kill $G; wait $G 2>/dev/null || true; rm -f "$DB"
echo "requests: $N"
grep -E "Failed requests|Requests per second" outputs/ab_cpu_run.txt
echo "gunicorn CPU time before: ${c0}s  after: ${c1}s"
awk -v a="$c0" -v b="$c1" -v n="$N" 'BEGIN {d=b-a; printf "CPU-seconds used: %.2f\nCPU-ms per request: %.3f\nrequests per CPU-second: %.1f\n", d, d*1000/n, n/d}'
