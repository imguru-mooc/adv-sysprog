#!/usr/bin/env bash
# 고급 Linux System Programming II - 실습 환경 점검
# 사용법: ./env_check.sh        (일부 항목은 sudo로 실행해야 정확합니다: sudo ./env_check.sh)
set -u

PASS=0; WARN=0; FAIL=0
ok()   { printf '  [ OK ] %s\n' "$1"; PASS=$((PASS+1)); }
warn() { printf '  [WARN] %s\n         -> %s\n' "$1" "$2"; WARN=$((WARN+1)); }
fail() { printf '  [FAIL] %s\n         -> %s\n' "$1" "$2"; FAIL=$((FAIL+1)); }
sec()  { printf '\n== %s\n' "$1"; }
have() { command -v "$1" >/dev/null 2>&1; }

IS_ROOT=0; [ "$(id -u)" -eq 0 ] && IS_ROOT=1

sec "1. System"
KVER="$(uname -r)"
KMAJ="${KVER%%.*}"; KREST="${KVER#*.}"; KMIN="${KREST%%[!0-9]*}"
echo "  Kernel : $KVER"
[ -r /etc/os-release ] && echo "  OS     : $(. /etc/os-release; echo "$PRETTY_NAME")"
echo "  Arch   : $(uname -m)"
if [ "$(uname -m)" = "x86_64" ]; then ok "x86_64 Architecture"
else warn "x86_64가 아닙니다" "Chapter 1, 2, 4의 Assembly·Register 설명은 x86-64 기준입니다"; fi
OSVER="$( . /etc/os-release 2>/dev/null; echo "${VERSION_ID:-}" )"
if [ "$OSVER" = "26.04" ]; then ok "Ubuntu 26.04 LTS"
else warn "Ubuntu ${OSVER:-?} (강의 기준은 26.04 LTS)" "실습은 가능하지만 출력과 Package 이름이 강의와 다를 수 있습니다"; fi
if [ "$KMAJ" -ge 7 ]; then ok "Kernel 7.0 이상"
elif [ "$KMAJ" -eq 6 ] || { [ "$KMAJ" -eq 5 ] && [ "${KMIN:-0}" -ge 15 ]; }; then
    warn "Kernel $KMAJ.${KMIN:-0} (강의 기준은 7.0)" "실습은 가능하지만 ftrace 함수 이름 등 Kernel 내부 관찰 결과가 강의와 다를 수 있습니다"
else fail "Kernel이 5.15보다 낮습니다" "Ubuntu Server 26.04 LTS를 새로 설치하세요"; fi
NCPU="$(nproc)"
if [ "$NCPU" -ge 2 ]; then ok "CPU $NCPU개"
else fail "CPU가 1개입니다" "VirtualBox 설정 > 시스템 > 프로세서에서 2개 이상으로 변경 (Chapter 4 Race, Memory Ordering, False Sharing 재현에 필수)"; fi
MEM_MB=$(( $(awk '/MemTotal/ {print $2}' /proc/meminfo) / 1024 ))
if [ "$MEM_MB" -ge 3500 ]; then ok "Memory ${MEM_MB}MB"
else warn "Memory ${MEM_MB}MB" "4GB 이상 권장 (Chapter 2 Page Fault, Chapter 3 Page Cache 실습)"; fi

sec "2. Build / Debug"
if have gcc; then echo "  gcc    : $(gcc -dumpfullversion 2>/dev/null)"; fi
for t in gcc make gdb strace ltrace git; do
    if have "$t"; then ok "$t"; else fail "$t 없음" "sudo ./install_tools.sh"; fi
done

sec "3. perf"
if have perf && perf --version >/dev/null 2>&1; then
    ok "perf 실행 가능 ($(perf --version 2>/dev/null))"
    PARANOID="$(cat /proc/sys/kernel/perf_event_paranoid 2>/dev/null || echo '?')"
    echo "  perf_event_paranoid = $PARANOID"
    if [ "$PARANOID" != "?" ] && [ "$PARANOID" -gt 1 ] && [ "$IS_ROOT" -eq 0 ]; then
        warn "일반 사용자 perf 사용이 제한되어 있습니다" "sudo perf ... 로 실행하거나: sudo sysctl kernel.perf_event_paranoid=1"
    fi
    OUT="$(perf stat -e page-faults,context-switches true 2>&1)"
    if echo "$OUT" | grep -q "page-faults" && ! echo "$OUT" | grep -qi "not supported\|permission\|denied"; then
        ok "Software Event (page-faults, context-switches)"
    else
        warn "Software Event 측정 실패" "sudo로 다시 점검하세요"
    fi
    OUT="$(perf stat -e cycles,cache-misses true 2>&1)"
    if echo "$OUT" | grep -qi "not supported\|not counted"; then
        warn "Hardware Counter 사용 불가 (cycles, cache-misses)" "VirtualBox에서는 정상입니다. 해당 실습은 Software Event와 실행 시간으로 진행하고, HW Counter 결과는 강의 화면을 참고합니다"
    elif echo "$OUT" | grep -q "cycles"; then
        ok "Hardware Counter (cycles, cache-misses)"
    else
        warn "Hardware Counter 확인 실패" "sudo로 다시 점검하세요"
    fi
else
    fail "perf 없음 또는 Kernel Version 불일치" "sudo apt install linux-tools-generic linux-tools-$(uname -r)"
fi

sec "4. ftrace"
TRACEFS=""
for d in /sys/kernel/tracing /sys/kernel/debug/tracing; do
    [ -d "$d" ] && TRACEFS="$d" && break
done
if [ -n "$TRACEFS" ]; then
    ok "tracefs: $TRACEFS"
    if [ "$IS_ROOT" -eq 1 ]; then
        if grep -qw function_graph "$TRACEFS/available_tracers" 2>/dev/null; then ok "function_graph tracer"
        else warn "function_graph tracer 없음" "Chapter 7 L34 실습이 제한됩니다"; fi
    else
        echo "  (available_tracers 확인은 sudo 필요)"
    fi
else
    fail "tracefs 없음" "sudo mount -t tracefs nodev /sys/kernel/tracing"
fi
if have trace-cmd; then ok "trace-cmd"; else warn "trace-cmd 없음" "sudo apt install trace-cmd"; fi

sec "5. eBPF / bpftrace"
if have bpftrace; then ok "bpftrace ($(bpftrace --version 2>/dev/null | head -1))"
else fail "bpftrace 없음" "sudo apt install bpftrace"; fi
if [ -r /sys/kernel/btf/vmlinux ]; then ok "BTF (/sys/kernel/btf/vmlinux)"
else warn "BTF 없음" "kprobe 인자 접근 예제 일부가 동작하지 않을 수 있습니다: sudo apt install linux-headers-$(uname -r)"; fi
if [ "$IS_ROOT" -eq 1 ] && have bpftrace; then
    if timeout 20 bpftrace -e 'BEGIN { printf("ok\n"); exit(); }' 2>/dev/null | grep -q ok; then ok "bpftrace Program 실행"
    else fail "bpftrace Program 실행 실패" "Kernel Lockdown 또는 BPF 설정을 확인하세요"; fi
else
    echo "  (bpftrace 실행 Test는 sudo 필요)"
fi

sec "6. io_uring"
if [ -r /usr/include/liburing.h ]; then ok "liburing header"
else fail "liburing.h 없음" "sudo apt install liburing-dev"; fi
if [ -r /proc/sys/kernel/io_uring_disabled ]; then
    V="$(cat /proc/sys/kernel/io_uring_disabled)"
    if [ "$V" = "0" ]; then ok "kernel.io_uring_disabled = 0"
    else fail "kernel.io_uring_disabled = $V" "sudo sysctl kernel.io_uring_disabled=0"; fi
else
    ok "io_uring_disabled sysctl 없음 (제한 없음)"
fi

sec "7. Isolation / Security"
CGFS="$(findmnt -no FSTYPE /sys/fs/cgroup 2>/dev/null | head -1)"
if [ "$CGFS" = "cgroup2" ]; then ok "cgroup v2"
else warn "cgroup v2 Unified Hierarchy가 아닙니다" "Chapter 8 L39는 cgroup v2 기준입니다. Ubuntu 26.04 LTS를 사용하세요"; fi
[ -r /usr/include/seccomp.h ] && ok "libseccomp header" || fail "seccomp.h 없음" "sudo apt install libseccomp-dev"
[ -r /usr/include/x86_64-linux-gnu/sys/capability.h ] || [ -r /usr/include/sys/capability.h ] \
    && ok "libcap header" || fail "sys/capability.h 없음" "sudo apt install libcap-dev"
for t in setcap getcap capsh unshare lsns; do
    if have "$t"; then ok "$t"; else warn "$t 없음" "sudo apt install libcap2-bin util-linux"; fi
done

sec "8. 참고 설정 (변경 불필요, 해당 Lecture에서 다룹니다)"
echo "  randomize_va_space (ASLR) = $(cat /proc/sys/kernel/randomize_va_space 2>/dev/null)   [L06]"
echo "  core_pattern              = $(cat /proc/sys/kernel/core_pattern 2>/dev/null)   [L32]"
echo "  ulimit -c                 = $(ulimit -c)   [L32]"
echo "  vm.dirty_ratio            = $(cat /proc/sys/vm/dirty_ratio 2>/dev/null)   [L15]"
echo "  /tmp Filesystem           = $(findmnt -no FSTYPE -T /tmp 2>/dev/null | head -1)   [tmpfs이면 File I/O 실습은 Home Directory에서, Ch3]"
echo "  Page Size                 = $(getconf PAGESIZE)   [L07]"

printf '\n== 결과: OK %d / WARN %d / FAIL %d\n' "$PASS" "$WARN" "$FAIL"
if [ "$FAIL" -gt 0 ]; then
    echo "FAIL 항목을 해결한 뒤 다시 실행하세요."
    exit 1
fi
[ "$IS_ROOT" -eq 0 ] && echo "sudo ./env_check.sh 로 한 번 더 실행하면 ftrace, bpftrace 항목까지 점검합니다."
echo "실습 준비 완료."
