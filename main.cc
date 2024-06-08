#include "globals.h"

#include "thread.h"

#include <stdio.h>
#include <stdlib.h>

#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */

#include "gengetopt/cmdline.h"

struct gengetopt_args_info	config;


int main(int argc, char** argv)
{
    if (cmdline_parser(argc, argv, &config) != 0)
    {
        exit(1);
    }
    printf("config.memsize: %d\n", config.memsize_arg);
//    printf("config.blocks: %d\n", config.blocks_arg);
    printf("config.threads: %d\n", config.threads_arg);
    printf("config.filename: %s\n", config.filename_arg);

    // open file with Records
    int fd;

    fd = open(config.filename_arg, O_RDWR | O_CREAT, 0666);
    if (fd < 0)
    {
        printf("Error: cannot open file:\n");
        return 1;
    }
    // read total count Records ( that contains in File )
    uint64_t countRecordTotal;
    uint64_t offset;

    read(fd, (void*)&countRecordTotal, sizeof(uint64_t));
    offset = sizeof(uint64_t);

    // calc count Records in Memory
    uint64_t countRecord;

    if (config.memsize_arg == 0)
    { // default value
        countRecord = COUNT_BLOCK_MEMORY;
    }
    else
    {
        countRecord = config.memsize_arg % sizeof(index_s);
    }
    // read count Thread
    uint64_t countThread;

    if (config.threads_arg == 0)
    { // default value
        countThread = COUNT_THREAD;
    }
    else
    {
        countThread = config.threads_arg;
    }
    // initialize Barrier
    pthread_barrier_t   barrier;

    if (pthread_barrier_init(&barrier, NULL, countThread) != 0)
    {
        printf("Error: cannot initialize Barrier:\n");
        return 1;
    }
    // initialize mutex to restrict access to Index Map
    pthread_mutex_t mutex;

    if (pthread_mutex_init(&mutex, NULL) != 0)
    {
        printf("Error: cannot initialize Mutex:\n");
        return 1;
    }
    // create working Threads
    unsigned        indexThread;
    TYPE_OPERATION  operation;
    uint64_t        indexRecord;        // Index Map

    struct dataThread* pThread;

    indexThread = 1;
    operation = TO_SORT;

    for (int i = 1; i < countThread; i++)
    {
        pThread = (dataThread*)malloc(sizeof(struct dataThread));
        pThread->index      = indexThread++;

        pThread->pIndexRecord = &indexRecord;
        pThread->pCountRecord = &countRecord;

        pThread->pOperation = &operation;
        pThread->pBarrier   = &barrier;
        pThread->pMutex     = &mutex;

        pthread_create(&pThread->idThread, NULL, &threadFunction, (void*)pThread);
        pthread_detach(pThread->idThread);			// will use detached threads
    }

    return 0;
}
