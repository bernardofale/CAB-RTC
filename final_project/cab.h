#ifndef CAB_H
#define CAB_H
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#define N_TASKS 2

struct cab;
typedef struct cab {
    char* name; /* arbitrary character string */
    uint16_t num; /* number of messages that the CAB may contain simultaneously */
    uint16_t dim; /* dimension (in bytes) of the message type for which that CAB is dedicated */
    uint16_t size; /* num * dim */
    void* buffers; /* Pointer of the buffers contained in the CAB */
    uint16_t flags[N_TASKS + 1]; /* Flags to identify which buffers are in use*/
    void* head; /* Most recent buffer */
    pthread_mutex_t mutex;

} cab;

cab* open_cab(char* name, uint16_t num, uint16_t dim, void* first);
void* reserve(cab* cab_id);
void put_mes(void* buf_pointer, cab* cab_id);
void* get_mes(cab* cab_id);
void unget(void* mesgointer, cab* cab_id);

#endif