/* raw_syscall.c - glibc를 거치지 않고 syscall 명령을 직접 실행한다 (x86-64 전용) */
#include <stdio.h>

/*
 * x86-64 System Call 규약
 *   rax = System Call 번호,  인자 = rdi, rsi, rdx, r10, r8, r9
 *   syscall 명령 실행 후 rax = 결과 (오류면 -errno)
 *   rcx, r11 은 CPU가 덮어쓴다 (복귀 주소, RFLAGS 저장)
 */
static long raw_syscall3(long nr, long a1, long a2, long a3)
{
    long ret;

    __asm__ volatile("syscall"
                     : "=a"(ret)
                     : "0"(nr), "D"(a1), "S"(a2), "d"(a3)
                     : "rcx", "r11", "memory");
    return ret;
}

int main(void)
{
    const char msg[] = "hello from raw syscall\n";
    long r;

    r = raw_syscall3(1 /* write */, 1, (long)msg, (long)(sizeof(msg) - 1));
    printf("write  -> %ld\n", r);

    r = raw_syscall3(1 /* write */, 99 /* 잘못된 fd */, (long)msg, 1);
    printf("write(99) -> %ld   (errno가 아니라 -EBADF 가 그대로 돌아온다)\n", r);

    r = raw_syscall3(39 /* getpid */, 0, 0, 0);
    printf("getpid -> %ld\n", r);
    return 0;
}
