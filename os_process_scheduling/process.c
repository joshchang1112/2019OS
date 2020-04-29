#define _GNU_SOURCE
#define _USE_GNU
#include <stdio.h>
#include <stdlib.h> 
#include <sys/types.h>
#include <unistd.h>
#include <sched.h>
const long long n = 1E9;
const int buffer_size = 1E2;

int main(int argc, char* argv[]){
    
    
    
    char* name = argv[1];
    pid_t pid = getpid();
    int exec_time;
    exec_time = strtol(argv[2], NULL, 10);
    long start_time = syscall(411);

    int i;
    for(i = 0; i < exec_time; i++){
        volatile unsigned long j;
        for(j = 0; j < 1000000UL; j++); 
    }
    
    long end_time = syscall(411);
    
    syscall(412, pid, start_time, end_time);
    return 0;
}
