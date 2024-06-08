#include <unistd.h>
#include <stdint.h>


#define COUNT_BLOCK_TOTAL   256
#define COUNT_BLOCK_MEMORY	25
#define COUNT_THREAD	    4


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
