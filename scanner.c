/**
 * @file scanner.c
 * 
 * @brief Lexical analyzer
 * 
 * IFJ Projekt 2021, Tým 133
 * 
 * @author <xnarus00> Alexey Narush
 * @author <xkravc02> Kravchuk Marina
 * @author <xsarti00> Sartin Andrei
*/

#include "scanner.h"
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <ctype.h>

FILE* source;

void setSourceFile(FILE *f)
{
  source = f;
}

void strFree(string *s)
// funkce uvolni retezec z pameti
{
    free(s->str);
}

void strClear(string *s)
// funkce vymaze obsah retezce
{
    s->str[0] = '\0';
    s->length = 0;
}

int strAddChar(string *s1, char c)
// prida na konec retezce jeden znak
{
    if (s1->length + 1 >= s1->allocSize)
    {
        // pamet nestaci, je potreba provest realokaci
        if ((s1->str = (char*) realloc(s1->str, sizeof(char*)*(s1->length + STR_LEN_INC))) == NULL){
            return STR_ERROR;
        }
        s1->allocSize = s1->length + STR_LEN_INC;
    }
    s1->str[s1->length] = c;
    s1->length++;
    s1->str[s1->length] = '\0';
    return STR_SUCCESS;
}

int strCopyString(string *s1, string *s2)
// prekopiruje retezec s2 do s1
{
    int newLength = s2->length;
    if (newLength >= s1->allocSize)
    {
        // pamet nestaci, je potreba provest realokaci
        if ((s1->str = (char*) realloc(s1->str, newLength + 1)) == NULL)
            return STR_ERROR;
        s1->allocSize = newLength + 1;
    }
    strcpy(s1->str, s2->str);
    s1->length = newLength;
    return STR_SUCCESS;
}

int strCmpString(string *s1, string *s2)
// porovna oba retezce a vrati vysledek
{
    return strcmp(s1->str, s2->str);
}

int strCmpConstStr(string *s1, char* s2)
// porovna nas retezec s konstantnim retezcem
{
    return strcmp(s1->str, s2);
}

char *strGetStr(string *s)
// vrati textovou cast retezce
{
    return s->str;
}

int strGetLength(string *s)
// vrati delku daneho retezce
{
    return s->length;
}

int getNextToken(string *attr){
    // hlavni funkce lexikalniho analyzatoru
    int state = 0;
    int c;
    attr->allocSize = STR_LEN_INC;
    attr->length = 0;
    attr->str = malloc(sizeof(char*) * STR_LEN_INC);
    // vymazeme obsah atributu a v pripade identifikatoru
    // budeme postupne do nej vkladat jeho nazev
    strClear(attr);
    while (1) {
        c = getc(source);               // nacteni dalsiho znaku
        switch (state) {
            case 0:                     // zakladni stav automatu
                if (isspace(c))         // bila mista - ignorovat
                    state = 0;
                else if (c == '_'){      // zacatek identifikatora
                    strAddChar(attr, c);
                    state = 7;}
                else if (c == '-')      //blokovy_komentar '--[['
                    state = 2;
                else if (isalpha(c))    // identifikator nebo klicove slovo
                {
                    strAddChar(attr, c);
                    state = 7;
                } 
                else if (c == '+')        // operator +
                    state = 8;
                else if (c == '*')          // operator *
                    state = 9;
                else if (c == '/')          // operator /
                    state = 10;
                else if (c == '{') return LEFT_VINCULUM;
                else if (c == '}') return RIGHT_VINCULUM;
                else if (c == ';') return SEMICOLON;
                else if (c == ':') return COLUMN;
                else if (c == '#') return HASH;
                else if (c == '.') state = 12;
                else if (c == '<') state = 13;
                else if (c == '>') state = 14;
                else if (c == '=') state = 15;
                else if (c == '~') state = 16;
                else if (c == '"') state = 17;
                else if (isdigit(c)) {strAddChar(attr, c); state = 18;}
                else if (c == '(') return LEFT_BRACKET;
                else if (c == ')') return RIGHT_BRACKET;
                else if (c == ',') return COMMA;
                
                else if (c == EOF) return END_OF_FILE;
                else{                    
                    changeError(-10);
                    return LEX_ERROR;
                }
                break;

            case 1:
                // komentar
                if (c == '\n') state = 0;
                else if(c == EOF){
                    return END_OF_FILE;
                }
                break;

            case 2:
                // pokracovani komentaru
                if (c == '-') state = 3;
                else if(isdigit(c)){
                    strAddChar(attr, '-');
                    strAddChar(attr, c);
                    state = 18;
                }
                else{
                    ungetc(c, source);
                    state = 0;
                    return DEC;
                }
                break;

            case 3:
                // pokracovani komentaru
                if (c == '[') state = 4; 
                else state = 1; 
                break;

            case 4:
                // pokracovani komentaru
                if (c == '[') state = 5;
                else state = 1;
                break;

            case 5:
                // komentar
                if (c == ']') state = 6;
                // komentar pokracuje libovolnymi znaky, zustan ve stavu 1,
                // ale kontroluj, zda neprijde EOF (neukonceny komentar)
                if (c == EOF){                    
                    changeError(-10);
                    return LEX_ERROR;                                                    ////добавить этот случай в "", (),...
                }
                break;

            case 6:
                // komentar
                if (c == ']') state = 0;  // --[[ ]]
                else if (c == EOF){                    
                    changeError(-10);
                    return LEX_ERROR; 
                }
                else state = 5;                                                   
                break;

            case 7:
                if ((c >= 'A' && c <= 'Z') && c != ')'){                    
                    changeError(-10);
                    return LEX_ERROR;
                }
                    // identifikator nebo klicove slovo
                else if (isalnum(c) || c == '_')
                    // identifikator pokracuje
                    strAddChar(attr, c);
                else
                    // konec identifikatoru
                {
                    ungetc(c, source); // POZOR! Je potreba vratit posledni nacteny znak

                    // kontrola, zda se nejedna o klicove slovo
                    if (strCmpConstStr(attr, "do") == 0) {state = 0; return DO;}
                    else if (strCmpConstStr(attr, "else") == 0) {state = 0; return ELSE;}
                    else if (strCmpConstStr(attr, "end") == 0) {state = 0; return END;}
                    else if (strCmpConstStr(attr, "function") == 0) {state = 0; return FUNCTION;}
                    else if (strCmpConstStr(attr, "global") == 0) {state = 0; return GLOBAL;}
                    else if (strCmpConstStr(attr, "if") == 0) {state = 0; return IF;}
                    else if (strCmpConstStr(attr, "integer") == 0) {state = 0; return INTEGER;}
                    else if (strCmpConstStr(attr, "local") == 0) {state = 0; return LOCAL;}
                    else if (strCmpConstStr(attr, "nil") == 0) {state = 0; return NIL;}
                    else if (strCmpConstStr(attr, "number") == 0) {state = 0; return NUMBER;}
                    else if (strCmpConstStr(attr, "require") == 0) {state = 0; return REQUIRE;}
                    else if (strCmpConstStr(attr, "return") == 0) {state = 0; return RETURN;}
                    else if (strCmpConstStr(attr, "string") == 0) {state = 0; return STRING;}
                    else if (strCmpConstStr(attr, "then") == 0) {state = 0; return THEN;}
                    else if (strCmpConstStr(attr, "while") == 0) {state = 0; return WHILE;}
                    else if (strCmpConstStr(attr, "write") == 0) {state = 0; return WRITE;}
                    else
                    {state = 0; return ID;}          // jednalo se skutecne o identifikator
                }
                break;

            case 8:
                // pokracovani operatoru +
                if (isspace(c)) {state = 0; return INC;}
                else{
                    ungetc(c, source);
                    state = 0;
                    return INC;
                }
                break;

            case 9:
                // pokracovani operatoru *
                if (isspace(c)) {state = 0; return MULTIPLY;}
                else if (c == '(' || c == ',' || c == '#' || isalnum(c)){
                    ungetc(c, source);
                    state = 0;
                    return MULTIPLY;
                }
                else{                    
                    changeError(-10);
                    return LEX_ERROR;
                }
                break;

            case 10:
                // pokracovani operatoru /
                if (c == '/') state = 11;
                else if (isspace(c)) {state = 0; return DIV;}
                else if (c == '(' || c == ',' || c == '#' || isalnum(c)){
                    ungetc(c, source);
                    state = 0;
                    return DIV;
                }
                else{                    
                    changeError(-10);
                    return LEX_ERROR;
                }
                break;

            case 11:
                // pokracovani operatoru //
                if (isspace(c)) {state = 0; return MOD;}
                else if (c == '(' || c == ',' || c == '#' || isalnum(c)){
                    ungetc(c, source);
                    state = 0;
                    return MOD;
                }
                else{                    
                    changeError(-10);
                    return LEX_ERROR;
                }
                break;

            case 12:
                // pokracovani operatoru ..
                if (c == '.') {state = 0; return DOTDOT;}
                else{                    
                    changeError(-10);
                    return LEX_ERROR;
                }
                break;

            case 13:
                // pokracovani operatoru </<=
                if (c == '=') {state = 0; return LESSOREQUAL;}
                else if (isspace(c)) return LESS;
                else if (c == '(' || c == ',' || c == '#' || isalnum(c)){
                    ungetc(c, source);
                    state = 0;
                    return LESS;
                }
                else{                    
                    changeError(-10);
                    return LEX_ERROR;
                }
                break;

            case 14:
                // pokracovani operatoru >/>=
                if (c == '=') {state = 0; return MOREOREQUAL;}
                else if (isspace(c)){return MORE;}
                else if (c == '(' || c == ','  || c == '#' || isalnum(c)){
                    ungetc(c, source);
                    state = 0;
                    return MORE;
                }
                else{                    
                    changeError(-10);
                    return LEX_ERROR;
                }
                break;

            case 15:
                // pokracovani operatoru ==
                if (c == '=') {state = 0; return EQUAL;}
                else if (isspace(c)) return ASSIGNED;
                else{
                    ungetc(c, source);
                    state = 0;
                    return ASSIGNED;
                }
                break;

            case 16:
                // pokracovani operatoru ~=
                if (c == '=') {state = 0; return NOTEQUAL;}
                else{                    
                    changeError(-10);
                    return LEX_ERROR;
                }
                break;

            case 17:
                // pokracovani operatoru ""
                if (c == '\"') {
                    state = 0;
                    return RETEZEC;
                }
                else if (c == EOF)
                {                    
                    changeError(-10);
                    return LEX_ERROR;
                }
                else if(c == '\\'){
                    state = 21;
                }
                else if(c == ' '){
                    strAddChar(attr, '\\');
                    strAddChar(attr, '0');
                    strAddChar(attr, '3');
                    strAddChar(attr, '2');
                }
                else if(!(isalnum(c))){
                    char sub[4];
                    sprintf(sub, "\\0%d", (int)c);
                    strAddChar(attr, sub[0]);
                    strAddChar(attr, sub[1]);
                    strAddChar(attr, sub[2]);
                    strAddChar(attr, sub[3]);
                }
                else
                    strAddChar(attr, c);
                break;

            case 18:
                if (isspace(c)) return INT;
                else if (c == '.') state = 19;
                else if (c == 'e' || c == 'E'){
                    state = 20;
                }
                else if (!isdigit(c)){   
                    ungetc(c, source);
                    state = 0;
                    return INT;
                }
                strAddChar(attr, c);
                break;

            case 19:
                if (c == 'e' || c == 'E'){
                    char *point = strstr(attr->str, ".");   
                    if(strlen(point) <= 1){
                        changeError(-10);
                    }
                    state = 20;
                }
                else if (c == ')' || c == ','){
                    ungetc(c, source);
                    state = 0;
                    return FLOAT;
                }
                else if (!isspace(c) && !isdigit(c) && c != 'e' && c != 'E'){            
                    changeError(-10);
                    return LEX_ERROR;
                }
                else if (isspace(c)){
                    state = 0; 
                    return FLOAT;
                }
                strAddChar(attr, c);
                break;

            case 20:
                if (!isspace(c) && !isdigit(c) && c != '+' && c != '-' && c != ')' && c != ','){                    
                    changeError(-10);
                    return LEX_ERROR;
                }
                else if (c == ')' || c == ',' || c == ')'){
                    ungetc(c, source);
                    state = 0;
                    return FLOAT;
                }
                else if (isspace(c)){state = 0; return FLOAT;}
                strAddChar(attr, c);
                break;

            case 21:
                if(c == 'n'){
                    strAddChar(attr, '\\');
                    strAddChar(attr, '0');
                    strAddChar(attr, '1');
                    strAddChar(attr, '0');
                    state = 17;
                }
                else if(c == 't'){
                    strAddChar(attr, '\\');
                    strAddChar(attr, '0');
                    strAddChar(attr, '0');
                    strAddChar(attr, '9');
                    state = 17;
                }
                else if(c == '\"'){
                    strAddChar(attr, '\\');
                    strAddChar(attr, '0');
                    strAddChar(attr, '3');
                    strAddChar(attr, '4');
                    state = 17;
                }
                else if(c == '\\'){
                    strAddChar(attr, '\\');
                    strAddChar(attr, '0');
                    strAddChar(attr, '9');
                    strAddChar(attr, '2');
                    state = 17;
                }
                if(isdigit(c)){
                    state = 17;
                    strAddChar(attr, '\\');
                    strAddChar(attr, c);
                }
                break;
        }
    }
}