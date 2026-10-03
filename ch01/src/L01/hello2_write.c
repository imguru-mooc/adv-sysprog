/* hello2_write.c - glibc의 System Call Wrapper(write)로 출력한다 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void)
{
    const char msg[] = "hello, kernel\n";

    if (write(STDOUT_FILENO, msg, sizeof(msg) - 1) < 0) {
        perror("write");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
