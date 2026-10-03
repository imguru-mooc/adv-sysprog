/* mini_container.c - clone() 의 Flag 몇 개로 만드는 가장 작은 "Container"
 *   새 UTS(hostname), PID, Mount, IPC Namespace 안에서 명령을 실행한다
 * 사용법: sudo ./mini_container [명령 ...]        기본: /bin/sh
 */
#define _GNU_SOURCE
#include <sched.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/wait.h>
#include <unistd.h>

#define STACK_SIZE (1024 * 1024)

static int child_main(void *arg)
{
    char **argv = arg;

    /* 새 UTS Namespace: hostname 을 바꿔도 Host 에는 영향이 없다 */
    if (sethostname("container", 9) < 0)
        perror("sethostname");

    /* 새 Mount Namespace: 여기서의 mount 가 Host 로 전파되지 않게 한다 */
    if (mount(NULL, "/", NULL, MS_REC | MS_PRIVATE, NULL) < 0)
        perror("mount(MS_PRIVATE)");

    /* 새 PID Namespace 의 /proc 을 Mount: ps 가 이 Namespace 의 Process 만 보게 된다 */
    if (mount("proc", "/proc", "proc", MS_NOSUID | MS_NODEV | MS_NOEXEC, NULL) < 0)
        perror("mount(/proc)");

    printf("[container] pid = %d, ppid = %d, hostname = container\n", (int)getpid(), (int)getppid());
    fflush(stdout);
    execvp(argv[0], argv);
    perror("execvp");
    return EXIT_FAILURE;
}

int main(int argc, char *argv[])
{
    char *default_cmd[] = { "/bin/sh", NULL };
    char *stack = malloc(STACK_SIZE);
    pid_t pid;
    int   status;

    if (stack == NULL)
        return EXIT_FAILURE;
    pid = clone(child_main, stack + STACK_SIZE,                 /* Stack 은 아래로 자라므로 끝 주소를 준다 */
                CLONE_NEWUTS | CLONE_NEWPID | CLONE_NEWNS | CLONE_NEWIPC | SIGCHLD,
                argc > 1 ? &argv[1] : default_cmd);
    if (pid < 0) {
        perror("clone (root 권한이 필요합니다)");
        return EXIT_FAILURE;
    }
    printf("[host]      Child 의 pid = %d  ← 같은 Process 를 Host 에서 본 번호\n", (int)pid);
    waitpid(pid, &status, 0);
    printf("[host]      Container 종료. Host 의 hostname 은 그대로: ");
    fflush(stdout);
    execlp("hostname", "hostname", (char *)NULL);
    return EXIT_SUCCESS;
}
