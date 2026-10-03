/* rawbpf.c - seccomp Filter 의 정체는 (classic) BPF Program 이다. Library 없이 직접 작성해 본다
 *   규칙: mkdir / mkdirat 은 EACCES 로 실패시키고, 나머지는 모두 허용
 */
#define _GNU_SOURCE
#include <errno.h>
#include <linux/audit.h>
#include <linux/filter.h>
#include <linux/seccomp.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <unistd.h>

int main(void)
{
    struct sock_filter filter[] = {
        /* [0] A = seccomp_data.arch */
        BPF_STMT(BPF_LD | BPF_W | BPF_ABS, offsetof(struct seccomp_data, arch)),
        /* [1] x86-64 가 아니면 죽인다 (32bit System Call 번호로 우회하는 것을 막는다) */
        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, AUDIT_ARCH_X86_64, 1, 0),
        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_KILL_PROCESS),
        /* [3] A = seccomp_data.nr (System Call 번호) */
        BPF_STMT(BPF_LD | BPF_W | BPF_ABS, offsetof(struct seccomp_data, nr)),
        /* [4][5] mkdir 또는 mkdirat 이면 [7] 로, 아니면 [6] 으로 */
        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, SYS_mkdir, 2, 0),
        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, SYS_mkdirat, 1, 0),
        /* [6] 허용 */
        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ALLOW),
        /* [7] 실행하지 않고 errno = EACCES 로 return */
        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ERRNO | (EACCES & SECCOMP_RET_DATA)),
    };
    struct sock_fprog prog = { .len = (unsigned short)(sizeof(filter) / sizeof(filter[0])), .filter = filter };
    char              line[128];
    FILE             *fp;

    /* root 가 아니면 NO_NEW_PRIVS 가 필수: Filter 를 건 채 setuid 프로그램을 실행해 속이는 것을 막는다 */
    if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0) < 0 || prctl(PR_SET_SECCOMP, SECCOMP_MODE_FILTER, &prog) < 0) {
        perror("seccomp");
        return EXIT_FAILURE;
    }
    printf("Filter 설치 완료 (BPF 명령어 %u 개)\n", prog.len);

    if (mkdir("/tmp/seccomp_test_dir", 0755) < 0)
        printf("mkdir  → 실패: %s   ← Kernel 은 mkdir 을 실행조차 하지 않았다\n", strerror(errno));
    printf("getpid → %d (허용)\n", (int)getpid());

    fp = fopen("/proc/self/status", "r");
    while (fp != NULL && fgets(line, sizeof(line), fp) != NULL)
        if (strncmp(line, "Seccomp", 7) == 0 || strncmp(line, "NoNewPrivs", 10) == 0)
            printf("   %s", line);
    return EXIT_SUCCESS;
}
