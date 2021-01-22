//PL5_COMPILADORES@2020
//Eduardo Filipe Ferreira Cruz          2018285164
//Rodrigo Fernando Henriques Sobral     2018298209
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ast.h"

//Abstract Syntax Tree Root
node *root;

/*Functions*/
void initTree(node *n)
{
    root = n; //set tree root
}

int programIsEmpty(void){
    if(root==NULL)
        return 1;
    else
        return 0;    
}

node *createNode(char *str, token *tk)
{
    node *n;
    if ((n = (node *)malloc(sizeof(node))) == NULL)
    {
        fprintf(stderr, "Error allocating memory");
        exit(-1);
    }
    n->next = NULL;
    n->child = NULL;
    n->str = strdup(str);
    n->tk=tk; 
    n->llvm_name=NULL;
    n->type=none;//for tree notation
    n-> param_list=NULL; //for tree notation
    return n;
}

node *getCopyNode(node *n){
    return createNode(n->str,n->tk);
}

node *getDeclarationNodes(node* n,node *typeSpecNode){
    //recebe o 1º nó d'uma lista ligada de declarator's
    //e recebe o nó com o typespec da declaration
    node *decAux=n;
    node *aux;
    while(decAux!=NULL){
        aux=decAux->child;
        decAux->child=getCopyNode(typeSpecNode);
        decAux->child->next=aux;

        decAux=decAux->next; //iterate no next declaration node
    }

    return n;
}

int isNullNode(node *n){
    if(n!=NULL&&(strcmp(n->str,"Null")==0))
        return 1;
    else
        return 0;
}

void addNext(node *n, node *next)
{
    if (n == NULL || next == NULL)
        return;
    node *aux = n;
    while (aux->next != NULL)
    {
        aux = aux->next;
    }
    aux->next = next;
}

void addChild(node *n, node *child)
{
    if (n->child == NULL)
        n->child = child;
    else
        addNext(n->child, child);
}

void printTree(int anotate)
{
    preOrder_(root, 0,anotate);
}

void preOrder_(node *n, int h,int anotate){ //used to be static :'(
    int i;
    param *p;
    if (n != NULL)
    {
        for (i = 0; i < h; i++)
            printf("..");
        printf("%s", n->str);
        /*ANOTATE TREE*/
        if(anotate){
            p=n->param_list;
            if(n->type!=none){
                printf(" - %s",type_to_str(n->type));
            }
            if(p!=NULL){
                printf("(%s",type_to_str(p->type));
                p=p->next;
                while(p!=NULL){
                    printf(",%s",type_to_str(p->type));
                    p=p->next;
                }
                printf(")");
            }
        }
        /**************/
        printf("\n");
        preOrder_(n->child, h + 1,anotate);
        preOrder_(n->next, h,anotate);
    }
}

void freeTree(void){
    freeTree_(root);
}

void freeTree_(node *n) //used to be static :'(
{
    if (n != NULL)
    {
        if (n->child != NULL)
            freeTree_(n->child);
        if (n->next != NULL)
            freeTree_(n->next);
        //free (heap) allocated memory
        free(n->str); //str
        if(n->tk->value!=NULL){freeToken(n->tk);}
        free_param_list(n->param_list);
        free(n);
    }
}

/*create token struct*/
token* createToken(char* str, int lineNum, int colNum){
    token *tk;
    if ((tk = (token *)malloc(sizeof(token))) == NULL)
    {
        fprintf(stderr, "Error allocating memory");
        exit(-1);
    }
    if(str!=NULL){
        tk->value = strdup(str);
    }
    else{
        tk->value=NULL;
    }
    tk->lineNum=lineNum;
    tk->colNum=colNum;
    return tk;
}

token* getCopyToken(token *tk){
    return createToken(tk->value,tk->lineNum,tk->colNum);
}

void freeToken(token *tk){
    free(tk->value);
    free(tk);
}

void print_param_list_node(node *n){
    param *p=n->param_list;
    if(p!=NULL){
        printf("(%s",type_to_str(p->type));
        p=p->next;
        while(p!=NULL){
            printf(",%s",type_to_str(p->type));
            p=p->next;
        }
        printf(")");
    }
}

void assign_llvm_name(node *n, char* s){
    if(n->llvm_name!=NULL){
        free(n->llvm_name);
    }
    n->llvm_name=strdup(s);
}