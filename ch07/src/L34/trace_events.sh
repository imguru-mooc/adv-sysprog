#!/bin/sh
# trace_events.sh - 지정한 명령 하나만 골라 Tracepoint Event 를 기록한다 (tracefs 를 직접 다루는 방법)
# 사용법: sudo ./trace_events.sh <명령> [인자...]
#   EVENTS 환경 변수로 Event 를 바꿀 수 있다.  예: sudo EVENTS="sched:sched_switch ext4:ext4_sync_file_enter" ./trace_events.sh ./marker
T=/sys/kernel/tracing
EVENTS=${EVENTS:-"syscalls:sys_enter_write syscalls:sys_enter_fsync syscalls:sys_exit_fsync syscalls:sys_enter_clock_nanosleep sched:sched_switch"}

[ -w "$T/tracing_on" ] || { echo "root 권한이 필요합니다 (또는 tracefs 가 Mount 되어 있지 않습니다)"; exit 1; }
[ $# -ge 1 ] || { echo "사용법: $0 <명령> [인자...]"; exit 1; }

echo 0 > $T/tracing_on                      # 1. 기록 중지, Buffer 비우기
echo   > $T/trace
for e in $EVENTS; do                        # 2. 볼 Event 를 켠다: events/<subsystem>/<event>/enable
    echo 1 > "$T/events/$(echo "$e" | tr ':' '/')/enable" || echo "  (없는 Event: $e)"
done

# 3. 이 Shell 의 PID 를 Filter 에 넣고 exec 로 대상 프로그램이 된다 → 그 Process 의 Event 만 기록된다
sh -c "echo \$\$ > $T/set_event_pid; echo 1 > $T/tracing_on; exec \"\$@\"" sh "$@"

echo 0 > $T/tracing_on                      # 4. 중지 후 결과 출력
grep -v '^#' $T/trace

echo 0 > $T/events/enable                   # 5. 뒷정리: 켜 둔 채로 두면 계속 Overhead 가 든다
echo   > $T/set_event_pid
echo   > $T/trace
