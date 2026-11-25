#!/usr/bin/env bash
# Task 1.2 demo: build the improved organisation in a scratch folder and show permissions,
# setgid project folder, an ACL and symbolic links. Then compare listing a flat directory of
# 12,000 student entries with listing one student's own directory.
# macOS: ACLs use `chmod +a`; on Linux use `setfacl -m u:lecturer:rx`. The project group is
# the existing group "staff" because creating groups needs root (Linux: groupadd proj07).
# The lecturer group is represented by the built-in "admin" group (read-only ACL).
set -euo pipefail
R=$(mktemp -d "${TMPDIR:-/tmp}/a4t1.XXXX"); cd "$R"; G=$(id -gn)
mkdir -p home/cse/2025/192511478 home/cse/2025/192511479 projects/csa0421/group07 staff backup
chmod 700 home/cse/2025/192511478 home/cse/2025/192511479        # private homes
chmod 2770 projects/csa0421/group07                                # rwx for group, setgid
echo "draft" > home/cse/2025/192511478/notes.txt
echo "shared report" > projects/csa0421/group07/report.md
ln -s ../../../../projects/csa0421/group07 home/cse/2025/192511478/group07-link   # sharing without copying
if chmod +a "group:admin allow list,search,read" projects/csa0421/group07 2>/dev/null; then ACL="macOS chmod +a"; else ACL="(ACL not supported here)"; fi
echo "== Tree (find)"; find . -mindepth 1 | sort
echo; echo "== Permissions (ls -ld; '@'/'+' = extended attributes/ACL)"
ls -ld home/cse/2025/192511478 home/cse/2025/192511479 projects/csa0421/group07 home/cse/2025/192511478/group07-link
echo; echo "== ACL on the project folder ($ACL): ls -led"; ls -led projects/csa0421/group07
echo; echo "== Symlink resolves to the shared folder"; cat home/cse/2025/192511478/group07-link/report.md
echo; echo "== New file in the setgid folder gets the folder's group ($G). (BSD/macOS always does this; on Linux it needs the setgid bit.)"
touch projects/csa0421/group07/new.txt; ls -l projects/csa0421/group07/new.txt

echo; echo "== Flat directory with 12,000 students vs per-student directory"
mkdir flat; ( cd flat && for i in $(seq 0 11999); do : > "s$((192500000 + i))_project.docx"; done )
python3 - "$R" <<'PY'
import os, sys, time
r = sys.argv[1]
def t(fn, n=50):
    best = 1e9
    for _ in range(n):
        s = time.perf_counter(); fn(); best = min(best, time.perf_counter() - s)
    return best * 1e3
flat = os.path.join(r, "flat"); home = os.path.join(r, "home/cse/2025/192511478")
print(f"list flat dir   ({len(os.listdir(flat)):5d} entries): {t(lambda: os.listdir(flat)):8.3f} ms")
print(f"list own dir    ({len(os.listdir(home)):5d} entries): {t(lambda: os.listdir(home)):8.3f} ms")
print(f"open one file by name in flat dir: {t(lambda: os.stat(os.path.join(flat, 's192506000_project.docx'))):8.4f} ms (APFS directories are B-trees, so lookup by exact name stays fast)")
names = os.listdir(flat)
print(f"linear search of the listing for one name: {t(lambda: [n for n in names if n.startswith('s192506000')]):8.3f} ms")
print("name clash: a second student cannot also create 's192506000_project.docx' in the flat directory ->",
      "exists" if os.path.exists(os.path.join(flat, 's192506000_project.docx')) else "free")
PY
rm -rf "$R"
