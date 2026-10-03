/* fd_share.c - File Offset 은 fd 가 아니라 "open file description(struct file)" 에 있다
 *   dup()  : 같은 struct file 을 가리키는 fd 가 하나 더 생긴다 → Offset 공유
 *   open() : 새 struct file 이 만들어진다                       → Offset 독립
 */
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

static void show(const char *tag, int fd)
{
    printf("  %-10s fd=%d  offset=%ld\n", tag, fd, (long)lseek(fd, 0, SEEK_CUR));
}

int main(void)
{
    char  buf[8];
    int   a, b, c;
    pid_t pid;

    a = open("/etc/passwd", O_RDONLY);
    b = dup(a);                             /* 같은 struct file */
    c = open("/etc/passwd", O_RDONLY);      /* 새로운 struct file, 같은 inode */
    if (a < 0 || b < 0 || c < 0) {
        perror("open/dup");
        return EXIT_FAILURE;
    }

    if (read(a, buf, 5) != 5)
        return EXIT_FAILURE;
    printf("a 에서 5 Byte 를 읽은 뒤:\n");
    show("a", a);
    show("b = dup(a)", b);
    show("c = open()", c);

    fflush(stdout);
    pid = fork();                           /* fork: fd Table 복사 → 같은 struct file 을 가리킴 */
    if (pid == 0) {
        if (read(a, buf, 7) != 7)
            _exit(EXIT_FAILURE);
        _exit(EXIT_SUCCESS);
    }
    waitpid(pid, NULL, 0);
    printf("Child 가 a 에서 7 Byte 를 읽은 뒤 (Parent 에서 확인):\n");
    show("a", a);
    show("c", c);

    close(a);
    close(b);
    close(c);
    return EXIT_SUCCESS;
}
