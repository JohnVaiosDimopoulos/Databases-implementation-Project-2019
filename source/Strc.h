
#include <stdio.h>
#include "BF.h"
#include <stdlib.h>
#include <string.h>
#include <limits.h>


#define NAME_SIZE 15
#define SURNAME_SIZE 20
#define ADDRESS_SIZE 40
#define MAX_RECORDS BLOCK_SIZE/sizeof(Record)
#define MAX_SECONDARY_RECORDS BLOCK_SIZE/sizeof(SecondaryRecord)

int HT_fd;
int SHT_fd;

typedef struct{
    int id;
    char name[NAME_SIZE];
    char surname[SURNAME_SIZE];
    char address[ADDRESS_SIZE];
}Record;

typedef struct{
    Record record;
    int blockId;
}SecondaryRecord;

typedef struct{
    int record_counter;
    int max_records;
    int next_block;
    int my_id;
    SecondaryRecord record[MAX_SECONDARY_RECORDS];

}SecondaryBlock;

typedef struct{
    int record_counter;
    int max_records;
    int next_block;
    int my_id;
    Record records[MAX_RECORDS];
}Block;

typedef struct{
    int file_type;
    int file_desc;
    char attr_type;
    int attr_length;
    char* attr_name;
    long int buckets_num;
}HT_info;

typedef struct{
    int file_type;
    int file_desc;
    int attr_length;
    char* attr_name;
    long int buckets_num;
    size_t HT_filename_len;
    char* HT_filename;
}SHT_info;
