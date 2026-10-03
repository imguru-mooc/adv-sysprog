/* heap_grow.c - 작은 malloc 은 brk(Heap), 큰 malloc 은 mmap 으로 처리됨을 확인한다 */
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* stdio(fopen, printf)는 내부에서 malloc 을 호출해 Heap 을 미리 만들어 버린다.
 * 관찰을 방해하지 않도록 System Call 만 사용해 출력한다. */
static void show(const char *tag)
{
    static char buf[16384];
    char        msg[128];
    char       *line, *next;
    ssize_t     n;
    int         len, found = 0;
    int         fd = open("/proc/self/maps", O_RDONLY);

    if (fd < 0)
        _exit(EXIT_FAILURE);
    n = read(fd, buf, sizeof(buf) - 1);
    close(fd);
    if (n < 0)
        _exit(EXIT_FAILURE);
    buf[n] = '\0';

    len = snprintf(msg, sizeof(msg), "== %s  (program break = %p)\n", tag, sbrk(0));
    if (write(STDOUT_FILENO, msg, (size_t)len) < 0)
        _exit(EXIT_FAILURE);
    for (line = buf; line != NULL && *line != '\0'; line = next) {
        next = strchr(line, '\n');
        if (next != NULL)
            *next++ = '\0';
        if (strstr(line, "[heap]") != NULL) {
            found = 1;
            len = snprintf(msg, sizeof(msg), "   %.100s\n", line);
            if (write(STDOUT_FILENO, msg, (size_t)len) < 0)
                _exit(EXIT_FAILURE);
        }
    }
    if (!found && write(STDOUT_FILENO, "   ([heap] 없음)\n", 19) < 0)
        _exit(EXIT_FAILURE);
}

static void say(const char *tag, void *p)
{
    char msg[128];
    int  len = snprintf(msg, sizeof(msg), "   %s = %p\n", tag, p);

    if (write(STDOUT_FILENO, msg, (size_t)len) < 0)
        _exit(EXIT_FAILURE);
}

int main(void)
{
    char *small, *big;

    show("start");

    small = malloc(1000);                   /* 작은 요청: brk 로 Heap 확장 */
    if (small == NULL)
        return EXIT_FAILURE;
    show("after malloc(1000)");
    say("small", (void *)small);

    big = malloc(4 * 1024 * 1024);          /* 큰 요청(128KB 이상): mmap */
    if (big == NULL)
        return EXIT_FAILURE;
    show("after malloc(4MB)");
    say("big  ", (void *)big);             /* [heap] 범위 밖의 주소 */

    free(big);                              /* munmap 으로 즉시 반환 */
    free(small);
    return EXIT_SUCCESS;
}
