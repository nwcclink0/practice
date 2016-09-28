#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "threadpool.h"

void test1(int val)
{
    printf("this is test1, and val is: %d\n", val);
}

void test2(int val)
{
    printf("this is test2, and val is: %d\n", val);
}

int main()
{
    threadpool thp = NULL;
    thp = thread_pool_init((int)4);
    thread_pool_add_task(thp, (void*)test1, (void *)4);
    thread_pool_add_task(thp, (void*)test2, (void *)3);
    usleep(10);
    thread_pool_destroy(thp);
    return 0;
}
