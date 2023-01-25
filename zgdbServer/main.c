#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include "zgdbXmlRequest.h"

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
    addr.sin_port = htons(25561);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(listener, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
        perror("bind");
        exit(2);
    }

    listen(listener, 1);

    zgdbFile* pFile = init("/tmp/server1.zgdb");

    document rootDoc;
    rootDoc.header = getDocumentHeader(pFile, 0);
    rootDoc.isRoot = true;
    rootDoc.indexParent = 0;
    documentSchema schema2 = initSchema(4);
    addBooleanToSchema(&schema2, "bool1", 1);
    addDoubleToSchema(&schema2, "double6", 1.5);
    addIntToSchema(&schema2, "key1", 1);
    addTextToSchema(&schema2, "key2", "hi");
    createDocument(pFile, "test1", &schema2, rootDoc);
    finish(pFile);

    while (1) {
        sock = accept(listener, NULL, NULL);
        pFile = init("/tmp/server1.zgdb");
        if (sock < 0) {
            perror("accept");
            exit(3);
        }

        int msgSize = 0;

        while (1) {
            bytes_read = recv(sock, &msgSize, sizeof(int), 0);
            if (bytes_read <= 0)
                break;

            char buf[msgSize];
            bytes_read = recv(sock, buf, msgSize, 0);
            if (bytes_read <= 0)
                break;

            xmlDocPtr doc = xmlReadMemory(buf, msgSize, 0, NULL, XML_PARSE_RECOVER);
            executeZgdbFromXml(doc, pFile);
            xmlFreeDoc(doc);

            char message[] = "Hello there from server!\n";
            msgSize = sizeof(message);

            send(sock, &msgSize, sizeof(int), 0);
            send(sock, message, msgSize, 0);
        }
        printf("%s", "finished");
        finish(pFile);
        close(sock);
    }
    return 0;
}