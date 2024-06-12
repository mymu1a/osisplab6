#ifndef __GLOBALS_H__
#define __GLOBALS_H__

#include <unistd.h>
#include <stdint.h>


#define COUNT_RECORD_TOTAL          256
#define COUNT_RECORD_INBLOCK	    4
#define COUNT_RECORD_INMEMORY	    256
#define COUNT_THREAD	            4


struct index_s
{
    double          time_mark;               // ��������� ����� (���������������� ��������� ����)
    uint64_t        recno;                   // ��������� ������ � ������� ��
};

struct index_hdr_s
{
    uint64_t        recsords;                // ���������� �������
    struct index_s  idx[1];                  // ������ ������� � ���������� records
};

#endif // __GLOBALS_H__
