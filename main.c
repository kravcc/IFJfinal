/**
 * @file main.c
 * 
 * @brief Main function implementation
 * 
 * IFJ Projekt 2021, Tým 133
 * 
 * @author <xnarus00> Alexey Narush
 * @author <xkravc02> Kravchuk Marina
*/

#include "scanner.h"
#include "parser.h"
#include <stdlib.h>

int main(int argc, char *argv[]){
    symtable *table;
    table = initST(table);
    FILE *f = stdin;
    setSourceFile(f);
    string *token;
    token = malloc(sizeof(struct Str));
    int a;
    setTable(table);
    a = program();
    if (a != 0){
        changeError(a);    
    }
    free(token);
    free(table);
    fclose(f);

    return 0;
}