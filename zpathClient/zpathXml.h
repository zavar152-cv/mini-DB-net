#ifndef ZPATHPROJECT_ZPATHXML_H
#define ZPATHPROJECT_ZPATHXML_H

#include <libxml/parser.h>
#include <libxml/tree.h>
#include "zgdbAst.h"

void zpathToXml(xmlDocPtr doc, ast* tree);

void printAnswer(xmlDocPtr doc);

#endif
