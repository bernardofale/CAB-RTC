#include "cab.h"

int main(void){

    cab* cab_id;
    cab* cab_2;
    float first[3] = {5.3, 10.1};
    uint16_t f = 0;
    
    float* mes;
    cab_id = open_cab("CAB", 2, 4, first);
    // cab_2 = open_cab("NOD", 1, 2, &f);
    // mes = first;
    // for(int i = 0; i < 3; i++){
    //     printf("Value in CAB -> %f\n", *(float *)(mes + i));
    // }
    

    float* buff;
    buff = (float *)reserve(cab_id);
    printf("%p\n", buff);

    *buff = 5.3;
    *(buff+1) = 6.8;

    float* buff2;
    buff2 = (float *) reserve(cab_id);
    printf("%p\n", buff2);

    buff2[0] = 7;
    buff2[1] = 8.2;
    put_mes(buff2, cab_id);
    put_mes(buff, cab_id);
    
    float* buff3;
    buff3 = (float *)get_mes(cab_id);
    printf("%f, %d\n", buff3[0], (uint16_t)*(buff3+1));

    // uint16_t* buff4;
    // buff4 = reserve(cab_id);
    // printf("%p\n", buff4);

    // uint16_t* buff5;
    // buff5 = get_mes(cab_id);

    // uint16_t* buff6;
    // buff6 = get_mes(cab_id);

    // uint16_t* buff9;
    // buff9 = reserve(cab_id);
    // printf("%p\n", buff9);
    
    // *buff3 = 10;
    // put_mes(buff3, cab_id);
    // uint16_t* buff8;
    // buff8 = get_mes(cab_id);

    // unget(buff6, cab_id);
    // unget(buff5, cab_id);
    // unget(buff8, cab_id);

    // *buff3 = 19;
    // put_mes(buff3, cab_id);

    // buff3 = get_mes(cab_id);
    // printf("%d\n", *buff3);
    


    return 0;
}