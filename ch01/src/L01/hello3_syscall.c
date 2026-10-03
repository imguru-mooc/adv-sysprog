/* hello3_syscall.c - Wrapper 없이 System Call 번호로 직접 호출한다 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <unistd.h>

int main(void)
{
    const char msg[] = "hello, kernel\n";

    /* SYS_write 는 x86-64에서 1 이다 */
    if (syscall(SYS_write, STDOUT_FILENO, msg, sizeof(msg) - 1) < 0) {
        perror("syscall(SYS_write)");
        return EXIT_FAILURE;
    }
    printf("SYS_write = %d\n", SYS_write);
    return EXIT_SUCCESS;
}
