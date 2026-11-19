#include "campuscompute.h"

#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static int shm_fd = -1;
static sem_t *sem = SEM_FAILED;
static shm_region_t *region = NULL;
static int cleanup_done = 0;


static void ipc_child_close(void) {
    if (region) { munmap(region, SHM_SIZE); region = NULL; }
    if (shm_fd >= 0) { close(shm_fd); shm_fd = -1; }
    if (sem != SEM_FAILED) { sem_close(sem); sem = SEM_FAILED; }
}


static void ipc_cleanup(void) {
    if (cleanup_done) return;
    cleanup_done = 1;
    ipc_child_close();
    shm_unlink(SHM_NAME);
    sem_unlink(SEM_NAME);
}


int ipc_producer_main(const char *logfile) {
    (void)logfile;
    
    shm_unlink(SHM_NAME);
    sem_unlink(SEM_NAME);
    shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd < 0) { perror("producer shm_open"); return -1; }
    if (ftruncate(shm_fd, SHM_SIZE) < 0) { perror("producer ftruncate"); return -1; }

    region = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (region == MAP_FAILED) { perror("producer mmap"); return -1; }

    sem = sem_open(SEM_NAME, O_CREAT, 0666, 1);
    if (sem == SEM_FAILED) { perror("producer sem_open"); return -1; }

    memset(region, 0, SHM_SIZE);
    region->count = 0;

    for (int i = 0; i < 5; i++) {
        sem_wait(sem); 
        int slot = region->count % 8;
        region->records[slot].producer_id = (int)getpid();
        region->records[slot].value = (i + 1) * 10;
        region->records[slot].sequence = i + 1;
        region->records[slot].done = 0;
        region->count++;
        sem_post(sem);

        printf("[Producer] wrote record seq=%d value=%d pid=%d\n",
               region->records[slot].sequence,
               region->records[slot].value,
               region->records[slot].producer_id);
        fflush(stdout);
        usleep(50000);
    }


    sem_wait(sem);
    region->records[region->count % 8].done = 1;
    sem_post(sem);

    printf("[Producer] finished.\n");
    fflush(stdout);
    ipc_child_close();
    return 0;
}


int ipc_consumer_main(const char *logfile) {
    (void)logfile;
    shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd < 0) { perror("consumer shm_open"); return -1; }

    region = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (region == MAP_FAILED) { perror("consumer mmap"); return -1; }

    sem = sem_open(SEM_NAME, 0);
    if (sem == SEM_FAILED) { perror("consumer sem_open"); return -1; }

    int consumed = 0;
    for (;;) {
        usleep(30000);
        sem_wait(sem); /* enter critical section */
        int slot = consumed % 8;
        shm_record_t *rec = &region->records[slot];
        int have = (rec->sequence != 0);
        int producer_done = region->records[region->count % 8].done;
        int all_consumed = (consumed >= region->count);
        if (have) {
            printf("[Consumer] read record seq=%d value=%d pid=%d\n",
                   rec->sequence, rec->value, rec->producer_id);
            fflush(stdout);
            rec->sequence = 0; 
            consumed++;
        }
        sem_post(sem); 


        if (producer_done && all_consumed) break;
    }

    printf("[Consumer] finished, consumed %d records.\n", consumed);
    fflush(stdout);
    ipc_child_close();
    return 0;
}


int ipc_run(const char *logfile) {
    FILE *fp = fopen(logfile, "w");
    if (!fp) { perror("fopen ipc log"); return -1; }
    fprintf(fp, "CampusCompute Shared-Memory IPC Log\n");
    fprintf(fp, "Shared memory: %s  Named semaphore: %s\n", SHM_NAME, SEM_NAME);
    fflush(fp);
    fclose(fp);

    pid_t pid = fork();
    if (pid < 0) { perror("fork producer"); ipc_cleanup(); return -1; }

    if (pid == 0) {
        int rc = ipc_producer_main(logfile); 
        _exit(rc == 0 ? 0 : 1);
    }

    int status;
    if (waitpid(pid, &status, 0) < 0) {
        perror("waitpid producer");
        ipc_cleanup();
        return -1;
    }
    printf("[IPC] Producer exited with status %d\n",
           WIFEXITED(status) ? WEXITSTATUS(status) : -1);
    fflush(stdout);

    pid_t cpid = fork();
    if (cpid < 0) { perror("fork consumer"); ipc_cleanup(); return -1; }

    if (cpid == 0) {
        int rc = ipc_consumer_main(logfile); 
        _exit(rc == 0 ? 0 : 1);
    }

    if (waitpid(cpid, &status, 0) < 0) {
        perror("waitpid consumer");
        ipc_cleanup();
        return -1;
    }
    printf("[IPC] Consumer exited with status %d\n",
           WIFEXITED(status) ? WEXITSTATUS(status) : -1);
    fflush(stdout);

    fp = fopen(logfile, "a");
    if (fp) {
        fprintf(fp, "Producer and consumer completed via shared memory + named semaphore.\n");
        fprintf(fp, "IPC resources cleaned up (shm_unlink, sem_unlink, munmap, close).\n");
        fclose(fp);
    }

    ipc_cleanup();
    printf("[IPC] Shared memory and named semaphore cleaned up.\n");
    return 0;
}
