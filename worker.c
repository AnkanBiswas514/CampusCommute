#include "campuscompute.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int worker_run(int simulate_failure) {
    fflush(stdout);
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return -1;
    }

    if (p
        printf("[Worker] child started, pid=%d, ppid=%d\n", getpid(), getppid());
        fflush(stdout);

        if (simulate_failure) {
            
            printf("[Worker] simulating failure after work...\n");
            fflush(stdout);
            _exit(42); 
        } else {
            printf("[Worker] completing normally.\n");
            fflush(stdout);
            _exit(0);
        }
    }

    /
    int status;
    pid_t waited = waitpid(pid, &status, 0);
    if (waited < 0) {
        perror("waitpid");
        return -1;
    }

    printf("[Parent] waitpid() reaped child pid=%d\n", waited);
    if (WIFEXITED(status)) {
        int code = WEXITSTATUS(status);
        printf("[Parent] child exited normally with code %d\n", code);
        if (code != 0)
            printf("[Parent] detected SIMULATED WORKER FAILURE (non-zero exit %d).\n", code);
        else
            printf("[Parent] normal completion confirmed.\n");
        return code;
    } else if (WIFSIGNALED(status)) {
        int sig = WTERMSIG(status);
        printf("[Parent] child terminated by signal %d (abnormal).\n", sig);
        printf("[Parent] detected SIMULATED WORKER FAILURE (signal %d).\n", sig);
        return -sig;
    } else {
        printf("[Parent] child status unclear.\n");
        return -1;
    }
}
