//#include "stddef.h"
#define STACK_OVERFLOW -100
#define STACK_UNDERFLOW -101
#define OUT_OF_MEMORY -102
#define INIT_SIZE 15
#define MULTIPLIER 2
#define PREC_TABLE_SIZE 19

typedef struct Tokens
{
    char *attr;
    int type;
} T;

typedef struct Stack_tag
{
    T *attr;
    int size;
    int top;
} Stack_t;
