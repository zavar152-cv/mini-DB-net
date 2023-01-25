#ifndef ZPATHPROJECT_ZGDBXMLREQUEST_H
#define ZPATHPROJECT_ZGDBXMLREQUEST_H

#include <libxml/parser.h>
#include <libxml/tree.h>
#include "zgdb.h"

xmlDocPtr executeZgdbFromXml(xmlDocPtr doc, zgdbFile* file);

#endif
