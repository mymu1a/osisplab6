#include "globals.h"

#include <stdio.h>
#include <stdlib.h>

#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */

#include "gengetopt/generate_cmdline.h"

struct gengetopt_args_info	config;

void generateRecord(index_s* pRecord, uint64_t index);
void writeRecord(index_s* pRecord, int fd);


int main(int argc, char** argv)
{
    if (cmdline_parser(argc, argv, &config) != 0)
    {
        exit(1);
    }
    int fd;

	fd = open(config.filename_arg, O_RDWR | O_CREAT, 0666);
    if (fd < 0)
    {
        printf("Error: cannot open file:\n");
        return 1;
    }
    uint64_t index;
    uint64_t countRecord;

    index = 1;
    
    if (config.recordNo_arg == 0)
    { // default value
        countRecord = COUNT_RECORD_TOTAL;
    }
    else
    {
        countRecord = config.recordNo_arg % 256;
    }
    write(fd, (void*)&countRecord, sizeof(uint64_t));

    index_s record;
    while (index++ <= countRecord)
    {
        generateRecord(&record, index);
        writeRecord(&record, fd);
    }
    close(fd);

    printf("config.blocks: %d\n", config.recordNo_arg);
    printf("config.filename: %s\n", config.filename_arg);

    return 0;
}

int day_first = 15020;
int day_latest = 2460470;

void generateRecord(index_s* pRecord, uint64_t index)
{
    int day = day_first + random() % (day_latest - day_first + 1);
    int hours = random() % 1000;

    pRecord->time_mark = day + hours / 1000;
    pRecord->recno = index;
}

void writeRecord(index_s* pRecord, int fd)
{
    write(fd, (void*)&pRecord->time_mark, sizeof(double));
    write(fd, (void*)&pRecord->recno, sizeof(uint64_t));
}
