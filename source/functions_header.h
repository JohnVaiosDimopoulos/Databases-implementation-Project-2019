#include "Strc.h"

void Init_Block(Block*,int);

void Init_SBlock(SecondaryBlock*,int);

int My_Hash_HT(void *,HT_info *);

int My_Hash_SHT(void*,SHT_info*);

int HT_CreateIndex(char*,char,char*,int,int);

HT_info* HT_OpenIndex(char*);

int HT_CloseIndex(HT_info*);

int HT_InsertEntry(HT_info,Record);

void Show_Record(Record,int*);

int HT_GetAllEntries(HT_info,void*);

void Delete_Record(int,Block*);

int HT_DeleteEntry(HT_info, void*);

int SHT_SecondaryInsertEntry(SHT_info,SecondaryRecord);

int SHT_CreateSecondaryIndex(char*,char*,int,int,char*);

SHT_info* SHT_OpenSecondaryIndex(char*);

int SHT_CloseSecondaryIndex(SHT_info*);

int FindRecordInHT(HT_info,int,void*,int,char*,int*);

int SHT_SecondaryGetAllEntries(SHT_info,HT_info,void*);

int Print_Stats(size_t,int,long int,int);

int HashStatistics(char*);

