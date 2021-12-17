/**
 * @file parser.c
 * 
 * @brief Parser implementation
 * 
 * IFJ Projekt 2021, Tým 133
 * 
 * @author <xnarus00> Alexey Narush
 * @author <xkravc02> Kravchuk Marina
*/

#include "scanner.h"
#include "parser.h"
#include <stdlib.h>
#include <string.h>
#include "expression.h"

//------------Global variables------------------------

int token;
string attr;
string funnamesv;
char *name_func_save;
char *nameFirstFunc = NULL;
char *nameLastFunc = NULL;
bool barfoo = false;
int deep;
int retnum = 0;
symtable *table;
bool mainWasFirst = true;
int error_flag;
bool if_spotted;
bool while_spotted;
SeznamOfVars *seznam;
SeznamOfVars *head;
DLList listOfIf;
DLList listOfWhile;

//-------------
void DLL_Init( DLList *list ) {

    list->firstElement = NULL;
    list->activeElement = NULL;
    list->lastElement = NULL;
}

void DLL_Dispose( DLList *list ) {
    DLLElementPtr tmp;
    tmp = list->firstElement;
    while(tmp != NULL)
    {
        DLLElementPtr tmp2;
        tmp2 = tmp->nextElement;
        free(tmp);
        tmp = tmp2;
    }
    list->lastElement = NULL;
    list->firstElement = NULL;
    list->activeElement = NULL;
}

void DLL_InsertLast( DLList *list) {
    DLLElementPtr tmp = (DLLElementPtr)malloc(sizeof(struct ifstruct));
    if(tmp == NULL){
        return;
    }
    tmp->nextElement = NULL;
    tmp->previousElement = list->lastElement;
    if (tmp->previousElement != NULL)
    {
        tmp->count = tmp->previousElement->count + 1;
    }
    else
    {
        tmp->count = 1;
    }
    if(list->firstElement != NULL){
        list->lastElement->nextElement = tmp;
    }
    else{
        list->firstElement = tmp;
    }
    list->lastElement = tmp;
}

void DLL_Last( DLList *list ) {

    list->activeElement = list->lastElement;

}

void DLL_Previous( DLList *list) {
    if(list->activeElement != NULL){
        if(list->activeElement == list->firstElement)
            list->activeElement = NULL;
        else
            list->activeElement = list->activeElement->previousElement;    
    }
}

void DLL_Next( DLList *list ) {
    if(list->activeElement != NULL){
        if(list->activeElement == list->lastElement)
            list->activeElement = NULL;
        else
            list->activeElement = list->activeElement->nextElement;    
    }
}

int DLL_GetValueCount( DLList *list) {
    if(list->activeElement == NULL){
        return 0;
    }
    return list->activeElement->count;
}

void initSeznam()
{
    seznam = malloc(sizeof(struct seznam));
    if (seznam == NULL)
    {
        changeError(99);
        return;
    }
    seznam->name = NULL;
    seznam->next = NULL;
    seznam->first = NULL;
    return;
}

void strcpy_for_var(char *src)
{
    SeznamOfVars *tmp = malloc(sizeof (struct seznam));
    if(tmp == NULL){
        changeError(99);
        return;
    }
    if (seznam == NULL){
        
        initSeznam();
    }
    tmp->name = calloc(strlen(src) + 1, sizeof(char));
    tmp->next = NULL;
    strcpy(tmp->name, src);
    if (head == NULL)
    {   
        head = malloc(sizeof (struct seznam));
        head->name = calloc(strlen(src) + 1, sizeof(char));
        strcpy(head->name, src);
        head->next = NULL;
        seznam = head;
        seznam->first = head;
    }
    else
    {
        seznam->next = tmp;
        seznam = seznam->next;
        seznam->first = head;
        if (head->next == NULL){
            head->next = seznam;
        }
    }
    return;
}

char *strcpy_for_func(char *src, char *dst)
{
    dst = malloc(strlen(src) + 1);
    strcpy(dst, src);
    funnamesv.str = malloc(strlen(src) + 1);
    funnamesv.str = strcpy(funnamesv.str, src);
    return dst;
}

void freeSeznam()
{
    if (seznam != NULL)
    {
        SeznamOfVars *tmp;
        tmp = seznam;
        while(tmp != NULL)
        {
            SeznamOfVars *tmp2;
            tmp2 = malloc(sizeof(tmp));
            if (tmp->next != NULL){
                tmp2 = tmp->next;
            }
            else{
                free(tmp2);
                head = NULL;
                return;
            }
            free(tmp);
            tmp = tmp2;
            free(tmp2);
        }
        free(head);
    }
    free(seznam);
    return;
}

int tryGetToken()
{
    if (attr.str != NULL){
        strFree(&attr);
    }
    if ((token = getNextToken(&attr)) == -10)
    {
        changeError(1);
        return LEX_ERROR;
    }
    //printf("%d   %s\n", token, attr.str);  // to check the token
    return token;
}

void changeError(int n)
{
    if(n == -10){
        fprintf(stderr, "%d\n", 1);
        freeFunc(table->func_tree);
        free(table->var_tree);
        strFree(&attr);
        free(table);
        exit(1);
    }
    else if(n == 15){
        fprintf(stderr, "%d\n", 3);
        free(table->var_tree);
        free(table);
        strFree(&attr);
        exit(3);
    }
    else{
        fprintf(stderr, "%d\n", n);
        freeFunc(table->func_tree);
        free(table->var_tree);
        free(table);
        strFree(&attr);
        exit(n);
    }
}

void setTable(symtable *st)
{
    table = st;
}

//-------------Function check-----------------

int inputIsOK()
{
    switch (token)
    {
    case ID:            
        strcpy_for_var(attr.str);
        token = tryGetToken();
        if (token == COLUMN)
        {
            token = tryGetToken();
            if (token == STRING || token == INTEGER || token == NUMBER || token == NIL)
            {
                insertInput(seznam->name, table->func_tree, name_func_save, token);
                insertVar(&(table->var_tree), deep, seznam->name, token);
                token = tryGetToken();
                if (token == COMMA)
                {
                    token = tryGetToken();
                    if (token == ID)
                    {
                        return inputIsOK();
                    }
                }
                else if(token == RIGHT_BRACKET)
                {
                    return SYNTAX_OK;
                }
            }
        }
        changeError(2);
        return SYNTAX_ERROR;
        break;  
    case RIGHT_BRACKET:
        return SYNTAX_OK;
        break;
    default:
        changeError(2);
        return SYNTAX_ERROR;
        break;
    }
}

int GlobalinputIsOK()
{
    switch (token)
    {
    case STRING:
    case NUMBER:
    case INTEGER:  
        insertInput(attr.str, table->func_tree, name_func_save, token);
        token = tryGetToken();
        if(token == COMMA)
        {
            token = tryGetToken();
            if (token == STRING || token == INTEGER || token == NUMBER || token == NIL)
            {
                return GlobalinputIsOK();
            }
            else if(token == RIGHT_BRACKET)
            {
                return SYNTAX_OK;
            }
        }
        else if(token == RIGHT_BRACKET){
            return SYNTAX_OK;
        }
        changeError(5);
        return SEM_ERROR_FUNCPARAM;
        break;  
    case RIGHT_BRACKET:
        return SYNTAX_OK;
        break;
    default:
        changeError(5);
        return SEM_ERROR_FUNCPARAM;
        break;
    }
}

int outputIsOK()
{
    if (token == STRING || token == INTEGER || token == NUMBER || token == NIL)
    {
        retnum++;
        insertOutput(table->func_tree, token, name_func_save);
        token = tryGetToken();
        if (token == COMMA)
        {
            token = tryGetToken();
            return outputIsOK();
        }
        else 
        {   
            return SYNTAX_OK;
        }    
    }
    else{
        changeError(5);
        return SEM_ERROR_FUNCPARAM;
    }
}

int GlobalCompare(funcs tmp)
{
    switch (token)
    {
    case ID:
    strcpy_for_var(attr.str);
        tmp->in->name = attr.str;
        if ((token = tryGetToken()) == COLUMN)
        {
            token = tryGetToken();
            if (token != tmp->in->type)
            {
                changeError(5);
                return SEM_ERROR_FUNCPARAM;
            }
            insertVar(&(table->var_tree), deep, seznam->name, token);
            if ((token = tryGetToken()) == COMMA)
            {
                token = tryGetToken();
                if (token == ID)
                {
                    tmp->in = tmp->in->next;
                    return GlobalCompare(tmp);
                }
                changeError(2);
                return SYNTAX_ERROR;
            }
            else if(token == RIGHT_BRACKET)
            {
                return SYNTAX_OK;
            }
        }
        break;
    case RIGHT_BRACKET:
        if (tmp->in == NULL)
        {
            return SYNTAX_OK;
        }
        changeError(2);
        return SYNTAX_ERROR;
        break;
    default:
        changeError(2);
        return SYNTAX_ERROR;
        break;
    }
    return SYNTAX_ERROR;
}

int GlobalCompareOut(funcs tmp)
{
    switch (token)
    {
    case STRING:
    case INTEGER:
    case NUMBER:
        if (tmp->out->type != token)
        {
            changeError(5);
            return SEM_ERROR_FUNCPARAM;
        }
        if ((token = tryGetToken()) == COMMA)
        {
            token = tryGetToken();
            if (token == STRING || token == INTEGER || token == NUMBER || token == NIL)
            {
                retnum++;
                tmp->out = tmp->out->next;
                return GlobalCompareOut(tmp);
            }
            changeError(2);
            return SYNTAX_ERROR;
        }
        retnum++;
        return SYNTAX_OK;
        break;
    default:
        changeError(2);
        return SYNTAX_ERROR;
        break;
    }
}

int functionIsOK()
{
    int inputWasComplited;
    int outputWasComplited;
    int globalCheck;
    int origin;
    if (token == FUNCTION){
        origin = 2;
        token = tryGetToken();
        if (token == ID){
            name_func_save = strcpy_for_func(attr.str, name_func_save);
            funcs tmp = findFunc(table->func_tree, name_func_save);
            if (tmp == NULL)
            {
                insertFunc(name_func_save, &(table->func_tree), origin);
            }
            else if (tmp != NULL && tmp->origin == 1)
            {
                tmp->origin = 2;
                if((token = tryGetToken()) != LEFT_BRACKET){
                    changeError(2);
                    return SYNTAX_ERROR; 
                }
                token = tryGetToken();
                globalCheck = GlobalCompare(tmp);
                if (globalCheck)
                {
                    changeError(5);
                    return SEM_ERROR_FUNCPARAM;
                }
                token = tryGetToken();
                if (token == COLUMN && tmp->out != NULL)
                {
                    token = tryGetToken();
                    globalCheck = GlobalCompareOut(tmp);
                    freeSeznam();
                    GEN_START_OF_FUNCTION(name_func_save, retnum, table->func_tree, seznam, mainWasFirst);
                    return globalCheck;
                }
                else if (token != COLUMN && tmp->out == NULL)
                {
                    GEN_START_OF_FUNCTION(name_func_save, retnum, table->func_tree, seznam, mainWasFirst);
                    return SYNTAX_OK;
                }
                else{
                    changeError(2);
                    return SYNTAX_ERROR;
                }
            }
            else if (tmp != NULL)
            {
                changeError(3);
                return SEM_ERROR_DEFINE;
            }
            name_func_save = strcpy_for_func(attr.str, name_func_save);
            token = tryGetToken();
            if (token == LEFT_BRACKET){
                token = tryGetToken();
                if ((inputWasComplited = inputIsOK()))
                {
                    changeError(5);
                    return SEM_ERROR_FUNCPARAM;
                }
                else
                {
                    token = tryGetToken();
                    switch (token)
                    {
                    case COLUMN:
                        token = tryGetToken();
                        outputWasComplited = outputIsOK();
                        GEN_START_OF_FUNCTION(name_func_save, retnum, table->func_tree, seznam, mainWasFirst);
                        return outputWasComplited;
                        break;
                    default:
                        GEN_START_OF_FUNCTION(name_func_save, retnum, table->func_tree, seznam, mainWasFirst);
                        return SYNTAX_OK;
                        break;
                    }
                }
                
            }
            else{
                changeError(2);
                return SYNTAX_ERROR; 
            }
        }
        else {
            changeError(2);
            return SEM_ERROR;
        }
    }
    else if (token == GLOBAL){
        origin = 1;
        token = tryGetToken();
        if (token == ID){
            name_func_save = strcpy_for_func(attr.str, name_func_save);
            insertFunc(name_func_save, &(table->func_tree), origin);
            token = tryGetToken();
            if (token == COLUMN){
                token = tryGetToken();
                if (token == FUNCTION){
                    token = tryGetToken();
                    if (token == LEFT_BRACKET){
                    token = tryGetToken();
                        if ((inputWasComplited = GlobalinputIsOK()) != SYNTAX_OK)    
                        {
                            changeError(5);
                            return SEM_ERROR_FUNCPARAM;
                        }
                        else
                        {
                            token = tryGetToken();
                            switch (token)
                            {
                            case COLUMN:
                                token = tryGetToken();
                                outputWasComplited = outputIsOK();
                                return outputWasComplited;
                                break;
                            
                            default:
                                return SYNTAX_OK;
                                break;
                            }
                        }  
                    }
                    else{
                        changeError(2);
                        return SYNTAX_ERROR; 
                    }
                }
            }
        }
        else{
            changeError(2);
            return SYNTAX_ERROR;
        }
    }
    changeError(2);
    return SYNTAX_ERROR;
}

int functionBodyIsOK()
{
    while ((token != END) || deep != 0){
        switch (token)
        {
        case LOCAL:
            if(seznam != NULL){
                freeSeznam();
            }
            token = tryGetToken();
            if (token == ID)
            {
                vars varcheck = findVar(table->var_tree, deep, attr.str);
                funcs maybefunc = findFunc(table->func_tree, attr.str);
                if (varcheck != NULL && varcheck->deepOfVar == deep){
                    changeError(3);
                    return SEM_ERROR_DEFINE;
                }
                else if (maybefunc != NULL){
                    changeError(3);
                    return SEM_ERROR_DEFINE;
                }
                strcpy_for_var(attr.str);
                token = tryGetToken();
                if (token == COLUMN)
                {
                    token = tryGetToken();
                    if (token == INTEGER || token == STRING || token == NUMBER || token == NIL)
                    {
                        seznam = seznam->first;
                        while(seznam != NULL){
                            insertVar(&(table->var_tree), deep, seznam->name, token);
                            if(seznam->next != NULL){
                                seznam = seznam->next;
                            }
                            else{
                                break;
                            }
                        }
                        GEN_DEFVAR_VAR(seznam);
                        int type = token;
                        token = tryGetToken();
                        switch (token)
                        {
                            case ASSIGNED:
                                token = tryGetToken();
                                token = express(NULL, token, &attr, table->var_tree, table->func_tree, deep, seznam, type, &listOfIf, &listOfWhile);
                                break;
                            default:
                                break;
                        }
                        if(seznam != NULL){
                            freeSeznam();
                        }
                    }
                }
                else if(token == COMMA){
                    while (token != COLUMN){
                        if(token == COMMA){
                            token = tryGetToken();
                        }
                        vars varcheck = findVar(table->var_tree, deep, attr.str);
                        if (varcheck != NULL){
                            changeError(3);
                            return SEM_ERROR_DEFINE;
                        }
                        strcpy_for_var(attr.str);
                        token = tryGetToken();
                    }
                    if (token == COLUMN)
                    {
                        token = tryGetToken();
                        if (token == INTEGER || token == STRING || token == NUMBER || token == NIL)
                        {
                            seznam = seznam->first;
                            while(seznam != NULL){
                                insertVar(&(table->var_tree), deep, seznam->name, token);
                                if(seznam->next != NULL){
                                    seznam = seznam->next;
                                }
                                else{
                                    break;
                                }
                            }
                            int type = token;
                            seznam = seznam->first;
                            GEN_DEFVAR_VAR(seznam);
                            token = tryGetToken();
                            switch (token)
                            {
                                case ASSIGNED:
                                    token = tryGetToken();
                                    seznam = seznam->first;
                                    token = express(NULL, token, &attr, table->var_tree, table->func_tree, deep, seznam, type, &listOfIf, &listOfWhile);
                                    break;
                                default:
                                    break;
                            }
                        }
                    }
                }
                else return SYNTAX_ERROR;
            }
            else return SYNTAX_ERROR;
            break;
        case WRITE:
            token = tryGetToken();
            if(token != LEFT_BRACKET){
                changeError(2);
                return SYNTAX_ERROR;
            }
            token = tryGetToken();
            if(token == RIGHT_BRACKET){
                changeError(2);
                return SYNTAX_ERROR;
            }
            while(token != RIGHT_BRACKET){
                switch (token){
                case ID:;
                    vars vareh = findVar(table->var_tree, deep, attr.str);
                    if(vareh == NULL){
                        changeError(3);
                        return SEM_ERROR_DEFINE;
                    }
                    GEN_PRINT_WRITE(token, attr, table->var_tree, deep);
                    token = tryGetToken();
                    if(token == COMMA){
                        token = tryGetToken();
                        continue;
                    }
                    else if(token == RIGHT_BRACKET){
                        break;
                    }
                    else{
                        changeError(2);
                        return SYNTAX_ERROR;
                    }
                    break;
                case INT:
                case RETEZEC:
                case FLOAT:
                case NIL:
                    GEN_PRINT_WRITE(token, attr, table->var_tree, deep);
                    token = tryGetToken();
                    if(token == COMMA){
                        token = tryGetToken();
                        continue;
                    }
                    else if(token == RIGHT_BRACKET){
                        break;
                    }
                    else{
                        changeError(2);
                        return SYNTAX_ERROR;
                    }
                    break;
                default:
                    changeError(2);
                    return SYNTAX_ERROR;
                }
            }
            token = tryGetToken();
            break;
        case WHILE:
            if(seznam != NULL){
                freeSeznam();
            }
            DLL_InsertLast(&listOfWhile);
            DLL_Last(&listOfWhile);
            while_spotted = true;
            whileSpotted(1);
            ifORwhileWasTheLast(2);
            token = tryGetToken();
            token = express(NULL, token, &attr, table->var_tree, table->func_tree, deep, seznam, 3, &listOfIf, &listOfWhile);
            if (token != DO)
            {
                changeError(2);
                return SYNTAX_ERROR;
            }
            deep++;
            token = tryGetToken();
            break;
        case IF:
            if(seznam != NULL){
                freeSeznam();
            }
            ifSpotted(1);
            DLL_InsertLast(&listOfIf);
            DLL_Last(&listOfIf);
            ifORwhileWasTheLast(1);
            if_spotted = true; 
            token = tryGetToken();
            token = express(NULL, token, &attr, table->var_tree, table->func_tree, deep, seznam, 3, &listOfIf, &listOfWhile);
            if (token != THEN)
            {
                changeError(2);
                return SYNTAX_ERROR;
            }
            deep++;
            token = tryGetToken();
            break;
        case ELSE:
            if(seznam != NULL){
                freeSeznam();
            }
            if(!(if_spotted) && deep == 0){
                changeError(2);
                return SYNTAX_ERROR;
            }
            token = tryGetToken();
            fprintf(stdout, "JUMP ifend%d\n", DLL_GetValueCount(&listOfIf));
            fprintf(stdout, "LABEL else%d\n", DLL_GetValueCount(&listOfIf));
            break;    
        case ID:;
            funcs maybefunc = findFunc(table->func_tree, attr.str);
            if (maybefunc != NULL)
            {
                token = tryGetToken();
                if (token != LEFT_BRACKET)
                {
                    changeError(2);
                    return SYNTAX_ERROR;
                }
                token = tryGetToken();
                if(maybefunc->in == NULL){
                    if(token != RIGHT_BRACKET){
                        changeError(5);
                    }
                }
                if(maybefunc->in != NULL){
                    fprintf(stdout, "CREATEFRAME\n");
                    if(token == RIGHT_BRACKET){
                        changeError(5);
                    }
                }
                while (token != RIGHT_BRACKET)
                {   
                    int typeCheck;
                    vars maybevar = findVar(table->var_tree, deep, attr.str);
                    if(maybevar != NULL){
                        typeCheck = maybevar->type;
                    }
                    else{
                        typeCheck = token + 15;
                    }
                    if (token == COMMA)
                    {
                        token = tryGetToken();
                        continue;
                    }
                    else if (typeCheck != maybefunc->in->type)
                    {
                        changeError(5);
                        return SEM_ERROR_FUNCPARAM;
                    }
                    if(maybefunc->in->next == NULL){
                        maybefunc->in = maybefunc->in->first;
                    }
                    else{
                        maybefunc->in = maybefunc->in->next;
                    }
                    GEN_FUNC_MAIN_END(attr.str, token);
                    token = tryGetToken();
                }
                GEN_FUNC_CALL(maybefunc->name, seznam, table->func_tree);
                token = tryGetToken();
            }
            else{
                vars maybevar = findVar(table->var_tree, deep, attr.str);
                strcpy_for_var(attr.str);
                if (maybevar != NULL){
                    token = tryGetToken();
                    if (token == COMMA){
                        token = tryGetToken();
                        continue;
                    }
                    else if (token != ASSIGNED){
                        changeError(2);
                        return SYNTAX_ERROR;
                    }
                    else{
                        token = tryGetToken();
                        if (token == ID){
                            funcs maybefunc = findFunc(table->func_tree, attr.str);
                            if (maybefunc == NULL){
                                seznam = seznam->first;
                                token = express(NULL, token, &attr, table->var_tree, table->func_tree, deep, seznam, 3, &listOfIf, &listOfWhile);
                                if (token == COMMA){
                                    token = tryGetToken();
                                    break;
                                }
                                if(seznam != NULL){
                                    freeSeznam();
                                }
                                break;
                            }
                            if (maybefunc != NULL){
                                seznam = seznam->first;
                                maybefunc->out = maybefunc->out->first;
                                while(seznam != NULL && maybefunc->out != NULL){
                                    vars varfind = findVar(table->var_tree, deep, seznam->name);
                                    if (maybefunc->out->nil)
                                    {
                                        varfind->nil = true;
                                    }
                                    else
                                    {
                                        varfind->nil = false;
                                    }
                                    if (maybefunc->out->next == NULL){
                                        if(varfind->type == maybefunc->out->type){
                                            if((seznam->next == NULL && maybefunc->out->next != NULL) 
                                            || (seznam->next != NULL && maybefunc->out->next == NULL)){
                                                changeError(5);
                                                return SEM_ERROR_FUNCPARAM;
                                            }
                                        }
                                        else{
                                            changeError(5);
                                            return SEM_ERROR_FUNCPARAM;
                                        }
                                        break;
                                    }
                                    if(varfind->type == maybefunc->out->type){
                                        if(seznam->next != NULL){
                                            seznam = seznam->next;
                                        }
                                        maybefunc->out = maybefunc->out->next;
                                        if((seznam->next == NULL && maybefunc->out->next != NULL) 
                                        || (seznam->next != NULL && maybefunc->out->next == NULL)){
                                                changeError(5);
                                                return SEM_ERROR_FUNCPARAM;
                                        }
                                        
                                    }
                                    else{
                                        changeError(5);
                                        return SEM_ERROR_FUNCPARAM;
                                    }
                                }
                            }
                            token = tryGetToken();
                            if (token != LEFT_BRACKET){
                                changeError(2);
                                return SYNTAX_ERROR;
                            }
                            else{
                                token = tryGetToken();
                                if (maybefunc != NULL)
                                {   
                                    if(maybefunc->in != NULL){
                                        maybefunc->in = maybefunc->in->first;
                                        int check;
                                        if(maybefunc->in != NULL){
                                            fprintf(stdout, "CREATEFRAME\n");
                                        }
                                        while(token != RIGHT_BRACKET){
                                            if (token == ID){
                                                vars varfind = findVar(table->var_tree, deep, attr.str);
                                                check = varfind->type;
                                            }
                                            else{
                                                check = token + 15;
                                            }
                                            if (check == maybefunc->in->type || check == NIL + 15){
                                                if(maybefunc->in->next != NULL){
                                                    maybefunc->in = maybefunc->in->next;
                                                }
                                                GEN_FUNC_MAIN_END(attr.str, token);
                                                token = tryGetToken();
                                            }
                                            else if(token == COMMA){
                                                token = tryGetToken();
                                                continue;
                                            }
                                            else{
                                                changeError(5);
                                                return SEM_ERROR_FUNCPARAM;
                                            }
                                        }
                                        seznam = seznam->first;
                                        token = tryGetToken();
                                        GEN_FUNC_CALL(maybefunc->name, seznam, table->func_tree);
                                        if(seznam != NULL){
                                            freeSeznam();
                                        }
                                        break;
                                    }
                                    else if(token != RIGHT_BRACKET){
                                        changeError(2);
                                        return SYNTAX_ERROR;
                                    }
                                    else{
                                        token = tryGetToken();
                                        GEN_FUNC_CALL(maybefunc->name, seznam, table->func_tree);
                                        if(seznam != NULL){
                                            freeSeznam();
                                        }
                                        break;
                                    }
                                }
                                else{
                                    seznam = seznam->first;
                                    token = express(NULL, token, &attr, table->var_tree, table->func_tree, deep, seznam, 3, &listOfIf, &listOfWhile);
                                    if (token == COMMA){
                                        token = tryGetToken();
                                        break;
                                    }
                                }
                            }
                        }
                        else{
                            seznam = seznam->first;
                            token = express(NULL, token, &attr, table->var_tree, table->func_tree, deep, seznam, 3, &listOfIf, &listOfWhile);
                            if (token == COMMA){
                                token = tryGetToken();
                                break;
                            }
                        }
                    }
                }
                else{
                    changeError(2);
                    return SYNTAX_ERROR;
                }
            }
            break;
        case RETURN:
            token = tryGetToken();
            if (token == ID || token == INT || token == FLOAT || token == RETEZEC || token == LEFT_BRACKET || token == NIL){
                if (token == ID){
                    funcs tmp1 = findFunc(table->func_tree, attr.str);
                    vars tmp2 = findVar(table->var_tree, deep, attr.str);
                    if (tmp1 != NULL){
                        break;
                    }
                    else if(tmp2 != NULL){
                        checkSEEN(6);  
                        strcpy_for_var(tmp2->name);
                        token = express(name_func_save, token, &attr, table->var_tree, table->func_tree, deep, NULL, 3, &listOfIf, &listOfWhile);
                        fprintf(stdout, "JUMP %s_RET\n", funnamesv.str);  
                        break;
                    }
                    else {
                        return SEM_ERROR_DEFINE;
                    }
                }
                else{
                    checkSEEN(6);
                    token = express(name_func_save, token, &attr, table->var_tree, table->func_tree, deep, NULL, 3, &listOfIf, &listOfWhile);
                    fprintf(stdout, "JUMP %s_RET\n", funnamesv.str);    
                    break;
                }
            } 
            fprintf(stdout, "JUMP %s_RET\n", funnamesv.str);  
            break;
        case COMMA:
            break;
        case END:
            if(table->var_tree->deepOfVar != 0){ 
                table->var_tree = freeVarTree(table->var_tree);  
            }
            deep--;  
            if(seznam != NULL){
                if(seznam != NULL){
                    freeSeznam();
                }
            }
            if ((ifORwhileWasTheLast(0) == 1))
            {
                fprintf(stdout, "LABEL ifend%d\n", DLL_GetValueCount(&listOfIf));
                if_spotted = false;
                DLL_Previous(&listOfIf);
            }
            token = tryGetToken();
            if ((ifORwhileWasTheLast(0) == 2))
            {
                fprintf(stdout, "JUMP while%d\n", DLL_GetValueCount(&listOfWhile));
                fprintf(stdout, "LABEL whileend%d\n", DLL_GetValueCount(&listOfWhile));
                while_spotted = false;
                DLL_Previous(&listOfWhile);
            }
            if (if_spotted)
            {
                ifORwhileWasTheLast(1);
            }
            else if (while_spotted)
            {
                ifORwhileWasTheLast(2);
            }          
            break;
        default:
            changeError(2);
            return SYNTAX_ERROR;
        }
    }
    return SYNTAX_OK;
}

int syntaxCheck(){
    int result;
    while(token != END_OF_FILE){
        if (token == FUNCTION){
            result = functionIsOK();
            if (result != 0){
                changeError(result);
                return result;
            }
            result = functionBodyIsOK();
            if (result != 0){
                changeError(result);
                return result;
            }
            else if (token != END){
                changeError(2);
                return SYNTAX_ERROR;
            }
            else{
                checkSEEN(token);
                GEN_END_OF_FUNCTION(funnamesv);
                free(funnamesv.str);
                table->var_tree = freeAllVars(table->var_tree);
                deep = 0;
                retnum = 0;
                initSeznam();
            }
        }
        else if(token == GLOBAL){
            result = functionIsOK();
            if (result != 0){
                changeError(result);
                return result;
            }
            else{
                continue;
            }
        }
        else if(token == ID){
            funcs maybefunc = findFunc(table->func_tree, attr.str);
            if (maybefunc != NULL)
            {
                if((strcmp(maybefunc->name, "main"))){
                    mainWasFirst = false;
                    if (!barfoo)
                    {
                        nameFirstFunc = maybefunc->name;
                        barfoo = true;
                        fprintf(stdout, "LABEL start\n");
                    }
                    else
                    {
                        nameLastFunc = maybefunc->name;
                    }
                    
                }
                token = tryGetToken();
                if (token != LEFT_BRACKET)
                {
                    changeError(2);
                    return SYNTAX_ERROR;
                }
                token = tryGetToken();
                if(maybefunc->in != NULL){
                    maybefunc->in = maybefunc->in->first;
                }
                while (token != RIGHT_BRACKET)
                {
                    int typeCheck = token + 15;
                    if (token == COMMA)
                    {
                        token = tryGetToken();
                        continue;
                    }
                    else if (typeCheck != maybefunc->in->type)
                    {
                        changeError(5);
                        return SEM_ERROR_FUNCPARAM;
                    }
                    if (maybefunc->in->next == NULL)
                    {
                        if(strcmp(maybefunc->name, "main")){
                            GEN_FUNC_MAIN_END(attr.str, token);
                        }
                        token = tryGetToken();
                        break;
                    }
                    
                    maybefunc->in = maybefunc->in->next;
                    if(strcmp(maybefunc->name, "main")){
                        GEN_FUNC_MAIN_END(attr.str, token);
                    }
                    token = tryGetToken();
                }
            }
            if(strcmp(maybefunc->name, "main")){
                GEN_FUNC_CALL(maybefunc->name, NULL, table->func_tree);
            }
            if (maybefunc == NULL){
                changeError(2);
                return SYNTAX_ERROR;
            }
        }
        else{
            changeError(2);
            return SYNTAX_ERROR;
        }
        token = tryGetToken();
    }
    return SYNTAX_OK;
}

int program()
{
    table->func_tree = insertInbuiltFuncs(table->func_tree);
    DLL_Init(&listOfIf);
    DLL_Init(&listOfWhile);
    deep = 0;
    error_flag = 0;
    int error_flag = 0;
    token = tryGetToken();
    if (token == REQUIRE){
        token = tryGetToken();
        if ((token == RETEZEC) && (!strCmpConstStr(&attr, "ifj21")))
        {
            token = tryGetToken();
            fprintf(stdout, ".IFJcode21\n");
            fprintf(stdout, "DEFVAR GF@NILEQ$1\n");
            fprintf(stdout, "DEFVAR GF@MUL_RES$1\n");
            fprintf(stdout, "DEFVAR GF@SUB_RES$1\n");
            fprintf(stdout, "DEFVAR GF@SUM_RES$1\n");
            fprintf(stdout, "DEFVAR GF@DIV_RES$1\n");
            fprintf(stdout, "DEFVAR GF@IDIV_RES$1\n");
            fprintf(stdout, "DEFVAR GF@HASH_RES$1\n");
            fprintf(stdout, "DEFVAR GF@DOTDOT_RES$1\n");
            fprintf(stdout, "DEFVAR GF@LESS_RES$1\n");
            fprintf(stdout, "DEFVAR GF@MORE_RES$1\n");
            fprintf(stdout, "DEFVAR GF@EQUAL_RES$1\n");
            fprintf(stdout, "DEFVAR GF@LESSOREQUAL_RES$1\n");
            fprintf(stdout, "DEFVAR GF@LESSOREQUAL$1\n");
            fprintf(stdout, "DEFVAR GF@LESSOREQUAL1$1\n");
            fprintf(stdout, "DEFVAR GF@MOREOREQUAL_RES$1\n");
            fprintf(stdout, "DEFVAR GF@MOREOREQUAL$1\n");
            fprintf(stdout, "DEFVAR GF@MOREOREQUAL1$1\n");
            fprintf(stdout, "DEFVAR GF@NOTEQUAL_RES$1\n");
            fprintf(stdout, "DEFVAR GF@NOTEQUAL$1\n");
            fprintf(stdout, "DEFVAR GF@INTTONUM$1\n");
            fprintf(stdout, "JUMP $main\n\n");
            GEN_CALL_INBUILDS();
            error_flag = syntaxCheck();
            if(error_flag != 0){
                changeError(error_flag);
            }
        }
        else changeError(7);
    }
    else{
        changeError(2);
    }
    DLL_Dispose(&listOfIf);
    DLL_Dispose(&listOfWhile);
    freeFunc(table->func_tree);
    free(table->var_tree);
    free(name_func_save);
    return error_flag;
}