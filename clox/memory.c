
#include <stdlib.h>
#include "common.h"
#include "object.h"
#include "vm.h"

void* reallocate(void* pointer,size_t oldSize,size_t newSize){
    if(newSize==0){
        free(pointer);
        return NULL;
    }

    void* res = realloc(pointer,newSize);

    if(res==NULL){
        exit(1);
    }


    return res;

}
