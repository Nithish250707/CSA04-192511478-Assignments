# B3 – Measured results

Host: MacBook Air, Apple Silicon (arm64), 10 cores, 16 GiB RAM, macOS 27.0.1 (Darwin 27.0.0).
App: `Task_B2/app/app.py`, gunicorn 23.0.0 with 2 sync workers, SQLite backend, Python 3.13.2.
Load: ApacheBench 2.3, `ab -n 10000 -c 50 http://127.0.0.1:8020/result/20260001`, client on the same machine.
Raw output: `outputs/bare_metal_measurements.txt`, `outputs/ab_bare_metal_run{1,2,3}.txt`.

| Deployment | Start-up time | Memory (RSS) | Disk / image size | Requests/sec | Status |
|---|---|---|---|---|---|
| Bare metal (directly on host OS) | 115 ms median (113–122 ms, 5 cold starts) | 94,064 KiB idle (≈ 92 MiB); 95,152 KiB after load | 32,028 KiB (≈ 31 MiB: app + virtualenv, excluding system Python) | **5,099.78** median (5,026.06 / 5,123.63 / 5,099.78), 0 failed | Measured on macOS |
| Virtual machine (VirtualBox/KVM) | – | – | – | – | **Not measured**: no hypervisor installed on this host |
| Docker container | – | – | – | – | **Not measured**: Docker not installed on this host |
| Kubernetes (minikube/kind), 3 → 6 replicas, pod deletion | – | – | – | – | **Not run**: minikube/kind/kubectl not installed |

Notes
- "Bare metal" here means macOS, not Linux, so the number shows what the app can do but is not the Linux figure the brief asks for. `run_b3_lab.sh` repeats it on Linux and fills in the other three rows.
- The `ab` client and the server share the same 10 cores, so the requests/sec figure is a lower bound for the server alone.
- `deployment.yaml` and `hpa.yaml` were checked only by parsing them (`outputs/k8s_yaml_check.txt`). They were not applied to a cluster.
