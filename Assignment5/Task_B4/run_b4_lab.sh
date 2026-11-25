#!/usr/bin/env bash
# B4.1 on a Linux machine with Docker Compose v2. NOT executed here (no Docker on this macOS host).
set -uxo pipefail
cd "$(dirname "$0")"; mkdir -p outputs
docker compose up -d --build > outputs/lab_compose_up.txt 2>&1
sleep 8
{ docker compose ps
  curl -s http://localhost:8000/health; echo
  curl -s http://localhost:8081/ | head -3
  docker stats --no-stream
  for c in $(docker compose ps -q); do
    docker inspect -f '{{.Name}} NanoCpus={{.HostConfig.NanoCpus}} Memory={{.HostConfig.Memory}}' "$c"
  done
} > outputs/lab_two_services.txt 2>&1
docker compose down
