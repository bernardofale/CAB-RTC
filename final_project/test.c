#include <stdio.h>
#include "cab.h"

int main(int argc, char** argv){
    
    cab *cab_id;
    uint16_t first = 5;
    cab_id = open_cab("CAB ONE", 1, 2, &first);
    printf("Name -> %s \nNum -> %d\nDim -> %d\nLatest message -> %d\n", cab_id->name, cab_id->num, cab_id->dim, *cab_id->head);
   
    uint16_t* buffer;
    buffer = reserve(cab_id);

    printf("%d", buffer);
}