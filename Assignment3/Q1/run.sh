#!/usr/bin/env bash
# A3 Q1 - build static and dynamic versions, compare, replace the library, remove it.
# macOS equivalents are used where the Linux tool does not exist (printed in each output).
set -uo pipefail
cd "$(dirname "$0")"; O=outputs; mkdir -p $O; rm -f *.o *.a *.dylib bank_static bank_dynamic dlopen_demo
CF="-std=c11 -Wall -Wextra -O2"
{
echo "\$ gcc $CF -c bankcrypt_v1.c && ar rcs libbankcrypt.a bankcrypt_v1.o"
gcc $CF -c bankcrypt_v1.c && ar rcs libbankcrypt.a bankcrypt_v1.o
echo "\$ gcc $CF bank.c libbankcrypt.a -o bank_static          # library linked statically"
gcc $CF bank.c libbankcrypt.a -o bank_static
echo "\$ gcc $CF -static bank.c libbankcrypt.a -o bank_fullstatic   # fully static (libc too)"
gcc $CF -static bank.c libbankcrypt.a -o bank_fullstatic 2>&1 | head -3 || true
echo "\$ gcc $CF -fPIC -dynamiclib -install_name @rpath/libbankcrypt.dylib bankcrypt_v1.c -o libbankcrypt.dylib"
gcc $CF -fPIC -dynamiclib -install_name @rpath/libbankcrypt.dylib bankcrypt_v1.c -o libbankcrypt.dylib
echo "\$ gcc $CF bank.c -L. -lbankcrypt -Wl,-rpath,@executable_path -o bank_dynamic"
gcc $CF bank.c -L. -lbankcrypt -Wl,-rpath,@executable_path -o bank_dynamic
echo "\$ gcc $CF dlopen_demo.c -o dlopen_demo"
gcc $CF dlopen_demo.c -o dlopen_demo
} > $O/01_build.txt 2>&1

{
echo "== Executable sizes (bytes)"; ls -l bank_static bank_dynamic libbankcrypt.dylib libbankcrypt.a | awk '{print $5, $9}'
echo; echo "== Shared libraries (Linux: ldd; macOS: otool -L)"
echo "\$ otool -L bank_static"; otool -L bank_static
echo "\$ otool -L bank_dynamic"; otool -L bank_dynamic
echo; echo "== Undefined symbols resolved at load time (nm -u)"
echo "\$ nm -u bank_static";  nm -u bank_static
echo "\$ nm -u bank_dynamic"; nm -u bank_dynamic
} > $O/02_sizes_and_libs.txt 2>&1

{ echo "\$ ./bank_static --where"; ./bank_static --where; echo; echo "\$ ./bank_dynamic --where"; ./bank_dynamic --where
  echo; echo "\$ ./bank_dynamic --where   (second run: ASLR moves everything)"; ./bank_dynamic --where; } > $O/03_load_addresses.txt 2>&1

{ echo "\$ ./dlopen_demo"; ./dlopen_demo; } > $O/04_dlopen.txt 2>&1

{
echo "== Start-up time: 500 runs each (wall clock for the whole loop)"
for b in bank_static bank_dynamic; do
  s=$(python3 -c 'import time;print(time.perf_counter())')
  for i in $(seq 500); do ./$b >/dev/null; done
  e=$(python3 -c 'import time;print(time.perf_counter())')
  python3 -c "print('$b: %.3f ms per run' % (($e-$s)*1000/500))"
done
echo; echo "== Memory: maximum resident set size (Linux: /usr/bin/time -v; macOS: /usr/bin/time -l)"
for b in bank_static bank_dynamic; do
  echo "$b: $(/usr/bin/time -l ./$b 2>&1 >/dev/null | grep -E 'maximum resident set size|page reclaims' | xargs)"
done
} > $O/05_startup_and_memory.txt 2>&1

{
echo "== Replace the shared library WITHOUT recompiling bank_dynamic"
echo "before:"; ./bank_dynamic; ./bank_static
echo "\$ gcc $CF -fPIC -dynamiclib -install_name @rpath/libbankcrypt.dylib bankcrypt_v2.c -o libbankcrypt.dylib"
gcc $CF -fPIC -dynamiclib -install_name @rpath/libbankcrypt.dylib bankcrypt_v2.c -o libbankcrypt.dylib
echo "after (same binaries, no rebuild):"; ./bank_dynamic; ./bank_static
./dlopen_demo
echo "bank_dynamic checksum unchanged: $(shasum bank_dynamic)"
} > $O/06_replace_library.txt 2>&1

{
echo "== Two copies running at once: which pages are shared (Linux: /proc/<pid>/maps, pmap; macOS: vmmap)"
./bank_dynamic --wait >/dev/null & P1=$!; ./bank_dynamic --wait >/dev/null & P2=$!; sleep 1
for p in $P1 $P2; do
  echo "--- vmmap $p (libbankcrypt and libsystem_c text regions)"
  vmmap $p 2>/dev/null | grep -E "^__TEXT .*(libbankcrypt|libsystem_c\.dylib|bank_dynamic)" | head -6
done
echo "--- vmmap -summary $P1 (resident vs shared)"; vmmap -summary $P1 2>/dev/null | grep -E "^(Physical footprint|TOTAL|__TEXT )" | head -5
wait
} > $O/07_two_copies_shared_pages.txt 2>&1

{
echo "== Shared library missing"
mv libbankcrypt.dylib libbankcrypt.dylib.hidden
echo "\$ ./bank_dynamic"; ./bank_dynamic; echo "exit status: $?"
echo "\$ ./bank_static"; ./bank_static; echo "exit status: $?"
echo "\$ ./dlopen_demo"; ./dlopen_demo; echo "exit status: $?"
mv libbankcrypt.dylib.hidden libbankcrypt.dylib
} > $O/08_library_missing.txt 2>&1
rm -f *.o bank_fullstatic
