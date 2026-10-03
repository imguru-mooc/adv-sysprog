/* sandbox.c - libseccomp 로 Allowlist 를 만든다: "이 프로그램에 필요한 것만 허용, 나머지는 거부"
 * 사용법: ./sandbox            위반 시 EPERM 을 돌려준다 (SCMP_ACT_ERRNO)
 *         ./sandbox kill       위반 시 Process 를 죽인다 (SCMP_ACT_KILL_PROCESS)
 */
#include <errno.h>
#include <fcntl.h>
#include <seccomp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int install(int kill_mode)
{
    /* 기본 동작: Allowlist 에 없는 모든 System Call */
    scmp_filter_ctx ctx = seccomp_init(kill_mode ? SCMP_ACT_KILL_PROCESS : SCMP_ACT_ERRNO(EPERM));
    int             rc = 0;

    if (ctx == NULL)
        return -1;
    rc |= seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(read), 0);
    rc |= seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(exit_group), 0);
    rc |= seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(rt_sigreturn), 0);
    rc |= seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(fstat), 0);
    rc |= seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(newfstatat), 0);
    /* 인자까지 검사: write 는 fd 1(stdout), 2(stderr) 에만 허용 */
    rc |= seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(write), 1, SCMP_A0(SCMP_CMP_LE, 2));
    if (rc == 0)
        rc = seccomp_load(ctx);                         /* BPF 로 Compile 해서 Kernel 에 설치 (NO_NEW_PRIVS 자동 설정) */
    seccomp_release(ctx);
    return rc;
}

int main(int argc, char *argv[])
{
    int kill_mode = (argc > 1 && strcmp(argv[1], "kill") == 0);
    int fd = open("sandbox.out", O_WRONLY | O_CREAT | O_TRUNC, 0644);      /* Filter 설치 전에 연 fd 3 */

    setvbuf(stdout, NULL, _IONBF, 0);
    printf("Mode: %s\n", kill_mode ? "KILL_PROCESS" : "ERRNO(EPERM)");
    if (install(kill_mode) < 0) {
        fprintf(stderr, "seccomp 설치 실패\n");
        return EXIT_FAILURE;
    }
    printf("1. write(stdout)            → 허용\n");
    if (write(fd, "x", 1) < 0)
        printf("2. write(fd 3, 이미 열린 File) → %s   ← 인자(fd) 검사에 걸림\n", strerror(errno));
    if (open("/etc/passwd", O_RDONLY) < 0)
        printf("3. open(\"/etc/passwd\")       → %s\n", strerror(errno));
    if (fork() < 0)
        printf("4. fork()                   → %s\n", strerror(errno));
    printf("5. 끝까지 실행됨\n");
    return EXIT_SUCCESS;
}
