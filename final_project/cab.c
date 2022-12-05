#include "cab.h"


cab* open_cab(char* name, uint16_t num, uint16_t dim, uint16_t* first){

    cab* buffer = (cab *) malloc(sizeof(cab));
    buffer->num = num;
    buffer->dim = dim;
    buffer->size = num * dim;
    buffer->c = 0;
    /* The minimum number of buffers needed for a CAB to avoid conflicts 
    must be equal to the number of tasks which share the CAB plus one */
    buffer->buffers = (uint16_t *) malloc(buffer->size * N_TASKS); 
    buffer->name = (char *) malloc(strlen(name));
    buffer->head = (uint16_t *) malloc(buffer->size);
    strcpy(buffer->name, name);
    buffer->head = first;
    return buffer;
}

uint16_t* reserve(cab* cab_id){
    uint16_t* buffer;
    for(int i = 0; i< N_TASKS; i++){
        if(cab_id->flags[i] == 0){
            buffer = cab_id->buffers + (i * cab_id->num);
            cab_id->flags[i] = 1;
            return buffer;
        }
    }
    
    return buffer;
} 

void put_mes(uint16_t* buf_pointer, cab* cab_id){
    cab_id->head = buf_pointer;
    int i = buf_pointer - cab_id->buffers;
    cab_id->flags[i] = 0;
    printf("%d", i);
}

uint16_t* get_mes(cab* cab_id){
    
}
void unget(uint16_t* mesgointer, cab* cab_id){
    

}
