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
#include <ctype.h>

typedef enum str2intStatus {
    STR2INT_SUCCESS,
    STR2INT_OVERFLOW,
    STR2INT_UNDERFLOW,
    STR2INT_INCONVERTIBLE
} str2intStatus;

str2intStatus str2long(int64_t *out, char *s) {
    char *end;
    if (s[0] == '\0' || isspace(s[0]))
        return STR2INT_INCONVERTIBLE;
    errno = 0;
    int64_t l = strtol(s, &end, 10);
    if (l > INT32_MAX || errno == ERANGE)
        return STR2INT_OVERFLOW;
    if (l < INT32_MIN || errno == ERANGE)
        return STR2INT_UNDERFLOW;
    if (*end != '\0')
        return STR2INT_INCONVERTIBLE;
    *out = l;
    return STR2INT_SUCCESS;
}

int main(int argc, char** argv) {
    uint32_t ip = INADDR_LOOPBACK;
    uint16_t port = 25562;
    if (argc > 1) {
        int64_t t;
        str2long(&t, argv[1]);
        port = (uint16_t) t;
    } else {
        printf("Enter port\n");
        exit(1);
    }

    int sock;
    struct sockaddr_in addr;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        exit(1);
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(ip);
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
        //xmlSaveFormatFileEnc("-", answer, "UTF-8", 1);
        xmlFree(outXml);
        xmlFreeDoc(doc);
    }
    close(sock);

    return 0;
}