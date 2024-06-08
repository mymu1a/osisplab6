#include "globals.h"

#include <pthread.h>

enum TYPE_OPERATION { TO_SORT, TO_MERGE };


struct dataThread
{
	unsigned	index;					// thread index
	pthread_t	idThread;

	uint64_t* pIndexRecord;
	uint64_t* pCountRecord;

	pthread_barrier_t*  pBarrier;		// barrier
	pthread_mutex_t*	pMutex;			// mutual exclusive access to the Index Map
	TYPE_OPERATION*		pOperation;
};


void* threadFunction(void* pData);
