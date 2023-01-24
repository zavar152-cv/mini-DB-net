#include "zpathXml.h"

void zpathCompOperatorToXml(xmlDocPtr doc, xmlNodePtr root, astCompOperator compOperator) {
    switch (compOperator) {
        case AST_CONTAINS: {
            xmlNewProp(root, BAD_CAST "compOperator", BAD_CAST "contains");
            break;
        }
        case AST_EQUALS: {
            xmlNewProp(root, BAD_CAST "compOperator", BAD_CAST "EQ");
            break;
        }
        case AST_EQ_GREATER: {
            xmlNewProp(root, BAD_CAST "compOperator", BAD_CAST "EQ_GR");
            break;
        }
        case AST_EQ_LESS: {
            xmlNewProp(root, BAD_CAST "compOperator", BAD_CAST "EQ_LE");
            break;
        }
        case AST_GREATER: {
            xmlNewProp(root, BAD_CAST "compOperator", BAD_CAST "GR");
            break;
        }
        case AST_LESS: {
            xmlNewProp(root, BAD_CAST "compOperator", BAD_CAST "LE");
            break;
        }
        case AST_NOT_EQUALS: {
            xmlNewProp(root, BAD_CAST "compOperator", BAD_CAST "NEQ");
            break;
        }
    }
}

void zpathPredicateToXml(xmlDocPtr doc, xmlNodePtr root, astPredicate* first) {
    xmlNodePtr predicate = xmlNewChild(root, NULL, BAD_CAST "predicate", NULL);
    astPredicate* temp = first;
    size_t size = 0;
    while (temp != NULL) {
        xmlNodePtr predicateElement = xmlNewChild(predicate, NULL, BAD_CAST "predicateElement", NULL);
        switch (temp->logOp) {
            case AST_AND: {
                xmlNewProp(predicateElement, BAD_CAST "logOp", BAD_CAST "AND");
                break;
            }
            case AST_OR: {
                xmlNewProp(predicateElement, BAD_CAST "logOp", BAD_CAST "OR");
                break;
            }
            case AST_NONE:
                xmlNewProp(predicateElement, BAD_CAST "logOp", BAD_CAST "NONE");
                break;
        }
        xmlNewProp(predicateElement, BAD_CAST "inverted", BAD_CAST (temp->isInverted ? "1" : "0"));

        switch (temp->type) {
            case AST_BY_DOCUMENT_NUMBER: {
                xmlNewProp(predicateElement, BAD_CAST "type", BAD_CAST "BY_DOCUMENT_NUMBER");
                char sizeChar[50];
                sprintf(sizeChar, "%zu", temp->index);
                xmlNewProp(predicateElement, BAD_CAST "index", BAD_CAST sizeChar);
                break;
            }
            case AST_BY_ELEMENT_VALUE: {
                xmlNewProp(predicateElement, BAD_CAST "type", BAD_CAST "BY_ELEMENT_VALUE");
                xmlNewProp(predicateElement, BAD_CAST "key", BAD_CAST temp->byValue.key);
                xmlNewProp(predicateElement, BAD_CAST "value", BAD_CAST temp->byValue.input);
                zpathCompOperatorToXml(doc, predicateElement, temp->byValue.operator);
                break;
            }
            case AST_BY_ELEMENT: {
                xmlNewProp(predicateElement, BAD_CAST "type", BAD_CAST "BY_ELEMENT");
                xmlNewProp(predicateElement, BAD_CAST "element1", BAD_CAST temp->byElement.key1);
                xmlNewProp(predicateElement, BAD_CAST "element2", BAD_CAST temp->byElement.key2);
                zpathCompOperatorToXml(doc, predicateElement, temp->byElement.operator);
                break;
            }
        }
        temp = temp->nextPredicate;
        size++;
    }
    char sizeChar[50];
    sprintf(sizeChar, "%zu", size);
    xmlNewProp(predicate, BAD_CAST "size", BAD_CAST sizeChar);
}

void zpathPathToXml(xmlDocPtr doc, xmlNodePtr root, astStep* first) {

    astStep* temp = first;
    while (temp != NULL) {
        xmlNodePtr step = xmlNewChild(root, NULL, BAD_CAST "step", BAD_CAST temp->stepName);
        switch (temp->pType) {
            case AST_ABSOLUTE_PATH: {
                xmlNewProp(step, BAD_CAST "pathType", BAD_CAST "abs");
                break;
            }
            case AST_RELATIVE_PATH: {
                xmlNewProp(step, BAD_CAST "pathType", BAD_CAST "rel");
                break;
            }
        }
        switch (temp->sType) {
            case AST_DOCUMENT_STEP: {
                xmlNewProp(step, BAD_CAST "stepType", BAD_CAST "doc");
                break;
            }
            case AST_ELEMENT_STEP: {
                xmlNewProp(step, BAD_CAST "stepType", BAD_CAST "el");
                break;
            }
        }
        zpathPredicateToXml(doc, step, temp->pred);
        temp = temp->nextStep;
    }
}

void zpathToXml(xmlDocPtr doc, ast* tree) {
    xmlNodePtr root = xmlNewNode(NULL, BAD_CAST "request");
    xmlDocSetRootElement(doc, root);
    switch (tree->type) {
        case AST_ADD:
            xmlNewProp(root, BAD_CAST "type", BAD_CAST "add");
            break;
        case AST_DELETE:
            xmlNewProp(root, BAD_CAST "type", BAD_CAST "delete");
            break;
        case AST_UPDATE:
            xmlNewProp(root, BAD_CAST "type", BAD_CAST "update");
            break;
        case AST_FIND:
            xmlNewProp(root, BAD_CAST "type", BAD_CAST "find");
            break;
        case AST_JOIN:
            xmlNewProp(root, BAD_CAST "type", BAD_CAST "join");
            break;
        case AST_PARENT:
            xmlNewProp(root, BAD_CAST "type", BAD_CAST "parent");
            break;
    }

    if (tree->type == AST_ADD) {
        xmlNewChild(root, NULL, BAD_CAST "document", BAD_CAST tree->docName);
        xmlNodePtr schema = xmlNewChild(root, NULL, BAD_CAST "schema", NULL);
        astAddSchema* temp = tree->first;
        size_t size = 0;
        while (temp != NULL) {
            xmlNodePtr schemaElement = xmlNewChild(schema, NULL, BAD_CAST "schemaElement", NULL);
            char valueChar[50];
            switch (temp->type) {
                case SCHEMA_TYPE_INT: {
                    sprintf(valueChar, "%d", temp->integer);
                    xmlNewProp(schemaElement, BAD_CAST "type", BAD_CAST "INT");
                    xmlNewProp(schemaElement, BAD_CAST "key", BAD_CAST temp->name);
                    xmlNewProp(schemaElement, BAD_CAST "value", BAD_CAST valueChar);
                    break;
                }
                case SCHEMA_TYPE_DOUBLE: {
                    sprintf(valueChar, "%f", temp->dbl);
                    xmlNewProp(schemaElement, BAD_CAST "type", BAD_CAST "DBL");
                    xmlNewProp(schemaElement, BAD_CAST "key", BAD_CAST temp->name);
                    xmlNewProp(schemaElement, BAD_CAST "value", BAD_CAST valueChar);
                    break;
                }
                case SCHEMA_TYPE_BOOLEAN: {
                    sprintf(valueChar, "%d", temp->boolean);
                    xmlNewProp(schemaElement, BAD_CAST "type", BAD_CAST "BOOL");
                    xmlNewProp(schemaElement, BAD_CAST "key", BAD_CAST temp->name);
                    xmlNewProp(schemaElement, BAD_CAST "value", BAD_CAST valueChar);
                    break;
                }
                case SCHEMA_TYPE_STRING: {
                    xmlNewProp(schemaElement, BAD_CAST "type", BAD_CAST "STR");
                    xmlNewProp(schemaElement, BAD_CAST "key", BAD_CAST temp->name);
                    xmlNewProp(schemaElement, BAD_CAST "value", BAD_CAST temp->string);
                    break;
                }
            }
            temp = temp->next;
            size++;
        }
        char sizeChar[50];
        sprintf(sizeChar, "%zu", size);
        xmlNewProp(schema, BAD_CAST "size", BAD_CAST sizeChar);
    } else if (tree->type == AST_UPDATE) {
        xmlNodePtr element = xmlNewChild(root, NULL, BAD_CAST "element", NULL);
        xmlNewProp(element, BAD_CAST "name", BAD_CAST tree->elName);
        xmlNewProp(element, BAD_CAST "newValue", BAD_CAST tree->value);
    } else if (tree->type == AST_JOIN) {
        zpathPredicateToXml(doc, root, tree->pred);
    }

    xmlNodePtr path = xmlNewChild(root, NULL, BAD_CAST "path", NULL);
    char sizeChar[50];
    sprintf(sizeChar, "%zu", tree->path.size);
    xmlNewProp(path, BAD_CAST "size", BAD_CAST sizeChar);
    zpathPathToXml(doc, path, tree->path.firstStep);
    xmlSaveFormatFileEnc("-", doc, "UTF-8", 1);
}
