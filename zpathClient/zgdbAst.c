#include "zgdbAst.h"

void printReqType(uint8_t t) {
    printf("Request type: ");
    switch (t) {
        case AST_ADD: {
            printf("%s\n", "ADD");
            break;
        }
        case AST_DELETE: {
            printf("%s\n", "DELETE");
            break;
        }
        case AST_UPDATE: {
            printf("%s\n", "UPDATE");
            break;
        }
        case AST_FIND: {
            printf("%s\n", "FIND");
            break;
        }
        case AST_JOIN: {
            printf("%s\n", "JOIN");
            break;
        }
        case AST_PARENT: {
            printf("%s\n", "PARENT");
            break;
        }
        default: {
            printf("%s\n", "unsupported");
            break;
        }
    }
}

void printSchema(astAddSchema* first) {
    printf("\n    ");
    printf("Schema\n");
    astAddSchema* temp = first;
    while(temp != NULL) {
        switch (temp->type) {
            case SCHEMA_TYPE_INT: {
                printf("    ");
                printf("INT:%s:%d\n", temp->name, temp->integer);
                break;
            }
            case SCHEMA_TYPE_DOUBLE: {
                printf("    ");
                printf("DBL:%s:%f\n", temp->name, temp->dbl);
                break;
            }
            case SCHEMA_TYPE_BOOLEAN: {
                printf("    ");
                printf("BOOL:%s:%d\n", temp->name, temp->boolean);
                break;
            }
            case SCHEMA_TYPE_STRING: {
                printf("    ");
                printf("STR:%s:%s\n", temp->name, temp->string);
                break;
            }
        }
        temp = temp->next;
    }
}

void printPredicate(astPredicate* predicate, int i) {
    astPredicate* temp = predicate;
    int k = 1;
    while (temp != NULL) {
        for (int j = 0; j < i; ++j) {
            printf("    ");
        }
        printf("%dp) ", k);
        k++;
        if(temp->logOp != AST_NONE) {
            switch (temp->logOp) {
                case AST_AND: {
                    printf("connector: %s, ", "AND");
                    break;
                }
                case AST_OR: {
                    printf("connector: %s, ", "OR");
                    break;
                }
                case AST_NONE:
                    break;
            }
        }
        if(temp->isInverted)
            printf("inverted, ");
        switch (temp->type) {
            case AST_BY_DOCUMENT_NUMBER: {
                printf("by index: %lu;", temp->index);
                break;
            }
            case AST_BY_ELEMENT_VALUE: {
                printf("by value: @%s ", temp->byValue.key);

                switch (temp->byValue.operator) {
                    case AST_CONTAINS: {
                        printf("%s", "contains");
                        break;
                    }
                    case AST_EQUALS: {
                        printf("%s", "=");
                        break;
                    }
                    case AST_EQ_GREATER: {
                        printf("%s", ">=");
                        break;
                    }
                    case AST_EQ_LESS: {
                        printf("%s", "<=");
                        break;
                    }
                    case AST_GREATER: {
                        printf("%s", ">");
                        break;
                    }
                    case AST_LESS: {
                        printf("%s", "<");
                        break;
                    }
                    case AST_NOT_EQUALS: {
                        printf("%s", "!=");
                        break;
                    }
                }

                printf(" %s;", temp->byValue.input);

                break;
            }
            case AST_BY_ELEMENT: {

                printf("by element: @%s ", temp->byElement.key1);

                switch (temp->byValue.operator) {
                    case AST_CONTAINS: {
                        printf("%s", "contains");
                        break;
                    }
                    case AST_EQUALS: {
                        printf("%s", "=");
                        break;
                    }
                    case AST_EQ_GREATER: {
                        printf("%s", ">=");
                        break;
                    }
                    case AST_EQ_LESS: {
                        printf("%s", "<=");
                        break;
                    }
                    case AST_GREATER: {
                        printf("%s", ">");
                        break;
                    }
                    case AST_LESS: {
                        printf("%s", "<");
                        break;
                    }
                    case AST_NOT_EQUALS: {
                        printf("%s", "!=");
                        break;
                    }
                }

                printf(" @%s;", temp->byElement.key2);

                break;
            }
        }
        temp = temp->nextPredicate;
        printf("\n");
    }
}

void printPath(astPath* path) {
    printf("\n    ");
    printf("Path\n");
    astStep* temp = path->firstStep;
    int i = 1;
    while (temp != NULL) {
        for (int j = 0; j < i; ++j) {
            printf("    ");
        }
        printf("Step name: %s; ", temp->stepName);
        switch (temp->pType) {
            case AST_ABSOLUTE_PATH: {
                printf("(%s, ", "absolute path");
                break;
            }
            case AST_RELATIVE_PATH: {
                printf("(%s, ", "relative path");
                break;
            }
        }
        switch (temp->sType) {
            case AST_DOCUMENT_STEP: {
                printf("%s)\n", "document step");
                break;
            }
            case AST_ELEMENT_STEP: {
                printf("%s)\n", "element step");
                break;
            }
        }
        printPredicate(temp->pred, i);
        temp = temp->nextStep;
        i++;
        printf("\n");
    }
}

void printAst(ast* tree) {
    printReqType(tree->type);
    if(tree->type == AST_ADD) {
        printf("    ");
        printf("New document name: %s\n", tree->docName);
        printSchema(tree->first);
    } else if(tree->type == AST_UPDATE) {
        printf("    ");
        printf("Element name: %s\n", tree->elName);
        printf("    ");
        printf("New value: %s\n", tree->value);
    } else if(tree->type == AST_JOIN) {
        printPredicate(tree->pred, 1);
    }
    printPath(&tree->path);
}