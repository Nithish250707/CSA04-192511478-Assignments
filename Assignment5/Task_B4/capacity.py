"""B4.2 – How many portal containers are needed for 150,000 students in two hours?

MEASURED inputs come from this repo's own runs (macOS 27, Apple Silicon, see files named below).
ASSUMED inputs are stated choices, not measurements; change them and rerun.
Run: python3 capacity.py
"""
import math

# ---- Measured (do not edit without re-measuring) -------------------------------------------
MEASURED_RPS_PER_INSTANCE = 5099.78   # Task_B3/outputs/bare_metal_measurements.txt, median of 3 ab runs,
                                      # one gunicorn instance, 2 workers, no CPU limit
MEASURED_REQ_PER_CPU_SECOND = 2617.8  # Task_B4/outputs/cpu_per_request.txt (0.382 CPU-ms per request)

# ---- Workload (from the case study) ---------------------------------------------------------
STUDENTS = 150_000
WINDOW_SECONDS = 2 * 60 * 60

# ---- Assumptions -----------------------------------------------------------------------------
REQUESTS_PER_STUDENT = 5   # login page, login POST, result page, result data, logout
PEAK_FACTOR = 3            # busiest minutes carry 3x the 2-hour average rate
TARGET_UTILISATION = 0.70  # keep 30 % headroom per container
POD_CPU_LIMIT = 0.5        # deployment.yaml limit: 500m
MIN_REPLICAS = 3           # deployment.yaml / hpa.yaml minimum, for failure tolerance


def containers_needed(peak_rps: float, rps_per_container: float) -> int:
    return max(1, math.ceil(peak_rps / (rps_per_container * TARGET_UTILISATION)))


def main() -> None:
    avg_rps = STUDENTS * REQUESTS_PER_STUDENT / WINDOW_SECONDS
    peak_rps = avg_rps * PEAK_FACTOR
    worst_rps = STUDENTS * REQUESTS_PER_STUDENT / 600     # everyone in the first 10 minutes
    pod_rps = MEASURED_REQ_PER_CPU_SECOND * POD_CPU_LIMIT

    print(f"Students {STUDENTS:,} in {WINDOW_SECONDS // 3600} h, {REQUESTS_PER_STUDENT} requests each")
    print(f"{'Average load':<19}: {avg_rps:8.1f} req/s")
    print(f"{'Peak load (x' + str(PEAK_FACTOR) + ')':<19}: {peak_rps:8.1f} req/s")
    print(f"{'Worst case (10 min)':<19}: {worst_rps:8.1f} req/s")
    print()
    print(f"Measured: {MEASURED_RPS_PER_INSTANCE} req/s per unlimited 2-worker instance; "
          f"{MEASURED_REQ_PER_CPU_SECOND} req per CPU-second")
    print(f"=> one pod limited to {POD_CPU_LIMIT * 1000:.0f}m CPU can serve about {pod_rps:.0f} req/s")
    print()
    # Real result-day requests (TLS, sessions, a networked database, PDF results) cost more CPU
    # than this demo's SQLite lookup. The cost multiplier shows how the answer changes.
    print(f"{'CPU cost vs demo':>17} | {'pods @ peak':>11} | {'pods @ worst':>12} | {'pods to deploy (min ' + str(MIN_REPLICAS) + ')':>22}")
    print("-" * 74)
    for mult in (1, 10, 50, 100):
        per_pod = pod_rps / mult
        p = containers_needed(peak_rps, per_pod)
        w = containers_needed(worst_rps, per_pod)
        print(f"{'x' + str(mult):>17} | {p:>11} | {w:>12} | {max(MIN_REPLICAS, p):>22}")


if __name__ == "__main__":
    main()
