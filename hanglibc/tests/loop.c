#include <stdio.h>
#include "/home/uprhan/hanglibc/glibc/malloc/malloc.h"
#include <time.h>
#include <stdlib.h>
#include <unistd.h>



int main() {
    int count = 0;

    while (count < 1000) {
        void * x = malloc(1000);
        free(x);

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
        count++;
        sleep(5);
    }

    return 0;
}
