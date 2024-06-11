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

				pData->pDataSync->indexRecord += pData->pDataFile->countRecordInBlock;	// move to the next Block
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
			sort(pData->pDataFile->pHeapMemory, indexRecord, pData->pDataFile->countRecordInBlock, pData->index /* thread index */);
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

void merge(void* pHeapMemory, uint64_t indexRecord, unsigned countMerge, unsigned indexThread)
{
	printf("merge ST ( indexThread = %02d )\n", indexThread);

	uint64_t	countBlock;
	uint64_t	index1 = 0, index2 = 0, indexMerged = 0;
	index_s		*pBlock1, *pBlock2;
	index_s*	pMerged;

	countBlock = countMerge / 2;

	pBlock1 = (index_s*)pHeapMemory + indexRecord;
	pBlock2 = pBlock1 + countBlock;

	pMerged = (index_s*)malloc(sizeof(index_s) * countMerge);

	while (index1 < countBlock && index2 < countBlock)
	{
		if (pBlock1[index1].time_mark < pBlock2[index2].time_mark)
		{
			pMerged[indexMerged++] = pBlock1[index1++];
		}
		else
		{
			pMerged[indexMerged++] = pBlock2[index2++];
		}
	}
	while (index1 < countBlock)
	{
		pMerged[indexMerged++] = pBlock1[index1++];
	}
	while (index2 < countBlock)
	{
		pMerged[indexMerged++] = pBlock2[index2++];
	}
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

int compare(const void* pRecord1, const void* pRecord2)
{

	index_s* r1 = (index_s*)pRecord1;
	index_s* r2 = (index_s*)pRecord2;

	if (r1->time_mark < r2->time_mark)
	{
		return -1;
	}
	if (r1->time_mark > r2->time_mark)
	{
		return 1;
	}
	return 0;
}

void sort(void* pHeapMemory, uint64_t indexRecord, uint64_t countRecordInBlock, unsigned indexThread)
{
	printf("sort ST ( indexThread = %02d )\n", indexThread);

	index_s* pBlock = (index_s*)pHeapMemory + indexRecord;
	qsort((void*)pBlock, countRecordInBlock, sizeof(index_s), compare);

	printf("sort OK\n");

}

void switchNextOperation(struct dataThread* pData, unsigned& stepMerge)
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
		stepMerge = pData->pDataFile->countRecordInBlock *2;
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
