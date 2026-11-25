#!/usr/bin/env bash
# B2 – run on a Linux machine with Docker Engine + Compose v2. Every command's output is
# saved to outputs/. NOT executed in this repo: the host (macOS 27) has no Docker.
set -uxo pipefail
cd "$(dirname "$0")"
O=outputs; mkdir -p "$O"
{ docker version; docker compose version; } > "$O/docker_version.txt" 2>&1

# 1. Build and run (Complete-the-Code Dockerfile)
( time docker build -t result-portal app ) > "$O/build_slim.txt" 2>&1
docker run -d --name portal -p 8000:8000 result-portal > "$O/run.txt" 2>&1
sleep 3
docker ps > "$O/docker_ps.txt" 2>&1
curl -s http://localhost:8000/health > "$O/curl_health.txt" 2>&1
curl -s http://localhost:8000/result/20260001 > "$O/curl_result.txt" 2>&1
docker exec portal whoami > "$O/whoami.txt" 2>&1          # expected: portal
docker rm -f portal

# 2. CPU / memory limits, observed with docker stats while ab loads the container
docker run -d --name portal-limited --cpus 0.5 --memory 256m -p 8001:8000 result-portal
sleep 3
ab -n 5000 -c 50 http://127.0.0.1:8001/result/20260001 > "$O/ab_limited.txt" 2>&1 &
sleep 4; docker stats --no-stream portal-limited > "$O/docker_stats_limited.txt" 2>&1; wait
docker inspect -f 'NanoCpus={{.HostConfig.NanoCpus}} Memory={{.HostConfig.Memory}}' portal-limited >> "$O/docker_stats_limited.txt"
docker rm -f portal-limited

# 3. Image layers and size comparison of the three Dockerfiles
docker history result-portal > "$O/docker_history_slim.txt" 2>&1
( time docker build -f app/Dockerfile.ubuntu -t result-portal:ubuntu app ) > "$O/build_ubuntu.txt" 2>&1
( time docker build -f app/Dockerfile.multistage -t result-portal:multistage app ) > "$O/build_multistage.txt" 2>&1
docker history result-portal:multistage > "$O/docker_history_multistage.txt" 2>&1
docker images result-portal > "$O/image_sizes.txt" 2>&1

# 4. Compose with a database volume; prove data survives down/up
docker compose up -d --build > "$O/compose_up.txt" 2>&1
sleep 8
{ docker compose ps; curl -s http://localhost:8000/health; echo; docker volume ls; } > "$O/compose_ps.txt" 2>&1
docker compose exec -T db psql -U portal -d results -c "SELECT COUNT(*) FROM results;" > "$O/compose_db_count.txt" 2>&1
docker compose down
docker compose up -d; sleep 8
docker compose exec -T db psql -U portal -d results -c "SELECT COUNT(*) FROM results;" >> "$O/compose_db_count.txt" 2>&1
docker compose down
