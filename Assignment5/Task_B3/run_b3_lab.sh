#!/usr/bin/env bash
# B3 steps that need Linux + Docker + a hypervisor + minikube. NOT executed in this repo:
# the host is macOS 27 with no Docker, VirtualBox, minikube, kind or kubectl.
# Run on the Linux lab machine; outputs go to outputs/lab_*.txt.
set -uxo pipefail
cd "$(dirname "$0")"; O=outputs; mkdir -p "$O"

## 1. Bare metal on Linux: same script as on the Mac
./measure_bare_metal.sh > "$O/lab_bare_metal.txt" 2>&1   # needs python3 -m venv .venv && .venv/bin/pip install -r ../Task_B2/app/requirements.txt

## 2. Docker container (image built in Task_B2)
docker build -t result-portal ../Task_B2/app
t0=$(date +%s%3N); docker run -d --name b3 -p 8030:8000 result-portal
until curl -sf http://127.0.0.1:8030/health >/dev/null; do sleep 0.01; done
echo "container start-up: $(( $(date +%s%3N) - t0 )) ms" > "$O/lab_docker.txt"
docker stats --no-stream b3 >> "$O/lab_docker.txt"
docker images result-portal >> "$O/lab_docker.txt"
ab -n 10000 -c 50 http://127.0.0.1:8030/result/20260001 >> "$O/lab_docker.txt" 2>&1
docker rm -f b3

## 3. Virtual machine (VirtualBox/KVM, Ubuntu guest): inside the guest run step 1, and record
##    VM boot time (power-on -> SSH ready), guest `free -m`, and the size of the .vdi/.qcow2 disk:
##    VBoxManage startvm portal-vm --type headless ; ls -lh ~/VirtualBox\ VMs/portal-vm/*.vdi

## 4. Kubernetes on minikube
minikube start --cpus 4 --memory 4096 > "$O/lab_k8s.txt" 2>&1
minikube addons enable metrics-server >> "$O/lab_k8s.txt" 2>&1
docker tag result-portal simats/result-portal:1.0
minikube image load simats/result-portal:1.0
kubectl apply -f deployment.yaml -f hpa.yaml >> "$O/lab_k8s.txt"
kubectl rollout status deployment/result-portal >> "$O/lab_k8s.txt"
kubectl get pods -o wide >> "$O/lab_k8s.txt"                     # expect 3 Running
kubectl describe pod -l app=result-portal | grep -A6 -E "Limits|Requests" >> "$O/lab_k8s.txt"
victim=$(kubectl get pods -l app=result-portal -o jsonpath='{.items[0].metadata.name}')
kubectl delete pod "$victim" >> "$O/lab_k8s.txt"
sleep 5; kubectl get pods -o wide >> "$O/lab_k8s.txt"            # a new pod replaces $victim
# Load + scale to 6
kubectl port-forward svc/result-portal 8080:80 & PF=$!; sleep 3
ab -n 50000 -c 100 http://127.0.0.1:8080/result/20260001 > "$O/lab_k8s_ab.txt" 2>&1 &
kubectl scale deployment/result-portal --replicas=6 >> "$O/lab_k8s.txt"
sleep 20; kubectl get hpa,pods >> "$O/lab_k8s.txt"
wait; kill $PF
