/* mmap_shared.c - MAP_SHARED: Memory 에 쓰면 File 이 바뀐다 */
#include <ctype.h>
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
    off_t       i;
    int         fd = open("note.txt", O_RDWR);      /* 쓰기용 MAP_SHARED 는 O_RDWR 필요 */

    if (fd < 0 || fstat(fd, &st) < 0) {
        perror("note.txt (먼저: echo hello mmap world > note.txt)");
        return EXIT_FAILURE;
    }
    p = mmap(NULL, (size_t)st.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (p == MAP_FAILED) {
        perror("mmap");
        return EXIT_FAILURE;
    }
    close(fd);

    for (i = 0; i < st.st_size; i++)                /* write() 호출이 없다 */
        p[i] = (char)toupper((unsigned char)p[i]);

    if (msync(p, (size_t)st.st_size, MS_SYNC) < 0)  /* Disk 까지 반영을 기다린다 */
        perror("msync");
    munmap(p, (size_t)st.st_size);
    printf("Memory 에서 대문자로 변경했습니다. cat note.txt 로 확인하세요.\n");
    return EXIT_SUCCESS;
}
