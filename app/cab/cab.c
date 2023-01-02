#include "cab.h"

static uint16_t calculateIndex(void* buf,cab* cab_id){
    uint16_t i;
    i = (buf - cab_id->buffers) / cab_id->size;
    return i;
}

cab* open_cab(char* name, uint16_t num, uint16_t dim, void* first){

    cab* buffer = (cab *) k_malloc(sizeof(cab));
    buffer->num = num;
    buffer->dim = dim;
    buffer->size = num * dim;
    k_mutex_init(&buffer->mutex);
    memset(buffer->flags, 0, sizeof(buffer->flags));
    /* The minimum number of buffers needed for a CAB to avoid conflicts 
    must be equal to the number of tasks which share the CAB plus one */
    buffer->buffers = k_malloc(buffer->size * (N_TASKS + 1)); 
    buffer->name = (char *) k_malloc(strlen(name));
    buffer->head = k_malloc(buffer->size);
    strcpy(buffer->name, name);
    buffer->head = first;
    return buffer;
}

void* reserve(cab* cab_id){
    void* buffer;
    k_mutex_lock(&cab_id->mutex, K_FOREVER);
    for(int i = 0; i< N_TASKS + 1; i++){
        buffer = cab_id->buffers + (i * cab_id->size);
        if(cab_id->flags[i] == 0 && cab_id->head != buffer){
            cab_id->flags[i] = 1;
            k_mutex_unlock(&cab_id->mutex);
            return buffer;
        }
    }
    k_mutex_unlock(&cab_id->mutex);
    printk("No available buffers\n");
    return 0;
} 

void put_mes(void* buf_pointer, cab* cab_id){
    k_mutex_lock(&cab_id->mutex, K_FOREVER);
    cab_id->head = buf_pointer;
    uint16_t i = calculateIndex(buf_pointer, cab_id);
    cab_id->flags[i] = 0;
    k_mutex_unlock(&cab_id->mutex);
}

void* get_mes(cab* cab_id){
    void* buffer;
    k_mutex_lock(&cab_id->mutex, K_FOREVER);
    buffer = cab_id->head;
    uint16_t i = calculateIndex(buffer, cab_id);
    cab_id->flags[i]++;
    k_mutex_unlock(&cab_id->mutex);
    return buffer;
}

void unget(void* mes_pointer, cab* cab_id){
    k_mutex_lock(&cab_id->mutex, K_FOREVER);
    uint16_t i = calculateIndex(mes_pointer, cab_id);
    cab_id->flags[i]--;
    k_mutex_unlock(&cab_id->mutex);
}

