#include "campuscompute.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>

int limits_demo(void) {
    struct rlimit rl;

  
    if (getrlimit(RLIMIT_CPU, &rl) != 0) {
        perror("getrlimit RLIMIT_CPU");
        return -1;
    }
    printf("[Limits] RLIMIT_CPU before: ");
    if (rl.rlim_cur == RLIM_INFINITY) printf("unlimited\n");
    else printf("%ld seconds\n", (long)rl.rlim_cur);

  
    struct rlimit new_lim;
    new_lim.rlim_cur = 1;   
    new_lim.rlim_max = RLIM_INFINITY;
    if (setrlimit(RLIMIT_CPU, &new_lim) != 0) {
        perror("setrlimit RLIMIT_CPU");
        return -1;
    }
    printf("[Limits] RLIMIT_CPU soft limit set to 1 second.\n");

    fflush(stdout);
    pid_t pid = fork();
    if (pid < 0) { perror("fork"); return -1; }

    if (pid == 0) {
        
        printf("[Child] spinning to burn CPU (limit = 1s)...\n");
        fflush(stdout);
        volatile unsigned long sink = 0;
        for (unsigned long i = 0; i < 1000000000UL; i++) sink += i;
        printf("[Child] finished before limit (unexpected).\n");
        _exit(0);
    }

    int status;
    pid_t waited = waitpid(pid, &status, 0);
    printf("[Parent] waitpid reaped pid=%d\n", waited);
    if (WIFSIGNALED(status)) {
        int sig = WTERMSIG(status);
        printf("[Observed effect] Child terminated by signal %d (SIGXCPU=%d)\n",
               sig, SIGXCPU);
        printf("The RLIMIT_CPU soft limit caused the kernel to send SIGXCPU\n");
        printf("once the child exceeded 1 second of CPU time.\n");
    } else if (WIFEXITED(status)) {
        printf("[Observed effect] Child exited with code %d (finished early).\n",
               WEXITSTATUS(status));
    }
    fflush(stdout);

    
    if (setrlimit(RLIMIT_CPU, &rl) != 0) perror("setrlimit restore");
    printf("[Limits] original RLIMIT_CPU restored.\n");
    return 0;
}
