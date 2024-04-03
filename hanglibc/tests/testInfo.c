// #include <malloc.h>
#include "/home/uprhan/hanglibc/glibc/malloc/malloc.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main() {
    const int max_tcache_entries = 128;
    const size_t alloc_size = 64;

    size_t randtime = (size_t) (100 + rand() % (1000 - 100 + 1));;
    srand(time(NULL));


    void* ptrs[max_tcache_entries + 1];
    void* ptrs2[max_tcache_entries + 1];

    for (int i = 0; i < max_tcache_entries + 1; i++) {
        ptrs[i] = malloc(alloc_size);
    }

    for (int i = 0; i < max_tcache_entries + 1; i++) {
        ptrs2[i] = malloc(randtime);
    }


    for (int i = 0; i < max_tcache_entries + 1; i++) {
        free(ptrs[i]);
        free(ptrs2[i]);
    }

    malloc_info(0, stderr);

    fprintf(stderr, "===================================\n");

    struct malloc_state * ar_ptr = get_info(stdout);



    // void *handle;
    // int (*malloc_info_func)(int, FILE*);
    // char *error;

    

    // handle = dlopen("/lib/x86_64-linux-gnu/libm.so.6", RTLD_LAZY);
    // if (!handle) {
    //     fprintf(stderr, "1: %s\n", dlerror());
    //     exit(EXIT_FAILURE);
    // }

    // dlerror();

    // *(void **)(&malloc_info_func) = dlsym(handle, "malloc_info");

    // if ((error = dlerror()) != NULL) {
    //     fprintf(stderr, "%s\n", error);
    //     exit(EXIT_FAILURE);
    // }

    // if (malloc_info_func(0, stdout) != 0) {
    //     perror("malloc_info failed");
    //     exit(EXIT_FAILURE);
    // }


    
    // dlclose(handle);
    // exit(EXIT_SUCCESS);

    return 0;
}