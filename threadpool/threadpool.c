#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include "threadpool.h"
#include <sys/prctl.h>

typedef struct bsem {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int v;
} bsem;

typedef struct task {
    struct task* prev;
    void (*handler)(void* arg);
    void *arg;
} task;

typedef struct task_queue {
    pthread_mutex_t rw_mutex;
    task* front;
    task* rear;
    bsem* has_tasks;
    uint32_t size;
} task_queue;

typedef struct thread {
    int id;
    pthread_t thread;
    struct thpool_private* thpool;
} thread;

typedef struct thpool_private {
    thread** workers;
    task_queue* tasks;
    pthread_mutex_t thread_count_lock;
    pthread_cond_t threads_all_idle;
    volatile int num_thread_alive;
    volatile int num_thread_working;
    volatile int threads_keepalive;
    volatile int thread_on_hold;
} thpool_private;

static int thread_init(thpool_private *thpool_p, struct thread **th, int id);
static void thread_destroy(struct thread *th);
static void *thread_run(struct thread* worker);

static int task_queue_init(thpool_private *thpool_p);
static void task_queue_clear(thpool_private *thpool_p);
static void task_queue_push(thpool_private *thpool_p, struct task *task_p);
static struct task *task_queue_pull(thpool_private *thpool_p);
static void task_queue_destroy(thpool_private *thpool_p);

static void  bsem_init(struct bsem *bsem_p, int value);
static void  bsem_reset(struct bsem *bsem_p);
static void  bsem_post(struct bsem *bsem_p);
static void  bsem_post_all(struct bsem *bsem_p);
static void  bsem_wait(struct bsem *bsem_p);


#define THP_ERR_SUCCESS 0
#define THP_ERR_ERROR 1

struct thpool_private *thread_pool_init(int number_threads)
{
    if(number_threads < 0) {
        number_threads = 0;
    }

    thpool_private *thpool_p = NULL;
    thpool_p = (struct thpool_private*)malloc(sizeof(struct thpool_private));
    if(thpool_p == NULL) {
        fprintf(stderr, "failed allocateing thpoll private %s %d\n", __func__, __LINE__);
    }
    thpool_p->num_thread_working = 0;
    thpool_p->num_thread_alive = 0;

    if(task_queue_init(thpool_p) == THP_ERR_ERROR) {
        fprintf(stderr, "task queue init failed %s %d\n", __func__, __LINE__);
        free(thpool_p);
        thpool_p = NULL;
        return NULL;
    }

    thpool_p->workers = (struct thread**)malloc(number_threads * sizeof(struct thread*));
    if(thpool_p->workers == NULL) {
        fprintf(stderr, "new workers failed %s %d\n", __func__, __LINE__);
        task_queue_destroy(thpool_p);
        free(thpool_p->tasks);
        free(thpool_p);
        return NULL;
    }

    pthread_mutex_init(&(thpool_p->thread_count_lock), NULL);
    pthread_cond_init(&thpool_p->threads_all_idle, NULL);

    thpool_p->threads_keepalive = 1;
    int n;
    for(n = 0; n < number_threads; n++) {
        thread_init(thpool_p, &thpool_p->workers[n], n);
        printf("created thread %d in pool\n", n);
    }

    while(thpool_p->num_thread_alive != number_threads) {}

    return thpool_p;
}

int thread_pool_add_task(thpool_private *thpool_p, void(*handler)(void *), void *args)
{
    task *new_task;

    new_task = (struct task*)malloc(sizeof(struct task));
    if(new_task == NULL) {
        fprintf(stderr, "create new task failed %s %d\n", __func__, __LINE__);
        return THP_ERR_ERROR;
    }

    new_task->handler = handler;
    new_task->arg = args;

    pthread_mutex_lock(&thpool_p->tasks->rw_mutex);
    task_queue_push(thpool_p, new_task);
    pthread_mutex_unlock(&thpool_p->tasks->rw_mutex);

    return THP_ERR_SUCCESS;
}


void thread_pool_destroy(thpool_private *thpool_p)
{
    if(thpool_p == NULL) return;

    volatile int threads_total = thpool_p->num_thread_alive;


    thpool_p->threads_keepalive = 0;
    double TIMEOUT = 1.0;
    time_t start, end;
    double tpassed = 0.0;
    while(tpassed < TIMEOUT && thpool_p->num_thread_alive) {
        bsem_post_all(thpool_p->tasks->has_tasks);
        time(&end);
        tpassed = difftime(end, start);
    }

    while(thpool_p->num_thread_alive) {
        bsem_post_all(thpool_p->tasks->has_tasks);
        usleep(100000); //0.1 sec
    }

    task_queue_destroy(thpool_p);
    free(thpool_p->tasks);

    int n;
    for(n = 0; n < threads_total; n++) {
        thread_destroy(thpool_p->workers[n]);
    }

    free(thpool_p->workers);
    free(thpool_p);
}

static int thread_init(thpool_private *thpool_p, struct thread** worker, int id)
{
    *worker = (struct thread*)malloc(sizeof(struct thread));
    if(worker == NULL) {
        fprintf(stderr, "worker init failed %s %d\n", __func__, __LINE__);
        return THP_ERR_ERROR;
    }

    (*worker)->thpool = thpool_p;
    (*worker)->id = id;

    pthread_create(&(*worker)->thread, NULL, (void*)thread_run, (*worker));
    pthread_detach((*worker)->thread);
    return THP_ERR_SUCCESS;
}

static void *thread_run(struct thread* worker)
{
    char worker_name[128] = {0};
    sprintf(worker_name, "thread-poll-%d", worker->id);
    prctl(PR_SET_NAME, worker_name);
    thpool_private *thpool_p = worker->thpool;

    pthread_mutex_lock(&thpool_p->thread_count_lock);
    thpool_p->num_thread_alive += 1;
    pthread_mutex_unlock(&thpool_p->thread_count_lock);

    while(thpool_p->threads_keepalive) {

        bsem_wait(thpool_p->tasks->has_tasks);
        if(thpool_p->threads_keepalive) {
            pthread_mutex_lock(&thpool_p->thread_count_lock);
            thpool_p->num_thread_working++;
            pthread_mutex_unlock(&thpool_p->thread_count_lock);

            void (*cb_func)(void *arg);
            void *func_args;
            task *task_p;
            pthread_mutex_lock(&thpool_p->tasks->rw_mutex);
            task_p = task_queue_pull(thpool_p);
            pthread_mutex_unlock(&thpool_p->tasks->rw_mutex);
            if(task_p) {
                cb_func = task_p->handler;
                func_args = task_p->arg;
                cb_func(func_args);
                free(task_p);
            }
            pthread_mutex_lock(&thpool_p->thread_count_lock);
            thpool_p->num_thread_working--;
            if(!thpool_p->num_thread_working) {
                pthread_cond_signal(&thpool_p->threads_all_idle);
            }
            pthread_mutex_unlock(&thpool_p->thread_count_lock);
        }
    }

    pthread_mutex_lock(&thpool_p->thread_count_lock);
    thpool_p->num_thread_alive --;
    pthread_mutex_unlock(&thpool_p->thread_count_lock);

    return NULL;
}

static void thread_destroy(thread *worker)
{
    if(worker == NULL) return;

    free(worker);
    worker = NULL;
    return;
}

static int task_queue_init(thpool_private *thpool_p)
{
    thpool_p->tasks = (struct task_queue*)malloc(sizeof(struct task_queue));
    if(thpool_p->tasks == NULL) {
        fprintf(stderr, "task queue init failed %s %d\n", __func__, __LINE__);
        return THP_ERR_ERROR;
    }
    thpool_p->tasks->size = 0;
    thpool_p->tasks->front = NULL;
    thpool_p->tasks->rear = NULL;

    thpool_p->tasks->has_tasks = (struct bsem*)malloc(sizeof(struct bsem));
    pthread_mutex_init(&thpool_p->tasks->rw_mutex, NULL);
    bsem_init(thpool_p->tasks->has_tasks, 0);

    return THP_ERR_SUCCESS;
}

static void task_queue_clear(thpool_private *thpool_p)
{
    while(thpool_p->tasks->size) {
        free(task_queue_pull(thpool_p));
    }

    thpool_p->tasks->front = NULL;
    thpool_p->tasks->rear = NULL;
    bsem_reset(thpool_p->tasks->has_tasks);
    thpool_p->tasks->size = 0;
}

static void task_queue_push(thpool_private *thpool_p, struct task *new_task)
{
    new_task->prev = NULL;

    switch(thpool_p->tasks->size) {
    case 0:
        thpool_p->tasks->front = new_task;
        thpool_p->tasks->rear = new_task;
        break;
    default:
        thpool_p->tasks->rear->prev = new_task;
        thpool_p->tasks->rear = new_task;
        break;
    }
    thpool_p->tasks->size++;
    bsem_post(thpool_p->tasks->has_tasks);
}

static struct task *task_queue_pull(thpool_private *thpool_p)
{

    task *task_p;
    task_p = thpool_p->tasks->front;
    switch(thpool_p->tasks->size) {
    case 0:
        break;
    case 1:
        thpool_p->tasks->front = NULL;
        thpool_p->tasks->rear = NULL;
        thpool_p->tasks->size = 0;
        break;
    default:
        thpool_p->tasks->front = task_p->prev;
        thpool_p->tasks->size--;
        bsem_post(thpool_p->tasks->has_tasks);
        break;
    }

    return task_p;
}

static void task_queue_destroy(thpool_private *thpool_p)
{
    task_queue_clear(thpool_p);
    free(thpool_p->tasks->has_tasks);
    thpool_p->tasks->has_tasks = NULL;
}

static void bsem_init(bsem *bsem_p, int value)
{
    if(value < 0 || value > 1) {
        fprintf(stderr, "bsem_init falied %s %d\n", __func__, __LINE__);
        exit(1);
    }
    pthread_mutex_init(&(bsem_p->mutex), NULL);
    pthread_cond_init(&(bsem_p->cond), NULL);
    bsem_p->v = value;
}

static void bsem_reset(bsem *bsem_p)
{
    bsem_init(bsem_p, 0);
}

static void bsem_post(bsem *bsem_p)
{
    pthread_mutex_lock(&bsem_p->mutex);
    bsem_p->v = 1;
    pthread_cond_signal(&bsem_p->cond);
    pthread_mutex_unlock(&bsem_p->mutex);
}

static void bsem_post_all(bsem *bsem_p)
{
    pthread_mutex_lock(&bsem_p->mutex);
    bsem_p->v = 1;
    pthread_cond_broadcast(&bsem_p->cond);
    pthread_mutex_unlock(&bsem_p->mutex);
}

static void bsem_wait(bsem *bsem_p)
{
    pthread_mutex_lock(&bsem_p->mutex);
    while(bsem_p->v != 1) {
        pthread_cond_wait(&bsem_p->cond, &bsem_p->mutex);
    }

    bsem_p->v = 0;
    pthread_mutex_unlock(&bsem_p->mutex);
}
