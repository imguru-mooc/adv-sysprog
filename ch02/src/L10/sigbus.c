/* sigbus.c - Mapping 은 유효하지만 뒤에 있는 File 이 사라지면 SIGBUS 가 발생한다 */
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

int main(void)
{
    volatile char *p;
    int            fd = open("bus.dat", O_RDWR | O_CREAT | O_TRUNC, 0644);

    if (fd < 0 || ftruncate(fd, 8192) < 0) {        /* 2 Pages 크기의 File */
        perror("bus.dat");
        return EXIT_FAILURE;
    }
    p = mmap(NULL, 8192, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (p == MAP_FAILED) {
        perror("mmap");
        return EXIT_FAILURE;
    }

    p[4096] = 'A';
    printf("1. 두 번째 Page 에 쓰기 성공\n");

    if (ftruncate(fd, 4096) < 0)                    /* File 을 1 Page 로 줄인다 */
        perror("ftruncate");
    printf("2. File 을 4096 Bytes 로 줄였습니다. Mapping(8192)은 그대로입니다\n");
    fflush(stdout);

    p[4096] = 'B';                                  /* VMA 안이지만 File 밖 -> SIGBUS */
    printf("이 줄은 출력되지 않습니다\n");
    return EXIT_SUCCESS;
}
