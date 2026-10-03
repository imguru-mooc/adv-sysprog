/* inode_life.c - 이름(dentry)과 실체(inode)는 별개다
 * link, rename, unlink 를 거치며 inode 번호와 Link 수가 어떻게 변하는지 본다
 */
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

static void show(const char *tag, int fd)
{
    struct stat st;

    if (fstat(fd, &st) < 0) {
        perror("fstat");
        exit(EXIT_FAILURE);
    }
    printf("%-34s inode=%lu  nlink=%lu\n", tag, (unsigned long)st.st_ino, (unsigned long)st.st_nlink);
}

int main(void)
{
    char buf[32] = { 0 };
    int  fd = open("life.txt", O_RDWR | O_CREAT | O_TRUNC, 0644);

    if (fd < 0 || write(fd, "still alive\n", 12) != 12) {
        perror("life.txt");
        return EXIT_FAILURE;
    }
    show("1. 생성", fd);

    if (link("life.txt", "life2.txt") < 0)          /* 이름을 하나 더 붙인다 */
        perror("link");
    show("2. link(life.txt, life2.txt)", fd);

    if (rename("life2.txt", "life3.txt") < 0)       /* 이름만 바꾼다 */
        perror("rename");
    show("3. rename(life2 → life3)", fd);

    unlink("life.txt");
    unlink("life3.txt");                            /* 모든 이름을 지운다 */
    show("4. 이름을 모두 unlink", fd);
    printf("   access(\"life.txt\") = %d  (이름으로는 더 이상 찾을 수 없다)\n", access("life.txt", F_OK));

    if (pread(fd, buf, sizeof(buf) - 1, 0) < 0)     /* 그래도 열려 있는 fd 로는 읽힌다 */
        perror("pread");
    printf("   fd 로 읽기: %s", buf);

    close(fd);                                      /* 마지막 참조가 사라지는 순간 inode 와 Data 가 해제된다 */
    printf("5. close → 이제 실제로 삭제됨\n");
    return EXIT_SUCCESS;
}
