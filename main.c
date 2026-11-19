#include "campuscompute.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static const job_t DEFAULT_JOBS[] = {
    {1, 0,  7, 3, 7, 0,0,0,0, JOB_STATUS_PENDING},
    {2, 2,  4, 1, 4, 0,0,0,0, JOB_STATUS_PENDING},
    {3, 4,  1, 4, 1, 0,0,0,0, JOB_STATUS_PENDING},
    {4, 5,  5, 2, 5, 0,0,0,0, JOB_STATUS_PENDING},
    {5, 6,  3, 5, 3, 0,0,0,0, JOB_STATUS_PENDING},
};
#define NJOBS (sizeof(DEFAULT_JOBS) / sizeof(DEFAULT_JOBS[0]))

static void print_usage(const char *prog) {
    printf("CampusCompute - Intelligent Job Scheduling and Resource Management\n");
    printf("Usage: %s [command] [args...]\n\n", prog);
    printf("Commands:\n");
    printf("  %-14s Run interactive menu\n", "");
    printf("  %-14s First-Come First-Served scheduling\n", "fcfs");
    printf("  %-14s Round Robin (default quantum=2)\n", "rr [quantum]");
    printf("  %-14s Non-preemptive Priority scheduling\n", "priority");
    printf("  %-14s Run FCFS, RR, Priority + comparison\n", "schedule");
    printf("  %-14s Producer-consumer logger (pthreads)\n", "logger [logfile] [producers] [msgs]");
    printf("  %-14s Shared-memory + named semaphore IPC\n", "ipc [logfile]");
    printf("  %-14s Banker's safety algorithm tests\n", "banker");
    printf("  %-14s Worker failure detection (fork/waitpid)\n", "worker [0|1]");
    printf("  %-14s RLIMIT_CPU demonstration\n", "limits");
    printf("  %-14s Run every feature in sequence\n", "all");
    printf("  %-14s Show this help\n", "help");
    printf("\nFR1/FR2 fields: job ID, arrival time, CPU burst, priority, status.\n");
}

static void run_schedule_all(void) {
    printf("\n########################################\n");
    printf("# Scheduling comparison (same job set) #\n");
    printf("########################################\n");

    schedule_result_t *r;

    r = scheduler_fcfs(DEFAULT_JOBS, NJOBS);
    scheduler_print_result(r);
    scheduler_free_result(r);

    r = scheduler_round_robin(DEFAULT_JOBS, NJOBS, 2);
    scheduler_print_result(r);
    scheduler_free_result(r);

    r = scheduler_priority(DEFAULT_JOBS, NJOBS);
    scheduler_print_result(r);
    scheduler_free_result(r);

    printf("\n--- Comparison (avg turnaround / avg waiting) ---\n");
    printf("FCFS        : -\n");  
    printf("RR (q=2)    : -\n");
    printf("Priority    : -\n");
    printf("(See per-policy tables above for exact averages.)\n");
}

static void run_logger(int argc, char *argv[]) {
    const char *logfile = "activity.log";
    int producers = 2;
    int msgs = 5;
    if (argc > 2) logfile = argv[2];
    if (argc > 3) producers = atoi(argv[3]);
    if (argc > 4) msgs = atoi(argv[4]);
    if (producers <= 0) producers = 2;
    if (msgs <= 0) msgs = 5;
    printf("[Logger] producers=%d msgs_each=%d logfile=%s\n",
           producers, msgs, logfile);
    logger_run(logfile, producers, msgs);
}

static void run_ipc(int argc, char *argv[]) {
    const char *logfile = "ipc.log";
    if (argc > 2) logfile = argv[2];
    printf("[IPC] logfile=%s\n", logfile);
    ipc_run(logfile);
}

static void run_worker(int argc, char *argv[]) {
    int simulate_failure = 0;
    if (argc > 2) simulate_failure = atoi(argv[2]) ? 1 : 0;
    printf("[Worker] simulate_failure=%d\n", simulate_failure);
    worker_run(simulate_failure);
}

static int interactive_menu(void) {
    char line[256];
    for (;;) {
        printf("\nCampusCompute menu:\n");
        printf("  1) fcfs    2) rr    3) priority  4) schedule(all)\n");
        printf("  5) logger  6) ipc   7) banker    8) worker\n");
        printf("  9) limits  10) all  0) exit\n");
        printf("> ");
        if (!fgets(line, sizeof(line), stdin)) break;
        
        size_t len = strlen(line);
        while (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r'))
            line[--len] = '\0';

        if (len == 0 || strcmp(line, "0") == 0 || strcmp(line, "exit") == 0) {
            printf("Goodbye.\n");
            break;
        } else if (strcmp(line, "1") == 0 || strcmp(line, "fcfs") == 0) {
            scheduler_print_result(scheduler_fcfs(DEFAULT_JOBS, NJOBS));
        } else if (strcmp(line, "2") == 0 || strcmp(line, "rr") == 0) {
            scheduler_print_result(scheduler_round_robin(DEFAULT_JOBS, NJOBS, 2));
        } else if (strcmp(line, "3") == 0 || strcmp(line, "priority") == 0) {
            scheduler_print_result(scheduler_priority(DEFAULT_JOBS, NJOBS));
        } else if (strcmp(line, "4") == 0 || strcmp(line, "schedule") == 0 ||
                   strcmp(line, "all") == 0) {
            run_schedule_all();
        } else if (strcmp(line, "5") == 0 || strcmp(line, "logger") == 0) {
            run_logger(0, NULL);
        } else if (strcmp(line, "6") == 0 || strcmp(line, "ipc") == 0) {
            run_ipc(0, NULL);
        } else if (strcmp(line, "7") == 0 || strcmp(line, "banker") == 0) {
            banker_run_tests();
        } else if (strcmp(line, "8") == 0 || strcmp(line, "worker") == 0) {
            worker_run(0);
        } else if (strcmp(line, "9") == 0 || strcmp(line, "limits") == 0) {
            limits_demo();
        } else if (strcmp(line, "help") == 0) {
            print_usage("campuscompute");
        } else {
            printf("Unknown command: '%s'. Type 'help' for usage.\n", line);
        }
    }
    return 0;
}

int cli_run(int argc, char *argv[]) {
    if (argc < 2) {
        print_usage("campuscompute");
        return interactive_menu();
    }

    const char *cmd = argv[1];

    if (strcmp(cmd, "help") == 0 || strcmp(cmd, "--help") == 0 ||
        strcmp(cmd, "-h") == 0) {
        print_usage("campuscompute");
        return 0;
    } else if (strcmp(cmd, "fcfs") == 0) {
        scheduler_print_result(scheduler_fcfs(DEFAULT_JOBS, NJOBS));
    } else if (strcmp(cmd, "rr") == 0) {
        int quantum = (argc > 2) ? atoi(argv[2]) : 2;
        if (quantum <= 0) quantum = 2;
        scheduler_print_result(scheduler_round_robin(DEFAULT_JOBS, NJOBS, quantum));
    } else if (strcmp(cmd, "priority") == 0) {
        scheduler_print_result(scheduler_priority(DEFAULT_JOBS, NJOBS));
    } else if (strcmp(cmd, "schedule") == 0 || strcmp(cmd, "all") == 0) {
        run_schedule_all();
        if (strcmp(cmd, "all") == 0) {
            run_logger(argc, argv);
            run_ipc(argc, argv);
            banker_run_tests();
            worker_run(0);
            worker_run(1);
            limits_demo();
        }
    } else if (strcmp(cmd, "logger") == 0) {
        run_logger(argc, argv);
    } else if (strcmp(cmd, "ipc") == 0) {
        run_ipc(argc, argv);
    } else if (strcmp(cmd, "banker") == 0) {
        banker_run_tests();
    } else if (strcmp(cmd, "worker") == 0) {
        run_worker(argc, argv);
    } else if (strcmp(cmd, "limits") == 0) {
        limits_demo();
    } else {
        printf("Unknown command: '%s'\n\n", cmd);
        print_usage("campuscompute");
        return 1;
    }
    return 0;
}

int main(int argc, char *argv[]) {
    return cli_run(argc, argv);
}
