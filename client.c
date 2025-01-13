#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

void listen_for_msg(char* ip, int port) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("[!] Error Creating Socket!");
        return;
    }

    int optval = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, (socklen_t) sizeof(optval)) < 0) {
        perror("[!] Error setting up sockopt!");
        close(sockfd);
        return;
    }

    // setting up sockaddr_in
    struct sockaddr_in server = { AF_INET, htons(port), inet_addr(ip) };
    socklen_t serverlen = sizeof(server);

    if (connect(sockfd, (struct sockaddr*) &server, serverlen) < 0) {
        perror("[!] error connecting to server!");
        close(sockfd);
        return;
    }

    // Sending username
    char username[20];
    printf("[-] Enter Your Username : ");
    fgets(username, sizeof(username), stdin);
    size_t len = strlen(username);
    if (len < 1) {
        printf("[!] Enter a username. Exitting...\n");
        return;
    }
    if (username[len - 1] == '\n') {
        username[len - 1] = '\0';
    }
    send(sockfd, username, len, 0);
    *username = '\0';
    // logged in

    printf("[+] Welcome To The Server!\n");
    fd_set readfds;
    int max_fd;
    char buffer[256], cmsg[256];

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(0, &readfds);
        FD_SET(sockfd, &readfds);

        max_fd = sockfd > 0 ? sockfd : 0;

        int activity = select(max_fd + 1, &readfds, NULL, NULL, NULL);
        if (activity < 0) {
            perror("[!] Error creating select!");
            break;
        }

        // data in stdin
        if (FD_ISSET(0, &readfds)) {
            fgets(cmsg, sizeof(cmsg)-1, stdin);
            cmsg[strlen(cmsg) - 1] = '\0';
            send(sockfd, cmsg, strlen(cmsg), 0);
            *cmsg = '\0';
        }

        // data from server
        if (FD_ISSET(sockfd, &readfds)) {

            int bytes = read(sockfd, buffer, sizeof(buffer) - 1);
            buffer[strlen(buffer)] = '\0';

            if (bytes <= 0) {
                printf("[!] Disconnected From Server!\n");
                close(sockfd);
                exit(1);
            } else {
                printf("%s\n", buffer);
                *buffer = '\0';
            }

        }
    }
    close(sockfd);
    return;
}

int main(int args, char** argv) {
    if (args < 3) {
        printf("[?] Usage: ./client <ip> <port_number>\n");
        return 1;
    }

    listen_for_msg(argv[1], atoi(argv[2]));
    return 0;
}
