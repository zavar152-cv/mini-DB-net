#define YYDEBUG 1

#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
#include "lexer.h"
#include "zpathXml.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

int main(int argc, char** argv) {
    if (argc > 1) {
        yyin = fopen(argv[1], "r");
        if (yyin == NULL) {
            printf("syntax: %outXml filename\n", argv[0]);
        }
    }

    int sock;
    struct sockaddr_in addr;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        exit(1);
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(25562);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (connect(sock, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
        perror("connect");
        exit(2);
    }

    int i = yyparse();
    if(i == 0) {
        ast tree = getAst();
        printAst(&tree);
        xmlDocPtr doc = xmlNewDoc(BAD_CAST "1.0");

        zpathToXml(doc, &tree);

        xmlChar* outXml;
        int size;
        xmlDocDumpMemory(doc, &outXml, &size);

        char* out = (char*) outXml;

        size_t bytes_read;
        int msgSize = size;

        send(sock, &msgSize, sizeof(int), 0);
        send(sock, out, msgSize, 0);


        bytes_read = recv(sock, &msgSize, sizeof(int), 0);
        if (bytes_read <= 0) return 0;

        char buf[msgSize];
        bytes_read = recv(sock, buf, msgSize, 0);
        if (bytes_read <= 0) return 0;

        xmlDocPtr answer = xmlReadMemory(buf, msgSize, 0, NULL, XML_PARSE_RECOVER);
        printAnswer(answer);
        xmlSaveFormatFileEnc("-", answer, "UTF-8", 1);
        xmlFree(outXml);
        xmlFreeDoc(doc);
    }
    close(sock);

    return 0;
}