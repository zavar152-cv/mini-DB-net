#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include "zgdbXmlRequest.h"

int main(int argc, char** argv) {
    uint16_t port = 25562;
    if (argc > 2) {
        int64_t t;
        str2long(&t, argv[2]);
        port = (uint16_t) t;
    } else {
        printf("Enter port\n");
        exit(1);
    }

    int sock, listener;
    struct sockaddr_in addr;
    size_t bytes_read;

    listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener < 0) {
        perror("socket");
        exit(1);
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(listener, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
        perror("bind");
        exit(2);
    }

    listen(listener, 1);

    zgdbFile* pFile;

    while (1) {
        sock = accept(listener, NULL, NULL);
        pFile = init("/tmp/server1.zgdb");
        if (sock < 0) {
            perror("accept");
            exit(3);
        }

        int msgSize = 0;

        printf("%s", "Client connected\n");
        while (1) {
            bytes_read = recv(sock, &msgSize, sizeof(int), 0);
            if (bytes_read <= 0)
                break;

            char buf[msgSize];
            bytes_read = recv(sock, buf, msgSize, 0);
            if (bytes_read <= 0)
                break;

            xmlDocPtr request = xmlReadMemory(buf, msgSize, 0, NULL, XML_PARSE_RECOVER);
            xmlDocPtr answer = executeZgdbFromXml(request, pFile);
            xmlFreeDoc(request);

            xmlChar* outXml;
            int size;
            xmlDocDumpMemory(answer, &outXml, &size);
            msgSize = size;

            char* out = (char*) outXml;

            send(sock, &msgSize, sizeof(int), 0);
            send(sock, out, msgSize, 0);
        }
        printf("%s", "Client disconnected\n");
        finish(pFile);
        close(sock);
    }
    return 0;
}