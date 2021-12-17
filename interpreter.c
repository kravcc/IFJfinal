/**
 * @file interpreter.c
 * 
 * @brief IFJcode21 generation
 * 
 * IFJ Projekt 2021, Tým 133
 * 
 * @author <xnarus00> Alexey Narush
 * @author <xkravc02> Kravchuk Marina
 * @author <xsarti00> Sartin Andrei
 * @author <xtiuri02> Tiurin Danil
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


int count_start = 1;
int count_end = 1;
bool main_flag = false;
bool wasEQLTGT = false;

void GEN_WRITE_VAR_LITERAL(int token, char *attr){
	//funkce pro vypis hodnoty promene
    switch (token)
	{
		case INT:
			fprintf(stdout, "int@%s ", attr);
			break;
		case FLOAT:
			fprintf(stdout, "float@%a ", atof(attr));
			break;
		case RETEZEC:
			fprintf(stdout, "string@%s ", attr);
			break;
		case NIL:
		    fprintf(stdout, "nil@nil ");
            break;
        case ID:
			if (strcmp(attr, "SUM_RES") && strcmp(attr, "MUL_RES") 
			&& strcmp(attr, "SUB_RES") && strcmp(attr, "DIV_RES") 
			&& strcmp(attr, "IDIV_RES") && strcmp(attr, "HASH_RES") 
			&& strcmp(attr, "DOTDOT_RES") && strcmp(attr, "LESS_RES")
			&& strcmp(attr, "MORE_RES") && strcmp(attr, "EQUAL_RES")
			&& strcmp(attr, "LESSOREQUAL_RES") && strcmp(attr, "LESSOREQUAL")
			&& strcmp(attr, "LESSOREQUAL1") && strcmp(attr, "MOREOREQUAL_RES")
			&& strcmp(attr, "MOREOREQUAL") && strcmp(attr, "MOREOREQUAL1")
			&& strcmp(attr, "NOTEQUAL_RES") && strcmp(attr, "NOTEQUAL") && strcmp(attr, "INTTONUM"))
			{
				fprintf(stdout, "LF@%s$%d ", attr, 1);
			}
			else
			{
				fprintf(stdout, "GF@%s$%d ", attr, 1);
			}
			break;
	}
}

void GEN_PRINT_WRITE(int token, string attr, vars vartree, int deep){
	//funkce pro vypis kdyz nacteme instrukce write 
	static int i = 0;
	fprintf(stdout, "EQ GF@NILEQ$1 ");
	GEN_WRITE_VAR_LITERAL(token, attr.str);
	fprintf(stdout, "nil@nil\n");
	fprintf(stdout, "JUMPIFEQ writenil%d GF@NILEQ$1 bool@true\n", i);
    fprintf(stdout, "WRITE ");
    GEN_WRITE_VAR_LITERAL(token, attr.str);
	fprintf(stdout, "\n");
	fprintf(stdout, "JUMP endwrite%d\n", i);
	fprintf(stdout, "LABEL writenil%d\n", i);
	fprintf(stdout, "WRITE string@nil\n");
	fprintf(stdout, "LABEL endwrite%d\n", i);
	i++;
	return;
}

void GEN_MAIN_GLOBAL(char *name, int token){
	fprintf(stdout, "DEFVAR LF@in%d$1\n",count_start);
	fprintf(stdout, "MOVE LF@in%d$1 ",count_start);
	GEN_WRITE_VAR_LITERAL(token, name);
	fprintf(stdout, "\n");
	count_start++;
	return;
}

void GEN_START_OF_FUNCTION(char *attr, int value, funcs func_tree, SeznamOfVars *seznam, bool mainWas){
	funcs maybefunc = findFunc(func_tree, attr);
	if(strcmp(attr, "main")){
		fprintf(stdout, "LABEL $%s\n", attr);
		fprintf(stdout, "PUSHFRAME\n");
		for(int i = 1; i <= value; i++){
			fprintf(stdout, "DEFVAR LF@ret%d$1\n", i);
			fprintf(stdout, "MOVE LF@ret%d$1 nil@nil\n", i);
		}
	}
	else{
		if (mainWas)
		{
			fprintf(stdout, "LABEL $main\n");
		}
		else
		{
			fprintf(stdout, "JUMP back\n\n");
			fprintf(stdout, "LABEL $main\n");
			fprintf(stdout, "JUMP start\n");
			fprintf(stdout, "LABEL back\n");
		}
		
		fprintf(stdout, "CREATEFRAME\n");
		fprintf(stdout, "PUSHFRAME\n");
		main_flag = true;
	}
	if(seznam != NULL){
		seznam = seznam->first;
	}
	if(maybefunc->in != NULL){
		int counter = 1;
		if(strcmp(maybefunc->name, "main")){
			fprintf(stdout, "DEFVAR LF@%s$1\n", seznam->name);
			fprintf(stdout, "MOVE LF@%s$1 LF@in%d$1\n", seznam->name, counter);
		}
		else{
			fprintf(stdout, "DEFVAR LF@%s$1\n", seznam->name);
			fprintf(stdout, "MOVE LF@%s$1 GF@in%d$1\n", seznam->name, counter);
		}
		counter++;
		seznam = seznam->next;
		if(strcmp(maybefunc->name, "main")){
			while(maybefunc->in->next != NULL){
				fprintf(stdout, "DEFVAR LF@%s$1\n", seznam->name);
				fprintf(stdout, "MOVE LF@%s$1 LF@in%d$1\n", seznam->name, counter);
				counter++;
				maybefunc->in = maybefunc->in->next;
				seznam = seznam->next;
			}
		}
		else{
			while(maybefunc->in->next != NULL){
				fprintf(stdout, "DEFVAR LF@%s$1\n", seznam->name);
				fprintf(stdout, "MOVE LF@%s$1 GF@in%d$1\n", seznam->name, counter);
				counter++;
				maybefunc->in = maybefunc->in->next;
				seznam = seznam->next;
			}
		}
	}
}

void GEN_DEFVAR_VAR(SeznamOfVars *param){
	while(param != NULL){
		fprintf(stdout, "DEFVAR ");
		GEN_WRITE_VAR_LITERAL(0, param->name);
		fprintf(stdout, "\n");
		fprintf(stdout, "MOVE ");
		GEN_WRITE_VAR_LITERAL(0, param->name);
		GEN_WRITE_VAR_LITERAL(NIL, NULL);
		fprintf(stdout, "\n");
		if(param->next != NULL){
			param = param->next;
		}
		else{
			return;
		}
    }
}

void GEN_FUNC_MAIN_END(char* name, int token){
    fprintf(stdout, "DEFVAR TF@in%d$1\n",count_start);
	fprintf(stdout, "MOVE TF@in%d$1 ",count_start);
	GEN_WRITE_VAR_LITERAL(token, name);
	fprintf(stdout, "\n");
	count_start++;
	return;
}

void GEN_FUNC_CALL(char *name_func, SeznamOfVars *param, funcs func_tree){
	funcs maybefunc = findFunc(func_tree, name_func);
	if(maybefunc->in == NULL){
		fprintf(stdout, "CREATEFRAME\n");
	}
	
	fprintf(stdout, "CALL $%s\n", name_func);

	if(param != NULL){
		if(param->name != NULL){
			param = param->first;
			while(param->name != NULL){
				fprintf(stdout, "MOVE ");
				GEN_WRITE_VAR_LITERAL(0, param->name);
				fprintf(stdout, "TF@ret%d$1\n", count_end);
				if(param->next != NULL){
					param = param->next;
				}
				else{
					break;
				}
				count_end++;
			}
		}
	}
	count_end = 1;
	return;
}

void GEN_END_OF_FUNCTION(string attr){
    if(!strcmp(attr.str, "main")){
        fprintf(stdout, "POPFRAME\n");
    } 
	else{
		fprintf(stdout, "LABEL %s_RET\n", attr.str);
        fprintf(stdout, "POPFRAME\n");
        fprintf(stdout, "RETURN \n\n");
    }
}

int ifORwhileWasTheLast(int c)
{
	static int ifORwhile = 0;
	if (c == 1)
	{
		ifORwhile = 1; /// if
	}
	else if (c == 2)
	{
		ifORwhile = 2; /// while
	}
	
	return ifORwhile;
}

bool ifSpotted(int spotted){
	static bool here = false;
	if (spotted == 1){
		here = true;
	}
	else if (spotted == 0){
		here = false;
	}
	return here;
}

bool whileSpotted(int spotted){
	static bool here = false;
	if (spotted == 1){
		here = true;
	}
	else if (spotted == 0){
		here = false;
	}
	return here;
}

bool checkSEEN(int token){
	static bool seen[13];
	static bool count = false;
	static bool askforret = true;
	switch (token)
	{
		case 5:
			return askforret;
			break;
		case 6:
			askforret = true;
			break;
		case 7:
			askforret = false;
			break;
		case END:
			for (int i = 0; i < 14; i++){
				seen[i] = false;
			}
			count = true;
			count_start = 1;
			return false;
			break;
		case INC:
			if(seen[0] == false){
				seen[0] = true;
				return false;
			}
			count = false;
			return seen[0];
			break;
		case DEC:
			if(seen[1] == false){
				seen[1] = true;
				return false;
			}
			count = false;
			return seen[1];
			break;
		case MULTIPLY:
			if(seen[2] == false){
				seen[2] = true;
				return false;
			}
			count = false;
			return seen[2];
			break;
		case DIV:
			if(seen[3] == false){
				seen[3] = true;
				return false;
			}
			count = false;
			return seen[3];
			break;
		case MOD:
			if(seen[4] == false){
				seen[4] = true;
				return false;
			}
			count = false;
			return seen[4];
			break;
		case HASH:
			if(seen[5] == false){
				seen[5] = true;
				return false;
			}
			count = false;
			return seen[5];
			break;
		case DOTDOT:
			if(seen[6] == false){
				seen[6] = true;
				return false;
			}
			count = false;
			return seen[6];
			break;
		case LESS:
			if(seen[7] == false){
				seen[7] = true;
				return false;
			}
			count = false;
			return seen[7];
			break;
		case MORE:
			if(seen[8] == false){
				seen[8] = true;
				return false;
			}
			count = false;
			return seen[8];
			break;
		case EQUAL:
			if(seen[9] == false){
				seen[9] = true;
				return false;
			}
			count = false;
			return seen[9];
			break;
		case LESSOREQUAL:
			if(seen[10] == false){
				seen[10] = true;
				return false;
			}
			count = false;
			return seen[10];
			break;
		case MOREOREQUAL:
			if(seen[11] == false){
				seen[11] = true;
				return false;
			}
			count = false;
			return seen[11];
			break;
		case NOTEQUAL:
			if(seen[12] == false){
				seen[12] = true;
				return false;
			}
			count = false;
			return seen[12];
			break;
		default:
			return count;
			break;
	}
	return false;
}

void EXPRESSION_FUNC(char *attr, int token, bool end, char* var_name, DLList *listOfIf, DLList *listOfWhile, vars vartree, int deep){
	static char *param1;
	static char *param2;
	static int oper = 0;
	static int token1 = 0;
	static int token2 = 0;
	static int counter;
	static int type;
	static int counter1 = 1;
	static bool tmp = false;
	if (counter == 0){
		type = token;
		token1 = token;
		param1 = attr;
		counter++;
	}
	else if (counter == 1 && token == HASH){
		if(token != 35 && token != 36){
			oper = token;
			counter++;
			counter++;
		}
	}
	else if (counter == 1){
		if(token != 35 && token != 36){
			param2 = attr;
			token2 = token;
			counter++;
		}
	}
	else if (counter == 2){
		oper = token;
	}
	if (oper != 0){
		if (oper != 0){
			if((token1 == INT && token2 == FLOAT) || (token1 == FLOAT && token2 == INT)){
				if(token1 == INT && token1 == INTEGER){
					fprintf(stdout, "INT2FLOAT GF@INTTONUM$1 ");
					GEN_WRITE_VAR_LITERAL(token1, param1);
					fprintf(stdout, "\n");
					param1 = "INTTONUM";
				}
				else{
					fprintf(stdout, "INT2FLOAT GF@INTTONUM$1 ");
					GEN_WRITE_VAR_LITERAL(token2, param2);
					fprintf(stdout, "\n");
					param2 = "INTTONUM";
				}
			}
			else if((token1 == INT && token2 == ID) || (token1 == FLOAT && token2 == ID)
				|| (token1 == ID && token2 == INT) || (token1 == ID && token2 == FLOAT) || (token1 == ID && token2 == ID)){
				
				if(token1 == ID && token2 == ID){
					vars var1 = findVar(vartree, deep, param1);
					vars var2 = findVar(vartree, deep, param2);
					if(var1 != NULL && var2 != NULL){
						if(!(var1->nil) && !(var2->nil)){
							if(var1->type == NUMBER && var2->type == INTEGER){
								fprintf(stdout, "INT2FLOAT GF@INTTONUM$1 ");
								GEN_WRITE_VAR_LITERAL(token2, param2);
								fprintf(stdout, "\n");
								param2 = "INTTONUM";
							}
							else if(var1->type == INTEGER && var2->type == NUMBER){
								fprintf(stdout, "INT2FLOAT GF@INTTONUM$1 ");
								GEN_WRITE_VAR_LITERAL(token1, param1);
								fprintf(stdout, "\n");
								param1 = "INTTONUM";
							}
						}
					}
				}
				else if(token1 == ID){
					vars var = findVar(vartree, deep, param1);
					if(var != NULL){
						if(!(var->nil)){
							if(var->type == NUMBER && token2 == INT){
								fprintf(stdout, "INT2FLOAT GF@INTTONUM$1 ");
								GEN_WRITE_VAR_LITERAL(token2, param2);
								fprintf(stdout, "\n");
								param2 = "INTTONUM";
							}
							else if(var->type == INTEGER && token2 == FLOAT){
								fprintf(stdout, "INT2FLOAT GF@INTTONUM$1 ");
								GEN_WRITE_VAR_LITERAL(token1, param1);
								fprintf(stdout, "\n");
								param1 = "INTTONUM";
							}
						}
					}
				}
				else if(token2 == ID){
					vars var = findVar(vartree, deep, param2);
					if(var != NULL){
						if(!(var->nil)){
							if(var->type == NUMBER && token1 == INT){
								fprintf(stdout, "INT2FLOAT GF@INTTONUM$1 ");
								GEN_WRITE_VAR_LITERAL(token1, param1);
								fprintf(stdout, "\n");
								param1 = "INTTONUM";
							}
							else if(var->type == INTEGER && token1 == FLOAT){
								fprintf(stdout, "INT2FLOAT GF@INTTONUM$1 ");
								GEN_WRITE_VAR_LITERAL(token2, param2);
								fprintf(stdout, "\n");
								param2 = "INTTONUM";
							}
						}
					}
				}
			}
			switch (oper)
			{
				case INC:
					if(!(checkSEEN(oper))){
						if(ifORwhileWasTheLast(0) == 2 && whileSpotted(2)){
							fprintf(stdout, "LABEL while%d\n", DLL_GetValueCount(listOfWhile));
						}
						fprintf(stdout, "ADD ");
						GEN_WRITE_VAR_LITERAL(0, "SUM_RES");
						GEN_WRITE_VAR_LITERAL(token1, param1);
						GEN_WRITE_VAR_LITERAL(token2, param2);
						fprintf(stdout, "\n");					
					}
					else{
						if(ifORwhileWasTheLast(0) == 2 && whileSpotted(2)){
							fprintf(stdout, "LABEL while%d\n", DLL_GetValueCount(listOfWhile));
						}
						fprintf(stdout, "ADD ");
						GEN_WRITE_VAR_LITERAL(0, "SUM_RES");
						GEN_WRITE_VAR_LITERAL(token1, param1);
						GEN_WRITE_VAR_LITERAL(token2, param2);
						fprintf(stdout, "\n");					
					}
					param1 = "SUM_RES"; 
					token1 = 0;
					param2 = NULL;
					oper = 0;
					counter = 1;
					break;
				case DEC:
					if(!(checkSEEN(oper))){
						if(ifORwhileWasTheLast(0) == 2 && whileSpotted(2)){
							fprintf(stdout, "LABEL while%d\n", DLL_GetValueCount(listOfWhile));
						}
						fprintf(stdout, "SUB ");
						GEN_WRITE_VAR_LITERAL(0, "SUB_RES");
						GEN_WRITE_VAR_LITERAL(token1, param1);
						GEN_WRITE_VAR_LITERAL(token2, param2);
						fprintf(stdout, "\n");					
					}
					else{
						if(ifORwhileWasTheLast(0) == 2 && whileSpotted(2)){
							fprintf(stdout, "LABEL while%d\n", DLL_GetValueCount(listOfWhile));
						}
						fprintf(stdout, "SUB ");
						GEN_WRITE_VAR_LITERAL(0, "SUB_RES");
						GEN_WRITE_VAR_LITERAL(token1, param1);
						GEN_WRITE_VAR_LITERAL(token2, param2);
						fprintf(stdout, "\n");					
					}
					param1 = "SUB_RES"; 
					token1 = 0;
					param2 = NULL;
					oper = 0;
					counter = 1;
					break;
				case MULTIPLY:
					if(!(checkSEEN(oper))){
						if(ifORwhileWasTheLast(0) == 2 && whileSpotted(2)){
							fprintf(stdout, "LABEL while%d\n", DLL_GetValueCount(listOfWhile));
						}
						fprintf(stdout, "MUL ");
						GEN_WRITE_VAR_LITERAL(0, "MUL_RES");
						GEN_WRITE_VAR_LITERAL(token1, param1);
						GEN_WRITE_VAR_LITERAL(token2, param2);
						fprintf(stdout, "\n");					
					}
					else{
						if(ifORwhileWasTheLast(0) == 2 && whileSpotted(2)){
							fprintf(stdout, "LABEL while%d\n", DLL_GetValueCount(listOfWhile));
						}
						fprintf(stdout, "MUL ");
						GEN_WRITE_VAR_LITERAL(0, "MUL_RES");
						GEN_WRITE_VAR_LITERAL(token1, param1);
						GEN_WRITE_VAR_LITERAL(token2, param2);
						fprintf(stdout, "\n");					
					}
					param1 = "MUL_RES"; 
					token1 = 0;
					param2 = NULL;
					oper = 0;
					counter = 1;
					break;
				case DIV:
					if(!(checkSEEN(oper))){
						if(ifORwhileWasTheLast(0) == 2 && whileSpotted(2)){
							fprintf(stdout, "LABEL while%d\n", DLL_GetValueCount(listOfWhile));
						}
						fprintf(stdout, "DIV ");
						GEN_WRITE_VAR_LITERAL(0, "DIV_RES");
						GEN_WRITE_VAR_LITERAL(token1, param1);
						GEN_WRITE_VAR_LITERAL(token2, param2);
						fprintf(stdout, "\n");					
					}
					else{
						if(ifORwhileWasTheLast(0) == 2 && whileSpotted(2)){
							fprintf(stdout, "LABEL while%d\n", DLL_GetValueCount(listOfWhile));
						}
						fprintf(stdout, "DIV ");
						GEN_WRITE_VAR_LITERAL(0, "DIV_RES");
						GEN_WRITE_VAR_LITERAL(token1, param1);
						GEN_WRITE_VAR_LITERAL(token2, param2);
						fprintf(stdout, "\n");					
					}
					param1 = "DIV_RES"; 
					token1 = 0;
					param2 = NULL;
					oper = 0;
					counter = 1;
					break;
				case MOD:
					if(!(checkSEEN(oper))){
						if(ifORwhileWasTheLast(0) == 2 && whileSpotted(2)){
							fprintf(stdout, "LABEL while%d\n", DLL_GetValueCount(listOfWhile));
						}
						fprintf(stdout, "IDIV ");
						GEN_WRITE_VAR_LITERAL(0, "IDIV_RES");
						GEN_WRITE_VAR_LITERAL(token1, param1);
						GEN_WRITE_VAR_LITERAL(token2, param2);
						fprintf(stdout, "\n");					
					}
					else{
						if(ifORwhileWasTheLast(0) == 2 && whileSpotted(2)){
							fprintf(stdout, "LABEL while%d\n", DLL_GetValueCount(listOfWhile));
						}
						fprintf(stdout, "IDIV ");
						GEN_WRITE_VAR_LITERAL(0, "IDIV_RES");
						GEN_WRITE_VAR_LITERAL(token1, param1);
						GEN_WRITE_VAR_LITERAL(token2, param2);
						fprintf(stdout, "\n");					
					}
					param1 = "IDIV_RES"; 
					token1 = 0;
					param2 = NULL;
					oper = 0;
					counter = 1;
					break;
				case HASH:
					if(!(checkSEEN(oper))){
						if(ifORwhileWasTheLast(0) == 2 && whileSpotted(2)){
							fprintf(stdout, "LABEL while%d\n", DLL_GetValueCount(listOfWhile));
						}
						fprintf(stdout, "STRLEN ");
						GEN_WRITE_VAR_LITERAL(0, "HASH_RES");
						GEN_WRITE_VAR_LITERAL(token1, param1);
						fprintf(stdout, "\n");					
					}
					else{
						if(ifORwhileWasTheLast(0) == 2 && whileSpotted(2)){
							fprintf(stdout, "LABEL while%d\n", DLL_GetValueCount(listOfWhile));
						}
						fprintf(stdout, "STRLEN ");
						GEN_WRITE_VAR_LITERAL(0, "HASH_RES");
						GEN_WRITE_VAR_LITERAL(token1, param1);
						fprintf(stdout, "\n");					
					}
					param1 = "HASH_RES";
					token1 = 0;
					param2 = NULL;
					oper = 0;
					counter = 1;
					end = true;
					break;
				case DOTDOT:
					if(!(checkSEEN(oper))){
						if(ifORwhileWasTheLast(0) == 2 && whileSpotted(2)){
							fprintf(stdout, "LABEL while%d\n", DLL_GetValueCount(listOfWhile));
						}
						fprintf(stdout, "CONCAT ");
						GEN_WRITE_VAR_LITERAL(0, "DOTDOT_RES");
						GEN_WRITE_VAR_LITERAL(token1, param1);
						GEN_WRITE_VAR_LITERAL(token2, param2);
						fprintf(stdout, "\n");					
					}
					else{
						if(ifORwhileWasTheLast(0) == 2 && whileSpotted(2)){
							fprintf(stdout, "LABEL while%d\n", DLL_GetValueCount(listOfWhile));
						}
						fprintf(stdout, "CONCAT ");
						GEN_WRITE_VAR_LITERAL(0, "DOTDOT_RES");
						GEN_WRITE_VAR_LITERAL(token1, param1);
						GEN_WRITE_VAR_LITERAL(token2, param2);
						fprintf(stdout, "\n");					
					}
					param1 = "DOTDOT_RES";
					token1 = 0;
					param2 = NULL;
					oper = 0;
					counter = 1;
					break;
				case LESS:
					if(!(checkSEEN(oper))){
						if(ifORwhileWasTheLast(0) == 2 && whileSpotted(2)){
							fprintf(stdout, "LABEL while%d\n", DLL_GetValueCount(listOfWhile));
						}
						fprintf(stdout, "LT ");
						GEN_WRITE_VAR_LITERAL(0, "LESS_RES");
						GEN_WRITE_VAR_LITERAL(token1, param1);
						GEN_WRITE_VAR_LITERAL(token2, param2);
						fprintf(stdout, "\n");	
						wasEQLTGT = true;				
					}
					else{
						if(ifORwhileWasTheLast(0) == 2 && whileSpotted(2)){
							fprintf(stdout, "LABEL while%d\n", DLL_GetValueCount(listOfWhile));
						}
						fprintf(stdout, "LT ");
						GEN_WRITE_VAR_LITERAL(0, "LESS_RES");
						GEN_WRITE_VAR_LITERAL(token1, param1);
						GEN_WRITE_VAR_LITERAL(token2, param2);
						fprintf(stdout, "\n");	
						wasEQLTGT = true;				
					}
					wasEQLTGT = true;
					param1 = "LESS_RES";
					token1 = 0;
					param2 = NULL;
					oper = 0;
					counter = 1;
					tmp = true;
					break;
                case MORE:
					if(!(checkSEEN(oper))){
						if(ifORwhileWasTheLast(0) == 2 && whileSpotted(2)){
							fprintf(stdout, "LABEL while%d\n", DLL_GetValueCount(listOfWhile));
						}
						fprintf(stdout, "GT ");
						GEN_WRITE_VAR_LITERAL(0, "MORE_RES");
						GEN_WRITE_VAR_LITERAL(token1, param1);
						GEN_WRITE_VAR_LITERAL(token2, param2);
						fprintf(stdout, "\n");
						wasEQLTGT = true;					
					}
					else{
						if(ifORwhileWasTheLast(0) == 2 && whileSpotted(2)){
							fprintf(stdout, "LABEL while%d\n", DLL_GetValueCount(listOfWhile));
						}
						fprintf(stdout, "GT ");
						GEN_WRITE_VAR_LITERAL(0, "MORE_RES");
						GEN_WRITE_VAR_LITERAL(token1, param1);
						GEN_WRITE_VAR_LITERAL(token2, param2);
						fprintf(stdout, "\n");
						wasEQLTGT = true;					
					}
					wasEQLTGT = true;
					param1 = "MORE_RES";
					token1 = 0;
					param2 = NULL;
					oper = 0;
					counter = 1;
					tmp = true;
					break;
				case EQUAL:
					if(!(checkSEEN(oper))){
						if(ifORwhileWasTheLast(0) == 2 && whileSpotted(2)){
							fprintf(stdout, "LABEL while%d\n", DLL_GetValueCount(listOfWhile));
						}
						fprintf(stdout, "EQ ");
						GEN_WRITE_VAR_LITERAL(0, "EQUAL_RES");
						GEN_WRITE_VAR_LITERAL(token1, param1);
						GEN_WRITE_VAR_LITERAL(token2, param2);
						fprintf(stdout, "\n");
						wasEQLTGT = true;					
					}
					else{
						if(ifORwhileWasTheLast(0) == 2 && whileSpotted(2)){
							fprintf(stdout, "LABEL while%d\n", DLL_GetValueCount(listOfWhile));
						}
						fprintf(stdout, "EQ ");
						GEN_WRITE_VAR_LITERAL(0, "EQUAL_RES");
						GEN_WRITE_VAR_LITERAL(token1, param1);
						GEN_WRITE_VAR_LITERAL(token2, param2);
						fprintf(stdout, "\n");
						wasEQLTGT = true;					
					}
					wasEQLTGT = true;
					param1 = "EQUAL_RES";
					token1 = 0;
					param2 = NULL;
					oper = 0;
					counter = 1;
					tmp = true;
					break;
				case LESSOREQUAL:
					if(!(checkSEEN(oper))){
						if(ifORwhileWasTheLast(0) == 2 && whileSpotted(2)){
							fprintf(stdout, "LABEL while%d\n", DLL_GetValueCount(listOfWhile));
						}
						fprintf(stdout, "LT ");
						GEN_WRITE_VAR_LITERAL(0, "LESSOREQUAL");
						GEN_WRITE_VAR_LITERAL(token1, param1);
						GEN_WRITE_VAR_LITERAL(token2, param2);
						fprintf(stdout, "\n");
						fprintf(stdout, "EQ ");
						GEN_WRITE_VAR_LITERAL(0, "LESSOREQUAL1");
						GEN_WRITE_VAR_LITERAL(token1, param1);
						GEN_WRITE_VAR_LITERAL(token2, param2);
						fprintf(stdout, "\n");
						fprintf(stdout, "OR ");
						GEN_WRITE_VAR_LITERAL(0, "LESSOREQUAL_RES");
						GEN_WRITE_VAR_LITERAL(0, "LESSOREQUAL");
						GEN_WRITE_VAR_LITERAL(0, "LESSOREQUAL1");
						fprintf(stdout, "\n");
					}
					else{
						if(ifORwhileWasTheLast(0) == 2 && whileSpotted(2)){
							fprintf(stdout, "LABEL while%d\n", DLL_GetValueCount(listOfWhile));
						}
						fprintf(stdout, "LT ");
						GEN_WRITE_VAR_LITERAL(0, "LESSOREQUAL");
						GEN_WRITE_VAR_LITERAL(token1, param1);
						GEN_WRITE_VAR_LITERAL(token2, param2);
						fprintf(stdout, "\n");
						fprintf(stdout, "EQ ");
						GEN_WRITE_VAR_LITERAL(0, "LESSOREQUAL1");
						GEN_WRITE_VAR_LITERAL(token1, param1);
						GEN_WRITE_VAR_LITERAL(token2, param2);
						fprintf(stdout, "\n");
						fprintf(stdout, "OR ");
						GEN_WRITE_VAR_LITERAL(0, "LESSOREQUAL_RES");
						GEN_WRITE_VAR_LITERAL(0, "LESSOREQUAL");
						GEN_WRITE_VAR_LITERAL(0, "LESSOREQUAL1");
						fprintf(stdout, "\n");
					}
					wasEQLTGT = true;
					param1 = "LESSOREQUAL_RES";
					token1 = 0;
					param2 = NULL;
					oper = 0;
					counter = 1;
					tmp = true;
					break;
				case MOREOREQUAL:
					if(!(checkSEEN(oper))){
						if(ifORwhileWasTheLast(0) == 2 && whileSpotted(2)){
							fprintf(stdout, "LABEL while%d\n", DLL_GetValueCount(listOfWhile));
						}
						fprintf(stdout, "GT ");
						GEN_WRITE_VAR_LITERAL(0, "MOREOREQUAL");
						GEN_WRITE_VAR_LITERAL(token1, param1);
						GEN_WRITE_VAR_LITERAL(token2, param2);
						fprintf(stdout, "\n");
						fprintf(stdout, "EQ ");
						GEN_WRITE_VAR_LITERAL(0, "MOREOREQUAL1");
						GEN_WRITE_VAR_LITERAL(token1, param1);
						GEN_WRITE_VAR_LITERAL(token2, param2);
						fprintf(stdout, "\n");
						fprintf(stdout, "OR ");
						GEN_WRITE_VAR_LITERAL(0, "MOREOREQUAL_RES");
						GEN_WRITE_VAR_LITERAL(0, "MOREOREQUAL");
						GEN_WRITE_VAR_LITERAL(0, "MOREOREQUAL1");
						fprintf(stdout, "\n");
					}
					else{
						if(ifORwhileWasTheLast(0) == 2 && whileSpotted(2)){
							fprintf(stdout, "LABEL while%d\n", DLL_GetValueCount(listOfWhile));
						}
						fprintf(stdout, "GT ");
						GEN_WRITE_VAR_LITERAL(0, "MOREOREQUAL");
						GEN_WRITE_VAR_LITERAL(token1, param1);
						GEN_WRITE_VAR_LITERAL(token2, param2);
						fprintf(stdout, "\n");
						fprintf(stdout, "EQ ");
						GEN_WRITE_VAR_LITERAL(0, "MOREOREQUAL1");
						GEN_WRITE_VAR_LITERAL(token1, param1);
						GEN_WRITE_VAR_LITERAL(token2, param2);
						fprintf(stdout, "\n");
						fprintf(stdout, "OR ");
						GEN_WRITE_VAR_LITERAL(0, "MOREOREQUAL_RES");
						GEN_WRITE_VAR_LITERAL(0, "MOREOREQUAL");
						GEN_WRITE_VAR_LITERAL(0, "MOREOREQUAL1");
						fprintf(stdout, "\n");
					}
					wasEQLTGT = true;
					param1 = "MOREOREQUAL_RES";
					token1 = 0;
					param2 = NULL;
					oper = 0;
					counter = 1;
					tmp = true;
					break;
				case NOTEQUAL:
					if(!(checkSEEN(oper))){
						if(ifORwhileWasTheLast(0) == 2 && whileSpotted(2)){
							fprintf(stdout, "LABEL while%d\n", DLL_GetValueCount(listOfWhile));
						}
						fprintf(stdout, "EQ ");
						GEN_WRITE_VAR_LITERAL(0, "NOTEQUAL");
						GEN_WRITE_VAR_LITERAL(token1, param1);
						GEN_WRITE_VAR_LITERAL(token2, param2);
						fprintf(stdout, "\n");
						fprintf(stdout, "NOT ");
						GEN_WRITE_VAR_LITERAL(0, "NOTEQUAL_RES");
						GEN_WRITE_VAR_LITERAL(0, "NOTEQUAL");
						fprintf(stdout, "\n");
					}
					else{
						if(ifORwhileWasTheLast(0) == 2 && whileSpotted(2)){
							fprintf(stdout, "LABEL while%d\n", DLL_GetValueCount(listOfWhile));
						}
						fprintf(stdout, "EQ ");
						GEN_WRITE_VAR_LITERAL(0, "NOTEQUAL");
						GEN_WRITE_VAR_LITERAL(token1, param1);
						GEN_WRITE_VAR_LITERAL(token2, param2);
						fprintf(stdout, "\n");
						fprintf(stdout, "NOT ");
						GEN_WRITE_VAR_LITERAL(0, "NOTEQUAL_RES");
						GEN_WRITE_VAR_LITERAL(0, "NOTEQUAL");
						fprintf(stdout, "\n");
					}
					wasEQLTGT = true;
					param1 = "NOTEQUAL_RES";
					token1 = 0;
					param2 = NULL;
					oper = 0;
					counter = 1;
					tmp = true;
					break;
			}
		}
	}
	if (!wasEQLTGT)
	{
		if(end && type != 38 && param2 == NULL && !(ifSpotted(2) || whileSpotted(2))){
			if(var_name != NULL){
				fprintf(stdout, "MOVE ");
				GEN_WRITE_VAR_LITERAL(0, var_name);
				var_name = NULL;
			}
			else{
				if(checkSEEN(5)){
					counter1 = 1;
					checkSEEN(7);
				}
				fprintf(stdout, "MOVE LF@ret%d$1 ", counter1);
				counter1++;
			}
			GEN_WRITE_VAR_LITERAL(token1, param1);
			fprintf(stdout, "\n");
			counter = 0;
			param1 = NULL;
			oper = 0;
		}
		else if(type == 38){
			counter = 0;
			type = 0;
			oper = 0;
		}
		else if(end && oper != 0){
			counter = 0;
		}
	}
	else
	{
		wasEQLTGT = false;
	}
	if((ifSpotted(2) || whileSpotted(2)) && tmp){
		if(ifORwhileWasTheLast(0) == 1){
			fprintf(stdout, "JUMPIFNEQ else%d ", DLL_GetValueCount(listOfIf));
			GEN_WRITE_VAR_LITERAL(0, param1);
			fprintf(stdout, "bool@true\n");
			ifSpotted(0);
			whileSpotted(0);
			counter = 0;
			type = 0;
			oper = 0;
			tmp = false;
		}
		if(ifORwhileWasTheLast(0) == 2){
			fprintf(stdout, "JUMPIFEQ whileend%d ", DLL_GetValueCount(listOfWhile));
			GEN_WRITE_VAR_LITERAL(0, param1);
			fprintf(stdout, "bool@false\n");
			whileSpotted(0);
			ifSpotted(0);
			counter = 0;
			type = 0;
			oper = 0;
			tmp = false;
		}
	}
}

//INBUILDS FUNCTION

void GENERATION_READI(){
fprintf(stdout, "LABEL $readi\n");
    fprintf(stdout, "PUSHFRAME\n");
    fprintf(stdout, "DEFVAR LF@ret1$1\n");
    fprintf(stdout, "DEFVAR LF@ret2$1\n");
    fprintf(stdout, "READ LF@ret1$1 int\n");
    fprintf(stdout, "DEFVAR LF@ret_check$1\n");

    fprintf(stdout, "TYPE LF@ret_check$1 LF@ret1$1\n");
    fprintf(stdout, "JUMPIFNEQ $READI_END LF@ret_check$1 string@int\n");

    fprintf(stdout, "PUSHS LF@ret1$1\n");
    fprintf(stdout, "MOVE LF@ret2$1 int@0\n");
    fprintf(stdout, "PUSHS LF@ret2$1\n");
    fprintf(stdout, "JUMP $END_READI\n");

	fprintf(stdout, "LABEL $READI_END\n");
    fprintf(stdout, "PUSHS LF@ret1$1\n");
    fprintf(stdout, "MOVE LF@ret2$1 int@1\n");
    fprintf(stdout, "PUSHS LF@ret2$1\n");

    fprintf(stdout, "LABEL $END_READI\n");
	fprintf(stdout, "POPFRAME\n");
	fprintf(stdout, "RETURN\n\n");
}

void GENERATION_READS(){
    fprintf(stdout, "LABEL $reads\n");
    fprintf(stdout, "PUSHFRAME\n");
    fprintf(stdout, "DEFVAR LF@ret1$1\n");
    fprintf(stdout, "DEFVAR LF@ret2$1\n");
    fprintf(stdout, "READ LF@ret1$1 string\n");
    fprintf(stdout, "DEFVAR LF@ret_check$1\n");

    fprintf(stdout, "TYPE LF@ret_check$1 LF@ret1$1\n");
    fprintf(stdout, "JUMPIFNEQ $READS_END LF@ret_check$1 string@string\n");

    fprintf(stdout, "PUSHS LF@ret1$1\n");
    fprintf(stdout, "MOVE LF@ret2$1 int@0\n");
    fprintf(stdout, "PUSHS LF@ret2$1\n");
    fprintf(stdout, "JUMP $END_READS\n");

	fprintf(stdout, "LABEL $READS_END\n");
    fprintf(stdout, "PUSHS LF@ret1$1\n");
    fprintf(stdout, "MOVE LF@ret2$1 int@1\n");
    fprintf(stdout, "PUSHS LF@ret2$1\n");

    fprintf(stdout, "LABEL $END_READS\n");
	fprintf(stdout, "POPFRAME\n");
	fprintf(stdout, "RETURN\n\n");
}

void GENERATION_READN(){
    fprintf(stdout, "LABEL $readn\n");
    fprintf(stdout, "PUSHFRAME\n");
    fprintf(stdout, "DEFVAR LF@ret1$1\n");
    fprintf(stdout, "DEFVAR LF@ret2$1\n");
    fprintf(stdout, "DEFVAR LF@ret_check$1\n");
    fprintf(stdout, "READ LF@ret1$1 float\n");

    fprintf(stdout, "TYPE LF@ret_check$1 LF@ret1$1\n");
    fprintf(stdout, "JUMPIFNEQ $READN_END LF@ret_check$1 string@float\n");

    fprintf(stdout, "PUSHS LF@ret1$1\n");
    fprintf(stdout, "MOVE LF@ret2$1 int@0\n");
    fprintf(stdout, "PUSHS LF@ret2$1\n");
    fprintf(stdout, "JUMP $END_READN\n");

	fprintf(stdout, "LABEL $READN_END\n");
    fprintf(stdout, "MOVE LF@ret1$1 nil@nil\n");
    fprintf(stdout, "PUSHS LF@ret1$1\n");
    fprintf(stdout, "MOVE LF@ret2$1 int@1\n");
    fprintf(stdout, "PUSHS LF@ret2$1\n");

    fprintf(stdout, "LABEL $END_READN\n");
	fprintf(stdout, "POPFRAME\n");
	fprintf(stdout, "RETURN\n\n");
}

void GENERATION_TOINTEGER(){
	fprintf(stdout, "LABEL $tointeger\n");
	fprintf(stdout, "PUSHFRAME\n");
	fprintf(stdout, "DEFVAR LF@ret1$1\n");
	fprintf(stdout, "DEFVAR LF@param1$1\n");
	fprintf(stdout, "MOVE LF@param1$1 LF@f$1\n");//f
	fprintf(stdout, "FLOAT2INT LF@ret1$1 LF@param1$1\n");
	fprintf(stdout, "PUSHS LF@ret1$1\n");
	fprintf(stdout, "POPFRAME\n");
	fprintf(stdout, "RETURN\n\n");
}

void GENERATION_SUBSTR(){
	fprintf(stdout, "LABEL $substr\n");
	fprintf(stdout, "PUSHFRAME\n");
	fprintf(stdout, "DEFVAR LF@ret1$1\n");//string

	fprintf(stdout, "DEFVAR LF@string$1\n");
	fprintf(stdout, "DEFVAR LF@from$1\n");
	fprintf(stdout, "DEFVAR LF@konec$1\n");
	fprintf(stdout, "DEFVAR LF@length$1\n");

	fprintf(stdout, "DEFVAR LF@length_helper$1\n");
	fprintf(stdout, "DEFVAR LF@char$1\n");
	fprintf(stdout, "DEFVAR LF@new_strlen$1\n");

	fprintf(stdout, "MOVE LF@ret1$1 string@\n");
	fprintf(stdout, "MOVE LF@string$1 LF@in1$1\n");
	fprintf(stdout, "MOVE LF@from$1 LF@in2$1\n");
	fprintf(stdout, "SUB LF@from$1 LF@from$1 int@1\n");
	fprintf(stdout, "MOVE LF@konec$1 LF@in3$1\n");

	fprintf(stdout, "MOVE LF@length_helper$1 int@0\n");

	fprintf(stdout, "STRLEN LF@length$1 LF@string$1\n");//length = STRLEN(STRING)
	fprintf(stdout, "SUB LF@new_strlen$1 LF@length$1 int@1\n");//new_strlen = length - 1

	fprintf(stdout, "DEFVAR LF@result$1\n");
	fprintf(stdout, "LT LF@result$1 LF@konec$1 int@0\n"); //n < O
	fprintf(stdout, "JUMPIFEQ $SUBSTR_END LF@result$1 bool@true\n");

	fprintf(stdout, "EQ LF@result$1 LF@konec$1 int@0\n"); //n == O
	fprintf(stdout, "JUMPIFEQ $SUBSTR_EMPTY LF@result$1 bool@true\n");

	fprintf(stdout, "GT LF@result$1 LF@konec$1 LF@new_strlen$1\n");//i >= length - 1
	fprintf(stdout, "JUMPIFEQ $SUBSTR_END LF@result$1 bool@true\n");

	fprintf(stdout, "LT LF@result$1 LF@from$1 int@0\n"); //i < O
	fprintf(stdout, "JUMPIFEQ $SUBSTR_END LF@result$1 bool@true\n");

	fprintf(stdout, "GT LF@result$1 LF@from$1 LF@new_strlen$1\n");//i >= length - 1
	fprintf(stdout, "JUMPIFEQ $SUBSTR_END LF@result$1 bool@true\n");

	fprintf(stdout, "ADD LF@length_helper$1 LF@length_helper$1 LF@from$1\n");
	fprintf(stdout, "ADD LF@length_helper$1 LF@length_helper$1 LF@konec$1\n");//i + n

	fprintf(stdout, "GT LF@result$1 LF@length_helper$1 LF@length$1\n");//i + n > n ? n : i+n
	fprintf(stdout, "JUMPIFEQ $SUBSTR_LEN LF@result$1 bool@true\n");
	fprintf(stdout, "JUMP $FOR_LOOP\n");
	fprintf(stdout, "LABEL $SUBSTR_LEN\n");
	fprintf(stdout, "MOVE LF@length_helper$1 LF@length$1\n");

	fprintf(stdout, "LABEL $FOR_LOOP\n");
	fprintf(stdout, "JUMPIFEQ $SUBSTR_RET_0 LF@konec$1 LF@from$1\n");
	fprintf(stdout, "GETCHAR LF@char$1 LF@string$1 LF@from$1\n");
	fprintf(stdout, "CONCAT LF@ret1$1 LF@ret1$1 LF@char$1\n");
	fprintf(stdout, "ADD LF@from$1 LF@from$1 int@1\n");
	fprintf(stdout, "JUMP $FOR_LOOP\n");

	fprintf(stdout, "LABEL $SUBSTR_RET_0\n");
	fprintf(stdout, "PUSHS LF@ret1$1\n");
	fprintf(stdout, "JUMP $END\n");

	fprintf(stdout, "LABEL $SUBSTR_END\n");
	fprintf(stdout, "MOVE LF@ret1$1 string@\n");
	fprintf(stdout, "PUSHS LF@ret1$1\n");

	fprintf(stdout, "LABEL $END\n");
	fprintf(stdout, "POPFRAME\n");
	fprintf(stdout, "RETURN\n\n");

	fprintf(stdout, "LABEL $SUBSTR_EMPTY\n");
	fprintf(stdout, "MOVE LF@ret1$1 string@\n");
	fprintf(stdout, "PUSHS LF@ret1$1\n");
	fprintf(stdout, "JUMP $END\n\n");
}

void GENERATION_ORD(){
	fprintf(stdout, "LABEL $ord\n");
	fprintf(stdout, "PUSHFRAME\n");
	fprintf(stdout, "DEFVAR LF@string$1\n");
	fprintf(stdout, "DEFVAR LF@int$1\n");
	fprintf(stdout, "DEFVAR LF@length$1\n");
	fprintf(stdout, "DEFVAR LF@right_int$1\n");
	fprintf(stdout, "DEFVAR LF@ret1$1\n");//string
	fprintf(stdout, "DEFVAR LF@ret2$1\n");//int

	fprintf(stdout, "MOVE LF@string$1 LF@s$1\n");
	fprintf(stdout, "MOVE LF@int$1 LF@i$1\n");

	fprintf(stdout, "STRLEN LF@length$1 LF@string$1\n");//5
	fprintf(stdout, "SUB LF@length$1 LF@length$1 int@1\n");//4

	fprintf(stdout, "GT LF@right_int$1 LF@int$1 LF@length$1\n");
	fprintf(stdout, "JUMPIFEQ $ORD_END LF@right_int$1 bool@true\n");//i > len(n)-1

	fprintf(stdout, "LT LF@right_in$1t LF@int$1 int@0\n");
	fprintf(stdout, "JUMPIFEQ $ORD_END LF@right_int$1 bool@true\n");//i < 0

	fprintf(stdout, "STRI2INT LF@ret1$1 LF@string$1 LF@int$1\n");

	fprintf(stdout, "PUSHS LF@ret1$1\n");
	fprintf(stdout, "MOVE LF@ret2$1 int@0\n");
	fprintf(stdout, "PUSHS LF@ret2$1\n");
	fprintf(stdout, "JUMP $ORD_RET\n");

	fprintf(stdout, "LABEL $ORD_END\n");
	fprintf(stdout, "MOVE LF@ret1$1 nil@nil\n");
	fprintf(stdout, "PUSHS LF@ret1$1\n");
	fprintf(stdout, "MOVE LF@ret2$1 int@1\n");
	fprintf(stdout, "PUSHS LF@ret2$1\n");

	fprintf(stdout, "LABEL $ORD_RET\n");
	fprintf(stdout, "POPFRAME\n");
	fprintf(stdout, "RETURN\n\n");
}

void GENERATION_CHR(){
fprintf(stdout, "LABEL $chr\n");
fprintf(stdout, "PUSHFRAME\n");
fprintf(stdout, "DEFVAR LF@int$1\n");
fprintf(stdout, "DEFVAR LF@right_int$1\n");
fprintf(stdout, "DEFVAR LF@ret1$1\n");//string
fprintf(stdout, "DEFVAR LF@ret2$1\n");//int

fprintf(stdout, "MOVE LF@int$1 LF@i$0\n");

fprintf(stdout, "LT LF@right_int$1 LF@int$1 int@0\n");
fprintf(stdout, "JUMPIFEQ $CHR_END LF@right_int$1 bool@true\n");//i < 0

fprintf(stdout, "GT LF@right_int$1 LF@int$1 int@255\n");
fprintf(stdout, "JUMPIFEQ $CHR_END LF@right_int$1 bool@true\n");//i > len(n)-1

fprintf(stdout, "INT2CHAR LF@ret1$1 LF@int$1\n");

fprintf(stdout, "PUSHS LF@ret1$1\n");
fprintf(stdout, "MOVE LF@ret2$1 int@0\n");
fprintf(stdout, "PUSHS LF@ret2$1\n");
fprintf(stdout, "JUMP $CHR_RET\n");

fprintf(stdout, "LABEL $CHR_END\n");

fprintf(stdout, "MOVE LF@ret1$1 nil@nil\n");
fprintf(stdout, "PUSHS LF@ret1$1\n");
fprintf(stdout, "MOVE LF@ret2$1 int@1\n");
fprintf(stdout, "PUSHS LF@ret2$1\n");

fprintf(stdout, "LABEL $CHR_RET\n");
fprintf(stdout, "POPFRAME\n");
fprintf(stdout, "RETURN\n\n");
}

void GEN_CALL_INBUILDS(){
    GENERATION_READS();
    GENERATION_READI();
    GENERATION_READN();
    GENERATION_TOINTEGER();
    GENERATION_SUBSTR();
    GENERATION_ORD();
    GENERATION_CHR();
}
