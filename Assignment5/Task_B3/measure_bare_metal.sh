#!/usr/bin/env bash
# B3.2 – measure the portal running directly on the host OS ("bare metal"):
# start-up time, memory (RSS), disk size and requests per second with ab.
# Uses the same app and the same gunicorn command (2 sync workers) as the container image.
# Usage: ./measure_bare_metal.sh   (expects .venv created from ../Task_B2/app/requirements.txt)
set -euo pipefail
cd "$(dirname "$0")"
HERE=$PWD; APP=$HERE/../Task_B2/app; VENV=$HERE/.venv; PORT=8020
URL=http://127.0.0.1:$PORT
now_ms() { python3 -c 'import time; print(int(time.time()*1000))'; }
rss_kib() { # total RSS of the gunicorn master and its workers
  local pids; pids="$1 $(pgrep -P "$1" | tr '\n' ' ')"
  ps -o rss= -p "${pids// /,}" | awk '{s+=$1} END {print s}'
}
start_portal() {
  rm -f "$DB"
  ( cd "$APP" && SQLITE_PATH=$DB exec "$VENV/bin/gunicorn" --workers 2 --bind 127.0.0.1:$PORT app:app ) \
    > "$TMPDIR/b3_gunicorn.log" 2>&1 &
  GPID=$!
}
DB=$TMPDIR/b3_portal.db

echo "== Host"; uname -srm; (sw_vers 2>/dev/null || cat /etc/os-release) | tr '\n' ' '; echo
echo "CPU cores: $(sysctl -n hw.ncpu 2>/dev/null || nproc)   RAM: $(( $(sysctl -n hw.memsize 2>/dev/null || echo 0) / 1024 / 1024 )) MiB"
echo "python: $("$VENV/bin/python" --version 2>&1)   gunicorn: $("$VENV/bin/gunicorn" --version 2>&1)   $(ab -V | head -1)"

echo; echo "== Start-up time (launch -> first HTTP 200 on /health), 5 cold starts, empty DB each time"
for run in 1 2 3 4 5; do
  t0=$(now_ms); start_portal
  until curl -s -o /dev/null -f "$URL/health"; do python3 -c 'import time; time.sleep(0.01)'; done
  t1=$(now_ms); echo "run $run: $((t1 - t0)) ms"
  kill "$GPID"; wait "$GPID" 2>/dev/null || true
done

echo; echo "== Disk size"
du -sk "$APP/app.py" "$APP/requirements.txt" "$VENV" | awk '{print $1 " KiB\t" $2; s+=$1} END {print s " KiB\ttotal (app + virtualenv; the system Python itself is not counted)"}'

start_portal
until curl -s -o /dev/null -f "$URL/health"; do python3 -c 'import time; time.sleep(0.05)'; done
curl -s -o /dev/null "$URL/result/20260001"
sleep 1
echo; echo "== Memory (RSS of gunicorn master + 2 workers)"
echo "idle: $(rss_kib "$GPID") KiB"

echo; echo "== Requests per second: ab -n 10000 -c 50 $URL/result/20260001 (3 runs)"
for run in 1 2 3; do
  ab -q -n 10000 -c 50 "$URL/result/20260001" > "$HERE/outputs/ab_bare_metal_run$run.txt" 2>&1
  grep -E "Requests per second|Failed requests|Time per request:.*\(mean\)$|Complete requests" "$HERE/outputs/ab_bare_metal_run$run.txt" | sed "s/^/run $run: /"
done
echo "after load: $(rss_kib "$GPID") KiB RSS"

kill "$GPID"; wait "$GPID" 2>/dev/null || true
rm -f "$DB"
