#include "cab.h"

int main(void){

    cab* cab_id;
    uint16_t first[3] = {5, 10, 1};
    cab_id = open_cab("CAB", 3, 2, first);

    for(int i = 0; i < 3; i++){
        printf("Value in CAB -> %d\n", *(cab_id->head + i));
    }
    

    uint16_t* buff;
    buff = reserve(cab_id);
    printf("%p\n", buff);

    *buff = 5;
    put_mes(buff, cab_id);

    uint16_t* buff2;
    buff2 = reserve(cab_id);
    printf("%p\n", buff2);

    *buff2 = 7;
    put_mes(buff2, cab_id);

    uint16_t* buff3;
    buff3 = reserve(cab_id);
    printf("%p\n", buff3);

    uint16_t* buff4;
    buff4 = reserve(cab_id);
    printf("%p\n", buff4);

    uint16_t* buff5;
    buff5 = get_mes(cab_id);

    uint16_t* buff6;
    buff6 = get_mes(cab_id);

    uint16_t* buff9;
    buff9 = reserve(cab_id);
    printf("%p\n", buff9);
    
    *buff3 = 10;
    put_mes(buff3, cab_id);
    uint16_t* buff8;
    buff8 = get_mes(cab_id);

    unget(buff6, cab_id);
    unget(buff5, cab_id);
    unget(buff8, cab_id);

    *buff3 = 19;
    put_mes(buff3, cab_id);

    buff3 = get_mes(cab_id);
    printf("%d\n", *buff3);
    


    return 0;
}