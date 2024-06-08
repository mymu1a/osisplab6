#include "thread.h"

#include "globals.h"

#include <stdio.h>
#include <stdlib.h>


void* threadFunction(void* pData)
{
	pthread_barrier_wait(pData->pBarrier);

	bool isWork = false;

	pthread_mutex_lock(pData->pMutex);									// mutex lock
	if (pData->pOperation == TO_SORT)
	{
		if (pData->indexRecord < pData->pCountRecord)
		{
			isWork = true;
			pData->indexRecord++;
		}
	}
	pthread_mutex_unlock(pData->pMutex);								// mutex unlock

	if (isWork == true)
	{
		if (pData->pOperation == TO_SORT)
		{
		}
	}
}
