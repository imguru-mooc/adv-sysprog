/* stack_layout.c - main 이 받는 argc/argv/envp 와 그 뒤의 auxv 가 Stack 에 연속으로 놓여 있음을 확인한다 */
#include <elf.h>
#include <stdio.h>

int main(int argc, char *argv[], char *envp[])
{
    char **p;
    Elf64_auxv_t *aux;
    int n = 0;

    printf("argc        = %d\n", argc);
    printf("argv        = %p   argv[0] = \"%s\"\n", (void *)argv, argv[0]);
    printf("envp        = %p   (argv 바로 뒤: NULL 하나 건너)\n", (void *)envp);

    for (p = envp; *p != NULL; p++)
        n++;
    printf("env count   = %d\n", n);

    aux = (Elf64_auxv_t *)(p + 1);          /* envp 의 NULL 다음이 auxv */
    printf("auxv        = %p\n", (void *)aux);
    for (; aux->a_type != AT_NULL; aux++) {
        if (aux->a_type == AT_ENTRY)
            printf("  AT_ENTRY  = 0x%lx\n", (unsigned long)aux->a_un.a_val);
        if (aux->a_type == AT_BASE)
            printf("  AT_BASE   = 0x%lx\n", (unsigned long)aux->a_un.a_val);
    }
    return 0;
}
