/* nostart.c - glibc 의 시작 코드 없이 _start 를 직접 작성한다 (x86-64 전용)
 * Build: gcc -nostdlib -static -o nostart nostart.c
 * main 이 없어도 프로그램은 실행된다. Kernel 이 아는 것은 Entry Point 뿐이다.
 */
static long sys3(long nr, long a1, long a2, long a3)
{
    long ret;

    __asm__ volatile("syscall"
                     : "=a"(ret)
                     : "0"(nr), "D"(a1), "S"(a2), "d"(a3)
                     : "rcx", "r11", "memory");
    return ret;
}

void _start(void)
{
    static const char msg[] = "no main, no libc\n";

    sys3(1 /* write */, 1, (long)msg, (long)(sizeof(msg) - 1));
    sys3(60 /* exit */, 0, 0, 0);
    for (;;) { }        /* 여기로는 오지 않는다. return 하면 돌아갈 곳이 없어 Crash */
}
