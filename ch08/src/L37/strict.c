/* strict.c - seccomp Strict Mode: read, write, _exit, sigreturn 네 개만 허용. 그 외에는 즉시 SIGKILL */
#define _GNU_SOURCE
#include <fcntl.h>
#include <linux/seccomp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <unistd.h>

int main(void)
{
    const char *m1 = "1. Strict Mode 진입 후: write 는 허용된다\n";
    const char *m2 = "2. 이제 open 을 시도한다...\n";

    printf("0. 진입 전: 무엇이든 할 수 있다 (pid %d)\n", (int)getpid());
    fflush(stdout);

    if (prctl(PR_SET_SECCOMP, SECCOMP_MODE_STRICT) < 0) {
        perror("prctl(SECCOMP_MODE_STRICT)");
        return EXIT_FAILURE;
    }
    syscall(SYS_write, 1, m1, strlen(m1));
    syscall(SYS_write, 1, m2, strlen(m2));
    open("/etc/hostname", O_RDONLY);                    /* 허용되지 않은 System Call → SIGKILL */
    syscall(SYS_write, 1, "3. 여기는 실행되지 않는다\n", 35);
    syscall(SYS_exit, 0);
    return 0;
}
