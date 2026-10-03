#!/usr/bin/env bash
# 고급 Linux System Programming II - 실습 도구 설치 (Ubuntu Server 26.04 LTS / Kernel 7.0 기준)
# 사용법: sudo ./install_tools.sh
set -u

if [ "$(id -u)" -ne 0 ]; then
    echo "root 권한이 필요합니다: sudo $0" >&2
    exit 1
fi

KVER="$(uname -r)"
export DEBIAN_FRONTEND=noninteractive

REQUIRED="build-essential git gdb strace ltrace manpages-dev util-linux
linux-tools-common linux-tools-generic
trace-cmd bpftrace
liburing-dev libseccomp-dev libcap-dev libcap2-bin"

# 현재 실행 중인 Kernel과 Version이 맞는 perf. Package가 없을 수도 있으므로 따로 설치한다.
OPTIONAL="linux-tools-${KVER} linux-headers-${KVER}"

echo "== apt update"
apt-get update -y || { echo "apt update 실패: Network 설정을 확인하세요." >&2; exit 1; }

FAILED=""
echo "== 필수 Package 설치"
for p in $REQUIRED; do
    if apt-get install -y "$p" >/dev/null 2>&1; then
        echo "  [ OK ] $p"
    else
        echo "  [FAIL] $p"
        FAILED="$FAILED $p"
    fi
done

echo "== Kernel Version 의존 Package 설치 (${KVER})"
for p in $OPTIONAL; do
    if apt-get install -y "$p" >/dev/null 2>&1; then
        echo "  [ OK ] $p"
    else
        echo "  [SKIP] $p (Repository에 없음. perf가 동작하면 무시해도 됩니다)"
    fi
done

echo
if [ -n "$FAILED" ]; then
    echo "설치 실패:$FAILED"
    echo "위 Package를 개별로 다시 설치한 뒤 ./env_check.sh를 실행하세요."
    exit 1
fi
echo "설치 완료. 이어서 ./env_check.sh 를 실행하세요."
