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
#include <sys/types.h>
#include <string.h>
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
            
            // Create timestamp
            time_t now = time(NULL);
            struct tm *local = localtime(&now);
            char time_str[25];
            
            
            // get program name
            char* exe_name = basename(program_path);

            // get program heap info location
            char file_path[100] = "/home/uprhan/heapInfo/";
            strcat(file_path, exe_name);
            
            // Check directory exist, create if not
            struct stat st = {0};
            if (stat(file_path, &st) == -1) {
                if (mkdir(file_path, 0700) == -1) {
                    perror("Error creating directory\n");
                }
            }
            // parent path is now the folder to save all heap info for this executable


            strftime(time_str, sizeof(time_str), "/%Y_%m_%d_%H_%M_%S.txt", local);
            strcat(file_path, time_str);

            // write file
            outfile = fopen(file_path, "ab+");
            // TODO: file open failed
            fprintf(outfile, "%s\n", exe_name);
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
}


__attribute__((destructor))
void libclosed(void) {
    time_t now = time(NULL);
    struct tm *local = localtime(&now);
    char time_str[20];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", local);
    // outfile = fopen("/home/uprhan/hanglibc/tests/test_info.txt", "a");
    // if (outfile) {
    //     fprintf(outfile, "lib end at %s with process:%d.\n", time_str, getpid());
    //     fflush(outfile);
    //     fclose(outfile);
    // }
}