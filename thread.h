#include "globals.h"

#include <pthread.h>

enum TYPE_OPERATION { TO_NONE, TO_SORT, TO_MERGE, TO_EXIT };


struct dataFileStruct
{
	const char* name;
	uint64_t	countRecordOnDisk;			// total count Records in File ( on disk )
	
	// current disk state
	int			fd;
	off_t		sizeFile;					// file size
	uint64_t	offset;						// number of bytes actually read

	// file part read to the Memory
	uint64_t	countRecordInMemory;		// count Records in Memory ( max )
	uint64_t	countRecord;				// count Records in Memory ( real )
	void*		pHeapMemory;
};

struct dataSyncStruct
{
	uint64_t			countThread;		// count Threads
	uint64_t			countOnBarrier;		// count Threads on Barrier
	uint64_t			indexRecord;		// Index Map
	TYPE_OPERATION		operation;

	pthread_barrier_t	barrier;			// barrier
	pthread_mutex_t		mutex;				// mutual exclusive access to the Index Map
};

struct dataThread
{
	unsigned	index;						// thread index
	pthread_t	idThread;

	struct dataFileStruct*	pDataFile;
	struct dataSyncStruct*	pDataSync;
};


void* threadFunction(void* pData);
bool readNextRecordBlock(struct dataFileStruct& file);
void sort(uint64_t indexRecord, unsigned indexThread);
void switchNextOperation(struct dataThread* pData);
