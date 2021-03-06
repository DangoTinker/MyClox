#include "vm.h"
#include "common.h"
#include "debug.h"
#include "compiler.h"
#include <stdarg.h>
#include <stdio.h>
#include "value.h"
#include "object.h"

VM vm;

InterpretResult interpret(Chunk* chunk){
    vm.chunk=chunk;
    vm.ip=vm.chunk->code;
    return run();
}

InterpretResult interpret(const char* source){
    complie(source);
    return INTERPRET_OK;
}



InterpretResult run(){
#define READ_BYTE() (*vm.ip++)
#define READ_CONSTANT (vm.chunk -> constants.values[READ_BYTE()])
#define READ_STRING() AS_STRING(READ_CONSTANT())

// #define BINARY_OP(op) \
//     do{\
//         double b=pop();\
//         double a=pop();\
//         push(a op b);\
//     }while (false)
    


//     while(true){


#define BINARY_OP(valueType, op) \
    do { \
      if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) { \
        runtimeError("Operands must be numbers."); \
        return INTERPRET_RUNTIME_ERROR; \
      } \
      double b = AS_NUMBER(pop()); \
      double a = AS_NUMBER(pop()); \
      push(valueType(a op b)); \
    } while (false)



#ifdef DEBUG_TRACE_EXECUTION

    printf("          ");
    
    for(Value* i=vm.stack;i<vm.stackTop;i++){
        printf("[ ");
        printValue(*i);
        printf(" ]");
    }

    printf("\n");


    disassembleInstruction(vm.chunk,
                           (int)(vm.ip - vm.chunk->code));
#endif



        uint8_t instruction;
        instruction=READ_BYTE();
        switch ( instruction ){
            case OP_RETURN:{
                // printValue(pop());
                // printf("\n");
                return INTERPRET_OK;
            }

            case OP_CONSTANT:{
                Value constant = (vm.chunk -> constants.values[READ_BYTE()]);
                printValue(constant);
                printf("\n");
                push(constant);
                break;


            }

            case OP_NEGATE:{
                if(!IS_NUMBER(peek(0))){
                    runtimeError("Operand must be a number.");
                    return INTERPRET_RUNTIME_ERROR;
                }

                push(NUMBER_VAL(-AS_NUMBER(pop())));
                break;
            }

            case OP_ADD:  {
                if (IS_STRING(peek(0)) && IS_STRING(peek(1))) {
                    concatenate();
                }else if(IS_NUMBER(peek(0)) && IS_NUMBER(peek(1))){
                    double b = AS_NUMBER(pop());
                    double a = AS_NUMBER(pop());
                    push(NUMBER_VAL(a + b));
                }else {
                    runtimeError(
                    "Operands must be two numbers or two strings.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            case OP_SUBTRACT: BINARY_OP(NUMBER_VAL, -); break;
            case OP_MULTIPLY: BINARY_OP(NUMBER_VAL, *); break;
            case OP_DIVIDE:   BINARY_OP(NUMBER_VAL, /); break;
            
            case OP_NIL: push(NIL_VAL); break;
            case OP_TRUE: push(BOOL_VAL(true)); break;
            case OP_FALSE: push(BOOL_VAL(false)); break;
            case OP_NOT:
                push(BOOL_VAL(isFalsey(pop())));
                break;

            case OP_EQUAL: {
                Value b = pop();
                Value a = pop();
                push(BOOL_VAL(valuesEqual(a, b)));
                break;
            }
            case OP_GREATER:  BINARY_OP(BOOL_VAL, >); break;
            case OP_LESS:     BINARY_OP(BOOL_VAL, <); break;


            case OP_PRINT: {
                printValue(pop());
                printf("\n");
                break;
            }

            case OP_POP: pop(); break;

            case OP_DEFINE_GLOBAL: {
                ObjString* name = READ_STRING();
                tableSet(&vm.globals,name,peek(0));
                pop();
                break;
            }   

            case OP_GET_GLOBAL: {
                ObjString* name = READ_STRING();
                Value value;
                if (!tableGet(&vm.globals, name, &value)) {
                    runtimeError("Undefined variable '%s'.", name->chars);
                    return INTERPRET_RUNTIME_ERROR;
                }
                push(value);
                break;

            }

            case OP_SET_GLOBAL:{
                ObjString* name = READ_STRING();
                if(tableSet(&vm.globals,name,peek(0))){
                    tableDelete(&vm.globals, name); 
                    runtimeError("Undefined variable '%s'.", name->chars);
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            case OP_GET_LOCAL: {
                uint8_t slot = READ_BYTE();
                push(vm.stack[slot]);
                break;
            }
            case OP_SET_LOCAL: {
                uint8_t slot = READ_BYTE();
                vm.stack[slot] = peek(0);
                break;
            }


        }

#undef READ_BYTE
#undef READ_CONSTANT
#undef READ_STRING
#undef BINARY_OP

}










void initVM(){
    resetStack();
    vm.objects=NULL;
    initTable(&vm.globals);
    initTable(&vm.strings);
}
void freeVM(){
    freeTable(&vm.globals);
    freeTable(&vm.strings);

    freeObjects();
    
}
void freeObjects(){
    Obj* object=vm.objects;

    while(object!=NULL){
        Obj* next=object->next;
        freeObject(object);
        object = next;
    }

}

void freeObject(Obj* object){
    switch(object->type){
        case OBJ_STRING :{
            ObjString* string = (ObjString*)object;
            FREE_ARRAY(char,string->chars,string->length+1);
            FREE(ObjString,object);
        }
    }
}



static void resetStack(){
    vm.stackTop=vm.stack;
}


void push(Value value){

    *vm.stackTop=value;
    vm.stackTop++;

}
Value pop(){
    vm.stackTop--;
    return *vm.stackTop;
}


static Value peek(int distance){
    return vm.stackTop[-1-distance];
}

static void runtimeError(const char* format, ...) {
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  fputs("\n", stderr);

  size_t instruction = vm.ip - vm.chunk->code - 1;
  int line = vm.chunk->lines[instruction];
  fprintf(stderr, "[line %d] in script\n", line);

  resetStack();
}


static bool isFalsey(Value value) {
    return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}


static void concatenate(){
    ObjString* b = AS_STRING(pop());
    ObjString* a = AS_STRING(pop());
    int length= a->length + b->length;
    char* chars=ALLOCATE(char,length+1);
    memcpy(chars, a->chars, a->length);
    memcpy(chars + a->length, b->chars, b->length);
    chars[length] = '\0';
    ObjString* objString = takeString(chars,length);
    push(OBJ_VAL(ObjString));
}