#ifndef _THREADPOLL_
#define _THREADPOLL_
typedef struct thpool_private* threadpool;

threadpool thread_pool_init(int number_threads);
void thread_pool_destroy(threadpool );
int thread_pool_add_task(threadpool , void(*task_p)(void*), void *arg);

#endif
