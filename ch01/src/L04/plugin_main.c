/* plugin_main.c - 실행 중에 Library 를 직접 Load 한다 (dlopen) */
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    void *handle;
    void (*fn)(const char *);

    handle = dlopen("./libgreet.so", RTLD_NOW);
    if (handle == NULL) {
        fprintf(stderr, "dlopen: %s\n", dlerror());
        return EXIT_FAILURE;
    }

    *(void **)(&fn) = dlsym(handle, "greet");
    if (fn == NULL) {
        fprintf(stderr, "dlsym: %s\n", dlerror());
        dlclose(handle);
        return EXIT_FAILURE;
    }

    fn("dlopen");
    dlclose(handle);
    return EXIT_SUCCESS;
}
