/* pause_write.c - gdb의 catch syscall 실습용. write 직전의 Register를 관찰한다 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void)
{
    const char msg[] = "REGISTER\n";

    if (write(STDOUT_FILENO, msg, sizeof(msg) - 1) < 0) {
        perror("write");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
