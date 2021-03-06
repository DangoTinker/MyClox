#ifndef clox_memory_h
#define clox_memory_h

//#include <stdlib.h>
#include "common.h"
#include "object.h"
//#include "vm.h"


#define GROW_CAPACITY(capacity) \
    ((capacity) < 8 ? 8 : (capacity) * 2)

#define GROW_ARRAY(type, pointer, oldCount, newCount) \
    (type*)reallocate(pointer, sizeof(type) * (oldCount), \
        sizeof(type) * (newCount))

#define FREE_ARRAY(type, pointer, oldCount) \
    reallocate(pointer, sizeof(type) * (oldCount), 0)


#define FREE(type, pointer) reallocate(pointer, sizeof(type), 0)


#define ALLOCATE(type, count) \
    (type*)reallocate(NULL, 0, sizeof(type) * (count))

    
void* reallocate(void* pointer,size_t oldSize,size_t newSize);
// void* reallocate(void* pointer,size_t oldSize,size_t newSize){
//     if(newSize==0){
//         free(pointer);
//         return NULL;
//     }

//     void* res = realloc(pointer,newSize);

//     if(res==NULL){
//         exit(1);
//     }


//     return res;

// }



#endif