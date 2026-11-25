# Assignment 5 (CO5): Linux, Containers and Virtualisation

| | |
|---|---|
| **Name** | Nithish G |
| **Register number** | 192511478 |
| **Course** | CSA0421 – Operating Systems |
| **Case study** | Deploying the University Result Portal |

## Environment (where the outputs in this folder came from)

All outputs committed here were produced on **macOS, not Linux**. No Linux distribution was used for these runs. Steps that need Linux, Docker, a hypervisor or Kubernetes are listed as lab scripts and marked **not executed**.

| Item | Version |
|---|---|
| Machine | MacBook Air, Apple Silicon (arm64), 10 cores, 16 GiB RAM |
| OS | macOS 27.0.1 (build 26A434), Darwin kernel 27.0.0 |
| Shell | GNU bash 3.2.57 (`/bin/bash`) |
| awk | BSD awk 20200816 |
| Python | 3.13.2 (virtualenv: Flask 3.1.2, gunicorn 23.0.0, psycopg 3.2.10) |
| Load generator | ApacheBench 2.3 (`ab`) |
| curl / git | curl 8.7.1, git 2.54.0 |
| C compiler | Not used. This assignment has no C programs. |
| Docker, Docker Compose, minikube, kind, kubectl, VirtualBox, wrk | **Not installed**, so those steps were not run |

The demo database holds 1,000 **made-up** students (register numbers 20260000–20260999) with random grades. These are not real results.

## Folder layout

```
Assignment5/
  Task_B1/  log_summary.sh, observe.sh, disk_alert.sh, crontab.txt, linux_lab/ (systemd, Nginx, ufw), outputs/
  Task_B2/  app/ (app.py, requirements.txt, Dockerfile, Dockerfile.ubuntu, Dockerfile.multistage), docker-compose.yml, run_b2.sh, outputs/
  Task_B3/  measure_bare_metal.sh, deployment.yaml, hpa.yaml, run_b3_lab.sh, results_table.md, outputs/
  Task_B4/  measure_cpu_per_request.sh, capacity.py, docker-compose.yml, lms/, run_b4_lab.sh, outputs/
```

## Task B1: Administer the Linux server

**Commands**
```bash
cd Assignment5/Task_B1
bash log_summary.sh > outputs/log_summary_output.txt
diff expected_output.txt outputs/log_summary_output.txt     # exit 0 = identical
bash observe.sh  > outputs/observe_output.txt
bash disk_alert.sh 80 ; bash disk_alert.sh 10                # default threshold, and a low one to force alerts
crontab crontab.txt && crontab -l                            # hourly schedule
```

**Complete the Code:** TODO 1 is `awk '{print $7}' | sort -n | uniq -c | awk '{printf "  %s %s\n", $2, $1}'`. TODO 2 counts field 1 the same way, sorts by count in descending order and prints the first line.

**Result**
- `log_summary.sh` output is **identical** to the expected output (`200 4 / 302 1 / 403 1 / 404 1`, busiest `10.0.0.5 4`, total bytes `43200`). The `diff` is empty (`outputs/log_summary_diff.txt`).
- `observe.sh` ran `ps`, `top`, `iostat` and `df`. macOS has no `free`, `vmstat` or `ss`, so the script used `vm_stat`/`sysctl` and `netstat` instead and prints the command that actually ran (`outputs/observe_output.txt`).
- `disk_alert.sh` at 80% raised **one real alert**: `/System/Volumes/Data` was 97% full (about 7 GB free). Exit status was 1. At a 10% threshold it raised 3 alerts (`outputs/disk_alert_*.txt`).
- Crontab installed with `log_summary.sh` at minute 0 of every hour and `disk_alert.sh 80` at minute 5 (`outputs/crontab_l.txt`).
- **Not executed:** B1.3 systemd service, Nginx reverse proxy and ufw firewall. macOS has none of these. The files and the exact Ubuntu commands are in `linux_lab/`.

## Task B2: Containerise the portal

**Commands (Linux + Docker; script: `run_b2.sh`)**
```bash
cd Assignment5/Task_B2
docker build -t result-portal app
docker run -p 8000:8000 result-portal
docker exec <container> whoami                               # expected: portal
docker run --cpus 0.5 --memory 256m ... ; docker stats --no-stream
docker history result-portal
docker build -f app/Dockerfile.ubuntu -t result-portal:ubuntu app
docker build -f app/Dockerfile.multistage -t result-portal:multistage app
docker images result-portal
docker compose up -d                                         # portal + postgres with volume pgdata
```

**Complete the Code:** TODO 1 is `WORKDIR /app`. TODO 2 is `COPY app.py .`, then `useradd portal`, `chown` and `USER portal`. TODO 3 is `EXPOSE 8000` and `CMD ["gunicorn", ..., "--bind", "0.0.0.0:8000", "app:app"]`.

**Result**
- Executed here: the app was smoke-tested with gunicorn outside Docker. `/health` and `/result/20260001` returned HTTP 200, and an unknown register number returned 404 (`outputs/app_smoke_test.txt`). `docker-compose.yml` parsed correctly (`outputs/compose_yaml_check.txt`).
- **Not executed:** docker build, run, `exec whoami`, stats, history, the image-size comparison and `docker compose up`. Docker is not installed on this host (`outputs/NOT_RUN.txt`). No image sizes or `whoami` output are recorded because none were measured.

## Task B3: Bare metal vs virtual machine vs container

**Commands**
```bash
cd Assignment5/Task_B3
python3 -m venv .venv && .venv/bin/pip install -r ../Task_B2/app/requirements.txt
./measure_bare_metal.sh | tee outputs/bare_metal_measurements.txt
./run_b3_lab.sh          # Linux lab: Docker, VM and minikube steps
```

**Complete the Code:** TODO 1 is `replicas: 3` plus `selector.matchLabels.app: result-portal`. TODO 2 is `resources.requests {cpu: 250m, memory: 256Mi}` and `limits {cpu: 500m, memory: 512Mi}`. `hpa.yaml` scales between 3 and 6 replicas at 50% CPU.

**Measured results** (full table: `results_table.md`)

| Deployment | Start-up | Memory (RSS) | Disk | Requests/sec |
|---|---|---|---|---|
| Bare metal (macOS host) | 115 ms median (113–122) | ≈ 92 MiB (94,064 KiB) | ≈ 31 MiB (app + venv) | **5,099.78** median of 3 runs, 0 failed |
| Virtual machine | not measured | not measured | not measured | not measured |
| Docker container | not measured | not measured | not measured | not measured |
| Kubernetes 3 → 6 replicas, pod deletion | not run | | | |

`ab -n 10000 -c 50` against 2 gunicorn workers, with client and server on the same machine. The VM, Docker and Kubernetes rows are empty because none of those tools are installed here. `deployment.yaml` and `hpa.yaml` were only checked by parsing them (`outputs/k8s_yaml_check.txt`).

## Task B4: Final engineering decision

**Commands**
```bash
cd Assignment5/Task_B4
./measure_cpu_per_request.sh | tee outputs/cpu_per_request.txt
python3 capacity.py | tee outputs/capacity_output.txt
./run_b4_lab.sh          # Linux lab: two-service compose with limits
```

**Result**
- Measured CPU cost: **0.382 CPU-ms per request**, or 2,617.8 requests per CPU-second (20,000 requests, 0 failed, 5,072.92 req/s). One pod limited to 500m CPU therefore serves about **1,309 req/s** for this demo app.
- `capacity.py` uses the B3 median of 5,099.78 req/s. With the stated assumptions (5 requests per student, 3× peak factor, 70% target utilisation), 150,000 students in 2 h means an average of 104.2 req/s and a peak of 312.5 req/s. That needs **1 pod at the measured CPU cost, so 3 with the replica minimum**. It needs 4 pods if real requests cost 10× more CPU and 18 if they cost 50× more. If everyone arrives in the first 10 minutes (1,250 req/s), it needs 2 / 14 / 69 pods at 1× / 10× / 50×.
- **Not executed:** the two-service compose (portal + lms + db, each with its own CPU and memory limits) was only YAML-checked, because Docker is not installed here.

## Screenshots

To be added by the student.
