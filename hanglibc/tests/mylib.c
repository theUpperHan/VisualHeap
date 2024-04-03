// file: hi_lib.c
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include <time.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/stat.h>
// #include <malloc.h>
#include "/home/uprhan/hanglibc/glibc/malloc/malloc.h"

#define SIGHEAP 60

int count;
static FILE *outfile;
static char outfile_path[1024];


void signal_handler(int signum) {
    if (signum == SIGHEAP) {

        // Get program path
        char program_path[64];
        ssize_t len = readlink("/proc/self/exe", program_path, sizeof(program_path)-1);
        
        if (len != -1) {
            program_path[len] = '\0'; // terminate the string
            
            // get program name
            char* exe_name = basename(program_path);
            char* dir_path = dirname(program_path);

            // Create timestamp
            time_t now = time(NULL);
            struct tm *local = localtime(&now);
            char time_str[20];
            strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", local);
            
            // snprintf(outfile_path, sizeof(outfile_path), "%s_info.txt", dir_path, exe_name);
            snprintf(outfile_path, sizeof(outfile_path), "/home/uprhan/hanglibc/tests/test_info.txt");



            // write file
            outfile = fopen(outfile_path, "a");
            fprintf(outfile, "%s\n", time_str);
            get_info(outfile);
            fputc('\n', outfile);
            fflush(outfile);
            fclose(outfile);

        }
        else {
            perror("readlink");
        }

        count++;
    }
}


__attribute__((constructor))
void setup_signal_handler(void) {
    count = 0;
    signal(SIGHEAP, signal_handler);
    time_t now = time(NULL);
    struct tm *local = localtime(&now);
    char time_str[20];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d", local);
    fprintf(stdout, "%s\n", time_str);
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", local);
    fprintf(stdout, "%s\n", time_str);
}


__attribute__((destructor))
void libclosed(void) {
    time_t now = time(NULL);
    struct tm *local = localtime(&now);
    char time_str[20];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", local);
    outfile = fopen("/home/uprhan/hanglibc/tests/test_info.txt", "a");
    if (outfile) {
        fprintf(outfile, "lib end at %s with process:%d.\n", time_str, getpid());
        fflush(outfile);
        fclose(outfile);
    }
}