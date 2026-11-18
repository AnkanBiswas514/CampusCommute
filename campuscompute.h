#ifndef CAMPUSCOMPUTE_H
#define CAMPUSCOMPUTE_H

/* CampusCompute: Intelligent Job Scheduling and Resource Management
 * Level 3 Mini-Project prototype (C on Linux/Ubuntu/WSL).
 *
 * This header declares the shared data structures and function prototypes
 * used across the modular source files.
 */

#include <stddef.h>

/* ---------- Common job representation (FR1, FR2) ---------- */

typedef enum {
    JOB_STATUS_PENDING = 0,
    JOB_STATUS_RUNNING,
    JOB_STATUS_COMPLETED
} job_status_t;

typedef struct {
    int    job_id;
    int    arrival_time;   /* time at which job becomes ready */
    int    cpu_burst;      /* total CPU time required */
    int    priority;       /* lower number = higher priority */
    int    remaining;      /* remaining burst (used by RR) */
    int    start_time;     /* time job first starts running */
    int    completion_time;
    int    turnaround_time;
    int    waiting_time;
    job_status_t status;
} job_t;

/* Scheduling result for one policy */
typedef struct {
    char    name[32];
    int     num_jobs;
    job_t  *jobs;          /* array of num_jobs (ordered by completion) */
    int    *exec_order;    /* sequence of job_ids executed */
    int     exec_len;      /* length of exec_order */
    double  avg_turnaround;
    double  avg_waiting;
} schedule_result_t;

/* ---------- Scheduler prototypes (FR2) ---------- */

schedule_result_t *scheduler_fcfs(const job_t *jobs, int n);
schedule_result_t *scheduler_round_robin(const job_t *jobs, int n, int quantum);
schedule_result_t *scheduler_priority(const job_t *jobs, int n);
void scheduler_print_result(const schedule_result_t *r);
void scheduler_free_result(schedule_result_t *r);

/* ---------- Concurrent activity logger (FR3) ---------- */

#define LOGGER_QUEUE_CAP 16
#define LOGGER_MAX_MSG   256

typedef struct {
    char  messages[LOGGER_QUEUE_CAP][LOGGER_MAX_MSG];
    int   head;
    int   tail;
    int   count;
    int   producers_done;
} logger_queue_t;

/* Starts producer/consumer threads and writes a log file.
 * Returns 0 on success, -1 on error. */
int logger_run(const char *logfile, int num_producers, int msgs_per_producer);

/* ---------- Shared-memory IPC (FR4) ---------- */

#define SHM_NAME "/campuscompute_shm"
#define SEM_NAME "/campuscompute_sem"
#define SHM_SIZE 4096

typedef struct {
    int  producer_id;
    int  value;
    int  sequence;
    int  done;             /* 1 when producer is finished */
} shm_record_t;

typedef struct {
    shm_record_t records[8];
    int          count;
} shm_region_t;

/* Producer process entry point (forked child) */
int ipc_producer_main(const char *logfile);

/* Consumer process entry point (forked child) */
int ipc_consumer_main(const char *logfile);

/* Runs the full producer-consumer IPC demonstration */
int ipc_run(const char *logfile);

/* ---------- Banker's algorithm (FR5) ---------- */

#define BANKER_MAX_PROCESSES 10
#define BANKER_MAX_RESOURCES 10

typedef struct {
    int n;                 /* number of processes */
    int m;                 /* number of resource types */
    int available[BANKER_MAX_RESOURCES];
    int maximum[BANKER_MAX_PROCESSES][BANKER_MAX_RESOURCES];
    int allocation[BANKER_MAX_PROCESSES][BANKER_MAX_RESOURCES];
    int need[BANKER_MAX_PROCESSES][BANKER_MAX_RESOURCES];
} banker_state_t;

/* Computes Need = Maximum - Allocation and checks safety.
 * Returns 1 if safe (fills safe_seq), 0 if unsafe. */
int banker_is_safe(const banker_state_t *bs, int *safe_seq);

/* Runs a built-in safe-state and an unsafe-state test. */
void banker_run_tests(void);

/* ---------- Worker failure detection (FR6) ---------- */

/* Forks a child. If simulate_failure is nonzero the child exits with
 * a non-zero status (simulated failure); otherwise it exits 0.
 * The parent collects status via waitpid() and reports results.
 * Returns child exit status (or -1 on fork/wait error). */
int worker_run(int simulate_failure);

/* ---------- Resource limits (FR7) ---------- */

/* Demonstrates setrlimit() on RLIMIT_CPU and reports observed effect.
 * Returns 0 on success, -1 on error. */
int limits_demo(void);

/* ---------- CLI (FR8) ---------- */

/* Entry point: parses argv and dispatches to the requested feature. */
int cli_run(int argc, char *argv[]);

#endif /* CAMPUSCOMPUTE_H */
