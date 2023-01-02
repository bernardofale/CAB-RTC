#include "cab.h"

static uint16_t calculateIndex(void* buf,cab* cab_id){
    uint16_t i;
    i = (buf - cab_id->buffers) / cab_id->size;
    printf("index -> %d\n", i);
    return i;
}

cab* open_cab(char* name, uint16_t num, uint16_t dim, void* first){

    cab* buffer = (cab *) malloc(sizeof(cab));
    buffer->num = num;
    buffer->dim = dim;
    buffer->size = num * dim;
    pthread_mutex_init(&buffer->mutex, NULL);
    /* The minimum number of buffers needed for a CAB to avoid conflicts 
    must be equal to the number of tasks which share the CAB plus one */
    buffer->buffers = malloc(buffer->size * (N_TASKS + 1)); 
    buffer->name = (char *) malloc(strlen(name));
    buffer->head = malloc(buffer->size);
    strcpy(buffer->name, name);
    buffer->head = first;
    return buffer;
}

void* reserve(cab* cab_id){
    void* buffer;
    pthread_mutex_lock(&cab_id->mutex);
    for(int i = 0; i< N_TASKS + 1; i++){
        buffer = cab_id->buffers + (i * cab_id->size);
        if(cab_id->flags[i] == 0 && cab_id->head != buffer){
            cab_id->flags[i] = 1;
            pthread_mutex_unlock(&cab_id->mutex);
            return buffer;
        }
    }
    pthread_mutex_unlock(&cab_id->mutex);
    printf("%s", "No buffers available!\n");
    return 0;
} 

void put_mes(void* buf_pointer, cab* cab_id){
    pthread_mutex_lock(&cab_id->mutex);
    cab_id->head = buf_pointer;
    uint16_t i = calculateIndex(buf_pointer, cab_id);
    cab_id->flags[i] = 0;
    pthread_mutex_unlock(&cab_id->mutex);
}

void* get_mes(cab* cab_id){
    void* buffer;
    pthread_mutex_lock(&cab_id->mutex);
    buffer = cab_id->head;
    uint16_t i = calculateIndex(buffer, cab_id);
    cab_id->flags[i]++;
    printf("Link counter (Get) -> %d\n", cab_id->flags[i]);
    pthread_mutex_unlock(&cab_id->mutex);
    return buffer;
}
void unget(void* mesgointer, cab* cab_id){
    pthread_mutex_lock(&cab_id->mutex);
    uint16_t i = calculateIndex(mesgointer, cab_id);
    cab_id->flags[i]--;
    printf("Link counter (Unget) -> %d\n", cab_id->flags[i]);
    pthread_mutex_unlock(&cab_id->mutex);
}

