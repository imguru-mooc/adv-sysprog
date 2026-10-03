/* harden.h - 초기화가 끝난 뒤 스스로 권한을 내려놓는다 (L36 Capability + L37 seccomp) */
#ifndef HARDEN_H
#define HARDEN_H

#define _GNU_SOURCE
#include <errno.h>
#include <grp.h>
#include <seccomp.h>
#include <stdio.h>
#include <sys/capability.h>
#include <sys/prctl.h>
#include <unistd.h>

/* root 로 시작한 경우: 다른 사용자의 /proc/PID/fd 를 읽는 데 필요한 두 개만 남기고 nobody 가 된다 */
static inline int drop_privileges(void)
{
    cap_value_t keep[] = { CAP_SYS_PTRACE, CAP_DAC_READ_SEARCH };
    cap_t       caps;

    if (geteuid() != 0)
        return 0;                                       /* 이미 일반 사용자 */
    if (prctl(PR_SET_KEEPCAPS, 1) < 0 || setgroups(0, NULL) < 0 ||
        setresgid(65534, 65534, 65534) < 0 || setresuid(65534, 65534, 65534) < 0)
        return -1;
    caps = cap_init();
    cap_set_flag(caps, CAP_PERMITTED, 2, keep, CAP_SET);
    cap_set_flag(caps, CAP_EFFECTIVE, 2, keep, CAP_SET);
    if (cap_set_proc(caps) < 0)
        return -1;
    cap_free(caps);
    return 0;
}

/* 이 프로그램이 실행 중에 쓰는 System Call 만 허용한다. 목록은 strace -f -c 로 뽑았다 (L32) */
static inline int install_seccomp(void)
{
    static const int allow[] = {
        SCMP_SYS(read), SCMP_SYS(write), SCMP_SYS(openat), SCMP_SYS(close), SCMP_SYS(fstat), SCMP_SYS(newfstatat),
        SCMP_SYS(getdents64), SCMP_SYS(lseek), SCMP_SYS(epoll_wait), SCMP_SYS(epoll_pwait), SCMP_SYS(epoll_ctl),
        SCMP_SYS(timerfd_settime), SCMP_SYS(futex), SCMP_SYS(msync), SCMP_SYS(munmap), SCMP_SYS(mmap),
        SCMP_SYS(mprotect), SCMP_SYS(madvise), SCMP_SYS(brk), SCMP_SYS(clock_gettime), SCMP_SYS(clock_nanosleep),
        SCMP_SYS(rt_sigreturn), SCMP_SYS(rt_sigprocmask), SCMP_SYS(exit), SCMP_SYS(exit_group), SCMP_SYS(getpid),
        SCMP_SYS(sched_yield), SCMP_SYS(rseq), SCMP_SYS(ioctl), SCMP_SYS(fcntl),
    };
    scmp_filter_ctx ctx = seccomp_init(SCMP_ACT_ERRNO(EPERM));  /* 목록에 없으면 실행하지 않고 EPERM */
    size_t          i;
    int             rc = 0;

    if (ctx == NULL)
        return -1;
    for (i = 0; i < sizeof(allow) / sizeof(allow[0]); i++)
        rc |= seccomp_rule_add(ctx, SCMP_ACT_ALLOW, allow[i], 0);
    if (rc == 0)
        rc = seccomp_load(ctx);                         /* NO_NEW_PRIVS 도 함께 설정된다 */
    seccomp_release(ctx);
    return rc;
}

#endif
