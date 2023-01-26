// bison file: parser.y
%{
#include <stdio.h>
#include <stdlib.h>
#include "zgdbAst.h"

extern int yylineno;
extern char* yytext;
int yylex();
void yyerror(const char *s);

struct ast tree = {0};
const struct ast emptyTree = {0};
struct astStep* aStep;
struct astPredicate* aPredicate;

int64_t symbolValue;
bool isFirstPred = true;
bool inv = false;

size_t mallocSize = 0;

// gives good debug information
int yydebug=1;

%}

%union {
  int32_t number;
  int64_t n64;
  double decimal;
  char* key;
  char* string;
}
%error-verbose
%locations

%token ADD DELETE UPDATE FIND JOIN PARENT
%token INT DBL STR BOOL
%token ABSOLUTE RELATIVE LBRAC RBRAC LROBRAC RROBRAC COMMA COLON ON END
%token EQ NOT_EQ LS GR GR_EQ LS_EQ CONT
%token AND_SYM OR_SYM NOT_SYM
%token <decimal> DNUMBER
%token <number> NUMBER
%token <string> STRING
%token <key> KEY

%type <n64> op sym not

%start zpath

%%

zpath: add schema path {YYACCEPT;}
     | update path {YYACCEPT;}
     | delete path {YYACCEPT;}
     | find path {YYACCEPT;}
     | join path {YYACCEPT;}
     | parent path {YYACCEPT;}

add: ADD STRING {createAddRequest($2);}
update: UPDATE KEY STRING {createUpdateRequest($2, $3);}
delete: DELETE {createDeleteRequest();}
find: FIND {createFindRequest();}
join: JOIN {createJoinRequest();} jpredicates ON | JOIN {createJoinRequest();}
parent: PARENT {createParentRequest();}

schema: LROBRAC elements RROBRAC

elements: element COMMA elements
        | element

element: intschema
       | dblschema
       | boolschema
       | strschema

intschema: INT COLON KEY COLON NUMBER {addIntSchema($3, $5);}
dblschema: DBL COLON KEY COLON DNUMBER {addDblSchema($3, $5);}
boolschema: BOOL COLON KEY COLON STRING {addBoolSchema($3, $5);}
strschema: STR COLON KEY COLON STRING {addStrSchema($3, $5);}

path: entry {addStep();} terminal
    | entry {addStep();} path

entry: ABSOLUTE {
                 aStep = (astStep*) malloc(sizeof(astStep));
                 mallocSize += sizeof(astStep);
                 aStep->pType = AST_ABSOLUTE_PATH;
                 aStep->sType = AST_DOCUMENT_STEP;
                 isFirstPred = true;
                } base
     | ABSOLUTE {
                 aStep = (astStep*) malloc(sizeof(astStep));
                 mallocSize += sizeof(astStep);
                 aStep->pType = AST_ABSOLUTE_PATH;
                 aStep->sType = AST_ELEMENT_STEP;
                } KEY {aStep->stepName = strdup($3); cleanKey(aStep->stepName);}
     | RELATIVE {
                 aStep = (astStep*) malloc(sizeof(astStep));
                 mallocSize += sizeof(astStep);
                 aStep->pType = AST_RELATIVE_PATH;
                 aStep->sType = AST_DOCUMENT_STEP;
                 isFirstPred = true;
                } base
     | RELATIVE {
                 aStep = (astStep*) malloc(sizeof(astStep));
                 mallocSize += sizeof(astStep);
                 aStep->pType = AST_RELATIVE_PATH;
                 aStep->sType = AST_ELEMENT_STEP;
                } KEY {aStep->stepName = strdup($3); cleanKey(aStep->stepName);}

terminal: END

base: STRING {aStep->stepName = strdup($1); cleanString(aStep->stepName);}
    | STRING {aStep->stepName = strdup($1); cleanString(aStep->stepName);} predicates

predicates: not predicate {inv = $1; addPredicate();}
          | not predicate sym {inv = $1; addPredicate(); symbolValue = $3;} predicates

jpredicates: not predicate {inv = $1; addJPredicate();}
           | not predicate sym {inv = $1; addJPredicate(); symbolValue = $3;} jpredicates

sym: AND_SYM {$$ = 1;}
   | OR_SYM {$$ = 2;}

not:  {$$ = 0;}
    | NOT_SYM {$$ = 1;}

predicate: LBRAC NUMBER RBRAC {fillIndexPredicate($2);}
         | LBRAC KEY op STRING RBRAC {fillValuePredicate($2, $3, $4);}
         | LBRAC KEY op KEY RBRAC {fillElementPredicate($2, $3, $4);}

op: EQ {$$ = 0;}
  | NOT_EQ {$$ = 1;}
  | GR {$$ = 2;}
  | LS {$$ = 3;}
  | GR_EQ {$$ = 4;}
  | LS_EQ {$$ = 5;}
  | CONT {$$ = 6;}

%%

void yyerror(const char *s)
{
  fprintf(stderr,"error: %s on line %d\n", s, yylineno);
}

void createAddRequest(char* name) {
    tree.type = AST_ADD;
    cleanString(name);
    tree.docName = strdup(name);
}

void createUpdateRequest(char* name, char* input) {
    tree.type = AST_UPDATE; 
    cleanKey(name);
    tree.elName = strdup(name);
    cleanString(input);
    tree.value = strdup(input); 
}

void createDeleteRequest() {
    tree.type = AST_DELETE;
}

void createFindRequest() {
    tree.type = AST_FIND;
}

void createJoinRequest() {
    tree.type = AST_JOIN;
}

void createParentRequest() {
    tree.type = AST_PARENT;
}

void addIntSchema(char* name, int32_t v) {
    if(tree.first == NULL) {
        tree.first = (astAddSchema*) malloc(sizeof(astAddSchema));
        mallocSize += sizeof(astAddSchema);
        tree.first->type = SCHEMA_TYPE_INT;
        cleanKey(name);
        tree.first->name = strdup(name);
        tree.first->integer = v;
    } else {
        astAddSchema* temp = tree.first;
        while(temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = (astAddSchema*) malloc(sizeof(astAddSchema));
        mallocSize += sizeof(astAddSchema);
        temp->next->type = SCHEMA_TYPE_INT;
        cleanKey(name);
        temp->next->name = strdup(name);
        temp->next->integer = v;
    }
    
}

void addDblSchema(char* name, double v) {
    if(tree.first == NULL) {
        tree.first = (astAddSchema*) malloc(sizeof(astAddSchema));
        mallocSize += sizeof(astAddSchema);
        tree.first->type = SCHEMA_TYPE_DOUBLE;
        cleanKey(name);
        tree.first->name = strdup(name);
        tree.first->dbl = v;
    } else {
        astAddSchema* temp = tree.first;
        while(temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = (astAddSchema*) malloc(sizeof(astAddSchema));
        mallocSize += sizeof(astAddSchema);
        temp->next->type = SCHEMA_TYPE_DOUBLE;
        cleanKey(name);
        temp->next->name = strdup(name);
        temp->next->dbl = v;
    }
}

void addBoolSchema(char* name, char* v) {
    if(tree.first == NULL) {
        tree.first = (astAddSchema*) malloc(sizeof(astAddSchema));
        mallocSize += sizeof(astAddSchema);
        tree.first->type = SCHEMA_TYPE_BOOLEAN;
        cleanKey(name);
        tree.first->name = strdup(name);
        cleanString(v);
        if(strcmp(v, "true") == 0)
            tree.first->boolean = true;
        else
            tree.first->boolean = false;
    } else {
        astAddSchema* temp = tree.first;
        while(temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = (astAddSchema*) malloc(sizeof(astAddSchema));
        mallocSize += sizeof(astAddSchema);
        temp->next->type = SCHEMA_TYPE_BOOLEAN;
        cleanKey(name);
        temp->next->name = strdup(name);
        cleanString(v);
        if(strcmp(v, "true") == 0)
            temp->next->boolean = true;
        else
            temp->next->boolean = false;
    }
}

void addStrSchema(char* name, char* v) {
    if(tree.first == NULL) {
        tree.first = (astAddSchema*) malloc(sizeof(astAddSchema));
        mallocSize += sizeof(astAddSchema);
        tree.first->type = SCHEMA_TYPE_STRING;
        cleanKey(name);
        tree.first->name = strdup(name);
        cleanString(v);
        tree.first->string = strdup(v);
    } else {
        astAddSchema* temp = tree.first;
        while(temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = (astAddSchema*) malloc(sizeof(astAddSchema));
        mallocSize += sizeof(astAddSchema);
        temp->next->type = SCHEMA_TYPE_STRING;
        cleanKey(name);
        temp->next->name = strdup(name);
        cleanString(v);
        temp->next->string = strdup(v);
    }
}

void fillIndexPredicate(int32_t v) {
    aPredicate = (astPredicate*) malloc(sizeof(astPredicate));
    aPredicate->index = v;
    aPredicate->type = AST_BY_DOCUMENT_NUMBER;
}

void fillValuePredicate(char* key, astCompOperator op, char* str) {
    aPredicate = (astPredicate*) malloc(sizeof(astPredicate));
    mallocSize += sizeof(astPredicate);
    cleanKey(key);
    cleanString(str);
    aPredicate->byValue.key = strdup(key);
    aPredicate->byValue.operator = op;
    aPredicate->byValue.input = strdup(str);
    aPredicate->type = AST_BY_ELEMENT_VALUE;
}

void fillElementPredicate(char* key1, astCompOperator op, char* key2) {
    aPredicate = (astPredicate*) malloc(sizeof(astPredicate));
    mallocSize += sizeof(astPredicate);
    cleanKey(key1);
    cleanKey(key2);
    aPredicate->byElement.key1 = strdup(key1);
    aPredicate->byElement.operator = op;
    aPredicate->byElement.key2 = strdup(key2);
    aPredicate->type = AST_BY_ELEMENT;
}

void addStep() {
    if(tree.path.firstStep == NULL) {
        tree.path.firstStep = aStep;
    } else {
        astStep* temp = tree.path.firstStep;
        while(temp->nextStep != NULL) {
            temp = temp->nextStep;
        }
        temp->nextStep = aStep;
    }
    tree.path.size++;
}

void addPredicate() {
    if(isFirstPred) {
        isFirstPred = false;
        aStep->pred = aPredicate;
        aStep->pred->logOp = AST_NONE;
        aStep->pred->isInverted = inv;
    } else {
        astPredicate* temp = aStep->pred;
        while(temp->nextPredicate != NULL) {
            temp = temp->nextPredicate;
        }
        temp->nextPredicate = aPredicate;
        temp->nextPredicate->logOp = symbolValue;
        temp->nextPredicate->isInverted = inv;
    }
}

void addJPredicate() {
    if(isFirstPred) {
        isFirstPred = false;
        tree.pred = aPredicate;
        tree.pred->logOp = AST_NONE;
        tree.pred->isInverted = inv;
    } else {
        astPredicate* temp = tree.pred;
        while(temp->nextPredicate != NULL) {
            temp = temp->nextPredicate;
        }
        temp->nextPredicate = aPredicate;
        temp->nextPredicate->logOp = symbolValue;
        temp->nextPredicate->isInverted = inv;
    }
}

void cleanString(char* string) {
    memmove(string, string+1, strlen(string));
    string[strlen(string) - 1] = 0;
}

void cleanKey(char* string) {
    memmove(string, string+1, strlen(string));
    memmove(string, string+1, strlen(string));
    string[strlen(string) - 1] = 0;
}

ast getAst() {
    return tree;
}

void setAst() {
    tree.type = AST_ADD;
    tree.docName = NULL;
    tree.elName = NULL;
    tree.value = NULL;

                astAddSchema* temp4 = tree.first;
                while (temp4 != NULL) {
                    temp4 = temp4->next;
                    free(temp4);
                }
                    free(tree.first);
            astPredicate* temp2 = tree.pred;
            while (temp2 != NULL) {
                temp2 = temp2->nextPredicate;
                free(temp2);
            }
            free(tree.pred);
                    astStep* temp3 = tree.path.firstStep;
                    while (temp3 != NULL) {
                        temp3 = temp3->nextStep;
                        free(temp3);
                    }
                    free(tree.path.firstStep);
    tree.path.size = 0;
    tree = emptyTree;

        astPredicate* temp = aPredicate;
        while (temp != NULL) {
            temp = temp->nextPredicate;
            free(temp);
        }
        free(aPredicate);

    symbolValue = 0;
    isFirstPred = true;
    inv = false;
    mallocSize = 0;
}

size_t getSize() {
    return mallocSize;
}