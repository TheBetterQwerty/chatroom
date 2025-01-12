#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/select.h>

#define MAX_CLIENTS 5

typedef struct {
    int clientfd;
    char username[20];
} client_ino;

void whisper(char* buffer, client_ino* clients, int client, int sender_fd) {
    for (int i = 0; i < client; i++) {
        if (clients[i].clientfd != sender_fd)
            continue;
        send(clients[i].clientfd, buffer, strlen(buffer), 0);
    }
}

void broadcast(char* buffer, client_ino* clients, int client, int sender_fd) {
    for (int i = 0; i < client; i++) {
        if (clients[i].clientfd == sender_fd)
            continue;
        send(clients[i].clientfd, buffer, strlen(buffer), 0);
    }
}

void remove_client(client_ino* clients, int idx, int* client) {
    close(clients[idx].clientfd);
    char msg[47] = { 0 };
    sprintf(msg, "\n[SERVER] %s left the chat", clients[idx].username);
    for (int i = idx; i < (*client) - 1; i++) {
        if (i+1 < *client) {
            clients[i] = clients[i+1];
        }
    }
    (*client)--;
    broadcast(msg, clients, *client, 0);
}

int listener(char* ip, int port) {
    struct sockaddr_in server, client;
    int sockfd, clientfd;
    int optval = 1, nclients = 0, max;
    char username[20] = { 0 }, buffer[256] = { 0 }, msg[500];
    client_ino clients[MAX_CLIENTS] = { 0 };
    fd_set readfds;
    char* rules = "\t\t ==== RULES ====\n\t:exit -> disconnects you from server";

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("[!] Error Creating Socket!");
        return 1;
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, (socklen_t) sizeof(optval)) < 0) {
        perror("[!] Error setting up sockopt");
        close(sockfd);
        return 1;
    }

    // setting up sockaddr_in
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(ip);


    if (bind(sockfd, (struct sockaddr*) &server, (socklen_t) sizeof(server)) < 0) {
        perror("[!] Error binding to 127.0.0.1");
        close(sockfd);
        return 1;
    }

    printf("[*] Listening On 127.0.0.1 : %d\n", port);

    if (listen(sockfd, MAX_CLIENTS) < 0) {
        perror("[!] Error setting up Listener!");
        close(sockfd);
        return 1;
    }

    FD_ZERO(&readfds);
    FD_SET(sockfd, &readfds);
    max = sockfd;

    while (1) {
        fd_set tempfds = readfds;
        int activity = select(max + 1, &tempfds, NULL, NULL, NULL);
        if (activity < 0) {
            perror("[!] Error in select");
            break;
        }

        if(FD_ISSET(sockfd, &tempfds)) {
            socklen_t client_len = sizeof(client);
            clientfd = accept(sockfd, (struct sockaddr*) &client, &client_len);
            if (clientfd < 0) {
                perror("[!] Error accepting client!");
                continue;
            }

            if (nclients >= MAX_CLIENTS) {
                printf("[!] Max clients reached!\n");
                close(clientfd);
                continue;
            }

            char cmsg[50];
            read(clientfd, username, sizeof(username) - 1);
            username[sizeof(username)] = '\0';
            printf("[*] New Connection Accepted!\n");
            sprintf(cmsg, "\nSERVER~ %s joined the chat", username);
            broadcast(cmsg, clients, nclients, 0);
            *cmsg = '\0';

            // Store that shit
            clients[nclients].clientfd = clientfd;
            strcpy(clients[nclients].username, username);
            FD_SET(clientfd, &readfds);
            if (clientfd > max)
                max = clientfd;
            nclients++;
        }

        for (int i = 0; i < nclients; i++) {
            int client_socket = clients[i].clientfd;

            if (FD_ISSET(client_socket, &tempfds)) {
                int bytes = read(client_socket, buffer, sizeof(buffer)-1);
                if (bytes <= 0) {
                    FD_CLR(client_socket, &readfds);
                    remove_client(clients, i, &nclients);
                    i--;
                }

                if (bytes > 0) {
                    buffer[bytes] = '\0';
                    if (strcmp(buffer, ":rules") == 0) {
                        whisper(rules, clients, nclients, client_socket);
                        continue;
                    }
                    sprintf(msg, "\n[%s] %s", clients[i].username, buffer);
                    msg[strlen(msg)] = '\0';
                    broadcast(msg, clients, nclients, client_socket);
                    *msg = '\0';
                }
            }
        }
    }

    close(sockfd);
    return 0;
}

int main(int args, char** argv) {
    if (args < 3) {
        printf("[?] Usage: ./server <ip> <port_number>\n");
        return 1;
    }

    if (listener(argv[1], atoi(argv[2]))) {
        return 1;
    }
    return 0;
}
