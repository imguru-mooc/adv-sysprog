/* mmap_private.c - MAP_PRIVATE: Memory 에 써도 File 은 바뀌지 않는다 (Copy-On-Write) */
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

int main(void)
{
    struct stat st;
    char       *p;
    int         fd = open("note.txt", O_RDONLY);    /* O_RDONLY 로 열어도 PROT_WRITE 가능 */

    if (fd < 0 || fstat(fd, &st) < 0) {
        perror("note.txt");
        return EXIT_FAILURE;
    }
    p = mmap(NULL, (size_t)st.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    if (p == MAP_FAILED) {
        perror("mmap");
        return EXIT_FAILURE;
    }
    close(fd);

    p[0] = '#';                                     /* 이 순간 Page 가 복사된다 */
    printf("Memory : %.*s", (int)st.st_size, p);
    printf("File 은 그대로입니다. cat note.txt 로 확인하세요.\n");
    munmap(p, (size_t)st.st_size);
    return EXIT_SUCCESS;
}
