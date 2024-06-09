#include "thread.h"

#include "globals.h"

#include <stdio.h>
#include <stdlib.h>

#include <sys/mman.h>


void* threadFunction(void* pData_)
{
	dataThread* pData = (dataThread*)pData_;

	bool		isWork = false;
	unsigned	stepMerge = 2;

	while (1)
	{
		if(isWork == false)
		pthread_barrier_wait(&pData->pDataSync->barrier);								// Barrier

		if (pData->pDataSync->operation == TO_EXIT)
		{
			printf("Exit: Thread_%02d:\n", pData->index);
			pthread_exit(NULL);
		}
		uint64_t	indexRecord;

		pthread_mutex_lock(&pData->pDataSync->mutex);									// mutex lock
		if (pData->pDataSync->operation == TO_SORT)
		{
			if (pData->pDataSync->indexRecord < pData->pDataFile->countRecord)			// count Records in Memory ( real )
			{
				isWork = true;
				indexRecord = pData->pDataSync->indexRecord;

				pData->pDataSync->indexRecord++;
				printf("indexRecord = %02ld\n", indexRecord);
			}
			else
			{
				isWork = false;
				printf(" all read\n");
			}
		}

		if (pData->pDataSync->operation == TO_MERGE)
		{
			if (pData->pDataSync->indexRecord < pData->pDataFile->countRecord)			// count Records in Memory ( real )
			{
				isWork = true;
				indexRecord = pData->pDataSync->indexRecord;
				
				pData->pDataSync->indexRecord += stepMerge;
				printf("indexRecord = %02ld\n", indexRecord);
			}
			else
			{
				isWork = false;
				printf(" all merge\n");
			}
		}

		// we go on Barrier
		if (isWork == false)
		{
			pData->pDataSync->countOnBarrier++;
			printf("\tgo on Barrier - Thread_%02d\n", pData->index);
			if (pData->pDataSync->countOnBarrier == pData->pDataSync->countThread)
			{
				switchNextOperation(pData, stepMerge);
			}
		}
		pthread_mutex_unlock(&pData->pDataSync->mutex);								// mutex unlock
		if (isWork == false)
		{
			continue;
		}

		if (pData->pDataSync->operation == TO_SORT)
		{
			sort(pData->pDataFile->pHeapMemory, indexRecord, pData->index);
			usleep(1);
		}
		if (pData->pDataSync->operation == TO_MERGE)
		{
			merge(pData->pDataFile->pHeapMemory, indexRecord, stepMerge, pData->index);
			usleep(1);
		}
	}
	return NULL;
}

void merge(void* pMemory, uint64_t indexRecord, unsigned countMerge, unsigned indexThread)
{
	printf("merge ST ( indexThread = %02d )\n", indexThread);

	printf("merge OK\n");
}


// read next Block of Records from Disk to Memory
bool readNextRecordBlock(struct dataFileStruct& file)
{
	printf("readNextRecordBlock ST\n");
	printf("  file.sizeFile=%d\n", file.sizeFile);
	printf("  file.offset=%d\n", file.offset);

	// there are no space for Record
	size_t sizeToRead;
	
	sizeToRead = file.sizeFile - file.offset;
	printf("  sizeToRead=%d\n", sizeToRead);
	printf("  sizeof(index_s)=%ld\n", sizeof(index_s));

	if (sizeToRead < sizeof(index_s))
	{
		return false;
	}
	unsigned  count;

	count = sizeToRead / sizeof(index_s);
	if (count > file.countRecordInMemory)
	{
		count = file.countRecordInMemory;		// count Records in Memory ( max )
	}
	printf("  count=%d\n", count);
	file.countRecord = count;

	sizeToRead = count * sizeof(index_s);
	printf("  sizeToRead-2=%d\n", sizeToRead);

	file.pHeapMemory = mmap(NULL, sizeToRead, PROT_READ | PROT_WRITE, MAP_SHARED, file.fd, file.offset);
	if (file.pHeapMemory == NULL)
	{
		printf("Error: cannot map Records in the Memory\n");
		return false;
	}
	file.offset += sizeToRead;
	printf("  file.offset=%d\n", file.offset);

	printf("readNextRecordBlock OK\n");
	return true;
}

void sort(void* pHeapMemory, uint64_t indexRecord, unsigned indexThread)
{
	printf("sort ST ( indexThread = %02d )\n", indexThread);

	qsort();

	printf("sort OK\n");

}

void switchNextOperation(struct dataThread* pData, unsigned stepMerge)
{
	printf("switchNextOperation ST ( Thread_%02d )\n", pData->index);

	pData->pDataSync->countOnBarrier = 0;
	pData->pDataSync->indexRecord = 0;
	
	if (pData->pDataSync->operation == TO_NONE)
	{
		if (readNextRecordBlock(*pData->pDataFile) == false)
		{
			pData->pDataSync->operation = TO_EXIT;
			return;
		}
		pData->pDataSync->operation = TO_SORT;
		printf("switchNextOperation OK [ TO_NONE --> TO_SORT ]\n");
		return;
	}
	if (pData->pDataSync->operation == TO_SORT)
	{
		printf("switchNextOperation OK [ TO_SORT --> TO_MERGE ]\n");
		stepMerge = 2;
		pData->pDataSync->operation = TO_MERGE;
		return;
	}
	if (pData->pDataSync->operation == TO_MERGE)
	{
		stepMerge *= 2;
		if (stepMerge <= pData->pDataFile->countRecord)
		{
			return;												// continue Merge Operation
		}
		if (readNextRecordBlock(*pData->pDataFile) == true)
		{
			pData->pDataSync->operation = TO_SORT;				// sort next Record Block
			return;
		}
		pData->pDataSync->operation = TO_EXIT;
		printf("switchNextOperation OK [ TO_MERGE --> TO_EXIT ]\n");
		return;
	}
	printf("switchNextOperation OK\n");
}
