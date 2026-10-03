/* capdemo.c - root 만 할 수 있다고 알려진 일 네 가지를 시도하고, 각각 어떤 Capability 가 필요한지 보여 준다
 * 사용법: ./capdemo                                   (일반 사용자: 전부 실패)
 *         sudo ./capdemo                              (root: 전부 성공)
 *         sudo setcap cap_sys_nice+ep ./capdemo; ./capdemo     (일반 사용자 + File Capability: 하나만 성공)
 */
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <unistd.h>

static void result(const char *what, const char *cap, int ok)
{
    printf("%-34s %-22s %s\n", what, cap, ok ? "성공" : strerror(errno));
}

int main(void)
{
    void *p;
    int   fd;

    printf("uid=%d euid=%d\n", (int)getuid(), (int)geteuid());
    printf("%-36s %-22s %s\n", "시도", "필요한 Capability", "결과");

    fd = open("/etc/shadow", O_RDONLY);                         /* 권한 검사를 건너뛰고 읽기 */
    result("/etc/shadow 읽기", "CAP_DAC_READ_SEARCH", fd >= 0);
    if (fd >= 0)
        close(fd);

    errno = 0;
    result("우선순위 올리기 nice(-5)", "CAP_SYS_NICE", setpriority(PRIO_PROCESS, 0, -5) == 0);

    fd = open("/tmp/capdemo.tmp", O_WRONLY | O_CREAT, 0600);
    if (fd >= 0) {
        result("File 소유자를 uid 1 로 변경", "CAP_CHOWN", fchown(fd, 1, (gid_t)-1) == 0);
        close(fd);
        unlink("/tmp/capdemo.tmp");
    }

    p = mmap(NULL, 256UL << 20, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    result("256MB 를 mlock (RLIMIT_MEMLOCK 초과)", "CAP_IPC_LOCK", p != MAP_FAILED && mlock(p, 256UL << 20) == 0);
    return EXIT_SUCCESS;
}
