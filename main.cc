#include "globals.h"

#include "thread.h"

#include <stdio.h>
#include <stdlib.h>

#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <sys/mman.h>

#include "gengetopt/cmdline.h"

struct gengetopt_args_info	config;


int main(int argc, char** argv)
{
    if (cmdline_parser(argc, argv, &config) != 0)
    {
        exit(1);
    }
    printf("config.memsize: %d\n", config.memsize_arg);
    printf("config.blocks: %d\n", config.blocks_arg);
    printf("config.threads: %d\n", config.threads_arg);
    printf("config.filename: %s\n", config.filename_arg);

    //=== create File Object ===
    struct dataFileStruct   dataFile;

    // open file with Records

    dataFile.fd = open(config.filename_arg, O_RDWR, 0666);
    if (dataFile.fd < 0)
    {
        printf("Error: cannot open file: %s\n", config.filename_arg);
        return 1;
    }
    // read total count Records ( that contains in File )
    struct stat sb;

    fstat(dataFile.fd, &sb);
    dataFile.sizeFile = sb.st_size;

    if (dataFile.sizeFile < sizeof(uint64_t))
    {
        printf("Error: cannot read number of Records ( file is too small )\n");
        return 1;
    }
    read(dataFile.fd, (void*)&dataFile.countRecordOnDisk, sizeof(uint64_t));
    dataFile.offset = sizeof(uint64_t);

    printf("dataFile.sizeFile=%ld\n", dataFile.sizeFile);
    printf("countRecordOnDisk=%ld\n", dataFile.countRecordOnDisk);

    if (dataFile.countRecordOnDisk < COUNT_RECORD_INMEMORY)
    {
        printf("Error: file is too small. Min count Record: %ld\n", COUNT_RECORD_INMEMORY);
        return 1;
    }
    // calc count Records in Block

    if (config.blocks_arg == 0)
    { // default value
        dataFile.countRecordInBlock = COUNT_RECORD_INBLOCK;
    }
    else
    {
        dataFile.countRecordInBlock = config.blocks_arg;
    }
    printf("countRecordInBlock=%ld\n", dataFile.countRecordInBlock);

    // calc count Records in Memory

    if (config.memsize_arg == 0)
    { // default value
        dataFile.countRecordInMemory = COUNT_RECORD_INMEMORY;
    }
    else
    {
        dataFile.countRecordInMemory = config.memsize_arg / sizeof(index_s) / COUNT_RECORD_INMEMORY;
        dataFile.countRecordInMemory *= COUNT_RECORD_INMEMORY;
    }
    printf("countRecordInMemory=%ld\n", dataFile.countRecordInMemory);

    dataFile.sizePage = getpagesize();
    if (dataFile.sizePage != 4096)
    {
        printf("Error: size Page != 4096\n");
        return 1;
    }
    dataFile.offset = 0;

    //=== create Synchronization Objects ===
    struct dataSyncStruct    dataSync;

    // read count Thread
    if (config.threads_arg == 0)
    {
        dataSync.countThread = COUNT_THREAD;              // default value
    }
    else
    {
        dataSync.countThread = config.threads_arg;
    }
    dataSync.countOnBarrier = 0;		                  // count Threads on Barrier
    dataSync.indexRecord    = 0;		                  // Index Map
    dataSync.operation      = TO_NONE;

    // initialize Barrier
    if (pthread_barrier_init(&dataSync.barrier, NULL, dataSync.countThread) != 0)
    {
        printf("Error: cannot initialize Barrier:\n");
        return 1;
    }
    // initialize mutex to restrict access to Index Map
    if (pthread_mutex_init(&dataSync.mutex, NULL) != 0)
    {
        printf("Error: cannot initialize Mutex:\n");
        return 1;
    }

    //=== create Threads ===
    unsigned        indexThread;
    TYPE_OPERATION  operation;
    uint64_t        indexRecord;        // Index Map

    struct dataThread* pThread;

    indexThread = 1;

    // working Thread start
    for (int i = 1; i < dataSync.countThread; i++)
    {
        pThread = (dataThread*)malloc(sizeof(struct dataThread));

        pThread->index      = indexThread++;
        pThread->pDataFile  = &dataFile;
        pThread->pDataSync  = &dataSync;

        pthread_create(&pThread->idThread, NULL, &threadFunction, (void*)pThread);
        pthread_detach(pThread->idThread);			// will use detached threads
    }
    // main Thread
    pThread = (dataThread*)malloc(sizeof(struct dataThread));

    pThread->index      = 0;
    pThread->pDataFile  = &dataFile;
    pThread->pDataSync  = &dataSync;
    pThread->idThread   = 0;

    threadFunction(pThread);

    return 0;
}
