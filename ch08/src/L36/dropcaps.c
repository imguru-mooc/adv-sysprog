/* dropcaps.c - Least Privilege: root 로 시작해 초기화를 마친 뒤, 필요한 Capability 하나만 남기고 일반 사용자가 된다
 * 사용법: sudo ./dropcaps
 */
#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/capability.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <grp.h>
#include <unistd.h>

#define RUN_AS 65534                                    /* nobody */

static void try_ops(const char *tag)
{
    int fd = open("/etc/shadow", O_RDONLY);
    int nice_ok;

    errno = 0;
    nice_ok = setpriority(PRIO_PROCESS, 0, -5) == 0;
    printf("%-30s uid=%-5d /etc/shadow: %-22s nice(-5): %s\n", tag, (int)getuid(),
           fd >= 0 ? "읽힘" : "Permission denied", nice_ok ? "성공" : "Permission denied");
    if (fd >= 0)
        close(fd);
    setpriority(PRIO_PROCESS, 0, 0);
}

int main(void)
{
    cap_value_t keep[] = { CAP_SYS_NICE };
    cap_t       caps;

    if (geteuid() != 0) {
        fprintf(stderr, "root 로 실행하세요: sudo %s\n", "./dropcaps");
        return EXIT_FAILURE;
    }
    try_ops("1. root (초기화 단계)");

    /* 2. uid 를 바꿔도 Permitted 집합을 유지하도록 표시한다 (기본은 uid 0 을 떠나는 순간 전부 사라진다) */
    if (prctl(PR_SET_KEEPCAPS, 1) < 0)
        perror("PR_SET_KEEPCAPS");

    /* 3. 일반 사용자가 된다. 순서: 보조 Group → gid → uid */
    if (setgroups(0, NULL) < 0 || setresgid(RUN_AS, RUN_AS, RUN_AS) < 0 || setresuid(RUN_AS, RUN_AS, RUN_AS) < 0) {
        perror("setres[ug]id");
        return EXIT_FAILURE;
    }

    /* 4. CAP_SYS_NICE 하나만 남기고 전부 버린다 */
    caps = cap_init();                                  /* 빈 집합에서 시작 */
    cap_set_flag(caps, CAP_PERMITTED, 1, keep, CAP_SET);
    cap_set_flag(caps, CAP_EFFECTIVE, 1, keep, CAP_SET);
    if (cap_set_proc(caps) < 0) {
        perror("cap_set_proc");
        return EXIT_FAILURE;
    }
    cap_free(caps);

    /* 5. 이후 어떤 방법으로도(setuid 실행 File, File Capability) 권한을 다시 얻지 못하게 한다 */
    prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0);

    try_ops("2. nobody + CAP_SYS_NICE");

    caps = cap_get_proc();
    printf("현재 Capability: %s\n", cap_to_text(caps, NULL));
    cap_free(caps);

    if (setresuid(0, 0, 0) < 0)
        printf("3. root 로 돌아가기: %s  ← 되돌릴 수 없다\n", strerror(errno));
    return EXIT_SUCCESS;
}
