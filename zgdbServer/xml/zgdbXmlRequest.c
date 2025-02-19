#include "zgdbXmlRequest.h"

xmlNodePtr findNodeByName(xmlNodePtr rootNode, const xmlChar* nodeName) {
    xmlNodePtr node = rootNode;
    if (node == NULL) {
        return NULL;
    }

    while (node != NULL) {
        if (!xmlStrcmp(node->name, nodeName)) {
            return node;
        } else if (node->children != NULL) {
            node = node->children;
            xmlNodePtr intNode = findNodeByName(node, nodeName);
            if (intNode != NULL) {
                return intNode;
            }
        }
        node = node->next;
    }
    return NULL;
}

compOperator getCompOperator(xmlNodePtr predicateNode) {
    xmlChar* compOperatorChar = xmlGetProp(predicateNode, BAD_CAST "compOperator");
    if (!xmlStrcmp(compOperatorChar, BAD_CAST "contains")) {
        return CONTAINS;
    } else if (!xmlStrcmp(compOperatorChar, BAD_CAST "EQ")) {
        return EQUALS;
    } else if (!xmlStrcmp(compOperatorChar, BAD_CAST "EQ_GR")) {
        return EQ_GREATER;
    } else if (!xmlStrcmp(compOperatorChar, BAD_CAST "EQ_LE")) {
        return EQ_LESS;
    } else if (!xmlStrcmp(compOperatorChar, BAD_CAST "GR")) {
        return GREATER;
    } else if (!xmlStrcmp(compOperatorChar, BAD_CAST "LE")) {
        return LESS;
    } else if (!xmlStrcmp(compOperatorChar, BAD_CAST "NEQ")) {
        return NOT_EQUALS;
    }
}

predicate createPredicateFromXml(xmlNodePtr predicateNode, predicate* pred) {
    xmlChar* sizeChar = xmlGetProp(predicateNode, BAD_CAST "size");
    int64_t s = 0;
    str2long(&s, (char*) sizeChar);

    xmlNodePtr firstPredicateElement = predicateNode->last;
    while (firstPredicateElement->prev != NULL) {
        firstPredicateElement = firstPredicateElement->prev;
    }

    xmlNodePtr tempPredicateElement = firstPredicateElement;
    predicate* lastPred = NULL;
    for (int64_t i = 0; i < s; ++i) {
        predicate* tPred = (predicate*) malloc(sizeof(predicate));
        tPred->nextPredicate = NULL;

        xmlChar* logOpChar = xmlGetProp(tempPredicateElement, BAD_CAST "logOp");

        if (!xmlStrcmp(logOpChar, BAD_CAST "NONE")) {
            tPred->logOp = NONE;
        } else if (!xmlStrcmp(logOpChar, BAD_CAST "OR")) {
            tPred->logOp = OR;
        } else if (!xmlStrcmp(logOpChar, BAD_CAST "AND")) {
            tPred->logOp = AND;
        }

        xmlChar* invertedChar = xmlGetProp(tempPredicateElement, BAD_CAST "inverted");

        if (!xmlStrcmp(invertedChar, BAD_CAST "1")) {
            tPred->isInverted = true;
        } else {
            tPred->isInverted = false;
        }

        xmlChar* typeChar = xmlGetProp(tempPredicateElement, BAD_CAST "type");

        if (!xmlStrcmp(typeChar, BAD_CAST "BY_DOCUMENT_NUMBER")) {
            xmlChar* indexChar = xmlGetProp(tempPredicateElement, BAD_CAST "index");
            int64_t index;
            str2long(&index, (char*) indexChar);
            tPred->type = BY_DOCUMENT_NUMBER;
            tPred->index = index;
        } else if (!xmlStrcmp(typeChar, BAD_CAST "BY_ELEMENT_VALUE")) {
            xmlChar* keyChar = xmlGetProp(tempPredicateElement, BAD_CAST "key");
            xmlChar* valueChar = xmlGetProp(tempPredicateElement, BAD_CAST "value");
            tPred->type = BY_ELEMENT_VALUE;
            strcpy(tPred->byValue.key, (char*) keyChar);
            tPred->byValue.input = (char*) valueChar;
            tPred->byValue.operator = getCompOperator(tempPredicateElement);
        } else if (!xmlStrcmp(typeChar, BAD_CAST "BY_ELEMENT")) {
            xmlChar* key1Char = xmlGetProp(tempPredicateElement, BAD_CAST "key1");
            xmlChar* key2Char = xmlGetProp(tempPredicateElement, BAD_CAST "key2");
            tPred->type = BY_ELEMENT;
            strcpy(tPred->byElement.key1, (char*) key1Char);
            strcpy(tPred->byElement.key2, (char*) key2Char);
            tPred->byElement.operator = getCompOperator(tempPredicateElement);
        }

        if(lastPred == NULL) {
            pred = tPred;
            pred->nextPredicate = NULL;
            lastPred = pred;
        } else {
            lastPred->nextPredicate = tPred;
            lastPred = lastPred->nextPredicate;
        }
        tempPredicateElement = tempPredicateElement->next;
    }
    //pred = lastPred;
    return *pred;
}

void createPathFromXml(xmlNodePtr pathNode, path* p) {
    xmlNodePtr firstStepElement = pathNode->last;
    while (firstStepElement->prev != NULL) {
        firstStepElement = firstStepElement->prev;
    }

    xmlNodePtr temp = firstStepElement;
    while (temp != NULL) {
        step s;
        xmlChar* pathTypeChar = xmlGetProp(temp, BAD_CAST "pathType");
        xmlChar* stepTypeChar = xmlGetProp(temp, BAD_CAST "stepType");
        xmlChar* nameChar = xmlNodeGetContent(temp);
        if (nameChar == NULL) {
            nameChar = BAD_CAST "";
        }

        if (!xmlStrcmp(pathTypeChar, BAD_CAST "abs")) {
            s.pType = ABSOLUTE_PATH;
        } else if (!xmlStrcmp(pathTypeChar, BAD_CAST "rel")) {
            s.pType = RELATIVE_PATH;
        }

        if (!xmlStrcmp(stepTypeChar, BAD_CAST "el")) {
            s.sType = ELEMENT_STEP;
        } else if (!xmlStrcmp(stepTypeChar, BAD_CAST "doc")) {
            s.sType = DOCUMENT_STEP;
        }
        strcpy(s.stepName, (char*) nameChar);

        predicate pred;

        xmlNodePtr predicateNode = findNodeByName(pathNode, BAD_CAST "predicate");
        xmlChar* sizeChar = xmlGetProp(predicateNode, BAD_CAST "size");
        int64_t st = 0;
        str2long(&st, (char*) sizeChar);
        if(st != 0) {
            predicate xml = createPredicateFromXml(predicateNode, &pred);
            s.pred = &xml;
        } else {
            s.pred = NULL;
        }


        addStep(p, s);
        temp = temp->next;
    }
}

createStatus executeAddFromXml(xmlNodePtr root, zgdbFile* file) {
    xmlNodePtr documentNode = findNodeByName(root, BAD_CAST "document");
    xmlChar* name = xmlNodeGetContent(documentNode);

    xmlNodePtr schemaNode = findNodeByName(root, BAD_CAST "schema");
    xmlChar* sizeChar = xmlGetProp(schemaNode, BAD_CAST "size");

    xmlNodePtr pathNode = root->last;

    int64_t s;
    str2long(&s, (char*) sizeChar);

    documentSchema schema = initSchema(s);
    xmlNodePtr firstSchemaElement = schemaNode->last;
    while (firstSchemaElement->prev != NULL) {
        firstSchemaElement = firstSchemaElement->prev;
    }

    xmlNodePtr temp = firstSchemaElement;
    while (temp != NULL) {
        xmlChar* type = xmlGetProp(temp, BAD_CAST "type");
        xmlChar* key = xmlGetProp(temp, BAD_CAST "key");
        xmlChar* value = xmlGetProp(temp, BAD_CAST "value");

        if (!xmlStrcmp(type, BAD_CAST "INT")) {
            int32_t v;
            str2int(&v, (char*) value);
            addIntToSchema(&schema, (char*) key, v);
        } else if (!xmlStrcmp(type, BAD_CAST "DBL")) {
            double v;
            str2double(&v, (char*) value);
            addDoubleToSchema(&schema, (char*) key, v);
        } else if (!xmlStrcmp(type, BAD_CAST "STR")) {
            addTextToSchema(&schema, (char*) key, (char*) value);
        } else if (!xmlStrcmp(type, BAD_CAST "BOOL")) {
            uint8_t v;
            str2boolean(&v, (char*) value);
            addBooleanToSchema(&schema, (char*) key, v);
        }
        temp = temp->next;
    }

    xmlChar* pathSizeChar = xmlGetProp(pathNode, BAD_CAST "size");

    int64_t pSize;
    str2long(&pSize, (char*) pathSizeChar);
    path p;
    if(pSize != 0) {
        p = createPath(pSize);
        createPathFromXml(pathNode, &p);
    }

    findIfResult ifResult = findIfFromRoot(file, p);
    destroyPath(&p);

    resultList list = ifResult.documentList;
    return createDocument(file, (char*) name, &schema, list.head->document);
}

findIfResult executeDeleteFromXml(xmlNodePtr root, zgdbFile* file) {
    xmlNodePtr pathNode = root->last;

    xmlChar* pathSizeChar = xmlGetProp(pathNode, BAD_CAST "size");
    int64_t pSize;
    str2long(&pSize, (char*) pathSizeChar);
    path p;
    if(pSize != 0) {
        p = createPath(pSize);
        createPathFromXml(pathNode, &p);
    }

    findIfResult ifResult = findIfFromRoot(file, p);
    destroyPath(&p);

    if(ifResult.type == UNDEFINED_RESULT)
        return ifResult;

    deleteDocument(file, ifResult.documentList.head->document);
    return ifResult;
}

updateElementStatus executeUpdateFromXml(xmlNodePtr root, zgdbFile* file) {
    xmlNodePtr pathNode = root->last;

    xmlChar* pathSizeChar = xmlGetProp(pathNode, BAD_CAST "size");
    int64_t pSize;
    str2long(&pSize, (char*) pathSizeChar);
    path p;
    if(pSize != 0) {
        p = createPath(pSize);
        createPathFromXml(pathNode, &p);
    }

    findIfResult ifResult = findIfFromRoot(file, p);
    destroyPath(&p);

    xmlNodePtr elementNode = root->children;
    xmlChar* nameChar = xmlGetProp(elementNode, BAD_CAST "name");
    xmlChar* newValueChar = xmlGetProp(elementNode, BAD_CAST "newValue");
    if(ifResult.type == UNDEFINED_RESULT)
        return DOCUMENT_NOT_FOUND;
    return updateElement(file, ifResult.documentList.head->document, (char*) nameChar, (char*) newValueChar);
}

findIfResult executeFindFromXml(xmlNodePtr root, zgdbFile* file) {
    xmlNodePtr pathNode = root->last;

    xmlChar* pathSizeChar = xmlGetProp(pathNode, BAD_CAST "size");
    int64_t pSize;
    str2long(&pSize, (char*) pathSizeChar);
    path p;
    if(pSize != 0) {
        p = createPath(pSize);
        createPathFromXml(pathNode, &p);
    }

    findIfResult ifResult = findIfFromRoot(file, p);
    destroyPath(&p);

    return ifResult;
}

resultList executeJoinFromXml(xmlNodePtr root, zgdbFile* file) {
    xmlNodePtr pathNode = root->last;

    xmlChar* pathSizeChar = xmlGetProp(pathNode, BAD_CAST "size");
    int64_t pSize;
    str2long(&pSize, (char*) pathSizeChar);
    path p;
    if(pSize != 0) {
        p = createPath(pSize);
        createPathFromXml(pathNode, &p);
    }

    findIfResult ifResult = findIfFromRoot(file, p);
    //destroyPath(&p);

    predicate pred;

    xmlNodePtr predicateNode = findNodeByName(root, BAD_CAST "predicate");
    xmlChar* sizeChar = xmlGetProp(predicateNode, BAD_CAST "size");
    int64_t st = 0;
    str2long(&st, (char*) sizeChar);
    if(st != 0) {
        predicate xml = createPredicateFromXml(predicateNode, &pred);
        return join(file, ifResult.documentList.head->document, &xml);
    } else {
        return join(file, ifResult.documentList.head->document, NULL);
    }
    

}

document executeParentFromXml(xmlNodePtr root, zgdbFile* file) {
    xmlNodePtr pathNode = root->last;

    xmlChar* pathSizeChar = xmlGetProp(pathNode, BAD_CAST "size");
    int64_t pSize;
    str2long(&pSize, (char*) pathSizeChar);
    path p;
    if(pSize != 0) {
        p = createPath(pSize);
        createPathFromXml(pathNode, &p);
    }

    findIfResult ifResult = findIfFromRoot(file, p);
    return parent(file, ifResult.documentList.head->document);
}

xmlDocPtr executeZgdbFromXml(xmlDocPtr doc, zgdbFile* file) {
    xmlNodePtr root = xmlDocGetRootElement(doc);
    xmlChar* string = xmlGetProp(root, BAD_CAST "type");
    xmlDocPtr answer = xmlNewDoc(BAD_CAST "1.0");
    xmlNodePtr rootAnswer = xmlNewNode(NULL, BAD_CAST "answer");
    xmlDocSetRootElement(answer, rootAnswer);
    
    if (!xmlStrcmp(string, BAD_CAST "add")) {
        createStatus status = executeAddFromXml(root, file);
        xmlNewProp(rootAnswer, BAD_CAST "type", BAD_CAST "createStatus");
        switch (status) {
            case CREATE_OK:
                xmlNewChild(rootAnswer, NULL, BAD_CAST "info", BAD_CAST "CREATE OK");
                break;
            case OUT_OF_INDEX:
                xmlNewChild(rootAnswer, NULL, BAD_CAST "info", BAD_CAST "OUT OF INDEX");
                break;
            case CREATE_FAILED:
                xmlNewChild(rootAnswer, NULL, BAD_CAST "info", BAD_CAST "CREATE FAILED");
                break;
        }
    } else if (!xmlStrcmp(string, BAD_CAST "delete")) {
        findIfResult ifResult = executeDeleteFromXml(root, file);
        xmlNewProp(rootAnswer, BAD_CAST "type", BAD_CAST "deleteStatus");
        switch (ifResult.type) {
            case UNDEFINED_RESULT:
                xmlNewChild(rootAnswer, NULL, BAD_CAST "info", BAD_CAST "DOCUMENT NOT FOUND");
                break;
            case DOCUMENT_RESULT:
                xmlNewChild(rootAnswer, NULL, BAD_CAST "info", BAD_CAST "DOCUMENT DELETED");
                break;
            case ELEMENT_RESULT:
                xmlNewChild(rootAnswer, NULL, BAD_CAST "info", BAD_CAST "WTF (ELEMENT?)");
                break;
        }
    } else if (!xmlStrcmp(string, BAD_CAST "update")) {
        updateElementStatus status = executeUpdateFromXml(root, file);
        xmlNewProp(rootAnswer, BAD_CAST "type", BAD_CAST "updateStatus");
        switch (status) {
            case UPDATE_OK:
                xmlNewChild(rootAnswer, NULL, BAD_CAST "info", BAD_CAST "UPDATE OK");
                break;
            case ELEMENT_NOT_FOUND:
                xmlNewChild(rootAnswer, NULL, BAD_CAST "info", BAD_CAST "ELEMENT NOT FOUND");
                break;
            case INVALID_NAME:
                xmlNewChild(rootAnswer, NULL, BAD_CAST "info", BAD_CAST "INVALID NAME");
                break;
            case TYPE_PARSE_ERROR:
                xmlNewChild(rootAnswer, NULL, BAD_CAST "info", BAD_CAST "TYPE PARSE ERROR");
                break;
            case DOCUMENT_NOT_FOUND:
                xmlNewChild(rootAnswer, NULL, BAD_CAST "info", BAD_CAST "DOCUMENT NOT FOUND");
                break;
        }
    } else if (!xmlStrcmp(string, BAD_CAST "find")) {
        findIfResult ifResult = executeFindFromXml(root, file);
        xmlNewProp(rootAnswer, BAD_CAST "type", BAD_CAST "findStatus");
        switch (ifResult.type) {
            case UNDEFINED_RESULT: {
                xmlNewChild(rootAnswer, NULL, BAD_CAST "info", BAD_CAST "UNDEFINED RESULT");
                break;
            }
            case DOCUMENT_RESULT: {
                xmlNewChild(rootAnswer, NULL, BAD_CAST "info", BAD_CAST "DOCUMENT");
                result* tempRes = ifResult.documentList.head;
                while (tempRes != NULL) {
                    xmlNewChild(rootAnswer, NULL, BAD_CAST "document", BAD_CAST tempRes->document.header.name);
                    tempRes = tempRes->next;
                }
                break;
            }
            case ELEMENT_RESULT: {
                xmlNewChild(rootAnswer, NULL, BAD_CAST "info", BAD_CAST "ELEMENT");
                eLresult* tempRes = ifResult.elementList.head;
                while (tempRes != NULL) {
                    xmlNodePtr child = xmlNewChild(rootAnswer, NULL, BAD_CAST "element", BAD_CAST tempRes->el.key);
                    switch (tempRes->el.type) {
                        case TYPE_INT: {
                            xmlNewProp(child, BAD_CAST "type", BAD_CAST "INT");
                            char sizeChar[50];
                            sprintf(sizeChar, "%d", tempRes->el.integerValue);
                            xmlNewProp(child, BAD_CAST "value", BAD_CAST sizeChar);
                            break;
                        }
                        case TYPE_TEXT: {
                            xmlNewProp(child, BAD_CAST "type", BAD_CAST "TEXT");
                            xmlNewProp(child, BAD_CAST "value", BAD_CAST tempRes->el.textValue.data);
                            break;
                        }
                        case TYPE_DOUBLE: {
                            xmlNewProp(child, BAD_CAST "type", BAD_CAST "DOUBLE");
                            char sizeChar[50];
                            sprintf(sizeChar, "%f", tempRes->el.doubleValue);
                            xmlNewProp(child, BAD_CAST "value", BAD_CAST sizeChar);
                            break;
                        }
                        case TYPE_BOOLEAN: {
                            xmlNewProp(child, BAD_CAST "type", BAD_CAST "BOOLEAN");
                            char sizeChar[50];
                            sprintf(sizeChar, "%d", tempRes->el.booleanValue);
                            xmlNewProp(child, BAD_CAST "value", BAD_CAST sizeChar);
                            break;
                        }
                    }
                    xmlNewProp(child, BAD_CAST "document", BAD_CAST tempRes->document.header.name);
                    tempRes = tempRes->next;
                }
                break;
            }
        }

    } else if (!xmlStrcmp(string, BAD_CAST "join")) {
        resultList list = executeJoinFromXml(root, file);
        xmlNewProp(rootAnswer, BAD_CAST "type", BAD_CAST "joinStatus");
        result* tempRes = list.head;
        while (tempRes != NULL) {
            xmlNewChild(rootAnswer, NULL, BAD_CAST "document", BAD_CAST tempRes->document.header.name);
            tempRes = tempRes->next;
        }
    } else if (!xmlStrcmp(string, BAD_CAST "parent")) {
        document xml = executeParentFromXml(root, file);
        xmlNewProp(rootAnswer, BAD_CAST "type", BAD_CAST "parentStatus");
        xmlNewChild(rootAnswer, NULL, BAD_CAST "document", BAD_CAST xml.header.name);
    }
    xmlSaveFormatFileEnc("-", doc, "UTF-8", 1);
    return answer;
}