#define YYDEBUG 1

#include <stdio.h>
#include <stdlib.h>
#include "zgdbAst.h"
#include "parser.h"
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

extern FILE* yyin;

int main(int argc, char** argv) {
    int sock;
    struct sockaddr_in addr;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        exit(1);
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(25565);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (connect(sock, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
        perror("connect");
        exit(2);
    }

    size_t bytes_read;
    char message[] = "Hello there 2!\n";
    size_t msgSize = sizeof(message);

    send(sock, &msgSize, sizeof(size_t), 0);
    send(sock, message, msgSize, 0);


    bytes_read = recv(sock, &msgSize, sizeof(size_t), 0);
    if (bytes_read <= 0) return 0;

    char buf[msgSize];
    bytes_read = recv(sock, buf, msgSize, 0);
    if (bytes_read <= 0) return 0;

    printf("%s", buf);
    close(sock);

//    if (argc > 1) {
//        yyin = fopen(argv[1], "r");
//        if (yyin == NULL){
//            printf("syntax: %s filename\n", argv[0]);
//        }
//    }
//    yyparse();
//    ast tree = getAst();
//    printAst(&tree);
//
//    printf("%zu", getSize());

    return 0;
}
