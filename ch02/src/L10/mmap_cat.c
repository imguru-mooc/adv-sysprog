/* mmap_cat.c - read() 없이 File 내용을 출력한다: File 이 곧 배열이 된다 */
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    struct stat st;
    char       *p;
    size_t      i, lines = 0;
    int         fd;

    if (argc != 2) {
        fprintf(stderr, "사용법: %s <file>\n", argv[0]);
        return EXIT_FAILURE;
    }
    fd = open(argv[1], O_RDONLY);
    if (fd < 0 || fstat(fd, &st) < 0) {
        perror(argv[1]);
        return EXIT_FAILURE;
    }
    if (st.st_size == 0) {                  /* 길이 0 은 mmap 할 수 없다 (EINVAL) */
        close(fd);
        return EXIT_SUCCESS;
    }
    p = mmap(NULL, (size_t)st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (p == MAP_FAILED) {
        perror("mmap");
        return EXIT_FAILURE;
    }
    close(fd);                              /* Mapping 은 fd 와 독립적으로 유지된다 */

    for (i = 0; i < (size_t)st.st_size; i++)
        if (p[i] == '\n')
            lines++;
    if (write(STDOUT_FILENO, p, (size_t)st.st_size) < 0)
        perror("write");
    fprintf(stderr, "-- %zu lines, %ld bytes, mapped at %p\n", lines, (long)st.st_size, (void *)p);

    munmap(p, (size_t)st.st_size);
    return EXIT_SUCCESS;
}
