#!/bin/sh
# cg_run.sh - cgroup v2 에 Group 을 만들고 한도를 건 뒤 그 안에서 명령을 실행한다 (tracefs 처럼 File 에 echo 하는 것이 전부)
# 사용법: sudo ./cg_run.sh [-c CPU%] [-m Memory] [-p 최대 Process 수] <명령> [인자...]
#   예:   sudo ./cg_run.sh -c 20 ./burn 5          sudo ./cg_run.sh -m 64M ./memhog 200          sudo ./cg_run.sh -p 10 ./spawn 50
ROOT=/sys/fs/cgroup
CG=$ROOT/lsp2-lab
CPU=""; MEM=""; PIDS=""
while getopts "c:m:p:" o; do
    case $o in c) CPU=$OPTARG ;; m) MEM=$OPTARG ;; p) PIDS=$OPTARG ;; *) exit 1 ;; esac
done
shift $((OPTIND - 1))
[ $# -ge 1 ] || { echo "사용법: sudo $0 [-c CPU%] [-m Memory] [-p Pids] <명령> [인자...]"; exit 1; }
[ -f $ROOT/cgroup.controllers ] || { echo "cgroup v2 가 $ROOT 에 Mount 되어 있지 않습니다: $(stat -f -c %T $ROOT)"; exit 1; }

# 1. 하위 Group 에서 쓸 Controller 를 켠다 (부모의 cgroup.subtree_control)
echo "+cpu +memory +pids" > $ROOT/cgroup.subtree_control 2>/dev/null

# 2. Group 생성 = mkdir. Kernel 이 제어용 File 들을 자동으로 만들어 준다
mkdir -p $CG || exit 1

# 3. 한도 설정
#    cpu.max = "<quota µs> <period µs>": 100ms 마다 quota 만큼만 실행할 수 있다
[ -n "$CPU" ]  && echo "$((CPU * 1000)) 100000" > $CG/cpu.max
[ -n "$MEM" ]  && echo "$MEM" > $CG/memory.max && echo 0 > $CG/memory.swap.max 2>/dev/null
[ -n "$PIDS" ] && echo "$PIDS" > $CG/pids.max
echo "cpu.max=$(cat $CG/cpu.max)  memory.max=$(cat $CG/memory.max)  pids.max=$(cat $CG/pids.max)"

# 4. 이 Shell 을 Group 에 넣고 exec → 대상과 그 자손이 모두 이 Group 에 속한다
sh -c "echo \$\$ > $CG/cgroup.procs; exec \"\$@\"" sh "$@"
echo "exit code = $?"

# 5. 결과: Kernel 이 집계해 둔 통계
echo "--- cpu.stat";      grep -E "usage_usec|nr_periods|nr_throttled|throttled_usec" $CG/cpu.stat
echo "--- memory";        echo "peak=$(cat $CG/memory.peak 2>/dev/null)"; grep -E "^(max|oom|oom_kill) " $CG/memory.events
echo "--- pids.events";   cat $CG/pids.events 2>/dev/null

# 6. 뒷정리: 안에 Process 가 없어야 rmdir 이 된다
rmdir $CG
