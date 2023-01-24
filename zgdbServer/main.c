#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

int main(int argc, char** argv) {
    int sock, listener;
    struct sockaddr_in addr;
    size_t bytes_read;

    listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener < 0) {
        perror("socket");
        exit(1);
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(25565);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(listener, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
        perror("bind");
        exit(2);
    }

    listen(listener, 1);

    while (1) {
        sock = accept(listener, NULL, NULL);
        if (sock < 0) {
            perror("accept");
            exit(3);
        }

        int msgSize = 0;

        while (1) {
            bytes_read = recv(sock, &msgSize, sizeof(int), 0);
            if (bytes_read <= 0) break;

            char buf[msgSize];
            bytes_read = recv(sock, buf, msgSize, 0);
            if (bytes_read <= 0) break;

            xmlDocPtr pDoc = xmlReadMemory(buf, msgSize, 0, NULL, XML_PARSE_RECOVER);
            xmlSaveFormatFileEnc("-", pDoc, "UTF-8", 1);

            char message[] = "Hello there from server!\n";
            msgSize = sizeof(message);

            send(sock, &msgSize, sizeof(int), 0);
            send(sock, message, msgSize, 0);
        }

        close(sock);
    }
    return 0;
}