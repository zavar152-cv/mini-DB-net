#ifndef ZPATHPROJECT_ZGDBAST_H
#define ZPATHPROJECT_ZGDBAST_H

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

typedef enum astPathType {
    AST_ABSOLUTE_PATH = 0,
    AST_RELATIVE_PATH
} astPathType;

typedef enum astStepType {
    AST_DOCUMENT_STEP = 0,
    AST_ELEMENT_STEP
} astStepType;

typedef enum astLogOperator {
    AST_NONE = 0,
    AST_AND,
    AST_OR
} astLogOperator;

typedef enum astCompOperator {
    AST_EQUALS = 0,
    AST_NOT_EQUALS,
    AST_GREATER,
    AST_LESS,
    AST_EQ_GREATER,
    AST_EQ_LESS,
    AST_CONTAINS
} astCompOperator;

typedef enum astPredicateType {
    AST_BY_DOCUMENT_NUMBER = 0,
    AST_BY_ELEMENT_VALUE,
    AST_BY_ELEMENT
} astPredicateType;

typedef struct astCheckType {
    char* key;
    astCompOperator operator;
    char* input;
} astCheckType;

typedef struct astCheckTypeElement {
    char* key1;
    astCompOperator operator;
    char* key2;
} astCheckTypeElement;

typedef struct astPredicate astPredicate;

typedef struct astPredicate {
    astLogOperator logOp;
    bool isInverted;
    astPredicate* nextPredicate;
    enum astPredicateType type;
    union {
        uint64_t index;
        astCheckType byValue;
        astCheckTypeElement byElement;
    };
} astPredicate;

typedef struct astStep astStep;

typedef struct astStep {
    astPathType pType;
    char* stepName;
    astStepType sType;
    astPredicate* pred;
    astStep* nextStep;
} astStep;

typedef struct astPath {
    astStep* firstStep;
    size_t size;
} astPath;

typedef enum requestType {
    AST_ADD = 0,
    AST_DELETE,
    AST_UPDATE,
    AST_FIND,
    AST_JOIN,
    AST_PARENT
} requestType;

typedef enum schemaElementType {
    SCHEMA_TYPE_INT = 0x01,
    SCHEMA_TYPE_DOUBLE = 0x02,
    SCHEMA_TYPE_BOOLEAN = 0x03,
    SCHEMA_TYPE_STRING = 0x04,
} schemaElementType;

typedef struct astAddSchema astAddSchema;

typedef struct astAddSchema {
    schemaElementType type;
    char* name;
    union {
        int32_t integer;
        double dbl;
        bool boolean;
        char* string;
    };
    astAddSchema* next;
} astAddSchema;

typedef struct ast {
    requestType type;
    union {
        struct {
            char* docName;
            astAddSchema* first;
        };
        struct {
            char* elName;
            char* value;
        };
        struct {
            astPredicate* pred;
        };
    };
    astPath path;
} ast;

ast getAst();

void printAst(ast* tree);

size_t getSize();

#endif
