/* marker.c - Application 이 ftrace 의 Ring Buffer 에 직접 표식을 남긴다 (trace_marker)
 * Kernel Event 와 같은 Timeline 에 기록되므로 "내 코드의 이 구간에서 Kernel 은 무엇을 했는가" 를 볼 수 있다
 * 사용법: sudo ./trace_events.sh ./marker        (root 권한 필요)
 */
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int mfd = -1;

static void mark(const char *fmt, ...)
{
    char    buf[128];
    va_list ap;
    int     len;

    if (mfd < 0)
        return;
    va_start(ap, fmt);
    len = vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    if (len > 0 && write(mfd, buf, (size_t)len) < 0)    /* write 한 번 = trace 의 한 줄 */
        perror("trace_marker");
}

int main(void)
{
    char buf[4096];
    int  fd, i;

    mfd = open("/sys/kernel/tracing/trace_marker", O_WRONLY);
    if (mfd < 0)
        fprintf(stderr, "trace_marker 를 열 수 없습니다 (root 가 아니면 표식 없이 진행합니다)\n");

    mark("phase 1: write 시작");
    fd = open("marker.dat", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    memset(buf, 'm', sizeof(buf));
    for (i = 0; i < 4; i++)
        if (write(fd, buf, sizeof(buf)) < 0)
            return EXIT_FAILURE;
    mark("phase 2: fsync 시작");
    fsync(fd);
    mark("phase 3: sleep 시작");
    usleep(10 * 1000);
    mark("끝");
    close(fd);
    return EXIT_SUCCESS;
}
