/* same_api.c - 전혀 다른 네 가지 대상을 똑같은 read() 로 읽는다 */
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

static const char *kind(mode_t m)
{
    if (S_ISREG(m))  return "regular file";
    if (S_ISCHR(m))  return "character device";
    if (S_ISFIFO(m)) return "pipe / FIFO";
    if (S_ISDIR(m))  return "directory";
    return "other";
}

static void read_some(const char *name, int fd)
{
    unsigned char buf[16];
    struct stat   st;
    ssize_t       n, i;

    if (fstat(fd, &st) < 0) {
        perror("fstat");
        return;
    }
    n = read(fd, buf, sizeof(buf));         /* 어떤 대상이든 호출은 동일하다 */
    printf("%-14s %-17s size=%-6ld read()=%2zd : ", name, kind(st.st_mode), (long)st.st_size, n);
    for (i = 0; i < n; i++)
        putchar(buf[i] >= 32 && buf[i] < 127 ? buf[i] : '.');
    putchar('\n');
}

int main(void)
{
    int pfd[2];
    int fd;

    if ((fd = open("/etc/hostname", O_RDONLY)) >= 0) {      /* Disk 위의 File (ext4) */
        read_some("/etc/hostname", fd);
        close(fd);
    }
    if ((fd = open("/proc/uptime", O_RDONLY)) >= 0) {       /* Kernel 이 읽는 순간 만들어 내는 내용 (procfs) */
        read_some("/proc/uptime", fd);
        close(fd);
    }
    if ((fd = open("/dev/urandom", O_RDONLY)) >= 0) {       /* Device Driver */
        read_some("/dev/urandom", fd);
        close(fd);
    }
    if (pipe(pfd) == 0) {                                   /* Kernel Memory 안의 Buffer (pipefs) */
        if (write(pfd[1], "via pipe", 8) == 8)
            read_some("pipe", pfd[0]);
        close(pfd[0]);
        close(pfd[1]);
    }
    return EXIT_SUCCESS;
}
