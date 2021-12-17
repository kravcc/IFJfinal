/**
 * @file expression.c
 * 
 * @brief Expression parsing implementation
 * 
 * IFJ Projekt 2021, Tým 133
 * 
 * @author <xnarus00> Alexey Narush
*/

#include "expression.h"
#include "stdbool.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ctype.h>
#include "scanner.h"

//--------------Stack-----------

Stack_t* createStack(){
    Stack_t *out;
    out = malloc(sizeof(Stack_t));
    if (out == NULL) {
        exit(OUT_OF_MEMORY);
    }
    out->size = INIT_SIZE;
    out->attr = malloc(out->size * sizeof(T));
    if (out->attr == NULL) {
        free(out);
        free(out->attr);
        exit(OUT_OF_MEMORY);
    }
    out->top = 0;
    return out;
}
 
void deleteStack(Stack_t *stack){
    while(stack->top != 0){
        free(stack->attr[stack->top].attr);
        stack->top--;
    }
    stack = NULL;
    return;
}

void resize(Stack_t *stack){
    stack->size *= MULTIPLIER;
    stack->attr = realloc(stack->attr, stack->size * sizeof(T));
    if (stack->attr == NULL){
        deleteStack(stack);
        changeError(99);
        exit(STACK_OVERFLOW);
    }
}

void push(Stack_t *stack, string value, int type){
    if (stack->top >= stack->size){
        resize(stack);
    }
    stack->top++;
    stack->attr[stack->top].attr = calloc(sizeof(value.str), sizeof(char*));
    strcpy(stack->attr[stack->top].attr, value.str);
    stack->attr[stack->top].type = type;
    return;
}

void pop(Stack_t *stack, int token, string attr, int deep, SeznamOfVars *seznam, bool end, vars vartree, DLList *i, DLList *w){ 
    if (stack->top == 0){
        exit(STACK_UNDERFLOW);
    }
    char *name = NULL;
    if(seznam != NULL){
        name = seznam->name;
    }
    if(stack->attr[stack->top].type == 3){
        vars tmp2 = findVar(vartree, deep, stack->attr[stack->top].attr);
        if(tmp2 != NULL){
            stack->attr[stack->top].type = 0;
        }
    }
    EXPRESSION_FUNC(stack->attr[stack->top].attr, stack->attr[stack->top].type, end, name, i, w, vartree, deep);
    stack->top--;
}

void implode(Stack_t *stack){
    stack->size = stack->top;
    stack->attr = realloc(stack->attr, stack->size * sizeof(T));
}

//-----------------PrecTable----------------------

char precTable[PREC_TABLE_SIZE][PREC_TABLE_SIZE] =
{
//  +    -    \    *    /    (    )    i    $    =   ~=    <   <=    >   >=   str   #   ..   nil
  {'>', '>', '>', '>', '>', '<', '>', '<', '>', '>', '>', '>', '>', '>', '>', '#', '<', '#', '#'},  // +
  {'>', '>', '>', '>', '>', '<', '>', '<', '>', '>', '>', '>', '>', '>', '>', '#', '<', '#', '#'},  // -
  {'>', '>', '>', '<', '<', '<', '>', '<', '>', '>', '>', '>', '>', '>', '>', '#', '<', '#', '#'},  // \    //space after '\' is important
  {'>', '>', '>', '>', '>', '<', '>', '<', '>', '>', '>', '>', '>', '>', '>', '#', '<', '#', '#'},  // *
  {'>', '>', '>', '>', '>', '<', '>', '<', '>', '>', '>', '>', '>', '>', '>', '#', '<', '#', '#'},  // /
  {'<', '<', '<', '<', '<', '<', '=', '<', '#', '<', '<', '<', '<', '<', '<', '<', '<', '<', '#'},  // (
  {'>', '>', '>', '>', '>', '#', '>', '#', '>', '>', '>', '>', '>', '>', '>', '#', '#', '#', '#'},  // )
  {'>', '>', '>', '>', '>', '#', '>', '#', '>', '>', '>', '>', '>', '>', '>', '#', '#', '#', '#'},  // i
  {'<', '<', '<', '<', '<', '<', '#', '<', '#', '<', '<', '<', '<', '<', '<', '<', '<', '<', '#'},  // $
  {'<', '<', '<', '<', '<', '<', '>', '<', '>', '#', '#', '#', '#', '#', '#', '<', '<', '#', '<'},  // =
  {'<', '<', '<', '<', '<', '<', '>', '<', '>', '#', '#', '#', '#', '#', '#', '<', '<', '#', '<'},  // ~=
  {'<', '<', '<', '<', '<', '<', '>', '<', '>', '#', '#', '#', '#', '#', '#', '<', '<', '#', '#'},  // <
  {'<', '<', '<', '<', '<', '<', '>', '<', '>', '#', '#', '#', '#', '#', '#', '<', '<', '#', '#'},  // <=
  {'<', '<', '<', '<', '<', '<', '>', '<', '>', '#', '#', '#', '#', '#', '#', '<', '<', '#', '#'},  // >
  {'<', '<', '<', '<', '<', '<', '>', '<', '>', '#', '#', '#', '#', '#', '#', '<', '<', '#', '#'},  // >=
  {'>', '>', '#', '#', '#', '#', '>', '#', '>', '>', '>', '>', '>', '>', '>', '#', '#', '>', '#'},  // str
  {'<', '<', '<', '<', '<', '<', '>', '#', '>', '>', '>', '>', '>', '>', '>', '<', '#', '#', '#'},  // #
  {'#', '#', '#', '#', '#', '#', '>', '#', '>', '>', '>', '>', '>', '>', '>', '<', '#', '>', '#'},  // ..
  {'#', '#', '#', '#', '#', '#', '>', '#', '>', '>', '>', '#', '#', '#', '#', '#', '#', '#', '#'},  // nil
};

int TableCheck(Stack_t *stack, int token, string *attr, vars vartree, funcs functree, int deep, SeznamOfVars *seznam, bool end, int type, DLList *i, DLList *w){
    static int savetype = 0;
    if(savetype == 0){
        savetype = type;
    }
    int checktype;
    if ((token < 10 || token > 25) && token != COMMA){
        int stackVal = stack->attr[stack->top].type - 30;
        int inputNum = token - 30;
        if(token == RETEZEC || token == INT || token == FLOAT){
            
            if (type == STRING || type == INTEGER){
                if (token != type-15 && token != ID && type != 3){
                    if(stack->attr[stack->top].type != HASH && (token != 3 && token != STRING) 
                        && stack->attr[stack->top].type != RIGHT_BRACKET && stack->attr[stack->top].type != LEFT_BRACKET){
                        deleteStack(stack);
                        changeError(4);
                    }
                }
            }
            else if(type == NUMBER){
                if (token != INT || (token != FLOAT && token != ID && type != 3)){
                    deleteStack(stack);
                    changeError(4);
                }
            }
        }
        if (token == ID || token == INT || token == FLOAT){
            inputNum = 7;
        }
        else if (token == RETEZEC || token == STRING){
            inputNum = 15;
        }
        else if(token == NIL){
            if(stack->attr[stack->top].type == EQUAL || stack->attr[stack->top].type == NOTEQUAL || stack->attr[stack->top].type == 38){
                inputNum = 18;
            }
            else{
                deleteStack(stack);
                changeError(8);
            }
        }
        if (stack->attr[stack->top].type == ID || stack->attr[stack->top].type == INT || stack->attr[stack->top].type == FLOAT){
            stackVal = 7;
        }
        else if (stack->attr[stack->top].type == RETEZEC){
            stackVal = 15;
        }
        else if (stack->attr[stack->top].type == NIL){
            if(token == EQUAL || token == NOTEQUAL){
                stackVal = 18;
            }
            else{
                deleteStack(stack);
                changeError(8);
            }
        }
//---------------PrecTable Check------------
        if((stackVal == 2 || stackVal == 4) && attr->str == 0){
            return ZERO_DIVISION_ERROR;
        }
        if(stack->attr[stack->top].type == HASH && (token == 3 || token == STRING)){
            push(stack, *attr, token);
            pop(stack, token, *attr, deep, seznam, end, vartree, i, w);
            pop(stack, token, *attr, deep, seznam, end, vartree, i, w);
            token = tryGetToken();
            if ((token >= 10 || token <= 25) || token == COMMA){
                return token;
            }
        }
        if (precTable[stackVal][inputNum] == '<'){
            push(stack, *attr, token);
            token = tryGetToken();
            if (!(token > 9 && token < 48)){
                checktype = token;
                if(token == ID){
                    vars tmp2 = findVar(vartree, deep, attr->str);
                    if (tmp2 == NULL){
                        deleteStack(stack);
                        changeError(3);
                    }
                    else{
                        checktype = tmp2->type;
                    }
                }
                if ((savetype == NUMBER || savetype == INTEGER)){
                    if (checktype != INT && checktype != INTEGER && checktype != FLOAT && checktype != NUMBER && 
                        (checktype != NIL && (stack->attr[stack->top].type == EQUAL || stack->attr[stack->top].type == NOTEQUAL))){
                        deleteStack(stack);
                        changeError(6);
                    }
                }
                else if (savetype == STRING){
                    if (checktype == INT || checktype == INTEGER || checktype == FLOAT || checktype == NUMBER){
                        deleteStack(stack);
                        changeError(6);
                    }
                }
            }
            if(token == ID){
                funcs tmp = findFunc(functree, attr->str);
                if (tmp != NULL){
                    return token;
                }
                vars tmp2 = findVar(vartree, deep, attr->str);
                if (tmp2 == NULL){
                    deleteStack(stack);
                    changeError(6);
                    return SEM_ERROR_EXPRESS;
                }
                if(tmp2->type == STRING){
                    token = 3;
                }
                else if (tmp2->type == INTEGER || tmp2->type == NUMBER){
                    token = 0;
                }
                else if (tmp2->type == NIL){
                    if(stack->attr[stack->top].type == EQUAL || stack->attr[stack->top].type == NOTEQUAL){
                        stackVal = 18;
                    }
                    else{
                        deleteStack(stack);
                        changeError(8);
                    }
                }
            }
            TableCheck(stack, token, attr, vartree, functree, deep, seznam, end, type, i, w);
        }
        else if (precTable[stackVal][inputNum] == '>'){
            if (stack->attr[stack->top].type == INTEGER || stack->attr[stack->top].type == NUMBER || 
                stack->attr[stack->top].type == STRING || stack->attr[stack->top].type == NIL){
                savetype = stack->attr[stack->top].type;
            }
            else if(stack->attr[stack->top].type == ID){
                vars tmp2 = findVar(vartree, deep, stack->attr[stack->top].attr);
                savetype = tmp2->type;
            }
            pop(stack, token, *attr, deep, seznam, end, vartree, i, w);
            TableCheck(stack, token, attr, vartree, functree, deep, seznam, end, type, i, w);
        }
        else if (precTable[stackVal][inputNum] == '='){
            if (stack->attr[stack->top].type == INTEGER || stack->attr[stack->top].type == NUMBER || 
                stack->attr[stack->top].type == STRING || stack->attr[stack->top].type == NIL){
                savetype = stack->attr[stack->top].type;
            }
            else if(stack->attr[stack->top].type == ID){
                vars tmp2 = findVar(vartree, deep, stack->attr[stack->top].attr);
                savetype = tmp2->type;
            }
            pop(stack, token, *attr, deep, seznam, end, vartree, i, w);
            token = tryGetToken();
            TableCheck(stack, token, attr, vartree, functree, deep, seznam, end, type, i, w);
        }
        else{
            if(stack->attr[stack->top].type == ID || stack->attr[stack->top].type == INTEGER || stack->attr[stack->top].type == INT
                || stack->attr[stack->top].type == FLOAT || stack->attr[stack->top].type == NUMBER){
                if (token == INT){
                    if(strstr(attr->str, "-") != NULL){
                        char *saveptr;
                        string foo;
                        pop(stack, token, *attr, deep, seznam, end, vartree, i, w);
                        foo.str = malloc(sizeof(attr->str));
                        foo.str = strtok_r(attr->str, "-", &saveptr);
                        if (precTable[stackVal][1] == '<'){
                            push(stack, *attr, 31);
                        }
                        else if (precTable[stackVal][1] == '>'){
                            pop(stack, token, *attr, deep, seznam, end, vartree, i, w);
                            push(stack, *attr, 31);
                        }
                        else if (precTable[stackVal][1] == '='){
                            pop(stack, token, *attr, deep, seznam, end, vartree, i, w);
                            push(stack, *attr, 31);
                        }
                        attr->str = malloc(sizeof(saveptr));
                        strcpy(attr->str, foo.str);
                        return TableCheck(stack, token, attr, vartree, functree, deep, seznam, end, type, i, w); 
                    }
                }
            }
            funcs tmp = findFunc(functree, attr->str);
            vars tmp2 = findVar(vartree, deep, attr->str);
            if (tmp != NULL || tmp2 != NULL){
                token = ID;
                pop(stack, token, *attr, deep, seznam, true, vartree, i, w);
                savetype = 0;
                return token;
            }
            deleteStack(stack);
            changeError(6);
            return SEM_ERROR_EXPRESS;
        }
    }
    else if(token == COMMA){
        savetype = 0;
        return token;
    }
    else{
        end = true;
        int stackVal = stack->attr[stack->top].type - 30;
        vars tmppp = NULL;
        if (seznam)
        {
            tmppp = findVar(vartree, deep, seznam->name);
        }
        if (stack->attr[stack->top].type == ID || stack->attr[stack->top].type == INT 
            || stack->attr[stack->top].type == FLOAT){
            stackVal = 7;
            if (tmppp)
            {
                tmppp->nil = false;
            }
            
        }
        else if (stack->attr[stack->top].type == RETEZEC){
            stackVal = 15;
            if (tmppp)
            {
                tmppp->nil = false;
            }
        }
        else if (stack->attr[stack->top].type == NIL){

            stackVal = 18;
            if (tmppp)
            {
                tmppp->nil = true;
            }
        }
        if (precTable[stackVal][8] == '>'){
            if (token == NIL){
                char *name = NULL;
                if(seznam != NULL){
                    name = seznam->name;
                }
                EXPRESSION_FUNC("nil", token, false, name, i, w, vartree, deep);
            }
            pop(stack, token, *attr, deep, seznam, true, vartree, i, w);
            savetype = 0;
            return token;
        }
        else if (precTable[stackVal][8] == '='){
            pop(stack, token, *attr, deep, seznam, true, vartree, i, w);
            savetype = 0;
            return token;
        }
        else{
            if(stackVal == 8){
                return token;
            }
            changeError(6);
            return SEM_ERROR_EXPRESS;
        }
    }
}

int express(char *funcname, int token, string *attr, vars vartree, funcs functree, int deep, SeznamOfVars *seznam, int type, DLList *i, DLList *w)
{
    bool end = false;
    static int counter = 0;
    funcs tmp;
    Stack_t *stack = createStack();
    string buk;
    buk.str = "$";
    push(stack, buk, 38);
    if (funcname)
    {
        tmp = findFunc(functree, funcname);
        tmp->out = tmp->out->first;
        for (int i = 0; i < counter; i++)
        {
            tmp->out = tmp->out->next;
        }
        if (!((tmp->out->type == STRING && (token == NIL || token == RETEZEC || token == ID))
            || (tmp->out->type == INTEGER && (token == NIL || token == INT || token == ID))
            || (tmp->out->type == NUMBER && token != RETEZEC) || (tmp->out->type == NIL && token == NIL) ||
            (token == RIGHT_BRACKET || token == LEFT_BRACKET)))
        {
            deleteStack(stack);
            changeError(5);
        }
        if (token == NIL)
        {
            tmp->out->nil = true;
        }
        counter++;
    }
    if (token == ID || token == RETEZEC || token == LEFT_BRACKET || token == INT || token == FLOAT || token == HASH || token == NIL)
    {
        funcs tmp = findFunc(functree, attr->str);
        if (tmp != NULL){
            return token;
        }
        if (token == ID){
            vars tmp2 = findVar(vartree, deep, attr->str);
            if(tmp2 == NULL){
                deleteStack(stack);
                changeError(3);
                return SEM_ERROR_DEFINE;
            }
            if(tmp2->type == STRING){
                token = 3;
            }
            else if (tmp2->type == INTEGER || tmp2->type == NUMBER){
                token = 0;
            }
            if((tmp2->type != type && tmp2->type != type + 15) && (token >= 0 && token < 4) && type != 3){
                deleteStack(stack);
                changeError(4);
            }
        }
        if (token != type-15 && token != ID && type != 3 && token != NIL && ((token >= 0 && token < 4) || (token > 15 && token <= 18))){ 
            if(!(type == NUMBER && (token == FLOAT || token == INT))){
                deleteStack(stack);
                changeError(4);
            }
        }
        push(stack, *attr, token);
        token = tryGetToken();
        if (token == ID){
            vars tmp2 = findVar(vartree, deep, attr->str);
            if(tmp2->type == STRING){
                token = 3;
            }
            else if (tmp2->type == INTEGER || tmp2->type == NUMBER){
                token = 0;
            }
        }
        token = TableCheck(stack, token, attr, vartree, functree, deep, seznam, end, type, i, w);
        if (token == COMMA){
            if (stack != NULL){
                end = true;
                while (strcmp(stack->attr[stack->top].attr, "$") != 0){
                    pop(stack, token, *attr, deep, seznam, end, vartree, i, w);
                }
            }
            token = tryGetToken();
            if(seznam != NULL){
                seznam = seznam->next;
            }
            token = express(funcname, token, attr, vartree, functree, deep, seznam, type, i, w);
        }
        else{
            end = true;
            counter = 0;
            if (token == NIL){
                if (stack != NULL){
                    while (strcmp(stack->attr[stack->top].attr, "$") != 0){
                        pop(stack, token, *attr, deep, seznam, end, vartree, i, w);
                    }
                }
                token = tryGetToken();
                deleteStack(stack);
                return token; 
            }
            else if ((token >= 10 && token <= 25) || token == ID){
                if (stack != NULL){
                    while (strcmp(stack->attr[stack->top].attr, "$") != 0){
                        pop(stack, token, *attr, deep, seznam, end, vartree, i, w);
                    }
                }
                deleteStack(stack);
                return token; 
            }
            else{
                if (stack != NULL){
                    while (strcmp(stack->attr[stack->top].attr, "$") != 0){
                        pop(stack, token, *attr, deep, seznam, end, vartree, i, w);
                    }
                }
                deleteStack(stack);
                changeError(6);
                return SEM_ERROR_EXPRESS;
            }
        }
    }
    else{
        deleteStack(stack);
        changeError(6);
        return SEM_ERROR_EXPRESS;
    }
}