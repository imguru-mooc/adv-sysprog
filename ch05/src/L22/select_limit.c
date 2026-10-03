/* select_limit.c - select 의 fd_set 은 고정 크기 Bitmask 다: fd 번호가 FD_SETSIZE(1024) 이상이면 쓸 수 없다 */
#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <unistd.h>

int main(void)
{
    struct pollfd p;
    int           pfd[2], high, r;

    printf("FD_SETSIZE = %d, sizeof(fd_set) = %zu Byte\n", FD_SETSIZE, sizeof(fd_set));
    if (pipe(pfd) < 0)
        return EXIT_FAILURE;
    high = fcntl(pfd[0], F_DUPFD, 2000);            /* 2000 번 이상의 빈 번호로 복제 */
    if (high < 0) {
        perror("F_DUPFD (ulimit -n 이 2000 이하이면 실패합니다)");
        return EXIT_FAILURE;
    }
    printf("읽기용 fd = %d\n", high);
    printf("select: FD_SET(%d, &set) 은 fd_set 의 범위를 벗어난 Memory 를 건드립니다 (Buffer Overflow) → 사용 불가\n", high);

    if (write(pfd[1], "x", 1) != 1)
        return EXIT_FAILURE;
    p.fd = high;
    p.events = POLLIN;
    r = poll(&p, 1, 0);
    printf("poll  : poll() = %d, revents = 0x%x  → fd 번호에 제한이 없습니다\n", r, (unsigned)p.revents);
    return EXIT_SUCCESS;
}
