#include "campuscompute.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t  cond_empty;   
    pthread_cond_t  cond_full;    
    logger_queue_t  q;
    FILE           *logfile;
    int             num_producers;
    int             msgs_per_producer;
    int             producers_finished;
} logger_ctx_t;


static void log_write(logger_ctx_t *ctx, const char *msg) {
    fprintf(ctx->logfile, "%s\n", msg);
    fflush(ctx->logfile);
}

static int queue_push(logger_ctx_t *ctx, const char *msg) {
    logger_queue_t *q = &ctx->q;
    if (q->count >= LOGGER_QUEUE_CAP) return -1;
    strncpy(q->messages[q->tail], msg, LOGGER_MAX_MSG - 1);
    q->messages[q->tail][LOGGER_MAX_MSG - 1] = '\0';
    q->tail = (q->tail + 1) % LOGGER_QUEUE_CAP;
    q->count++;
    return 0;
}

static int queue_pop(logger_ctx_t *ctx, char *out) {
    logger_queue_t *q = &ctx->q;
    if (q->count <= 0) return -1;
    strncpy(out, q->messages[q->head], LOGGER_MAX_MSG - 1);
    out[LOGGER_MAX_MSG - 1] = '\0';
    q->head = (q->head + 1) % LOGGER_QUEUE_CAP;
    q->count--;
    return 0;
}


typedef struct {
    logger_ctx_t *ctx;
    int           index;
    int           msgs;
} producer_arg_t;

static void *producer_fn(void *arg) {
    producer_arg_t *pa = (producer_arg_t *)arg;
    logger_ctx_t *ctx = pa->ctx;
    char msg[LOGGER_MAX_MSG];

    for (int i = 0; i < pa->msgs; i++) {
        snprintf(msg, sizeof(msg),
                 "Producer[%d] produced message %d/%d",
                 pa->index, i + 1, pa->msgs);

        pthread_mutex_lock(&ctx->mutex);
        
        while (ctx->q.count >= LOGGER_QUEUE_CAP) {
            pthread_cond_wait(&ctx->cond_full, &ctx->mutex);
        }
        queue_push(ctx, msg);
        log_write(ctx, msg);
        pthread_cond_signal(&ctx->cond_empty);
        pthread_mutex_unlock(&ctx->mutex);

        usleep(10000); 
    }

    pthread_mutex_lock(&ctx->mutex);
    ctx->producers_finished++;
    int all_done = (ctx->producers_finished == ctx->num_producers);
    if (all_done) ctx->q.producers_done = 1;
    pthread_cond_broadcast(&ctx->cond_empty); 
    pthread_mutex_unlock(&ctx->mutex);

    free(pa);
    return NULL;
}



static void *consumer_fn(void *arg) {
    logger_ctx_t *ctx = (logger_ctx_t *)arg;
    char msg[LOGGER_MAX_MSG];

    for (;;) {
        pthread_mutex_lock(&ctx->mutex);
       
        while (ctx->q.count == 0 && !ctx->q.producers_done) {
            pthread_cond_wait(&ctx->cond_empty, &ctx->mutex);
        }
        if (ctx->q.count == 0 && ctx->q.producers_done) {
            pthread_mutex_unlock(&ctx->mutex);
            break;
        }
        if (queue_pop(ctx, msg) == 0) {
            pthread_cond_signal(&ctx->cond_full); 
            pthread_mutex_unlock(&ctx->mutex);

            snprintf(msg + strlen(msg), sizeof(msg) - strlen(msg),
                     " -> consumed");
            
            pthread_mutex_lock(&ctx->mutex);
            fprintf(ctx->logfile, "Consumer: %s\n", msg);
            fflush(ctx->logfile);
            pthread_mutex_unlock(&ctx->mutex);
        } else {
            pthread_mutex_unlock(&ctx->mutex);
        }
    }
    return NULL;
}

int logger_run(const char *logfile, int num_producers, int msgs_per_producer) {
    if (num_producers <= 0) num_producers = 2;
    if (msgs_per_producer <= 0) msgs_per_producer = 5;

    FILE *fp = fopen(logfile, "w");
    if (!fp) {
        perror("fopen logfile");
        return -1;
    }
    fprintf(fp, "CampusCompute Activity Log\n");
    fprintf(fp, "Producers: %d, Messages per producer: %d\n",
            num_producers, msgs_per_producer);
    fprintf(fp, "----------------------------------------\n");
    fflush(fp);

    logger_ctx_t ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.logfile = fp;
    ctx.num_producers = num_producers;
    ctx.msgs_per_producer = msgs_per_producer;
    ctx.q.head = ctx.q.tail = ctx.q.count = 0;
    ctx.q.producers_done = 0;
    ctx.producers_finished = 0;

    if (pthread_mutex_init(&ctx.mutex, NULL) != 0 ||
        pthread_cond_init(&ctx.cond_empty, NULL) != 0 ||
        pthread_cond_init(&ctx.cond_full, NULL) != 0) {
        perror("sync init");
        fclose(fp);
        return -1;
    }

    pthread_t *producers = malloc((size_t)num_producers * sizeof(pthread_t));
    pthread_t consumers[2];
    if (!producers) {
        pthread_mutex_destroy(&ctx.mutex);
        pthread_cond_destroy(&ctx.cond_empty);
        pthread_cond_destroy(&ctx.cond_full);
        fclose(fp);
        return -1;
    }

    for (int i = 0; i < num_producers; i++) {
        producer_arg_t *pa = calloc(1, sizeof(*pa));
        pa->ctx = &ctx;
        pa->index = i;
        pa->msgs = msgs_per_producer;
        if (pthread_create(&producers[i], NULL, producer_fn, pa) != 0) {
            perror("pthread_create producer");
        }
    }
    for (int i = 0; i < 2; i++) {
        if (pthread_create(&consumers[i], NULL, consumer_fn, &ctx) != 0) {
            perror("pthread_create consumer");
        }
    }

    for (int i = 0; i < num_producers; i++)
        pthread_join(producers[i], NULL);
    for (int i = 0; i < 2; i++)
        pthread_join(consumers[i], NULL);

    fprintf(fp, "----------------------------------------\n");
    fprintf(fp, "Logger terminated cleanly: all producers and consumers joined.\n");
    fflush(fp);

    free(producers);
    pthread_mutex_destroy(&ctx.mutex);
    pthread_cond_destroy(&ctx.cond_empty);
    pthread_cond_destroy(&ctx.cond_full);
    fclose(fp);

    printf("[Logger] Log written to %s\n", logfile);
    return 0;
}
