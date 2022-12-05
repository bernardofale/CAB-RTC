#ifndef CAB_H
#define CAB_H
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

struct cab;
typedef struct cab {
    char* name; /* arbitrary character string */
    uint16_t num; /* number of messages that the CAB may contain simultaneously */
    uint16_t dim; /* dimension (in bytes) of the message type for which that CAB is dedicated */
    uint16_t size; /* num * dim */
    uint16_t* buffers; /* Pointer of the buffers contained in the CAB */
    uint16_t* head; /* Most recent buffer */
    uint16_t c;

} cab;

cab* open_cab(char* name, uint16_t num, uint16_t dim, uint16_t* first);
uint16_t* reserve(cab* cab_id);
void put_mes(uint16_t* buf_pointer, cab* cab_id);
uint16_t* get_mes(cab* cab_id);
void unget(uint16_t* mesgointer, cab* cab_id);

#endif