#include "functions_header.h"


int main(int argc,char** argv) {


    if (argc < 7) {
        printf("Wrong Input!\n");
        printf("YOU MUST GIVE: proj -h HT_Filename  -s SHT_Filename -r Record_num -rf Recornd_file");
        exit(EXIT_FAILURE);
    }

    //default values for HT_info and SHT_info the file names must be given as argguments
    //HT_info default
    char HT_attr_type = 'i';//id
    char* HT_attr_name;
    HT_attr_name = malloc(sizeof(char)*(strlen("id")+1));
    strcpy(HT_attr_name, "id");
    long int HT_buckets_num = 7 ;

    //SHT_info default
    char* SHT_attr_name;
    SHT_attr_name = malloc(sizeof(char)*(strlen("surname")+1));
    strcpy(SHT_attr_name, "surname");
    int SHT_bucket_num = 5;

    //default records number
    int records_num = 200;

    char *HT_Filename, *SHT_Filename,*Record_Filename;
    for (int i =1 ;i<8;i++){
        if(!strcmp(argv[i],"-h")){
            ++i;
            HT_Filename = (char*)malloc(sizeof(char)*(strlen(argv[i]+1)));
            strcpy(HT_Filename,argv[i]);
        }
        if(!strcmp(argv[i],"-s")){
            ++i;
            SHT_Filename = (char*)malloc(sizeof(char)*(strlen(argv[i]+1)));
            strcpy(SHT_Filename,argv[i]);
        }
        if(!strcmp(argv[i],"-r")){
            ++i;
            records_num = atoi(argv[i]);
        }
        if(!strcmp(argv[i],"-rf")){
          i++;
          Record_Filename = (char*)malloc(sizeof(char)*(strlen(argv[i])+1));
          strcpy(Record_Filename,argv[i]);
        }
    }


    //Start getting the optional  arguments
    for (int i = 9; i < argc; i++) {
        if (!strcmp(argv[i], "-ht")) {
            ++i;
            sscanf(argv[i], "%c", &HT_attr_type);
        } else if (!strcmp(argv[i], "-hn")) {
            ++i;
            HT_attr_name = (char*)realloc(HT_attr_name, sizeof(char)*(strlen(argv[i])+1));
            strcpy(HT_attr_name, argv[i]);

        } else if (!strcmp(argv[i], "-hb")) {
            ++i;
            HT_buckets_num = atoi(argv[i]);
        } else if (!strcmp(argv[i], "-sn")) {
            ++i;
            SHT_attr_name =(char*)realloc(SHT_attr_name,sizeof(char)*(strlen(argv[i])+1));
            strcpy(SHT_attr_name, argv[i]);

        } else if (!strcmp(argv[i], "-sb")) {
            ++i;
            SHT_bucket_num = atoi(argv[i]);
        }
    }
    int HT_attr_lenght = strlen(HT_attr_name);
    int SHT_attr_lenght = strlen(SHT_attr_name);
    // get the two filenames



    BF_Init();
    int ch1 =0;
    FILE* Records_ptr = fopen(Record_Filename,"r");
    if(Records_ptr==NULL){
        perror("file open");
        exit(EXIT_FAILURE);
    }
    //create the index and open the HT_file
    ch1 = HT_CreateIndex(HT_Filename, HT_attr_type, HT_attr_name, HT_attr_lenght, HT_buckets_num);
    if(!ch1)
        printf("ch1 ok\n\n");
    else
        printf("ch1 fal\n\n");

    HT_info *header_info = HT_OpenIndex(HT_Filename);
    if(header_info)
        printf("ch2 ok\n\n");
    else
        printf("ch2 fail\n\n");
    HT_fd = header_info->file_desc;

    // start inserting records
    for(int i =records_num;i>=0;i--){
        Record record;
        fscanf(Records_ptr,"{%d,\"%[^\"]\",\"%[^\"]\",\"%[^\"]\"}\n",&record.id,record.name,record.surname,record.address);
        // record.id =i;
        // sprintf(record.name,"name_%d",i);
        // sprintf(record.surname,"surname_%d",i);
        // sprintf(record.address,"address_%d",i);
        printf("WRITING RECORD %d\n",record.id);
        HT_InsertEntry(*header_info,record);

    }

    // search for records
    int ch4 =0;
    for (int i=0;i<records_num;i++) {
        Record record;
        record.id =i;
        sprintf(record.name,"name_%d",i);
        sprintf(record.surname,"surname_%d",i);
        sprintf(record.address,"address_%d",i);
        int res = HT_GetAllEntries(*header_info, (void *) &record.id);
        if (res < 0)
            ch4+=1;
    }
    if(ch4<0)
        printf("ch4 fail\n\n");
    else
        printf("ch4 ok\n\n");

    int ch5 =0;
    for (int i =records_num;i<2*records_num;i++){
        Record record;
        record.id = i;
        sprintf(record.name, "name_%d", i);
        sprintf(record.surname, "surname_%d", i);
        sprintf(record.address, "address_%d", i);
        int res = HT_GetAllEntries(*header_info, (void *) &record.id);
        if (res < 0)
            ch5+=1;

    }

    if(ch5!=records_num)
        printf("ch5 fail\n\n");
    else
        printf("ch5 ok\n\n");

    int ch6 =0;
    int delete = records_num/2;
    for (int i = 0; i < delete; i++) {
        Record record;
        record.id = i;
        sprintf(record.name, "name_%d", i);
        sprintf(record.surname, "surname_%d", i);
        sprintf(record.address, "address_%d", i);
        printf("DELETING RECORD %d\n",record.id);
        int res = HT_DeleteEntry(*header_info, &record.id);
        if (res != 0)
            ch6+=1;
    }
    if(ch6!=0)
        printf("ch6 fail\n\n");
    else
        printf("ch6 ok\n\n");

    int ch7=0;
    for (int i = 0;i<records_num;i++ ){
        Record record;
        record.id = i;
        sprintf(record.name, "name_%d", i);
        sprintf(record.surname, "surname_%d", i);
        sprintf(record.address, "address_%d", i);
        int res = HT_GetAllEntries(*header_info,(void*)&record.id);
        if(res <0)
            ch7+=1;
    }
    if(ch7!=delete)
        printf("ch7 fail\n\n");
    else
        printf("ch7 ok\n\n");

    int res = SHT_CreateSecondaryIndex(SHT_Filename,SHT_attr_name,SHT_attr_lenght,SHT_bucket_num,HT_Filename);
    if(res<0)
        printf("ch8 fail\n\n");
    else
        printf("ch8 ok\n\n");

    SHT_info* sht_info = SHT_OpenSecondaryIndex(SHT_Filename);
    if(sht_info)
        printf("ch9 ok\n\n");
    else
        printf("ch9 fail\n\n");

    int ch10=0;
    for (int i = records_num; i>0; i--) {
        Record record;
        fscanf(Records_ptr, "{%d,\"%[^\"]\",\"%[^\"]\",\"%[^\"]\"}\n",&record.id,record.name,record.surname,record.address);
        printf("WRITING RECORD %d\n",record.id);
        int id =HT_InsertEntry(*header_info, record);
        if(id>0){
            SecondaryRecord s_record;
            s_record.record = record;
            s_record.blockId=id;
            int res = SHT_SecondaryInsertEntry(*sht_info,s_record);
            if(res<0)
               ch10+=1;
        }
    }
    if(!ch10)
        printf("ch10 ok\n\n");
    else
        printf("ch10 fail\n\n");

    int ch11 =0;
    for(int i = 0;i<records_num*2;i++) {
        Record record;
         record.id = i;
         sprintf(record.name, "name_%d", i);
         sprintf(record.surname, "surname_%d", i);
         sprintf(record.address, "address_%d", i);
        int res = SHT_SecondaryGetAllEntries(*sht_info, *header_info, (void *) record.surname);
        if (res < 0)
            ch11 += 1;
    }
    if(ch11>delete)
        printf("ch11 fail\n");
    else
        printf("ch11 ok\n");

    int htclose = HT_CloseIndex(header_info);
    int shtclose = SHT_CloseSecondaryIndex(sht_info);

    if(htclose==0 && shtclose==0)
        printf("ch12 ok\n");
    else
        printf("ch12 fail\n");

    printf("STATS FOR HT_FILE\n");
    HashStatistics(HT_Filename);

    printf("STATS FOR SHT_FILE\n");
    HashStatistics(SHT_Filename);



    free(HT_Filename);
    free(SHT_Filename);
    free(HT_attr_name);
    free(SHT_attr_name);
    fclose(Records_ptr);
}
