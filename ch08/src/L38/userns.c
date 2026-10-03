/* userns.c - User Namespace: root 권한 없이 "Namespace 안의 root" 가 된다
 * 사용법: ./userns           (일반 사용자로 실행)
 */
#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void write_file(const char *path, const char *text)
{
    int fd = open(path, O_WRONLY);

    if (fd < 0 || write(fd, text, strlen(text)) < 0)
        fprintf(stderr, "%s: %s\n", path, strerror(errno));
    if (fd >= 0)
        close(fd);
}

int main(void)
{
    char map[64];
    uid_t uid = getuid();
    gid_t gid = getgid();
    int   fd;

    printf("밖: uid=%d\n", (int)uid);
    if (unshare(CLONE_NEWUSER | CLONE_NEWUTS) < 0) {            /* root 가 아니어도 된다 */
        perror("unshare(CLONE_NEWUSER)");
        return EXIT_FAILURE;
    }
    /* Namespace 안의 uid 0 ↔ 밖의 내 uid 를 대응시킨다 */
    snprintf(map, sizeof(map), "0 %d 1", (int)uid);
    write_file("/proc/self/uid_map", map);
    write_file("/proc/self/setgroups", "deny");
    snprintf(map, sizeof(map), "0 %d 1", (int)gid);
    write_file("/proc/self/gid_map", map);

    printf("안: uid=%d  ← Namespace 안에서는 root\n", (int)getuid());

    if (sethostname("my-userns", 9) == 0)
        printf("    sethostname: 성공  ← 이 User Namespace 가 소유한 UTS Namespace 에 대해서는 CAP_SYS_ADMIN 이 있다\n");
    else
        printf("    sethostname: %s\n", strerror(errno));

    fd = open("/etc/shadow", O_RDONLY);
    printf("    /etc/shadow 읽기: %s  ← Host 의 자원에 대해서는 여전히 원래 사용자\n", fd >= 0 ? "성공" : strerror(errno));
    return EXIT_SUCCESS;
}
