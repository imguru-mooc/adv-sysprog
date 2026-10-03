/* anon_shared.c - MAP_SHARED | MAP_ANONYMOUS: File 없이 Parent 와 Child 가 공유하는 Memory */
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void)
{
    int   private_counter = 0;
    int  *shared_counter  = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE,
                                 MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    pid_t pid;

    if (shared_counter == MAP_FAILED) {
        perror("mmap");
        return EXIT_FAILURE;
    }
    *shared_counter = 0;

    pid = fork();
    if (pid < 0) {
        perror("fork");
        return EXIT_FAILURE;
    }
    if (pid == 0) {
        private_counter += 100;                     /* COW: Child 만의 복사본 */
        *shared_counter += 100;                     /* 공유: 같은 Physical Page */
        _exit(EXIT_SUCCESS);
    }
    if (waitpid(pid, NULL, 0) < 0)
        perror("waitpid");
    printf("private_counter = %d   (일반 변수: fork 후 각자의 복사본)\n", private_counter);
    printf("shared_counter  = %d   (MAP_SHARED: Child 의 쓰기가 보인다)\n", *shared_counter);
    munmap(shared_counter, sizeof(int));
    return EXIT_SUCCESS;
}
