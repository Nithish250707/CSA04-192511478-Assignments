"""Plot outputs/03_faults_3_to_10_frames.csv into one PNG per workload (needs matplotlib)."""
import csv, os, sys
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
here = os.path.dirname(os.path.abspath(sys.argv[0]))
rows = list(csv.DictReader(open(os.path.join(here, "outputs/03_faults_3_to_10_frames.csv"))))
for wl in ("locality", "random"):
    sub = [r for r in rows if r["workload"] == wl]
    x = [int(r["frames"]) for r in sub]
    plt.figure(figsize=(6.4, 4))
    for name, st in (("FIFO", "o-"), ("LRU", "s-"), ("OPT", "^-"), ("CLOCK", "d--")):
        plt.plot(x, [int(r[name]) for r in sub], st, label=name)
    plt.xlabel("Number of frames"); plt.ylabel("Page faults (100,000 references)")
    plt.title(f"{wl} workload, 50 distinct pages"); plt.grid(alpha=.3); plt.legend(); plt.tight_layout()
    plt.savefig(os.path.join(here, f"outputs/04_plot_{wl}.png"), dpi=120); plt.close()
print("saved plots")
