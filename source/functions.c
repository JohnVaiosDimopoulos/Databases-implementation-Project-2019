#include "functions_header.h"

void Init_Block(Block* block,int my_id){
    block->record_counter = 0;
    block->next_block = -1;
    block->max_records = MAX_RECORDS;
    block->my_id = my_id;

}

void Init_SBlock(SecondaryBlock* s_block,int myid){
    s_block->record_counter = 0;
    s_block->next_block = -1;
    s_block->max_records = MAX_SECONDARY_RECORDS;
    s_block->my_id = myid;
}

int My_Hash_HT(void *value, HT_info *header_info){
    int a =3;
    //if the value is int
    if(header_info->attr_type == 'i'){
        int val = *(int*)value;
        // +1 so we dont get 0
        return ((val*a)%header_info->buckets_num)+1;
    }
    else{
        //if the value is a string
        int h=0;
        size_t len = strlen(value);

        for(int i =0;i<len;i++) {
            h = (h * a +((char*)value)[i]);
        }
        return (h%header_info->buckets_num)+1;

    }
}

int My_Hash_SHT(void* value,SHT_info* sht_info){
    int a=3;
    int h=0;
    size_t len = strlen(value);
    for(int i = 0;i<len;i++)
        h = (h*a)+((char*)value)[i];
    return (h%sht_info->buckets_num)+1;
}

int HT_CreateIndex(char* filename,char attr_type,char* attr_name,int attr_length,int buckets_num){
    void* block_file_ptr;
    // create the block_file_ptr file
    if(BF_CreateFile(filename)<0){
        BF_PrintError("Create file failed");
        return -1;
    }
    // open the block_file_ptr file and get its file descriptor
    int fd = BF_OpenFile(filename);
    if(fd<0){
        BF_PrintError("Open file failed");
        return -1;
    }
    // allocate space for the first block_file_ptr which will hold a HT_info struct;
    if(BF_AllocateBlock(fd)<0){
        BF_PrintError("allocation failed");
        return -1;
    }
    //create a ht_info struct
    HT_info ht_info;
    ht_info.file_type = 0;
    ht_info.attr_length = attr_length;
    ht_info.attr_name = malloc(sizeof(char)*(strlen(attr_name)+1));
    strcpy(ht_info.attr_name,attr_name);
    ht_info.attr_type = attr_type;
    ht_info.buckets_num = buckets_num;
    // get the first block_file_ptr from the file from the disk to ram
    if(BF_ReadBlock(fd,0,&block_file_ptr)<0){
        BF_PrintError("read block_file_ptr failed");
        return -1;
    }
    // write the contents of the HT_info
    memcpy(block_file_ptr,&(ht_info.file_type), sizeof(int));
    block_file_ptr = (char*)block_file_ptr + sizeof(int);

    memcpy(block_file_ptr,&(ht_info.file_desc), sizeof(int));
    block_file_ptr = (char*)block_file_ptr+ sizeof(int);

    memcpy(block_file_ptr,&(ht_info.attr_type), sizeof(char));
    block_file_ptr = (char*)block_file_ptr+ sizeof(char);

    memcpy(block_file_ptr,&(ht_info.attr_length), sizeof(int));
    block_file_ptr = (char*)block_file_ptr +sizeof(int);


    memcpy(block_file_ptr,&(ht_info.attr_name),sizeof(char)*(ht_info.attr_length+1));
    block_file_ptr = (char*)block_file_ptr + sizeof(char)*(ht_info.attr_length+1);

    memcpy(block_file_ptr,&(ht_info.buckets_num), sizeof(long int));
    block_file_ptr = (char*)block_file_ptr + sizeof(long int);

    // write the block_file_ptr back inside the disk
    if(BF_WriteBlock(fd,0)<0){
        BF_PrintError("write block_file_ptr failed");
        return -1;
    }
    // create the first block_file_ptr for each bucket
    for(int i=1;i<=buckets_num;i++){
        if(BF_AllocateBlock(fd)<0){
            BF_PrintError("allocation failed");
            return -1;
        }
        // read the block from the file
        if(BF_ReadBlock(fd,i,&block_file_ptr)<0){
            BF_PrintError("read block_file_ptr failed");
            return -1;
        }
        // initalize a block_struct
        Block new_block;
        Init_Block(&new_block,i);
        // copy the contents of the new_block
        memcpy(block_file_ptr,&new_block, sizeof(Block));
        // write the block in the file
        if(BF_WriteBlock(fd,i)<0){
            BF_PrintError("write block_file_ptr failed");
            return -1;

        }
    }

    if(BF_CloseFile(fd)<0){
        BF_PrintError("close file failed");
        return -1;
    }
    return 0;
}

HT_info* HT_OpenIndex(char* filename){

    void* block_file_ptr;
    HT_info* header_info = (HT_info*)malloc(sizeof(HT_info));
    int fd;
    // open the block file
    if((fd=BF_OpenFile(filename))<0){
        BF_PrintError("open file failed");
        return NULL;
    }
    // we read the first block from the file
    if(BF_ReadBlock(fd,0,&block_file_ptr)<0){
        BF_PrintError("read file failed");
        return NULL;
    }
    // copy the contents of our block in the HT_info struct
    memcpy(&header_info->file_type,block_file_ptr, sizeof(int));
    block_file_ptr = (char*)block_file_ptr + sizeof(int);

    memcpy(&header_info->file_desc,block_file_ptr, sizeof(int));
    block_file_ptr = (char*)block_file_ptr + sizeof(int);

    memcpy(&header_info->attr_type,block_file_ptr, sizeof(char));
    block_file_ptr = (char*)block_file_ptr + sizeof(char);

    memcpy(&header_info->attr_length,block_file_ptr, sizeof(int));
    block_file_ptr = (char*)block_file_ptr + sizeof(int);

    header_info->attr_name = (char*)malloc(sizeof(char)*(header_info->attr_length+1));
    memcpy(&header_info->attr_name,block_file_ptr,sizeof(char)*(header_info->attr_length+1));
    block_file_ptr = (char*)block_file_ptr+(sizeof(char)*(header_info->attr_length+1));

    memcpy(&header_info->buckets_num,block_file_ptr, sizeof(long int));
    block_file_ptr = (char*)block_file_ptr + sizeof(long int);

    //return the struct
    return header_info;
}

int HT_CloseIndex(HT_info* header_info){
    //close the file
    if(BF_CloseFile(header_info->file_desc)<0){
        printf("close file failed");
        return -1;
    }
    // free the contents of header_info
    free(header_info->attr_name);
    free(header_info);
    return 0;
}

int HT_InsertEntry(HT_info header_info,Record record){

    // get the hash_value
    int hash_value,inserted_block_id;
    // depending on the value do the hashing
    if(header_info.attr_type=='i')
        hash_value = My_Hash_HT(&record.id, &header_info);
    else if(header_info.attr_type=='s')
        hash_value = My_Hash_HT(&record.surname, &header_info);
    else if(header_info.attr_type == 'n')
        hash_value = My_Hash_HT(&record.name, &header_info);
    else if(header_info.attr_type =='a')
        hash_value = My_Hash_HT(&record.address, &header_info);

    void* ptr_to_block;
    // reading the starting ptr_to_block block
    if(BF_ReadBlock(header_info.file_desc,hash_value,&ptr_to_block)<0){
        BF_PrintError("read block failed");
        return -1;
    }
    // search for the first block in this ptr_to_block with an empty spot
    Block current_block_strct;
    memcpy(&current_block_strct,ptr_to_block, sizeof(Block));
    while(current_block_strct.next_block!=-1){

        // if there is an empty spot due to deletion put it here
        if(current_block_strct.record_counter<current_block_strct.max_records){
            //put the record in
            current_block_strct.records[current_block_strct.record_counter]=record;
            //update counter
            current_block_strct.record_counter++;
            //get the id
            inserted_block_id = current_block_strct.my_id;
            //write the block in the memory
            printf("RECORD %d going to block with id %d\n",record.id,current_block_strct.my_id);
            memcpy(ptr_to_block,&current_block_strct, sizeof(Block));
            if(BF_WriteBlock(header_info.file_desc,current_block_strct.my_id)<0){
                BF_PrintError("write error failed");
                return -1;
            }
            return inserted_block_id;
        }
        // get the next block
        if(BF_ReadBlock(header_info.file_desc,current_block_strct.next_block,&ptr_to_block)<0){
            BF_PrintError("read block failed");
            return -1;
        }
        memcpy(&current_block_strct,ptr_to_block, sizeof(Block));
    }
    //check if there is room left
    if(current_block_strct.record_counter==current_block_strct.max_records){
        //there is no room left so we must create a new block
        void* new_block_ptr;
        Block new_block_strc;
        int new_block_id;

        if(BF_AllocateBlock(header_info.file_desc)<0){
            BF_PrintError("allocate block failed");
            return -1;
        }

        if((new_block_id=BF_GetBlockCounter(header_info.file_desc)-1)<0) {
            BF_PrintError("get block counter failed");
            return -1;
        }

        if(BF_ReadBlock(header_info.file_desc,new_block_id,&new_block_ptr)<0){
            BF_PrintError("read block failed");
            return -1;
        }

        memcpy(&new_block_strc,new_block_ptr, sizeof(Block));
        current_block_strct.next_block=new_block_id;
        Init_Block(&new_block_strc,new_block_id);
        new_block_strc.records[new_block_strc.record_counter]=record;
        new_block_strc.record_counter++;
        printf("RECORD %d going to block with id %d\n",record.id,new_block_strc.my_id);
        //write the two blocks that we changed in memory
        int currnet_block_id = current_block_strct.my_id;
        memcpy(ptr_to_block,&current_block_strct, sizeof(Block));
        memcpy(new_block_ptr,&new_block_strc, sizeof(Block));

        if(BF_WriteBlock(header_info.file_desc,currnet_block_id)<0){
            BF_PrintError("write block failed");
            return -1;
        }

        if(BF_WriteBlock(header_info.file_desc,new_block_id)<0){
            BF_PrintError("write block failed");
            return -1;
        }

        // for debugging
        BF_ReadBlock(header_info.file_desc,new_block_id,&new_block_ptr);
        memcpy(&new_block_strc,new_block_ptr, sizeof(Block));

        BF_ReadBlock(header_info.file_desc,currnet_block_id,&ptr_to_block);
        memcpy(&current_block_strct,ptr_to_block, sizeof(Block));



        return new_block_id;
    }
    else{
        //put the record inside the block
        current_block_strct.records[current_block_strct.record_counter]=record;
        current_block_strct.record_counter++;
        printf("RECORD %d going to block with id %d\n",record.id,current_block_strct.my_id);
        // keep the in to return it
        inserted_block_id = current_block_strct.my_id;
        // copy the struct to the pointer location
        memcpy(ptr_to_block,&current_block_strct, sizeof(Block));
        // write the block back in the file
        if(BF_WriteBlock(header_info.file_desc,inserted_block_id)<0){
            BF_PrintError("write block failed");
            return -1;
        }
        return inserted_block_id;
    }
}

void Show_Record(Record current_record,int* results_num){
    printf("RECORD FOUND!\n");
    printf("ID:%d,NAME:%s,SURNAME:%s,ADRESS:%s\n",current_record.id,current_record.name,current_record.surname,current_record.address);
    (*results_num)++;

}

int HT_GetAllEntries(HT_info header_info, void* value) {
    // all the entries we want will be in this bucket
    int block_id = My_Hash_HT(value, &header_info);
    int blocks_read = 0;
    int results_num = 0;
    // start searchig all the blocks in the bucket
    while (block_id != -1) {
        void *current_block_ptr;
        if (BF_ReadBlock(header_info.file_desc, block_id, &current_block_ptr) < 0) {
            BF_PrintError("read block failed");
            return -1;
        }

        Block current_block_strc;
        memcpy(&current_block_strc, current_block_ptr, sizeof(Block));
        // check every record in the bucket
        for (int i = 0; i < current_block_strc.record_counter; i++) {
            Record current_record = current_block_strc.records[i];
            // the value will have type of attr_type
            if (header_info.attr_type == 'i') {
                if (current_record.id == *(int*) value)
                    Show_Record(current_record, &results_num);
            } else if (header_info.attr_type == 'n') {
                if (!strcmp(current_record.name,value))
                    Show_Record(current_record, &results_num);
            } else if (header_info.attr_type == 's') {
                if (!strcmp(current_record.surname,value))
                    Show_Record(current_record, &results_num);
            } else if (header_info.attr_type == 'a') {
                if (!strcmp(current_record.address,value))
                    Show_Record(current_record, &results_num);
            }
        }
        // go to the next block
        block_id = current_block_strc.next_block;
        blocks_read++;
    }
    if(results_num)
        return blocks_read;
    return -1;


}

void Delete_Record(int counter,Block* current_block){
    int j = counter;
    int k = j+1;
    // move the contents of the record array so that the deleted record will leave the last spot empty
    while (k<current_block->record_counter){
        current_block->records[j] = current_block->records[k];
        j++;
        k++;
    }
    // mark the id of the record deletes as -1 and decrease the counter
    current_block->records[k-1].id = -1;
    current_block->record_counter--;
}

int HT_DeleteEntry(HT_info header_info, void* value){
    int total_deletions = 0;
    int block_id = My_Hash_HT(value, &header_info);
    // get the starting block

    // search all the way to the last block
    while(block_id!=-1){
        void* block_ptr;
        if(BF_ReadBlock(header_info.file_desc,block_id,&block_ptr)<0){
            BF_PrintError("read block failed");
            return -1;
        }
        Block current_block;
        memcpy(&current_block,block_ptr, sizeof(Block));
        // search all the records in the current block
        for(int i =0;i<current_block.record_counter;i++){
            Record current_record = current_block.records[i];

            if(header_info.attr_type=='i'){
                if(current_record.id==*(int*)value){
                    Delete_Record(i,&current_block);
                    // delete the record ,copy the block and then write the new block in the file
                    memcpy(block_ptr,&current_block,sizeof(Block));
                    if(BF_WriteBlock(header_info.file_desc,block_id)<0){
                        BF_PrintError("write block failed");
                        return -1;
                    }
                    return 0;
                }
            }else if(header_info.attr_type=='n'){
                if(!strcmp(current_record.name,value)){
                    Delete_Record(i,&current_block);
                    memcpy(block_ptr,&current_block,sizeof(Block));
                    if(BF_WriteBlock(header_info.file_desc,block_id)<0){
                        BF_PrintError("write block failed");
                        return -1;
                    }
                    return 0;
                }
            }else if(header_info.attr_type=='s'){
                if(!strcmp(current_record.surname,value)){
                    Delete_Record(i,&current_block);
                    memcpy(block_ptr,&current_block,sizeof(Block));
                    if(BF_WriteBlock(header_info.file_desc,block_id)<0){
                        BF_PrintError("write block failed");
                        return -1;
                    }
                    return 0;
                }
            }else if(header_info.attr_type=='a'){
                if(!strcmp(current_record.address,value)){
                    Delete_Record(i,&current_block);
                    memcpy(block_ptr,&current_block,sizeof(Block));
                    if(BF_WriteBlock(header_info.file_desc,block_id)<0){
                        BF_PrintError("write block failed");
                        return -1;
                    }
                    return 0;
                }
            }
        }
        // go to the next block
        block_id = current_block.next_block;
    }
    return -1;
}

int SHT_SecondaryInsertEntry(SHT_info header_info,SecondaryRecord s_record){

    // same procedure as HT_InsertEntry
    int hash_value;

    if(!strcmp(header_info.attr_name,"name"))
        hash_value = My_Hash_SHT(&s_record.record.name,&header_info);
    else if(!strcmp(header_info.attr_name,"surname"))
        hash_value = My_Hash_SHT(&s_record.record.surname,&header_info);
    else if(!strcmp(header_info.attr_name,"address"))
        hash_value = My_Hash_SHT(&s_record.record.address,&header_info);

    void* ptr_to_block;

    if(BF_ReadBlock(header_info.file_desc,hash_value,&ptr_to_block)<0){
        BF_PrintError("read block failed");
        return -1;
    }

    SecondaryBlock current_block_strct;
    memcpy(&current_block_strct,ptr_to_block, sizeof(SecondaryBlock));

    while(current_block_strct.next_block!=-1){
        if(current_block_strct.record_counter<current_block_strct.max_records){
            current_block_strct.record[current_block_strct.record_counter].record = s_record.record;
            current_block_strct.record[current_block_strct.record_counter].blockId = s_record.blockId;
            current_block_strct.record_counter++;
            printf("RECORD %d going to block with id %d\n",s_record.record.id,current_block_strct.my_id);
            memcpy(ptr_to_block,&current_block_strct, sizeof(SecondaryBlock));

            if(BF_WriteBlock(header_info.file_desc,current_block_strct.my_id)<0){
                BF_PrintError("write error failed");
                return -1;
            }
            return 0;
        }

        if(BF_ReadBlock(header_info.file_desc,current_block_strct.next_block,&ptr_to_block)<0){
            BF_PrintError("read block failed");
            return -1;
        }
        memcpy(&current_block_strct,ptr_to_block, sizeof(SecondaryBlock));
    }

    if(current_block_strct.record_counter == current_block_strct.max_records){

        void* new_block_ptr;
        SecondaryBlock new_block_strct;
        int new_block_id;

        if(BF_AllocateBlock(header_info.file_desc)<0){
            BF_PrintError("allocate block failed");
            return -1;
        }

        if((new_block_id=BF_GetBlockCounter(header_info.file_desc)-1)<0){
            BF_PrintError("get block counter failed");
            return -1;
        }

        if(BF_ReadBlock(header_info.file_desc,new_block_id,&new_block_ptr)<0){
            BF_PrintError("read block failed");
            return -1;
        }

        memcpy(&new_block_strct,new_block_ptr, sizeof(SecondaryBlock));
        current_block_strct.next_block = new_block_id;
        Init_SBlock(&new_block_strct,new_block_id);
        new_block_strct.record[new_block_strct.record_counter].record = s_record.record;
        new_block_strct.record[new_block_strct.record_counter].blockId = s_record.blockId;
        new_block_strct.record_counter++;
        printf("RECORD %d going to block with id %d\n",s_record.record.id,current_block_strct.my_id);
        int current_block_id = current_block_strct.my_id;
        memcpy(ptr_to_block,&current_block_strct, sizeof(SecondaryBlock));
        memcpy(new_block_ptr,&new_block_strct, sizeof(SecondaryBlock));

        if(BF_WriteBlock(header_info.file_desc,current_block_id)<0){
            BF_PrintError("write block failed");
            return -1;
        }

        if(BF_WriteBlock(header_info.file_desc,new_block_id)<0){
            BF_PrintError("write block failed");
            return -1;
        }

        BF_ReadBlock(header_info.file_desc,new_block_id,&new_block_ptr);
        memcpy(&new_block_strct,new_block_ptr, sizeof(SecondaryBlock));

        BF_ReadBlock(header_info.file_desc,current_block_id,&ptr_to_block);
        memcpy(&current_block_strct,ptr_to_block, sizeof(SecondaryBlock));

        return 0;
    } else{
        current_block_strct.record[current_block_strct.record_counter].record = s_record.record;
        current_block_strct.record[current_block_strct.record_counter].blockId = s_record.blockId;
        printf("RECORD %d going to block with id %d\n",s_record.record.id,current_block_strct.my_id);
        int current_block_id = current_block_strct.my_id;
        current_block_strct.record_counter++;
        memcpy(ptr_to_block,&current_block_strct, sizeof(SecondaryBlock));
        if(BF_WriteBlock(header_info.file_desc,current_block_id)<0){
            BF_PrintError("write block failed");
            return -1;
        }
        return 0;

    }


}

int SHT_CreateSecondaryIndex(char* sfileName,char* attrName,int attrLength,int buckets,char* fileName){

    //same procedure as HT_CreateIndex
    void* block_file_ptr;
    if(BF_CreateFile(sfileName)<0){
        BF_PrintError("create file failed");
        return -1;
    }

    int fd = BF_OpenFile(sfileName);
    if(fd<0){
        BF_PrintError("open file failed");
        return -1;
    }
    if(BF_AllocateBlock(fd)<0){
        BF_PrintError("allocate block failed");
        return -1;
    }

    SHT_info sht_info;
    sht_info.file_type =1;
    sht_info.file_desc =fd;
    sht_info.attr_length =attrLength;
    sht_info.attr_name = malloc(sizeof(char)*(attrLength+1));
    strcpy(sht_info.attr_name,attrName);
    sht_info.HT_filename = malloc(sizeof(char)*(strlen(fileName)+1));
    strcpy(sht_info.HT_filename,fileName);
    sht_info.buckets_num = buckets;
    sht_info.HT_filename_len =strlen(fileName);

    if(BF_ReadBlock(fd,0,&block_file_ptr)<0){
        BF_PrintError("read block_file failed");
        return -1;
    }
    memcpy(block_file_ptr,&(sht_info.file_type), sizeof(int));
    block_file_ptr = (char*)block_file_ptr + sizeof(int);

    memcpy(block_file_ptr,&(sht_info.file_desc), sizeof(int));
    block_file_ptr = (char*)block_file_ptr + sizeof(int);

    memcpy(block_file_ptr,&(sht_info.attr_length), sizeof(int));
    block_file_ptr = (char*)block_file_ptr + sizeof(int);

    memcpy(block_file_ptr,&(sht_info.attr_name), sizeof(char)*(attrLength+1));
    block_file_ptr = (char*)block_file_ptr + (sizeof(char)*(attrLength+1));

    memcpy(block_file_ptr,&(sht_info.buckets_num), sizeof(long int));
    block_file_ptr = (char*)block_file_ptr + sizeof(long int);
    // the only deference is that we also keep the HT_filename
    memcpy(block_file_ptr,&(sht_info.HT_filename_len), sizeof(size_t));
    block_file_ptr = (char*)block_file_ptr+ sizeof(size_t);

    memcpy(block_file_ptr,&(sht_info.HT_filename), sizeof(sht_info.HT_filename));
    block_file_ptr = (char*)block_file_ptr + sizeof(sht_info.HT_filename);

    if(BF_WriteBlock(fd,0)<0){
        BF_PrintError("write block failed");
        return -1;
    }

    for(int i =1;i<=buckets;i++){

        if(BF_AllocateBlock(fd)<0){
            BF_PrintError("allocation failed");
            return -1;
        }

        if(BF_ReadBlock(fd,i,&block_file_ptr)<0){
            BF_PrintError("read block_file_ptr failed");
            return -1;
        }
        // initalize a block_struct
        SecondaryBlock new_block;
        Init_SBlock(&new_block,i);

        memcpy(block_file_ptr,&new_block, sizeof(SecondaryBlock));

        if(BF_WriteBlock(fd,i)<0){
            BF_PrintError("write block_file_ptr failed");
            return -1;

        }
    }
    // sync the two files

    //1.get the total number of blocks in HT_file
    int HT_block_counter;
    if((HT_block_counter=BF_GetBlockCounter(HT_fd))<0) {
        BF_PrintError("get block couter failed");
        return -1;
    }
    // for every block in HT_file get its records and put them in SHT_file;
    for(int i =1;i<HT_block_counter;i++){
        Block HT_block;
        void* HT_block_ptr;

        if(BF_ReadBlock(HT_fd,i,&HT_block_ptr)<0){
            printf("read block failed");
            return -1;
        }
        memcpy(&HT_block,HT_block_ptr, sizeof(Block));
        // if the block is empty dont to anything
        for(int j =0;j<HT_block.record_counter;j++){
            SecondaryRecord s_record;
            s_record.blockId = i;
            s_record.record = HT_block.records[j];
            //insert the record in SHT_file;
            int res=SHT_SecondaryInsertEntry(sht_info,s_record);
        }
    }

    if(BF_CloseFile(fd)<0){
        BF_PrintError("close file failed");
        return -1;
    }
    return 0;
}

SHT_info* SHT_OpenSecondaryIndex(char* sfileName){
    // same procedure as HT_OpenIndex
    void* block_file_ptr;
    SHT_info * sht_info =(SHT_info*)malloc(sizeof(SHT_info));
    int fd;
    if((fd = BF_OpenFile(sfileName))<0){
        BF_PrintError("open file failed");
        return NULL;
    }

    if(BF_ReadBlock(fd,0,&block_file_ptr)<0){
        BF_PrintError("read block failed");
        return NULL;
    }
    memcpy(&sht_info->file_type,block_file_ptr, sizeof(int));
    block_file_ptr = (char*)block_file_ptr + sizeof(int);

    memcpy(&sht_info->file_desc,block_file_ptr, sizeof(int));
    block_file_ptr = (char*)block_file_ptr + sizeof(int);

    memcpy(&sht_info->attr_length,block_file_ptr, sizeof(int));
    block_file_ptr = (char*)block_file_ptr + sizeof(int);

    sht_info->attr_name = (char*)malloc(sizeof(char)*(sht_info->attr_length+1));
    memcpy(&sht_info->attr_name,block_file_ptr,sizeof(char)*(size_t )(sht_info->attr_length+1));
    block_file_ptr = (char*)block_file_ptr + (sizeof(char)*(sht_info->attr_length+1));

    memcpy(&sht_info->buckets_num,block_file_ptr, sizeof(long int));
    block_file_ptr = (char*)block_file_ptr + sizeof(long int);

    memcpy(&sht_info->HT_filename_len,block_file_ptr, sizeof(size_t));
    block_file_ptr = (char*)block_file_ptr + sizeof(size_t);

    sht_info->HT_filename = (char*)malloc(sizeof(char)*(sht_info->HT_filename_len+1));
    memcpy(&sht_info->HT_filename,block_file_ptr,sizeof(char)*(sht_info->HT_filename_len+1));
    block_file_ptr = (char*)block_file_ptr + (sizeof(char)*(sht_info->HT_filename_len+1));

    return sht_info;
}

int SHT_CloseSecondaryIndex(SHT_info* header_info){
    if(BF_CloseFile(header_info->file_desc)<0){
        printf("close file failed");
        return -1;
    }
    free(header_info->attr_name);
    free(header_info->HT_filename);
    free(header_info);
    return 0;

}

int FindRecordInHT(HT_info header_info_ht,int s_record_id,void* value,int ht_block_id,char* attrb_name,int* results_num){

    Block ht_block;
    void* ht_block_ptr;
    if(BF_ReadBlock(header_info_ht.file_desc,ht_block_id,&ht_block_ptr)<0){
        BF_PrintError("read block failed");
        return -1;
    }
    memcpy(&ht_block,ht_block_ptr, sizeof(Block));

    for(int i=0;i<ht_block.record_counter;i++){
        Record current_record = ht_block.records[i];
        if(!strcmp(attrb_name,"name")){
            if((!strcmp(current_record.name,value))&&(current_record.id==s_record_id)){
                Show_Record(current_record,results_num);
            }
        }
        else if(!strcmp(attrb_name,"surename")){
            if((!strcmp(current_record.surname,value))&&(current_record.id==s_record_id)){
                Show_Record(current_record,results_num);
            }
        }
        else if(!strcmp(attrb_name,"address")){
            if((!strcmp(current_record.address,value))&&(current_record.id==s_record_id)){
                Show_Record(current_record,results_num);
            }
        }
    }
}

int SHT_SecondaryGetAllEntries(SHT_info header_info_sht,HT_info header_info_ht,void* value){

    int block_id = My_Hash_SHT(value,&header_info_sht);
    int blocks_read =0;
    int results_num =0;

    while (block_id!=-1){
        void* current_sblock_ptr;
        if(BF_ReadBlock(header_info_sht.file_desc,block_id,&current_sblock_ptr)<0){
            BF_PrintError("read block failed");
            return -1;
        }
        SecondaryBlock current_sblock_strc;
        memcpy(&current_sblock_strc,current_sblock_ptr, sizeof(SecondaryBlock));

        for(int i = 0;i<current_sblock_strc.record_counter;i++){
            SecondaryRecord s_record;
            s_record.blockId = current_sblock_strc.record[i].blockId;
            s_record.record = current_sblock_strc.record[i].record;

            if(!strcmp(header_info_sht.attr_name,"name")){
                if(!strcmp(s_record.record.name,value)){
                    if(FindRecordInHT(header_info_ht,s_record.record.id,value,s_record.blockId,"name",&results_num)<0){
                        printf("find record in ht failed");
                        return -1;
                    }
                }
            }
            else if(!strcmp(header_info_sht.attr_name,"surname")){
                if(!strcmp(s_record.record.surname,value)){
                    if(FindRecordInHT(header_info_ht,s_record.record.id,value,s_record.blockId,"surename",&results_num)<0){
                        printf("find record in ht failed");
                        return -1;
                    }
                }
            }
            else if(!strcmp(header_info_sht.attr_name,"address")){
                if(!strcmp(s_record.record.address,value)){
                    if(FindRecordInHT(header_info_ht,s_record.record.id,value,s_record.blockId,"address",&results_num)<0){
                        printf("find record in ht failed");
                        return -1;
                    }
                }

            }
        }
        block_id = current_sblock_strc.next_block;
        blocks_read++;
    }
    if (results_num)
        return blocks_read;
    return -1;


}

int Print_Stats(size_t Block_type_size,int block_type,long int buckets_num,int fd){
    //for each bucket
    int max = INT_MIN;
    int min = INT_MAX;
    int rec_sum = 0;
    int block_sum =0;
    int overflow_buckets=0;
    // we know from the initialization phase that the first block in each bucket has id from 1 to buckets_num
    for(int i =1;i<=buckets_num;i++){
        int total_recs=0;
        int total_blocks=0;
        int overflow_blocks=-1;
        int cur_block_id =i;

        Block ht_block;
        SecondaryBlock sht_block;

        void* current_block_ptr;


        while (cur_block_id!=-1){
            //we traverse each block of the bucket
            if(BF_ReadBlock(fd,cur_block_id,&current_block_ptr)<0){
                printf("read block failed");
                return -1;
            }
            if(!block_type)
                memcpy(&ht_block,current_block_ptr,Block_type_size);
            else
                memcpy(&sht_block,current_block_ptr,Block_type_size);
            // the overflow blocks in each bucket will be the the times we do the loop-1 so we start overflow_blocks by -1
            overflow_blocks++;
            total_blocks++;
            if(!block_type){
                total_recs+=ht_block.record_counter;
                cur_block_id = ht_block.next_block;
            } else{
                total_recs+=sht_block.record_counter;
                cur_block_id = sht_block.next_block;
            }
        }

        printf("BUCKET WITH ID %d HAS %d OVERFLOW BLOCKS\n",i,overflow_blocks);

        if(total_recs<min)
            min = total_recs;
        if(total_recs>max)
            max = total_recs;
        if(overflow_blocks>0)
            overflow_buckets++;
        rec_sum +=total_recs;
        block_sum+=total_blocks;
    }
    printf("MIN RECORDS IN A BUCKET: %d\n",min);
    printf("MAX RECORDS IN A BUCKET: %d\n",max);
    printf("AVG RECORDS IN A BUCHET: %f\n", rec_sum/(double)buckets_num);
    printf("AVG BLOCKS IN A BUCKET: %f\n",block_sum/(double)buckets_num);
    printf("OVERFLOW BUCKETS: %d\n",overflow_buckets);

}

int HashStatistics(char* filename){

    int fd;
    fd = BF_OpenFile(filename);

    int blocks;
    if((blocks=BF_GetBlockCounter(fd))<0){
        BF_PrintError("get blocks counter failed");
        return -1;
    }

    printf("Total blocks int the file:%d\n",blocks);

    int file_type;
    void * block_ptr;
    if(BF_ReadBlock(fd,0,&block_ptr)){
        BF_PrintError("read block failed");
        return -1;
    }
    memcpy(&file_type,block_ptr, sizeof(int));
    block_ptr = (char*)block_ptr + sizeof(int);

    if(file_type==0){
        // Ht_file
        HT_info ht_info;
        ht_info.file_type=file_type;
        // get the rest of the first block info
        memcpy(&ht_info.file_desc,block_ptr, sizeof(int));
        block_ptr = (char*)block_ptr + sizeof(int);

        memcpy(&ht_info.attr_type,block_ptr, sizeof(char));
        block_ptr = (char*)block_ptr + sizeof(char);

        memcpy(&ht_info.attr_length,block_ptr, sizeof(int));
        block_ptr = (char*)block_ptr + sizeof(int);

        ht_info.attr_name = (char*)malloc(sizeof(char)*(ht_info.attr_length+1));
        memcpy(&ht_info.attr_name,block_ptr, sizeof(char)*(ht_info.attr_length+1));
        block_ptr = (char*)block_ptr + sizeof(char)*(ht_info.attr_length+1);

        memcpy(&ht_info.buckets_num,block_ptr, sizeof(long int));
        block_ptr = (char*)block_ptr + sizeof(long int);

        Print_Stats(sizeof(Block),0,ht_info.buckets_num,fd);


        free(ht_info.attr_name);
    }
    else{
        SHT_info sht_info;
        sht_info.file_type = file_type;
        //get the rest info
        memcpy(&sht_info.file_desc,block_ptr, sizeof(int));
        block_ptr = (char*)block_ptr + sizeof(int);

        memcpy(&sht_info.attr_length,block_ptr, sizeof(int));
        block_ptr=(char*)block_ptr+ sizeof(int);

        sht_info.attr_name = (char*)malloc(sizeof(char)*(sht_info.attr_length+1));
        memcpy(&sht_info.attr_name,block_ptr,sizeof(char)*(sht_info.attr_length+1));
        block_ptr = (char*)block_ptr+ (sizeof(char)*(sht_info.attr_length+1));

        memcpy(&sht_info.buckets_num,block_ptr, sizeof(long int));
        block_ptr = (char*)block_ptr + sizeof(long int);

        Print_Stats(sizeof(SecondaryBlock),1,sht_info.buckets_num,fd);

        free(sht_info.attr_name);
    }




}
