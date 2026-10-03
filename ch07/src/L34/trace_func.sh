#!/bin/sh
# trace_func.sh - function_graph Tracer 로 Kernel 함수 하나의 내부 호출 흐름을 본다
# 사용법: sudo ./trace_func.sh <kernel 함수> <명령> [인자...]      예: sudo ./trace_func.sh vfs_read cat /etc/hostname
T=/sys/kernel/tracing
FUNC=$1; shift
[ -w "$T/tracing_on" ] && [ -n "$FUNC" ] && [ $# -ge 1 ] || { echo "사용법: sudo $0 <kernel 함수> <명령> [인자...]"; exit 1; }
grep -qw function_graph $T/available_tracers || { echo "이 Kernel 은 function_graph Tracer 를 지원하지 않습니다: $(cat $T/available_tracers)"; exit 1; }

echo 0 > $T/tracing_on
echo   > $T/trace
echo "$FUNC" > $T/set_graph_function || { echo "추적할 수 없는 함수입니다. grep $FUNC $T/available_filter_functions 로 확인하세요"; exit 1; }
echo 5 > $T/max_graph_depth                 # 호출 깊이 제한
echo function_graph > $T/current_tracer

sh -c "echo \$\$ > $T/set_ftrace_pid; echo 1 > $T/tracing_on; exec \"\$@\"" sh "$@" > /dev/null

echo 0 > $T/tracing_on
grep -v '^#' $T/trace | head -n 60

echo nop > $T/current_tracer                # 뒷정리
echo     > $T/set_graph_function
echo     > $T/set_ftrace_pid
echo 0   > $T/max_graph_depth
echo     > $T/trace
