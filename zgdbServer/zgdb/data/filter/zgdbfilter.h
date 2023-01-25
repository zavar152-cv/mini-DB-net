#ifndef ZGDBPROJECT_ZGDBFILTER_H
#define ZGDBPROJECT_ZGDBFILTER_H

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
/*
 * context - current node;
 *
 * ABSOLUTE_PATH checks from context by only one depth step
 * RELATIVE_PATH checks from context and go through whole tree
 */
typedef enum pathType {
   ABSOLUTE_PATH = 0,
   RELATIVE_PATH
} pathType;

/*
 * DOCUMENT_STEP this step is document name
 * ELEMENT_STEP this step is element name
 */
typedef enum stepType {
  DOCUMENT_STEP = 0,
  ELEMENT_STEP
} stepType;

/*
 * NONE for first step only
 */
typedef enum logOperator {
    NONE = 0,
    AND,
    OR
} logOperator;

typedef enum compOperator {
    EQUALS = 0,
    NOT_EQUALS,
    GREATER,
    LESS,
    EQ_GREATER,
    EQ_LESS,
    CONTAINS
} compOperator;

/*
 * BY_DOCUMENT_NUMBER - index of document in result list
 * BY_ELEMENT_VALUE - check value of element using compOperator (see checkValue struct)
 */
typedef enum predicateType {
    BY_DOCUMENT_NUMBER = 0,
    BY_ELEMENT_VALUE,
    BY_ELEMENT
} predicateType;

typedef struct checkType {
    char key[13];
    compOperator operator;
    char* input;
} checkType;

typedef struct checkElement {
    char key1[13];
    compOperator operator;
    char key2[13];
} checkElement;

typedef struct predicate predicate;

typedef struct predicate {
    logOperator logOp;
    bool isInverted;
    predicate* nextPredicate;
    enum predicateType type;
    union {
        uint64_t index;
        checkType byValue;
        checkElement byElement;
    };
} predicate;

typedef struct step {
    pathType pType;
    char stepName[13];
    stepType sType;
    predicate* pred;
} step;

typedef struct path {
   step* steps;
   size_t size;
   size_t capacity;
} path;

/*
 * UNDEFINED for start of the search
 */
typedef enum resultType {
    UNDEFINED_RESULT = 0,
    DOCUMENT_RESULT,
    ELEMENT_RESULT
} resultType;

path createPath(size_t n);

void addStep(path* p, step newStep);

void destroyPath(path* p);

#endif
