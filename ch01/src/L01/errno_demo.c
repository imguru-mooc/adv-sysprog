/* errno_demo.c - Kernel의 오류가 errno로 전달되는 과정을 확인한다 */
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(void)
{
    ssize_t n;

    errno = 0;
    n = write(99, "x", 1);      /* 열려 있지 않은 fd */

    printf("return = %zd\n", n);
    printf("errno  = %d (%s)\n", errno, strerror(errno));
    return 0;
}
