/* banker.c - Banker's safety algorithm (FR5)
 *
 * Need = Maximum - Allocation. Implements the safety algorithm to find a
 * safe sequence for a safe state and to identify an unsafe state. Both
 * cases are tested with built-in inputs.
 */

#include "campuscompute.h"

#include <stdio.h>
#include <string.h>

/* Compute Need matrix. */
static void compute_need(banker_state_t *bs) {
    for (int i = 0; i < bs->n; i++)
        for (int j = 0; j < bs->m; j++)
            bs->need[i][j] = bs->maximum[i][j] - bs->allocation[i][j];
}

/* Safety algorithm. Returns 1 if safe (safe_seq filled), 0 if unsafe. */
int banker_is_safe(const banker_state_t *bs, int *safe_seq) {
    int work[BANKER_MAX_RESOURCES];
    int finish[BANKER_MAX_PROCESSES];
    memcpy(work, bs->available, sizeof(bs->available));
    memset(finish, 0, sizeof(finish));

    int count = 0;
    while (count < bs->n) {
        int found = 0;
        for (int i = 0; i < bs->n; i++) {
            if (finish[i]) continue;
            int can = 1;
            for (int j = 0; j < bs->m; j++) {
                if (bs->need[i][j] > work[j]) { can = 0; break; }
            }
            if (can) {
                for (int j = 0; j < bs->m; j++)
                    work[j] += bs->allocation[i][j];
                finish[i] = 1;
                safe_seq[count++] = i;
                found = 1;
            }
        }
        if (!found) break; /* no process can proceed -> unsafe */
    }
    return count == bs->n;
}

static void print_state(const banker_state_t *bs) {
    printf("\nAvailable: ");
    for (int j = 0; j < bs->m; j++) printf("%d ", bs->available[j]);
    printf("\n\n%-8s %-12s %-12s %-12s\n", "Proc", "Maximum", "Allocation", "Need");
    for (int i = 0; i < bs->n; i++) {
        printf("P%-7d ", i);
        for (int j = 0; j < bs->m; j++) printf("%d ", bs->maximum[i][j]);
        printf("   ");
        for (int j = 0; j < bs->m; j++) printf("%d ", bs->allocation[i][j]);
        printf("   ");
        for (int j = 0; j < bs->m; j++) printf("%d ", bs->need[i][j]);
        printf("\n");
    }
}

void banker_run_tests(void) {
    /* ---- TEST 1: SAFE STATE ---- */
    banker_state_t bs;
    bs.n = 5; bs.m = 3;
    int avail[] = {3, 3, 2};
    int maxm[][3] = {
        {7,5,3},{3,2,2},{9,0,2},{2,2,2},{4,3,3}
    };
    int alloc[][3] = {
        {0,1,0},{2,0,0},{3,0,2},{2,1,1},{0,0,2}
    };
    memcpy(bs.available, avail, sizeof(avail));
    for (int i = 0; i < bs.n; i++)
        for (int j = 0; j < bs.m; j++) {
            bs.maximum[i][j] = maxm[i][j];
            bs.allocation[i][j] = alloc[i][j];
        }
    compute_need(&bs);

    printf("========================================\n");
    printf("Banker's Algorithm - TEST 1: Safe state\n");
    printf("========================================\n");
    print_state(&bs);

    int safe_seq[BANKER_MAX_PROCESSES];
    int safe = banker_is_safe(&bs, safe_seq);
    printf("\nSafety result: %s\n", safe ? "SAFE" : "UNSAFE");
    if (safe) {
        printf("Safe sequence: ");
        for (int i = 0; i < bs.n; i++)
            printf("%sP%d", (i ? " -> " : ""), safe_seq[i]);
        printf("\n");
    }

    /* ---- TEST 2: UNSAFE STATE ---- */
    banker_state_t bs2;
    bs2.n = 4; bs2.m = 3;
    int avail2[] = {1, 1, 0};
    int maxm2[][3] = {
        {3,3,2},{2,2,2},{4,4,4},{2,2,2}
    };
    int alloc2[][3] = {
        {1,1,0},{1,1,0},{2,2,2},{0,0,0}
    };
    memcpy(bs2.available, avail2, sizeof(avail2));
    for (int i = 0; i < bs2.n; i++)
        for (int j = 0; j < bs2.m; j++) {
            bs2.maximum[i][j] = maxm2[i][j];
            bs2.allocation[i][j] = alloc2[i][j];
        }
    compute_need(&bs2);

    printf("\n========================================\n");
    printf("Banker's Algorithm - TEST 2: Unsafe state\n");
    printf("========================================\n");
    print_state(&bs2);

    int safe_seq2[BANKER_MAX_PROCESSES];
    int safe2 = banker_is_safe(&bs2, safe_seq2);
    printf("\nSafety result: %s\n", safe2 ? "SAFE" : "UNSAFE");
    if (!safe2) {
        printf("No safe sequence exists -> resource state is UNSAFE.\n");
        printf("(Allocating further requests may lead to deadlock.)\n");
    }
}
