/**
 * @file symtable.c
 * 
 * @brief Implementation of binary search tree
 * 
 * IFJ Projekt 2021, Tým 133
 * 
 * @author <xkravc02> Kravchuk Marina
*/


#include "scanner.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

symtable *ST;
struct inputFunc *start;
struct outputFunc *startOut;

symtable *initST(symtable *ST)
{

    ST = malloc(sizeof(struct Symtable));
    if(ST == NULL){
        changeError(99);
        return ST;
    }
    ST->func_tree = NULL;
    ST->var_tree = NULL;

    return ST;
}

////-------------------------------------------VARS----------------------------------------------////

bool insertVar(vars *var_tree, int deep, char *name, int type)     /// insert(&(ST->var_tree),...)
{
    if(*var_tree == NULL){
        *var_tree = malloc(sizeof(struct Var_tree));
        if(*var_tree == NULL){
            changeError(99);
            return false;
        }
        (*var_tree)->name = name;
        (*var_tree)->deepOfVar = deep;
        (*var_tree)->L = NULL;
        (*var_tree)->R = NULL;
        (*var_tree)->next = NULL;
        (*var_tree)->type = type;
        (*var_tree)->nil = true;
        return true;
    }
    else if ((*var_tree != NULL)&&(deep > (*var_tree)->deepOfVar))
    {
        vars tmp = malloc(sizeof(struct Var_tree));
        if (tmp == NULL)
        {
            changeError(99);
            return false;
        }
        tmp->deepOfVar = (*var_tree)->deepOfVar;
        tmp->name = (*var_tree)->name;
        tmp->L = (*var_tree)->L;
        tmp->R = (*var_tree)->R;
        tmp->next = NULL;
        tmp->type = (*var_tree)->type;
        (*var_tree)->name = name;
        (*var_tree)->deepOfVar = deep;
        (*var_tree)->L = NULL;
        (*var_tree)->R = NULL;
        (*var_tree)->next = tmp;
        (*var_tree)->type = type;
        return true;
    }
    else if (strcmp(name, (*var_tree)->name) < 0)
    {
        insertVar(&((*var_tree)->L), deep, name, type);
    }
    else if (strcmp(name, (*var_tree)->name) > 0)
    {
        insertVar(&((*var_tree)->R), deep, name, type);
    }
    else{
        return false;
    }
}

vars findVarFromTree(vars var_tree, int deep, char *name)
{
    if (var_tree == NULL)
    {
        return NULL;
    }
    
    if (strcmp(name, var_tree->name) == 0)
    {
        return var_tree;
    }
    else if (strcmp(name, var_tree->name) < 0)
    {
        findVarFromTree(var_tree->L, deep, name);
    }
    else if (strcmp(name, var_tree->name) > 0)
    {
        findVarFromTree(var_tree->R, deep, name);
    } 
}

vars findVar(vars var_tree, int deep, char *name)
{
    vars tmp = var_tree;
    if (var_tree != NULL)
    {
        if (deep >= var_tree->deepOfVar)
        {
            tmp = findVarFromTree(tmp, deep, name);
            if (tmp == NULL && deep > 0)
            {
                findVar(var_tree->next, deep-1, name);
            }
            else
            {
                return tmp;
            }
        }
        else
        {
            return NULL;
        }    
    }
    else
    {
        return NULL;
    }    
    
}

vars freeVarTree(vars var)
{
    if (var == NULL)
    {
        return var;
    }
    vars tmp = var;
    var = tmp->next;
    if(tmp->L != NULL){
        freeVarTree(tmp->L);
    }
    if(tmp->R != NULL){
        freeVarTree(tmp->R);
    }
    tmp = NULL;
    free(tmp);
    return var; 
}

vars freeAllVars(vars var)
{
    while (var != NULL)
    {
        var = freeVarTree(var);
    }
    return var;
}
////-------------------------------------FUNCTION-----------------------------------------/////

funcs insertFunc(char *name, funcs *func, int orig)
{
    if (*func == NULL)
    {
        *func = malloc(sizeof(struct Func_tree));
        if (*func == NULL)
        {
            changeError(99);
            return NULL;
        }
        (*func)->origin = orig;
        (*func)->name = name;
        (*func)->L = NULL;
        (*func)->R = NULL;
        (*func)->in = NULL;
        (*func)->out = NULL;
        return (*func);
    }
    else if (strcmp((*func)->name, name) < 0)
    {
        insertFunc(name, &((*func)->L), orig);
    }
    else if (strcmp((*func)->name, name) > 0)
    {
        insertFunc(name, &((*func)->R), orig);
    }
}

funcs findFunc(funcs func_tree, char *name)              //find(sym->func_tree, name)
{
    if (func_tree != NULL)
    {
        if (strcmp(func_tree->name, name) < 0)
        {
            findFunc(func_tree->L, name);
        }
        else if (strcmp(func_tree->name, name) > 0)
        {
            findFunc(func_tree->R, name);
        }
        else if (strcmp(func_tree->name, name) == 0)
        {
            return func_tree;
        }
    }
    else{ 
        return NULL;
    }
    
}

void insertInput(char *name_arg, funcs func, char *name_func, int type)
{
    funcs function = findFunc(func, name_func);
    if (function == NULL)
    {
        return;
    }
    if (function->in == NULL)
    {
        function->in = malloc(sizeof(struct inputFunc));
        if (function->in == NULL)
        {
            return;
        }
        function->in->name = name_arg;
        function->in->next = NULL;
        function->in->type = type;
        start = function->in;
        function->in->first = start;
        return;
    }
    inPar new_param = function->in;
    while (new_param->next != NULL)
    {
        new_param = new_param->next;
    }
    new_param->next = malloc(sizeof(struct inputFunc));
    if (new_param->next == NULL)
    {
        return;
    }
    new_param = new_param->next;
    new_param->name = name_arg;
    new_param->next = NULL;
    new_param->type = type;
    new_param->first = start;
}

void insertOutput(funcs func, int type, char *name)
{
    funcs function = findFunc(func, name);
    if (function == NULL)
    {
        changeError(99);
        return;
    }
    if (function->out == NULL)
    {
        function->out = malloc(sizeof(struct outputFunc));
        if (function->out == NULL)
        {
            return;
        }
        function->out->nil = false;
        function->out->next = NULL;
        function->out->type = type;
        startOut = function->out;
        function->out->first = startOut;
        return;
    }
    outPar new_param = function->out;
    while (new_param->next != NULL)
    {
        new_param = new_param->next;
    }
    new_param->next = malloc(sizeof(struct outputFunc));
    new_param = new_param->next;
    new_param->next = NULL;
    new_param->nil = false;
    new_param->type = type;
    new_param->first = startOut;
    if(new_param->first->next == NULL){
        new_param->first->next = new_param; 
    }
    
    return;
}

void freeFunc (funcs func)
{
    static bool orig = false;
    if (func == NULL)
    {
        return;
    }
    if(func->origin == 1){
        func->origin = 2;
        orig = true;
    }
    inPar tmp = func->in;
    if(func->in != NULL){
        if(func->in->first != NULL){
            tmp = func->in->first;
        }
    }
    while(tmp != NULL)
    {
        inPar tmp2;
        tmp2 = tmp->next;
        free(tmp);
        tmp = tmp2;
    }  
    outPar tmp3 = func->out;
    if(func->out != NULL){
        if(func->out->first != NULL){
            tmp3 = func->out->first;
        }
    }
    while(tmp3 != NULL)
    {
        outPar tmp4;
        tmp4 = tmp3->next;
        free(tmp3);
        tmp3 = tmp4;
    }
    if(func->L != NULL){
        freeFunc(func->L);
    }
    if(func->R != NULL){
        freeFunc(func->R);
    }
    if(orig){
        changeError(15);
    }
    func = NULL;
    return;
    
}

funcs insertInbuiltFuncs(funcs func)
{
    insertFunc("reads", &func, 2);
    insertOutput(func, STRING, "reads");
    insertFunc("readi", &func, 2);
    insertOutput(func, INTEGER, "readi");
    insertFunc("readn", &func, 2);
    insertOutput(func, NUMBER, "readn");

    insertFunc("tointeger", &func, 2);
    insertInput("f", func, "tointeger", NUMBER);
    insertOutput(func, INTEGER, "tointeger");

    insertFunc("substr", &func, 2);
    insertInput("s", func, "substr", STRING);
    insertInput("i", func, "substr", INTEGER);
    insertInput("j", func, "substr", INTEGER);
    insertOutput(func, STRING, "substr");

    insertFunc("ord", &func, 2);
    insertInput("s", func, "ord", STRING);
    insertInput("i", func, "ord", INTEGER);
    insertOutput(func, INTEGER, "ord");

    insertFunc("chr", &func, 2);
    insertInput("i", func, "chr", INTEGER);
    insertOutput(func, STRING, "chr");
    return func;
}
