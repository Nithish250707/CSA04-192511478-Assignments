"""Plot outputs/04_faults_vs_frames.csv -> outputs/04_faults_vs_frames.png (needs matplotlib)."""
import csv, os, sys
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
here = os.path.dirname(os.path.abspath(sys.argv[0]))
rows = list(csv.DictReader(open(os.path.join(here, "outputs/04_faults_vs_frames.csv"))))
x = [int(r["frames"]) for r in rows]
for name, style in (("FIFO", "o-"), ("LRU", "s-"), ("OPT", "^-")):
    plt.plot(x, [int(r[name]) for r in rows], style, label=name)
plt.xlabel("Number of frames"); plt.ylabel("Page faults")
plt.title("Page faults vs frames, reference string 7 0 1 2 0 3 0 4 2 3 0 3 2 1 2 0 1 7 0 1")
plt.grid(alpha=.3); plt.legend(); plt.tight_layout()
plt.savefig(os.path.join(here, "outputs/04_faults_vs_frames.png"), dpi=120)
print("saved outputs/04_faults_vs_frames.png")
