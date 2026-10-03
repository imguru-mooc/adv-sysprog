/* ctor.c - main 앞뒤에서 실행되는 코드: constructor, destructor, atexit */
#include <stdio.h>
#include <stdlib.h>

__attribute__((constructor)) static void before_main(void)
{
    printf("1. constructor  (main 이전, __libc_start_main 이 호출)\n");
}

__attribute__((destructor)) static void after_main(void)
{
    printf("4. destructor   (exit 처리 중)\n");
}

static void on_exit_handler(void)
{
    printf("3. atexit       (exit 처리 중, 등록의 역순)\n");
}

int main(void)
{
    if (atexit(on_exit_handler) != 0) {
        perror("atexit");
        return EXIT_FAILURE;
    }
    printf("2. main\n");
    return EXIT_SUCCESS;        /* return 값은 __libc_start_main 이 exit() 로 넘긴다 */
}
