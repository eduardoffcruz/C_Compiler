//PL5_COMPILADORES@2020
//Eduardo Filipe Ferreira Cruz          2018285164
//Rodrigo Fernando Henriques Sobral     2018298209

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "code_generator.h"

#define MAX_SIZE 4086 //bytes

int count=1, global_vars_count=1; //para variáveis intermédias
char buffer[1024]; //aux
char global_vars_code[MAX_SIZE]="";
_type funcRetType;
char *currFuncName;
//int funcCountParams;

void generate_llvm_code(node* ast_root){
    //incluir funções standard do C
    printf("declare i32 @putchar(i32)\n");  //nocapture e nounwind para dizer q a funcao esta na biblioteca standard do c
    printf("declare i32 @getchar()\n");  //para dizer q a funcao esta na biblioteca standard do c
    //TODO: 
    printf("\n");

    start_gen(ast_root);
}


void start_gen(node* ast_root){
    node* ast_node=ast_root->child; //lista ligada de func e variaveis globais
    /*GLOBAL*/
    /*funcDefinition funcDeclaration declaration */
    while(ast_node){
        if(strcmp(ast_node->str,"FuncDefinition")==0){
            get_funcDefs_code(ast_node->child); /*ast_node->child is typespec node of funcDef*/
        } else if(strcmp(ast_node->str,"FuncDeclaration")==0){
            get_funcDecs_code(ast_node->child); /*ast_node->child is typespec node of funcDec*/
        }else if(strcmp(ast_node->str,"Declaration")==0){
            get_varDecs_code(ast_node->child); /*ast_node->child is typespec node of funcDec*/
        }    
        ast_node=ast_node->next;
    }

    /*CREATE FUNCTION NO INITIALIZE GLOBAL VARIABLES IF NECESSARY*/
    printf("define void @_INIT_GLOBAL_VARS(){\n");
    printf("%s",global_vars_code);
    printf("\tret void\n}");
}
/***************************************************************************************************************/
void get_funcDefs_code(node *n){ //FUNC DEFS
    //receives typedef node as n
    node *aux=n; //typedef node
    _type t=str_to_type(aux->str);
    funcRetType=t;
    printf("define %s @%s(",type_to_llvm(t),aux->next->tk->value);
    currFuncName=aux->next->tk->value;
    aux=aux->next; //id node
    print_params_types(aux->next); //paramList node
    printf(") ");
    aux=aux->next; //paramList node
    aux=aux->next; //funcBody node
    //funcBody
    printf("{\n");
    if(strcmp(n->next->tk->value,"main")==0){
        printf("\tcall void @_INIT_GLOBAL_VARS()\n");
    }
    //alocar parametros
    alloca_params(n->next->next);
    print_funcBody_code(aux,1);
    
    printf("}\n\n");
    count=1; //reset counter
}

void get_funcDecs_code(node *n){ //FUNC DECS
    //receives typedef node as n
    node *aux=n; //typedef
    if(get_sym_table(aux->next->tk->value)->isDef){
        return ;
    }
    printf("declare %s @%s(",type_to_llvm(str_to_type(aux->str)),aux->next->tk->value);
    aux=aux->next; //id
    print_params_types(aux->next); //paramList node
    printf(")\n");

    //nota: acho q se podem omitir declarações de funções definidas..
}

void get_varDecs_code(node *n){ //VAR DECS
//COMO DECLARAR VARIAVEIS GLOBAIS EM LLVM? :(
    node *aux=n; //typespec node
    _type t=str_to_type(aux->str); //var type

    aux=aux->next;//id
    if(aux->next!=NULL){
        //VAR DEFINITION
        count=global_vars_count;
        handle_Global_varDef(aux->next);
        global_vars_count=count;
        printf("@%s = global %s ", aux->tk->value,type_to_llvm(t));
        if(t==reallit){
            printf("0.0\n");
        }
        else{
            printf("0\n");
        }
        sprintf(buffer,"@%s",aux->tk->value);
        assign_llvm_name(aux, buffer);
        sprintf(buffer,"\tstore %s %s, %s* %s\n",type_to_llvm(t),aux->next->llvm_name,type_to_llvm(t),aux->llvm_name);
        strcat(global_vars_code,buffer);
        count=1;
    }
    else{
        //VAR DECLARATION
        printf("@%s = common global %s ", aux->tk->value,type_to_llvm(t));
        if(t==reallit){
            printf("0.0\n");
        }
        else{
            printf("0\n");
        }
        sprintf(buffer,"@%s",aux->tk->value);
        assign_llvm_name(aux, buffer);
    }
}
/******************************************************************************************/
void print_funcBody_code(node* funcBody, int printFlag){
    //count==1 se funcao n devolver nada, count==2 se a função tiver um tipo de retorno diferente de void 
    node *funcDecAndStats=funcBody->child,*aux; 
    node *before_node=NULL;
    // ^^lista ligada de declarations and statements do func body
    _type t;
    while(funcDecAndStats){
        if(strcmp(funcDecAndStats->str,"Declaration")==0){
            aux=funcDecAndStats->child; //typedef 
            t=str_to_type(aux->str); //typedef of declared var
            aux=aux->next;//id (var name)
            if(printFlag){
                printf("\t%%%s = alloca %s\n", aux->tk->value, type_to_llvm(t));
            }
            sprintf(buffer,"%%%s",aux->tk->value);
            assign_llvm_name(aux, buffer);
            //count++;
            if(aux->next!=NULL){
                //atribuicao de valor à variavel declarada
                //TODO:
                //expressões...func calls...
                //traduzir as expressões todas e no final fazer o store com o valor resultante
                //...
                handle_statement(aux->next,printFlag,0);
                //FAZER CASTING!!!!!!!!!!!
                cast_llvm_type(type_to_llvm(aux->next->type), type_to_llvm(t),aux->next,printFlag);
                if(printFlag){
                    printf("\tstore %s ",type_to_llvm(t));
                    printf("%s, ",aux->next->llvm_name);
                    printf("%s* %%%s\n",type_to_llvm(t),aux->tk->value);
                }
                //...
                
            }
        }
        else{
            //pode ser:
            //while, if else, statlist, return
            //STATEMENTS
            handle_statement(funcDecAndStats,printFlag,0);
        }
        before_node=funcDecAndStats;
        funcDecAndStats=funcDecAndStats->next;
    }

    if(before_node==NULL){
        if(funcRetType!=voidlit){
            if(funcRetType==reallit){
                printf("\tret double 0.0\n");
            }
            else{
                printf("\tret %s 0\n", type_to_llvm(funcRetType));
            }
        } 
        else{
            printf("\tret void\n");
        }    
    }
    else{
        if(strcmp(before_node->str,"Return")!=0){
            if(funcRetType!=voidlit){
                if(funcRetType==reallit){
                    printf("\tret double 0.0\n");
                }
                else{
                    printf("\tret %s 0\n", type_to_llvm(funcRetType));
                }
            } 
            else{
                printf("\tret void\n");
            }
        }
    }

}

void print_statList(node* statList, int printFlag){
    //count==1 se funcao n devolver nada, count==2 se a função tiver um tipo de retorno diferente de void 
    node *funcDecAndStats=statList->child,*aux; 
    // ^^lista ligada de declarations and statements do func body
    _type t;
    while(funcDecAndStats){
        if(strcmp(funcDecAndStats->str,"Declaration")==0){
            aux=funcDecAndStats->child; //typedef 
            t=str_to_type(aux->str); //typedef of declared var
            aux=aux->next;//id (var name)
            if(printFlag){
                printf("\t%%%s = alloca %s\n", aux->tk->value, type_to_llvm(t));
            }
            sprintf(buffer,"%%%s",aux->tk->value);
            assign_llvm_name(aux, buffer);
            //count++;
            if(aux->next!=NULL){
                //atribuicao de valor à variavel declarada
                //TODO:
                //expressões...func calls...
                //traduzir as expressões todas e no final fazer o store com o valor resultante
                //...
                handle_statement(aux->next,printFlag,0);
                //FAZER CASTING!!!!!!!!!!!
                cast_llvm_type(type_to_llvm(aux->next->type), type_to_llvm(t),aux->next,printFlag);
                if(printFlag){
                    printf("\tstore %s ",type_to_llvm(t));
                    printf("%s, ",aux->next->llvm_name);
                    printf("%s* %%%s\n",type_to_llvm(t),aux->tk->value);
                }
                //...
                
            }
        }
        else{
            //pode ser:
            //while, if else, statlist, return 
            //STATEMENTS
            handle_statement(funcDecAndStats,printFlag,0);
        }

        funcDecAndStats=funcDecAndStats->next;
    }

}

void handle_statement(node* statement, int printFlag,int allocaAddr){
    //node* aux=statement;
    if(statement!=NULL){
        //PLUS
        if(strcmp(statement->str,"Plus")==0){
            //1 nó filho
            handle_statement(statement->child,printFlag,0);
            assign_llvm_name(statement, statement->child->llvm_name);
        }
        //MINUS
        else if(strcmp(statement->str,"Minus")==0){
            //1 nó filho
            handle_statement(statement->child,printFlag,0);
            if(statement->type==reallit){
                if(printFlag){
                    printf("\t%%%d = fsub double 0.0, %s\n", count, statement->child->llvm_name);
                }
            }
            else{
                if(printFlag){
                    printf("\t%%%d = sub i32 0, %s\n", count, statement->child->llvm_name);
                }
            }
            
            sprintf(buffer,"%%%d",count);
            assign_llvm_name(statement, buffer);
            count++;
        }
        //NOT
        else if(strcmp(statement->str,"Not")==0){
            //1 nó filho
            handle_statement(statement->child,printFlag,0);
            //statement->type sempre igual a int
            if(printFlag){
                printf("\t%%%d = icmp ne %s %s, 0\n",count,type_to_llvm(statement->type),statement->child->llvm_name);
                printf("\t%%%d = xor i1 %%%d, true\n", count+1, count);
                printf("\t%%%d = zext i1 %%%d to i32\n", count+2, count+1);
            }
            sprintf(buffer,"%%%d", count+2);
            assign_llvm_name(statement, buffer);
            count+=3;
        }
        //CALL
        else if(strcmp(statement->str,"Call")==0){
            handle_funcCall(statement,printFlag);
        }
        //STORE
        else if(strcmp(statement->str,"Store")==0){
            handle_statement(statement->child->next,printFlag,0);
            //cast if necessary
            cast_llvm_type(type_to_llvm(statement->child->next->type), type_to_llvm(statement->child->type),statement->child->next,printFlag);
            if(printFlag){
                if(isLocalVar(statement->child->tk->value,currFuncName)){
                    printf("\tstore %s ",type_to_llvm(statement->child->type));
                    printf("%s, ",statement->child->next->llvm_name);
                    printf("%s* %%%s\n",type_to_llvm(statement->child->type),statement->child->tk->value);   
                }
                else{
                    printf("\tstore %s ",type_to_llvm(statement->child->type));
                    printf("%s, ",statement->child->next->llvm_name);
                    printf("%s* @%s\n",type_to_llvm(statement->child->type),statement->child->tk->value);
                }
            }
            handle_statement(statement->child,printFlag,0);
            assign_llvm_name(statement,statement->child->llvm_name);
        }
        //COMMA
        else if(strcmp(statement->str,"Comma")==0){
            //evaluates first expr and discard result, evaluates second expr and returns result
            handle_statement(statement->child,printFlag,0);
            handle_statement(statement->child->next,printFlag,0);
            assign_llvm_name(statement, statement->child->next->llvm_name);
            //TODO:?
        }
        //OPERATIONS
        else if(strcmp(statement->str,"Add")==0){
            handle_statement(statement->child,printFlag,0);
            cast_llvm_type(type_to_llvm(statement->child->type),type_to_llvm(statement->type),statement->child,printFlag);
            handle_statement(statement->child->next,printFlag,0);
            cast_llvm_type(type_to_llvm(statement->child->next->type),type_to_llvm(statement->type),statement->child->next,printFlag);
            if(statement->type==reallit){
                if(printFlag){
                    printf("\t%%%d = fadd %s %s, %s\n",count,type_to_llvm(statement->type),statement->child->llvm_name,statement->child->next->llvm_name);
                }
            }
            else{
                if(printFlag){
                    printf("\t%%%d = add %s %s, %s\n",count,type_to_llvm(statement->type),statement->child->llvm_name,statement->child->next->llvm_name);
                }
            }
            
            sprintf(buffer,"%%%d", count);
            assign_llvm_name(statement, buffer);
            count++;
        }
        else if(strcmp(statement->str,"Mul")==0){
            handle_statement(statement->child,printFlag,0);
            cast_llvm_type(type_to_llvm(statement->child->type),type_to_llvm(statement->type),statement->child,printFlag);
            handle_statement(statement->child->next,printFlag,0);
            cast_llvm_type(type_to_llvm(statement->child->next->type),type_to_llvm(statement->type),statement->child->next,printFlag);
            
            if(statement->type==reallit){
                if(printFlag){
                    printf("\t%%%d = fmul %s %s, %s\n",count,type_to_llvm(statement->type),statement->child->llvm_name,statement->child->next->llvm_name);
                }
            }
            else{
                if(printFlag){
                    printf("\t%%%d = mul %s %s, %s\n",count,type_to_llvm(statement->type),statement->child->llvm_name,statement->child->next->llvm_name);
                }
            } 
            sprintf(buffer,"%%%d", count);
            assign_llvm_name(statement, buffer);
            count++;
        }
        else if(strcmp(statement->str,"Div")==0){
            handle_statement(statement->child,printFlag,0);
            cast_llvm_type(type_to_llvm(statement->child->type),type_to_llvm(statement->type),statement->child,printFlag);
            handle_statement(statement->child->next,printFlag,0);
            cast_llvm_type(type_to_llvm(statement->child->next->type),type_to_llvm(statement->type),statement->child->next,printFlag);
            if(statement->type==reallit){
                if(printFlag){
                    printf("\t%%%d = fdiv %s %s, %s\n",count,type_to_llvm(statement->type),statement->child->llvm_name,statement->child->next->llvm_name);
                }
            }
            else{
                if(printFlag){
                    printf("\t%%%d = sdiv %s %s, %s\n",count,type_to_llvm(statement->type),statement->child->llvm_name,statement->child->next->llvm_name);
                }
            } 
            sprintf(buffer,"%%%d", count);
            assign_llvm_name(statement, buffer);
            count++;
        }
        else if(strcmp(statement->str,"Sub")==0){
            handle_statement(statement->child,printFlag,0);
            cast_llvm_type(type_to_llvm(statement->child->type),type_to_llvm(statement->type),statement->child,printFlag);
            handle_statement(statement->child->next,printFlag,0);
            cast_llvm_type(type_to_llvm(statement->child->next->type),type_to_llvm(statement->type),statement->child->next,printFlag);
            if(statement->type==reallit){
                if(printFlag){
                printf("\t%%%d = fsub %s %s, %s\n",count,type_to_llvm(statement->type),statement->child->llvm_name,statement->child->next->llvm_name);
            }
            }
            else{
                if(printFlag){
                    printf("\t%%%d = sub %s %s, %s\n",count,type_to_llvm(statement->type),statement->child->llvm_name,statement->child->next->llvm_name);
                }
            } 
            sprintf(buffer,"%%%d", count);
            assign_llvm_name(statement, buffer);
            count++;
        }
        else if(strcmp(statement->str,"Mod")==0){
            //urem ou srem ?
            handle_statement(statement->child,printFlag,0);
            cast_llvm_type(type_to_llvm(statement->child->type),type_to_llvm(statement->type),statement->child,printFlag);
            handle_statement(statement->child->next,printFlag,0);
            cast_llvm_type(type_to_llvm(statement->child->next->type),type_to_llvm(statement->type),statement->child->next,printFlag);
            if(printFlag){
                printf("\t%%%d = srem %s %s, %s\n",count,type_to_llvm(statement->type),statement->child->llvm_name,statement->child->next->llvm_name);
            }
            sprintf(buffer,"%%%d", count);
            assign_llvm_name(statement, buffer);
            count++;
        }
        //COMPARISONS
        else if(strcmp(statement->str,"Eq")==0){
            handle_statement(statement->child,printFlag,0);
            handle_statement(statement->child->next,printFlag,0);
            if(statement->child->type==reallit||statement->child->next->type==reallit){
                cast_llvm_type(type_to_llvm(statement->child->type),"double",statement->child,printFlag);
                cast_llvm_type(type_to_llvm(statement->child->next->type),"double",statement->child->next,printFlag);
                if(printFlag){
                    printf("\t%%%d = fcmp oeq double %s, %s\n", count,statement->child->llvm_name, statement->child->next->llvm_name);
                }
            }
            else{
                if(printFlag){
                    printf("\t%%%d = icmp eq %s %s, %s\n", count, type_to_llvm(statement->type),statement->child->llvm_name, statement->child->next->llvm_name);
                }
            }
            count++;
            if(printFlag){
                printf("\t%%%d = zext i1 %%%d to i32\n", count, count-1);
            }
            sprintf(buffer,"%%%d",count); assign_llvm_name(statement,buffer); count++;
        }
        else if(strcmp(statement->str,"Ne")==0){
            handle_statement(statement->child,printFlag,0);
            handle_statement(statement->child->next,printFlag,0);
            if(statement->child->type==reallit||statement->child->next->type==reallit){
                cast_llvm_type(type_to_llvm(statement->child->type),"double",statement->child,printFlag);
                cast_llvm_type(type_to_llvm(statement->child->next->type),"double",statement->child->next,printFlag);
                if(printFlag){
                    printf("\t%%%d = fcmp une double %s, %s\n", count,statement->child->llvm_name, statement->child->next->llvm_name);
                }   
            }
            else{
                if(printFlag){
                    printf("\t%%%d = icmp ne %s %s, %s\n", count, type_to_llvm(statement->type),statement->child->llvm_name, statement->child->next->llvm_name);
                }
            }
            count++;
            if(printFlag){
                printf("\t%%%d = zext i1 %%%d to i32\n", count, count-1);
            }
            sprintf(buffer,"%%%d",count); assign_llvm_name(statement,buffer); count++;
        }
        else if(strcmp(statement->str,"Le")==0){
            handle_statement(statement->child,printFlag,0);
            handle_statement(statement->child->next,printFlag,0);
            if(statement->child->type==reallit||statement->child->next->type==reallit){
                cast_llvm_type(type_to_llvm(statement->child->type),"double",statement->child,printFlag);
                cast_llvm_type(type_to_llvm(statement->child->next->type),"double",statement->child->next,printFlag);
                if(printFlag){
                    printf("\t%%%d = fcmp ole double %s, %s\n", count,statement->child->llvm_name, statement->child->next->llvm_name);
                }
            }
            else{
                if(printFlag){
                    printf("\t%%%d = icmp sle %s %s, %s\n", count, type_to_llvm(statement->type),statement->child->llvm_name, statement->child->next->llvm_name);
                }
            }
            count++;
            if(printFlag){
                printf("\t%%%d = zext i1 %%%d to i32\n", count, count-1);
            }
            sprintf(buffer,"%%%d",count); assign_llvm_name(statement,buffer); count++;
        }
        else if(strcmp(statement->str,"Ge")==0){
            handle_statement(statement->child,printFlag,0);
            handle_statement(statement->child->next,printFlag,0);
            if(statement->child->type==reallit||statement->child->next->type==reallit){
                cast_llvm_type(type_to_llvm(statement->child->type),"double",statement->child,printFlag);
                cast_llvm_type(type_to_llvm(statement->child->next->type),"double",statement->child->next,printFlag);
                if(printFlag){
                    printf("\t%%%d = fcmp oge double %s, %s\n", count,statement->child->llvm_name, statement->child->next->llvm_name);
                }
            }
            else{
                if(printFlag){
                    printf("\t%%%d = icmp sge %s %s, %s\n", count, type_to_llvm(statement->type),statement->child->llvm_name, statement->child->next->llvm_name);
                }
            }
            count++;
            if(printFlag){
                printf("\t%%%d = zext i1 %%%d to i32\n", count, count-1);
            }
            sprintf(buffer,"%%%d",count); assign_llvm_name(statement,buffer); count++;
        }
        else if(strcmp(statement->str,"Lt")==0){
            handle_statement(statement->child,printFlag,0);
            handle_statement(statement->child->next,printFlag,0);
            if(statement->child->type==reallit||statement->child->next->type==reallit){
                cast_llvm_type(type_to_llvm(statement->child->type),"double",statement->child,printFlag);
                cast_llvm_type(type_to_llvm(statement->child->next->type),"double",statement->child->next,printFlag);
                if(printFlag){
                    printf("\t%%%d = fcmp olt double %s, %s\n", count,statement->child->llvm_name, statement->child->next->llvm_name);
                }
                
            }
            else{
                if(printFlag){
                    printf("\t%%%d = icmp slt %s %s, %s\n", count, type_to_llvm(statement->type),statement->child->llvm_name, statement->child->next->llvm_name);
                }
                
            }
            count++;
            if(printFlag){
                printf("\t%%%d = zext i1 %%%d to i32\n", count, count-1);
            }
            sprintf(buffer,"%%%d",count); assign_llvm_name(statement,buffer); count++;
        }
        else if(strcmp(statement->str,"Gt")==0){
            handle_statement(statement->child,printFlag,0);
            handle_statement(statement->child->next,printFlag,0);
            if(statement->child->type==reallit||statement->child->next->type==reallit){
                cast_llvm_type(type_to_llvm(statement->child->type),"double",statement->child,printFlag);
                cast_llvm_type(type_to_llvm(statement->child->next->type),"double",statement->child->next,printFlag);
                if(printFlag){
                    printf("\t%%%d = fcmp ogt double %s, %s\n", count,statement->child->llvm_name, statement->child->next->llvm_name);
                }
            }
            else{
                if(printFlag){
                    printf("\t%%%d = icmp sgt %s %s, %s\n", count, type_to_llvm(statement->type),statement->child->llvm_name, statement->child->next->llvm_name);
                }
            }
            count++;
            if(printFlag){
                printf("\t%%%d = zext i1 %%%d to i32\n", count, count-1);
            }
            sprintf(buffer,"%%%d", count);
            assign_llvm_name(statement, buffer);
            count++;
        }
        //BITWISE OPERATORS
        else if(strcmp(statement->str,"BitWiseAnd")==0){
            handle_statement(statement->child,printFlag,0);
            handle_statement(statement->child->next,printFlag,0);
            if(printFlag){
                 printf("\t%%%d = and %s %s, %s\n",count,type_to_llvm(statement->type),statement->child->llvm_name,statement->child->next->llvm_name);
            }
            sprintf(buffer,"%%%d", count);
            assign_llvm_name(statement, buffer);
            count++;
        }
        else if(strcmp(statement->str,"BitWiseOr")==0){
            handle_statement(statement->child,printFlag,0);
            handle_statement(statement->child->next,printFlag,0);
            if(printFlag){
                 printf("\t%%%d = or %s %s, %s\n",count,type_to_llvm(statement->type),statement->child->llvm_name,statement->child->next->llvm_name);
            }
            sprintf(buffer,"%%%d", count);
            assign_llvm_name(statement, buffer);
            count++;
        }
        else if(strcmp(statement->str,"BitWiseXor")==0){
            handle_statement(statement->child,printFlag,0);
            handle_statement(statement->child->next,printFlag,0);
            if(printFlag){
                 printf("\t%%%d = xor %s %s, %s\n",count,type_to_llvm(statement->type),statement->child->llvm_name,statement->child->next->llvm_name);
            }
            sprintf(buffer,"%%%d", count);
            assign_llvm_name(statement, buffer);
            count++;
        }
        //CONDITIONAL OPERATIONS
        else if(strcmp(statement->str,"Or")==0 || strcmp(statement->str,"And")==0){
            print_and_or_condition(statement,printFlag,allocaAddr);
        }
        //STATLIST
        else if(strcmp(statement->str,"StatList")==0){
            print_statList(statement,printFlag);
        }
        //WHILE
        else if(strcmp(statement->str,"While")==0){
            print_while(statement,printFlag);
        }
        //IF
        else if(strcmp(statement->str,"If")==0){        
            print_if(statement,printFlag);
        }
        //RETURN
        else if(strcmp(statement->str,"Return")==0){
            if(strcmp(statement->child->str,"Null")==0){
                if(printFlag){
                    printf("\tret void\n");
                }
            }
            else{
                handle_statement(statement->child,printFlag,0);
                cast_llvm_type(type_to_llvm(statement->child->type),type_to_llvm(funcRetType),statement->child,printFlag);
                if(printFlag){
                    printf("\tret %s %s\n", type_to_llvm(funcRetType),statement->child->llvm_name);
                }
            }  
            count++;          
        }
        else if(strcmp(statement->str,"Null")==0){
            //nada
        }
        //TERMINALS
        else if(isTerminal(statement)){
            if(strncmp(statement->str,"Id",2)==0){
                //se for ID
                if(printFlag){
                    if(isLocalVar(statement->tk->value,currFuncName)){
                        printf("\t%%%d = load %s, %s* %%%s\n",count,type_to_llvm(statement->type),type_to_llvm(statement->type),statement->tk->value);
                    }
                    else{
                        printf("\t%%%d = load %s, %s* @%s\n",count,type_to_llvm(statement->type),type_to_llvm(statement->type),statement->tk->value);
                    }
                }
                sprintf(buffer,"%%%d",count);
                assign_llvm_name(statement, buffer);
                count++;
            }
            else{
                if(strncmp(statement->str,"ChrLit",6)==0){
                    //ChrLit('a')
                    sprintf(buffer,"%d",get_chrlit_ascii_value(statement->tk->value));
                    assign_llvm_name(statement, buffer);
                }
                else if(strncmp(statement->str,"IntLit",6)==0){
                    if(statement->tk->value[0]=='0'&&strlen(statement->tk->value)>1){
                        sprintf(buffer,"%d",octal_to_int(statement->tk->value+1));
                        assign_llvm_name(statement,buffer); 
                    }
                    else{
                        assign_llvm_name(statement, statement->tk->value); 
                    }
                }
                else{ //reallit
                   assign_llvm_name(statement,adapt_double_value(statement)); 
                }
            }
        }

    }
    

}

void cast_llvm_type(char* got, char *expected, node* n,int printFlag){
    if(strcmp(got,expected)==0){
        return;
    }
    //got double expected int
    else if(strcmp(got,"double")==0&&strcmp(expected,"i32")==0){
        if(printFlag==1){
           printf("\t%%%d = fptosi double %s to i32\n",count,n->llvm_name); 
        }
        else if(printFlag==2){
            sprintf(buffer,"\t%%%d = fptosi double %s to i32\n",count,n->llvm_name); 
            strcat(global_vars_code,buffer);
        }
        sprintf(buffer,"%%%d", count);
        assign_llvm_name(n, buffer);
        count++;

    }
    //got int expected double
    else if(strcmp(expected,"double")==0&&strcmp(got,"i32")==0){
        if(printFlag==1){
            printf("\t%%%d = sitofp i32 %s to double\n",count,n->llvm_name);   
        }
        else if(printFlag==2){
            sprintf(buffer,"\t%%%d = sitofp i32 %s to double\n",count,n->llvm_name); 
            strcat(global_vars_code,buffer);
        }
        sprintf(buffer,"%%%d", count);
        assign_llvm_name(n, buffer);
        count++;
    }
    else{
        //TODO: ?
    }
}

void handle_funcCall(node *callNode, int printFlag){
    node *aux=callNode;
    param *p_aux;
    aux=aux->child->next; //first argument node
    p_aux=callNode->child->param_list;
    while(aux!=NULL){
        if(printFlag==2){
            handle_Global_varDef(aux);
        }
        else{
            handle_statement(aux,printFlag,0);
        }
        cast_llvm_type(type_to_llvm(aux->type),type_to_llvm(p_aux->type),aux,printFlag); //CAST if necessary
        aux=aux->next;
        p_aux=p_aux->next;
    }
    aux=callNode->child->next;//back to first argument node

    if(callNode->type==voidlit){
        if(printFlag==1){
            printf("\tcall %s @%s(",type_to_llvm(callNode->type),callNode->child->tk->value);
        }
        else if(printFlag==2){
            sprintf(buffer,"\tcall %s @%s(",type_to_llvm(callNode->type),callNode->child->tk->value);
            strcat(global_vars_code,buffer);
        }    
    }
    else{
        if(printFlag==1){
            printf("\t%%%d = call %s @%s(",count,type_to_llvm(callNode->type),callNode->child->tk->value);
        }
        else if(printFlag==2){
            sprintf(buffer,"\t%%%d = call %s @%s(",count,type_to_llvm(callNode->type),callNode->child->tk->value);
            strcat(global_vars_code,buffer);
        }
        sprintf(buffer,"%%%d", count);
        assign_llvm_name(callNode, buffer); //func ID
        count++;
    }
    
    //print arguments
    if(aux!=NULL){
        if(printFlag==1){
           printf("%s %s",type_to_llvm(aux->type),aux->llvm_name); 
        }
        else if(printFlag==2){
            sprintf(buffer,"%s %s",type_to_llvm(aux->type),aux->llvm_name); 
            strcat(global_vars_code,buffer);
        }
        
        aux=aux->next;
    }
    while(aux!=NULL){
        if(printFlag==1){
            printf(", %s %s",type_to_llvm(aux->type),aux->llvm_name);
        }
        else if(printFlag==2){
            sprintf(buffer,", %s %s",type_to_llvm(aux->type),aux->llvm_name);
            strcat(global_vars_code,buffer);
        }
        
        aux=aux->next;
    }
    if(printFlag==1){
        printf(")\n");   
    }
    else if(printFlag==2){
        strcat(global_vars_code,")\n");
    }
    
}

void print_while(node *whileNode,int printFlag){
    //whileNode has 2 child nodes
    node *aux=whileNode->child; //condition node
    int savedCount;
    int statCount,initCount;
    int allocaAddr=count;

    //
    if(printFlag==1){
            printf("\t%%%d = alloca i32\n",count); //aux to save value from operation && or ||
    }
    count++;

    if(printFlag){
        printf("\tbr label %%%d\n\n",count);
    }
    initCount=count;
    count++;
    handle_statement(aux,printFlag,allocaAddr); //handle while condition
    
    if(aux->type==reallit){
        if(printFlag){
            printf("\t%%%d = fcmp une %s %%%d, 0\n",count,type_to_llvm(aux->type),count-1);
        }
    }
    else{
        if(printFlag){
            printf("\t%%%d = icmp ne %s %%%d, 0\n",count,type_to_llvm(aux->type),count-1);
        }
    }
    sprintf(buffer,"%%%d", count);
    assign_llvm_name(whileNode->child, buffer);
    count++;







    savedCount=count;
    handle_statement(aux->next,0,0); //counting
    statCount=count;
    count=savedCount;
    if(printFlag){
        printf("\tbr i1 %%%d, label %%%d, label %%%d\n\n",savedCount-1,savedCount,statCount+1);
    }
    count++;
    handle_statement(aux->next,printFlag,0);
    
    if(printFlag){
        printf("\tbr label %%%d\n\n",initCount);
    }
    count++;
}

void print_if(node* ifNode,int printFlag){
    node *aux=ifNode->child; //condition node
    int savedCount;
    int ifCount;
    int elseCount;
    //1st node=if condition
    //2nd node=if stat body dec/stat ou statlist
    //3rd node=else condition (if Null..there's no else stat)

    handle_statement(aux,printFlag,0); //handle conditions
        if(aux->type==reallit){
            if(printFlag){
                printf("\t%%%d = fcmp une %s %s, 0\n",count,type_to_llvm(aux->type),aux->llvm_name);
            }
        }
        else{
            if(printFlag){
                printf("\t%%%d = icmp ne %s %s, 0\n",count,type_to_llvm(aux->type),aux->llvm_name);
            }
        }
        sprintf(buffer,"%%%d", count);
        assign_llvm_name(ifNode->child, buffer);
        count++;
    //IF
    aux=aux->next; //if statement 2nd node
    savedCount=count;
    handle_statement(aux,0,0);//if
    ifCount=count;
    handle_statement(aux->next,0,0);//else
    elseCount=count+1;
    count=savedCount;

    if(printFlag){
        printf("\tbr i1 %s, label %%%d, label %%%d\n\n",ifNode->child->llvm_name, savedCount,ifCount+1);
    }
    count++;
    handle_statement(aux,printFlag,0); //IF
    if(printFlag){
        printf("\tbr label %%%d\n\n",elseCount+1);
    }
    aux=aux->next; //else statement 3rd node
    count++;
    handle_statement(aux,printFlag,0); //ELSE
    if(printFlag){
        printf("\tbr label %%%d\n\n",elseCount+1);
    }
    count++;
}
/********************************************************************************TODO:**/
void print_and_or_condition(node *and_or, int printFlag, int allocaAddr){
    node *aux=and_or; //AND or OR
    int savedCount,labelCount;


    if(!allocaAddr){
        if(printFlag==1){
            printf("\t%%%d = alloca i32\n",count); //aux to save value from operation && or ||
        }
        else if(printFlag==2){
            sprintf(buffer,"\t%%%d = alloca i32\n",count); //aux to save value from operation && or ||
            strcat(global_vars_code,buffer);
        }
        allocaAddr=count;
        
        count++;
    }
    sprintf(buffer,"%%%d", allocaAddr);
    assign_llvm_name(and_or, buffer);

    /*************************_1ST_OP_********************************/
    handle_statement(aux->child,printFlag,allocaAddr); //1st condition`

            if(printFlag==1){
                printf("\t%%%d = icmp ne %s %s, 0\n",count,type_to_llvm(aux->child->type),aux->child->llvm_name);
            }
            else if(printFlag==2){
                sprintf(buffer,"\t%%%d = icmp ne %s %s, 0\n",count,type_to_llvm(aux->child->type),aux->child->llvm_name);
                strcat(global_vars_code,buffer);
            }
        sprintf(buffer,"%%%d", count); 
        assign_llvm_name(and_or->child, buffer);
        count++;

    //if output da 1ª condicao==0 e AND, valor ==0
    if(strcmp(and_or->str,"And")==0){   //AND
        savedCount=count;
        print_2nd_op_AndOr(and_or,0,allocaAddr);
        labelCount=count+1; //count zext command!
        count=savedCount;
        if(printFlag==1){
            printf("\tbr i1 %%%d, label %%%d, label %%%d\n\n",savedCount-1,savedCount,labelCount); 
        }
        else if(printFlag==2){
            sprintf(buffer,"\tbr i1 %%%d, label %%%d, label %%%d\n\n",savedCount-1,savedCount,labelCount); 
            strcat(global_vars_code,buffer);
        }
        
        count++;
        //if
        print_2nd_op_AndOr(and_or,printFlag,allocaAddr);
        if(printFlag==1){
            printf("\tbr label %%%d\n\n",labelCount+1+1);
        }
        else if(printFlag==2){
            sprintf(buffer,"\tbr label %%%d\n\n",labelCount+1+1);
            strcat(global_vars_code,buffer);
        }
        count++;
        //else
        if(printFlag==1){
            printf("\t%%%d = zext i1 %s to i32\n",count,and_or->child->llvm_name);
            //store argument value in result variable
            printf("\tstore i32 %%%d, i32* %s\n",count,and_or->llvm_name);
        }
        else if(printFlag==2){
            sprintf(buffer,"\t%%%d = zext i1 %s to i32\n",count,and_or->child->llvm_name);
            strcat(global_vars_code,buffer);
            sprintf(buffer,"\tstore i32 %%%d, i32* %s\n",count,and_or->llvm_name);
            strcat(global_vars_code,buffer);
        }
        count++;

        if(printFlag==1){
            printf("\tbr label %%%d\n\n",labelCount+1+1);
        }
        else if(printFlag==2){
            sprintf(buffer,"\tbr label %%%d\n\n",labelCount+1+1);
            strcat(global_vars_code,buffer);
        }
        count++;
    }
    else{   //OR
        if(printFlag==1){
            printf("\tbr i1 %%%d, label %%%d, label %%%d\n\n",count-1,count,count+1+1);
        }
        else if(printFlag==2){
            sprintf(buffer,"\tbr i1 %%%d, label %%%d, label %%%d\n\n",count-1,count,count+1+1);
            strcat(global_vars_code,buffer);
        }
        count++;
        //if
        if(printFlag==1){
            printf("\t%%%d = zext i1 %s to i32\n",count,and_or->child->llvm_name);
            //store argument value in result variable
            printf("\tstore i32 %%%d, i32* %s\n",count,and_or->llvm_name);
        }
        else if(printFlag==2){
            sprintf(buffer,"\t%%%d = zext i1 %s to i32\n",count,and_or->child->llvm_name);
            strcat(global_vars_code,buffer);
            //store argument value in result variable
            sprintf(buffer,"\tstore i32 %%%d, i32* %s\n",count,and_or->llvm_name);
            strcat(global_vars_code,buffer);
        }
        count++;
        /****/
        savedCount=count;
        print_2nd_op_AndOr(and_or,0,allocaAddr);
        labelCount=count+1;
        count=savedCount;
        /*****/
        if(printFlag==1){
            printf("\tbr label %%%d\n\n",labelCount);
        }
        else if(printFlag==2){
            sprintf(buffer,"\tbr label %%%d\n\n",labelCount);
            strcat(global_vars_code,buffer);
        }
        count++;
        //else
        print_2nd_op_AndOr(and_or,printFlag,allocaAddr);
        if(printFlag==1){
            printf("\tbr label %%%d\n\n",labelCount);
        }
        else if(printFlag==2)
        {
            sprintf(buffer,"\tbr label %%%d\n\n",labelCount);
            strcat(global_vars_code,buffer);
        }
        count++;
    }
    //LOAD RESULT
    if(printFlag==1){
        printf("\t%%%d = load i32, i32* %s\n",count, and_or->llvm_name);
    }
    else if(printFlag==2){
        sprintf(buffer,"\t%%%d = load i32, i32* %s\n",count, and_or->llvm_name);
        strcat(global_vars_code,buffer);
    }

    sprintf(buffer,"%%%d", count);
    assign_llvm_name(and_or, buffer);
    count++;
}

void print_2nd_op_AndOr(node *and_or, int printFlag, int allocaAddr){
    node *aux=and_or;
    /*************************_2ND_OP_********************************/
    handle_statement(aux->child->next,printFlag,allocaAddr); //2nd condition`
        if(aux->child->next->type==reallit){
            if(printFlag==1){
                printf("\t%%%d = fcmp une %s %s, 0\n",count,type_to_llvm(aux->child->next->type),aux->child->next->llvm_name);
            }
            else if(printFlag==2){
                sprintf(buffer,"\t%%%d = fcmp une %s %s, 0\n",count,type_to_llvm(aux->child->next->type),aux->child->next->llvm_name);
                strcat(global_vars_code,buffer);
            }
        }
        else{
            if(printFlag==1){
                printf("\t%%%d = icmp ne %s %s, 0\n",count,type_to_llvm(aux->child->next->type),aux->child->next->llvm_name);
            }
            else if(printFlag==2){
                sprintf(buffer,"\t%%%d = icmp ne %s %s, 0\n",count,type_to_llvm(aux->child->next->type),aux->child->next->llvm_name);
                strcat(global_vars_code,buffer);
            }
        }
    count++;
    if(strcmp(and_or->str,"And")==0){ //AND
        if(printFlag==1){
            printf("\t%%%d = and i1 %s, %%%d\n",count,and_or->child->llvm_name,count-1);
        }
        else if(printFlag==2){
            sprintf(buffer,"\t%%%d = and i1 %s, %%%d\n",count,and_or->child->llvm_name,count-1);
            strcat(global_vars_code,buffer);
        }
    }
    else{ //OR
        if(printFlag==1){
            printf("\t%%%d = or i1 %s, %%%d\n",count,and_or->child->llvm_name,count-1);
        }
        else if(printFlag==2){
            printf(buffer,"\t%%%d = or i1 %s, %%%d\n",count,and_or->child->llvm_name,count-1);
            strcat(global_vars_code,buffer);
        }
    }
    count++;
    if(printFlag==1){
        printf("\t%%%d = zext i1 %%%d to i32\n",count,count-1);
        printf("\tstore i32 %%%d, i32* %s\n",count,and_or->llvm_name);
    }
    else if(printFlag==2){
        sprintf(buffer,"\t%%%d = zext i1 %%%d to i32\n",count,count-1);
        strcat(global_vars_code,buffer);
        sprintf(buffer,"\tstore i32 %%%d, i32* %s\n",count,and_or->llvm_name);
        strcat(global_vars_code,buffer);
    }
    count++;

}

void print_params_types(node *paramList){ //types only..for func declarations
    node *paramDec=paramList->child;
    if(paramDec!=NULL){
        if(str_to_type(paramDec->child->str)!=voidlit){
            printf("%s",type_to_llvm(str_to_type(paramDec->child->str)));
        }
        paramDec=paramDec->next; //next paramDec
    }
    while(paramDec!=NULL){
        printf(", %s",type_to_llvm(str_to_type(paramDec->child->str)));
        paramDec=paramDec->next; //next paramDec
    }

}
/*
void print_params(node *paramList){ //types and var names...for func definitions
    _type t;
    node *paramDec=paramList->child;
    if(paramDec!=NULL){
        t=str_to_type(paramDec->child->str);
        if(t!=voidlit){
            printf("%s",type_to_llvm(t));
            if(paramDec->child->next!=NULL){
                printf(" %%%s",paramDec->child->next->tk->value);
            }
        }
        paramDec=paramDec->next; //next paramDec
    }
    while(paramDec!=NULL){
        t=str_to_type(paramDec->child->str);
        printf(" , %s",type_to_llvm(t));
        if(t!=voidlit&&paramDec->child->next!=NULL){
            printf(" %%%s",paramDec->child->next->tk->value);
        }
        paramDec=paramDec->next; //next paramDec
    }

} */

void alloca_params(node *paramList){
    _type t;
    node *paramDec=paramList->child;
    int i=0;

    while(paramDec!=NULL){
        t=str_to_type(paramDec->child->str);
        if(t!=voidlit&&paramDec->child->next!=NULL){
            printf("\t%%%s = alloca %s\n",paramDec->child->next->tk->value,type_to_llvm(t));
            sprintf(buffer,"%%%s",paramDec->child->next->tk->value);
            assign_llvm_name(paramDec->child->next, buffer);
            count++;
            //store argument value
            printf("\tstore %s ",type_to_llvm(t));
            printf("%%%d, ",i);
            printf("%s* %%%s\n",type_to_llvm(t),paramDec->child->next->tk->value);
            i++;
        }
        paramDec=paramDec->next; //next paramDec
    }
}

int count_params(node *paramList){
    int i=0;
    _type t;
    node *paramDec=paramList->child;
    while(paramDec!=NULL){
        t=str_to_type(paramDec->child->str);
        if(t!=voidlit){
            i++;
        }
        paramDec=paramDec->next; //next paramDec
    }
    return i;
}

int isComparison(node *n){ 
    if(strcmp(n->str,"Eq")==0||strcmp(n->str,"Ne")==0||strcmp(n->str,"Le")==0||strcmp(n->str,"Gt")==0||strcmp(n->str,"Lt")==0||strcmp(n->str,"Ge")==0){
        return 1;
    }
    else{
        return 0;
    }
}

int get_chrlit_ascii_value(char* value){
    char aux[4];
    int len=strlen(value);
    if(len==4){ //escape char
        switch(value[2]){
            case 'n': return (int)'\n';
            case 't': return (int)'\t';
            case '\\': return (int)'\\';
            case '\'': return (int)'\'';
            case '\"': return (int)'\"';
            default: 
            strncpy(aux,value+2,1);
            return octal_to_int(aux);
        }
    }
    else if(len==5){
        strncpy(aux,value+2,2);
        return octal_to_int(aux);
    }
    else if(len==6){
        strncpy(aux,value+2,3);
        return octal_to_int(aux);
    }
    else{
        return (int)value[1];
    }
}

int octal_to_int(char *str){
    //verificar se é um octal válido primeiro? TODO:
    int octal=atoi(str);
    int decimal=0, i=0,j=0;
    char ch;
    //check if its a valid octal
    while((ch=str[j++])!='\0'){
        if(ch-'0'>=8){return octal;}
    }

    while(octal!=0){
        decimal+=(octal%10)*power(8,i); ++i; octal/=10;
    }
    return decimal;
}

int power(int b, int exp){
    int i,res=1;
    for(i=exp;i>0;i--){
        res*=b;
    }
    return res;
}

char* type_to_llvm(_type t){
    switch(t){
        case charlit: return "i32"; 
        case intlit: return "i32";
        case reallit: return "double";
        case voidlit: return "void";
        case shortlit: return "i32";
        default: return "";
    }
}
char* adapt_double_value(node *num){
    size_t len = strlen(num->tk->value);
    char *aux=(char*)malloc((len+2)*sizeof(char));
    strcpy(aux,"0");
    if(num->tk->value[0]=='.'){
        strcat(aux,num->tk->value);
        return aux;
    }
    else{
        free(aux);
        return num->tk->value;
    }
}

void handle_Global_varDef(node* statement){
    //node* aux=statement;
    int allocaAddr=0;
    if(statement!=NULL){
        //PLUS
        if(strcmp(statement->str,"Plus")==0){
            //1 nó filho
            handle_Global_varDef(statement->child);
            assign_llvm_name(statement, statement->child->llvm_name);
        }
        //MINUS
        else if(strcmp(statement->str,"Minus")==0){
            //1 nó filho
            handle_Global_varDef(statement->child);
            if(statement->type==reallit){
                    sprintf(buffer,"\t%%%d = fsub double 0.0, %s\n", count, statement->child->llvm_name);
                    strcat(global_vars_code,buffer);
            }
            else{
                    sprintf(buffer,"\t%%%d = sub i32 0, %s\n", count, statement->child->llvm_name);
                    strcat(global_vars_code,buffer);
            }
            sprintf(buffer,"%%%d",count);
            assign_llvm_name(statement, buffer);
            count++;
        }
        //NOT
        else if(strcmp(statement->str,"Not")==0){
            //1 nó filho
            handle_Global_varDef(statement->child);
            //statement->type sempre igual a int
                sprintf(buffer,"\t%%%d = icmp ne %s %s, 0\n",count,type_to_llvm(statement->type),statement->child->llvm_name);
                strcat(global_vars_code,buffer);
                sprintf(buffer,"\t%%%d = xor i1 %%%d, true\n", count+1, count);
                strcat(global_vars_code,buffer);
                sprintf(buffer,"\t%%%d = zext i1 %%%d to i32\n", count+2, count+1);
                strcat(global_vars_code,buffer);
            sprintf(buffer,"%%%d", count+2);
            assign_llvm_name(statement, buffer);
            count+=3;
        }
        //CALL
        else if(strcmp(statement->str,"Call")==0){
            handle_funcCall(statement,2);
        }
        //STORE
        else if(strcmp(statement->str,"Store")==0){
            handle_Global_varDef(statement->child->next);
            //cast if necessary
            cast_llvm_type(type_to_llvm(statement->child->next->type), type_to_llvm(statement->child->type),statement->child->next,2);

                    sprintf(buffer,"\tstore %s ",type_to_llvm(statement->child->type));
                    strcat(global_vars_code,buffer);
                    sprintf(buffer,"%s, ",statement->child->next->llvm_name);
                    strcat(global_vars_code,buffer);
                    sprintf(buffer,"%s* @%s\n",type_to_llvm(statement->child->type),statement->child->tk->value);
                    strcat(global_vars_code,buffer);
           handle_Global_varDef(statement->child);
           assign_llvm_name(statement,statement->child->llvm_name); 
                
            
        }
        //COMMA
        else if(strcmp(statement->str,"Comma")==0){
            //evaluates first expr and discard result, evaluates second expr and returns result
            handle_Global_varDef(statement->child);
            handle_Global_varDef(statement->child->next);
            assign_llvm_name(statement, statement->child->next->llvm_name);
            //TODO:?
        }
        //OPERATIONS
        else if(strcmp(statement->str,"Add")==0){
            handle_Global_varDef(statement->child);
            cast_llvm_type(type_to_llvm(statement->child->type),type_to_llvm(statement->type),statement->child,2);
            handle_Global_varDef(statement->child->next);
            cast_llvm_type(type_to_llvm(statement->child->next->type),type_to_llvm(statement->type),statement->child->next,2);
            if(statement->type==reallit){
                    sprintf(buffer,"\t%%%d = fadd %s %s, %s\n",count,type_to_llvm(statement->type),statement->child->llvm_name,statement->child->next->llvm_name);
                    strcat(global_vars_code,buffer);
            }
            else{
                    sprintf(buffer,"\t%%%d = add %s %s, %s\n",count,type_to_llvm(statement->type),statement->child->llvm_name,statement->child->next->llvm_name);
                    strcat(global_vars_code,buffer);
            }
            sprintf(buffer,"%%%d", count);
            assign_llvm_name(statement, buffer);
            count++;
        }
        else if(strcmp(statement->str,"Mul")==0){
            handle_Global_varDef(statement->child);
            cast_llvm_type(type_to_llvm(statement->child->type),type_to_llvm(statement->type),statement->child,2);
            handle_Global_varDef(statement->child->next);
            cast_llvm_type(type_to_llvm(statement->child->next->type),type_to_llvm(statement->type),statement->child->next,2);
            
            if(statement->type==reallit){
                    sprintf(buffer,"\t%%%d = fmul %s %s, %s\n",count,type_to_llvm(statement->type),statement->child->llvm_name,statement->child->next->llvm_name);
                    strcat(global_vars_code,buffer);
            }
            else{
                    sprintf(buffer,"\t%%%d = mul %s %s, %s\n",count,type_to_llvm(statement->type),statement->child->llvm_name,statement->child->next->llvm_name);
                    strcat(global_vars_code,buffer);
            } 
            sprintf(buffer,"%%%d", count);
            assign_llvm_name(statement, buffer);
            count++;
        }
        else if(strcmp(statement->str,"Div")==0){
            handle_Global_varDef(statement->child);
            cast_llvm_type(type_to_llvm(statement->child->type),type_to_llvm(statement->type),statement->child,2);
            handle_Global_varDef(statement->child->next);
            cast_llvm_type(type_to_llvm(statement->child->next->type),type_to_llvm(statement->type),statement->child->next,2);
            if(statement->type==reallit){

                    sprintf(buffer,"\t%%%d = fdiv %s %s, %s\n",count,type_to_llvm(statement->type),statement->child->llvm_name,statement->child->next->llvm_name);
                    strcat(global_vars_code,buffer);
            }
            else{
                    sprintf(buffer,"\t%%%d = sdiv %s %s, %s\n",count,type_to_llvm(statement->type),statement->child->llvm_name,statement->child->next->llvm_name);
                    strcat(global_vars_code,buffer);
            } 
            sprintf(buffer,"%%%d", count);
            assign_llvm_name(statement, buffer);
            count++;
        }
        else if(strcmp(statement->str,"Sub")==0){
            handle_Global_varDef(statement->child);
            cast_llvm_type(type_to_llvm(statement->child->type),type_to_llvm(statement->type),statement->child,2);
            handle_Global_varDef(statement->child->next);
            cast_llvm_type(type_to_llvm(statement->child->next->type),type_to_llvm(statement->type),statement->child->next,2);
            if(statement->type==reallit){
                sprintf(buffer,"\t%%%d = fsub %s %s, %s\n",count,type_to_llvm(statement->type),statement->child->llvm_name,statement->child->next->llvm_name);
                strcat(global_vars_code,buffer);
            }
            else{
                sprintf(buffer,"\t%%%d = sub %s %s, %s\n",count,type_to_llvm(statement->type),statement->child->llvm_name,statement->child->next->llvm_name);
                strcat(global_vars_code,buffer);
            } 
            sprintf(buffer,"%%%d", count);
            assign_llvm_name(statement, buffer);
            count++;
        }
        else if(strcmp(statement->str,"Mod")==0){
            //urem ou srem ?
            handle_Global_varDef(statement->child);
            cast_llvm_type(type_to_llvm(statement->child->type),type_to_llvm(statement->type),statement->child,2);
            handle_Global_varDef(statement->child->next);
            cast_llvm_type(type_to_llvm(statement->child->next->type),type_to_llvm(statement->type),statement->child->next,2);
            sprintf(buffer,"\t%%%d = srem %s %s, %s\n",count,type_to_llvm(statement->type),statement->child->llvm_name,statement->child->next->llvm_name);
            strcat(global_vars_code,buffer);
            sprintf(buffer,"%%%d", count);
            assign_llvm_name(statement, buffer);
            count++;
        }
        //COMPARISONS
        else if(strcmp(statement->str,"Eq")==0){
            handle_Global_varDef(statement->child);
            handle_Global_varDef(statement->child->next);
            if(statement->child->type==reallit||statement->child->next->type==reallit){
                cast_llvm_type(type_to_llvm(statement->child->type),"double",statement->child,2);
                cast_llvm_type(type_to_llvm(statement->child->next->type),"double",statement->child->next,2);
                sprintf(buffer,"\t%%%d = fcmp oeq double %s, %s\n", count,statement->child->llvm_name, statement->child->next->llvm_name);
                strcat(global_vars_code,buffer);
            }
            else{
                sprintf(buffer,"\t%%%d = icmp eq %s %s, %s\n", count, type_to_llvm(statement->type),statement->child->llvm_name, statement->child->next->llvm_name);
                strcat(global_vars_code,buffer);
            }
            count++;
            sprintf(buffer,"\t%%%d = zext i1 %%%d to i32\n", count, count-1);
            strcat(global_vars_code,buffer);
            
            sprintf(buffer,"%%%d",count); assign_llvm_name(statement,buffer); count++;
        }
        else if(strcmp(statement->str,"Ne")==0){
            handle_Global_varDef(statement->child);
            handle_Global_varDef(statement->child->next);
            if(statement->child->type==reallit||statement->child->next->type==reallit){
                cast_llvm_type(type_to_llvm(statement->child->type),"double",statement->child,2);
                cast_llvm_type(type_to_llvm(statement->child->next->type),"double",statement->child->next,2);
                sprintf(buffer,"\t%%%d = fcmp une double %s, %s\n", count,statement->child->llvm_name, statement->child->next->llvm_name);
                strcat(global_vars_code,buffer);
            }
            else{
                sprintf(buffer,"\t%%%d = icmp ne %s %s, %s\n", count, type_to_llvm(statement->type),statement->child->llvm_name, statement->child->next->llvm_name);
                strcat(global_vars_code,buffer);
            }
            count++;
            sprintf(buffer,"\t%%%d = zext i1 %%%d to i32\n", count, count-1);
            strcat(global_vars_code,buffer);
            sprintf(buffer,"%%%d",count); assign_llvm_name(statement,buffer); count++;
        }
        else if(strcmp(statement->str,"Le")==0){
            handle_Global_varDef(statement->child);
            handle_Global_varDef(statement->child->next);
            if(statement->child->type==reallit||statement->child->next->type==reallit){
                cast_llvm_type(type_to_llvm(statement->child->type),"double",statement->child,2);
                cast_llvm_type(type_to_llvm(statement->child->next->type),"double",statement->child->next,2);
                sprintf(buffer,"\t%%%d = fcmp ole double %s, %s\n", count,statement->child->llvm_name, statement->child->next->llvm_name);
                strcat(global_vars_code,buffer);
            }
            else{
                sprintf(buffer,"\t%%%d = icmp sle %s %s, %s\n", count, type_to_llvm(statement->type),statement->child->llvm_name, statement->child->next->llvm_name);
                strcat(global_vars_code,buffer);
            }
            count++;
            sprintf(buffer,"\t%%%d = zext i1 %%%d to i32\n", count, count-1);
            strcat(global_vars_code,buffer);
            sprintf(buffer,"%%%d",count); assign_llvm_name(statement,buffer); count++;
        }
        else if(strcmp(statement->str,"Ge")==0){
            handle_Global_varDef(statement->child);
            handle_Global_varDef(statement->child->next);
            if(statement->child->type==reallit||statement->child->next->type==reallit){
                cast_llvm_type(type_to_llvm(statement->child->type),"double",statement->child,2);
                cast_llvm_type(type_to_llvm(statement->child->next->type),"double",statement->child->next,2);
                sprintf(buffer,"\t%%%d = fcmp oge double %s, %s\n", count,statement->child->llvm_name, statement->child->next->llvm_name);
                strcat(global_vars_code,buffer);
            }
            else{

                sprintf(buffer,"\t%%%d = icmp sge %s %s, %s\n", count, type_to_llvm(statement->type),statement->child->llvm_name, statement->child->next->llvm_name);
                strcat(global_vars_code,buffer);
            }
            count++;
            sprintf(buffer,"\t%%%d = zext i1 %%%d to i32\n", count, count-1);
            strcat(global_vars_code,buffer);    
            sprintf(buffer,"%%%d",count); assign_llvm_name(statement,buffer); count++;
        }
        else if(strcmp(statement->str,"Lt")==0){
            handle_Global_varDef(statement->child);
            handle_Global_varDef(statement->child->next);
            if(statement->child->type==reallit||statement->child->next->type==reallit){
                cast_llvm_type(type_to_llvm(statement->child->type),"double",statement->child,2);
                cast_llvm_type(type_to_llvm(statement->child->next->type),"double",statement->child->next,2);
                sprintf(buffer,"\t%%%d = fcmp olt double %s, %s\n", count,statement->child->llvm_name, statement->child->next->llvm_name);
                strcat(global_vars_code,buffer);
            }
            else{
                sprintf(buffer,"\t%%%d = icmp slt %s %s, %s\n", count, type_to_llvm(statement->type),statement->child->llvm_name, statement->child->next->llvm_name);
                strcat(global_vars_code,buffer);
            }
            count++;

            sprintf(buffer,"\t%%%d = zext i1 %%%d to i32\n", count, count-1);
            strcat(global_vars_code,buffer);
            sprintf(buffer,"%%%d",count); assign_llvm_name(statement,buffer); count++;
        }
        else if(strcmp(statement->str,"Gt")==0){
            handle_Global_varDef(statement->child);
            handle_Global_varDef(statement->child->next);
            if(statement->child->type==reallit||statement->child->next->type==reallit){
                cast_llvm_type(type_to_llvm(statement->child->type),"double",statement->child,2);
                cast_llvm_type(type_to_llvm(statement->child->next->type),"double",statement->child->next,2);
                sprintf(buffer,"\t%%%d = fcmp ogt double %s, %s\n", count,statement->child->llvm_name, statement->child->next->llvm_name);
                strcat(global_vars_code,buffer);
            }
            else{
                sprintf(buffer,"\t%%%d = icmp sgt %s %s, %s\n", count, type_to_llvm(statement->type),statement->child->llvm_name, statement->child->next->llvm_name);
                strcat(global_vars_code,buffer);
            }
            count++;
            sprintf(buffer,"\t%%%d = zext i1 %%%d to i32\n", count, count-1);
            strcat(global_vars_code,buffer);
            sprintf(buffer,"%%%d", count);
            assign_llvm_name(statement, buffer);
            count++;
        }
        //BITWISE OPERATORS
        else if(strcmp(statement->str,"BitWiseAnd")==0){
            handle_Global_varDef(statement->child);
            handle_Global_varDef(statement->child->next);
            sprintf(buffer,"\t%%%d = and %s %s, %s\n",count,type_to_llvm(statement->type),statement->child->llvm_name,statement->child->next->llvm_name);
            strcat(global_vars_code,buffer);
            sprintf(buffer,"%%%d", count);
            assign_llvm_name(statement, buffer);
            count++;
        }
        else if(strcmp(statement->str,"BitWiseOr")==0){
            handle_Global_varDef(statement->child);
            handle_Global_varDef(statement->child->next);
            sprintf(buffer,"\t%%%d = or %s %s, %s\n",count,type_to_llvm(statement->type),statement->child->llvm_name,statement->child->next->llvm_name);
            strcat(global_vars_code,buffer);
            sprintf(buffer,"%%%d", count);
            assign_llvm_name(statement, buffer);
            count++;
        }
        else if(strcmp(statement->str,"BitWiseXor")==0){
            handle_Global_varDef(statement->child);
            handle_Global_varDef(statement->child->next);
            sprintf(buffer,"\t%%%d = xor %s %s, %s\n",count,type_to_llvm(statement->type),statement->child->llvm_name,statement->child->next->llvm_name);
            strcat(global_vars_code,buffer);
            sprintf(buffer,"%%%d", count);
            assign_llvm_name(statement, buffer);
            count++;
        }
        //CONDITIONAL OPERATIONS
        else if(strcmp(statement->str,"Or")==0 || strcmp(statement->str,"And")==0){
            print_and_or_condition(statement,2,allocaAddr);
        }
        //TERMINALS
        else if(isTerminal(statement)){
            if(strncmp(statement->str,"Id",2)==0){
                //se for ID
                        sprintf(buffer,"\t%%%d = load %s, %s* @%s\n",count,type_to_llvm(statement->type),type_to_llvm(statement->type),statement->tk->value);
                        strcat(global_vars_code,buffer);
                    
                sprintf(buffer,"%%%d",count);
                assign_llvm_name(statement, buffer);
                count++;
            }
            else{
                if(strncmp(statement->str,"ChrLit",6)==0){
                    //ChrLit('a')
                    sprintf(buffer,"%d",get_chrlit_ascii_value(statement->tk->value));
                    assign_llvm_name(statement, buffer);
                }
                else if(strncmp(statement->str,"IntLit",6)==0){
                    if(statement->tk->value[0]=='0'&&strlen(statement->tk->value)>1){
                        sprintf(buffer,"%d",octal_to_int(statement->tk->value+1));
                        assign_llvm_name(statement,buffer); 
                    }
                    else{
                        assign_llvm_name(statement, statement->tk->value); 
                    }
                }
                else{ //reallit
                   assign_llvm_name(statement, adapt_double_value(statement)); 
                }
            }
        }

    }
}


