#!/usr/bin/env bash
# Disk-space alert: warn when any mounted disk is at or above THRESHOLD percent used.
# Usage: disk_alert.sh [threshold_percent]   (default 80)
# Exit status: 0 = all below threshold, 1 = at least one alert.
set -euo pipefail

threshold=${1:-80}
alerts=0
echo "Disk check at $(date '+%Y-%m-%d %H:%M:%S') (threshold ${threshold}%)"

# df -P gives one POSIX-format line per filesystem on both Linux and macOS.
# Only real block devices (/dev/...) are checked; tmpfs, devfs etc. are skipped.
while read -r fs size used avail pct mount; do
  use=${pct%\%}
  if [ "$use" -ge "$threshold" ]; then
    echo "ALERT: $mount ($fs) is ${use}% full, ${avail} KiB free"
    alerts=$((alerts + 1))
  else
    echo "OK:    $mount ($fs) is ${use}% full"
  fi
done < <(df -Pk | awk 'NR > 1 && $1 ~ /^\/dev\// {print $1, $2, $3, $4, $5, $6}')

if [ "$alerts" -gt 0 ]; then
  echo "$alerts filesystem(s) at or above ${threshold}%"
  # On the server this line would also send mail/Slack, e.g.
  #   echo "..." | mail -s "Disk alert on $(hostname)" admin@example.edu
  exit 1
fi
echo "All filesystems below ${threshold}%"
