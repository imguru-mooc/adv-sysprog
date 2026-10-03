/* segv_info.c - 잘못된 접근의 Page Fault 는 SIGSEGV 가 된다. si_code 로 원인을 구분한다
 * 사용법: ./segv_info null     Mapping 이 없는 주소 접근      -> SEGV_MAPERR
 *         ./segv_info ro       읽기 전용 영역(.rodata)에 쓰기  -> SEGV_ACCERR
 */
#define _POSIX_C_SOURCE 200809L
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static const char ro_data[] = "read only";

static void handler(int sig, siginfo_t *si, void *ctx)
{
    char msg[160];
    int  len;

    (void)sig;
    (void)ctx;
    len = snprintf(msg, sizeof(msg), "SIGSEGV  si_addr=%p  si_code=%d (%s)\n", si->si_addr, si->si_code,
                   si->si_code == SEGV_MAPERR ? "SEGV_MAPERR: 그 주소를 포함하는 VMA 가 없음"
                 : si->si_code == SEGV_ACCERR ? "SEGV_ACCERR: VMA 는 있지만 권한이 없음"
                 : "기타");
    if (write(STDERR_FILENO, msg, (size_t)len) < 0)
        _exit(2);
    _exit(EXIT_FAILURE);                    /* return 하면 같은 명령이 다시 실행되어 무한 반복된다 */
}

int main(int argc, char *argv[])
{
    struct sigaction sa;
    volatile char   *p;

    memset(&sa, 0, sizeof(sa));
    sa.sa_sigaction = handler;
    sa.sa_flags     = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGSEGV, &sa, NULL) < 0) {
        perror("sigaction");
        return EXIT_FAILURE;
    }

    if (argc > 1 && strcmp(argv[1], "ro") == 0) {
        p = (volatile char *)(unsigned long)ro_data;
        printf("읽기 전용 영역 %p 에 씁니다\n", (void *)(unsigned long)p);
    } else {
        p = (volatile char *)0x10;
        printf("Mapping 이 없는 주소 %p 에 씁니다\n", (void *)(unsigned long)p);
    }
    fflush(stdout);
    *p = 'X';
    printf("이 줄은 출력되지 않습니다\n");
    return EXIT_SUCCESS;
}
