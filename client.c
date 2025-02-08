#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "argparse.h"
#include "template.h"

char messages[MAX_MESSAGES][MAX_INPUT] = { 0 };
int message_count = 0;

int client_listener(char* ip, int port) {
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (!clientfd) {
        printf("[!] Error Creating socket!\n");
        return 1;
    }

    struct sockaddr_in client;
    client.sin_family = AF_INET;
    client.sin_port = htons(port);
    client.sin_addr.s_addr = inet_addr(ip);

    int status = connect(clientfd, (const struct sockaddr*) &client, (socklen_t) sizeof(client));
    if (!status) {
        printf("[!] Error Connecting with %s:%d", ip, port);
        return 1;
    }


}

int main(int args, char** argv) {
    char ip[11], username[30];
    int port;
    argparse(args, argv, ip, port, username);

    if (!client_listener(ip, port))
        return 1;

    return 0;
}
