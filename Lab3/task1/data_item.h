#ifndef __DATA_ITEM
#define __DATA_ITEM

typedef struct Data_Item {
    unsigned int qid;
    unsigned long long time;
    // char *msg;
    char msg[50];
} Data_Item;

#endif
