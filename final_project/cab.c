#include "cab.h"
#define N_TASKS 2

cab* open_cab(char* name, uint16_t num, uint16_t dim, uint16_t* first){

    cab* buffer = (cab *) malloc(sizeof(cab));
    buffer->num = num;
    buffer->dim = dim;
    buffer->size = num * dim;
    buffer->c = 0;
    buffer->buffers
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
    buffer = cab_id->buffers;
    
    return buffer;
} 

void put_mes(uint16_t* buf_pointer, cab* cab_id){

}

uint16_t* get_mes(cab* cab_id){
    
}
void unget(uint16_t* mesgointer, cab* cab_id){
    

}
