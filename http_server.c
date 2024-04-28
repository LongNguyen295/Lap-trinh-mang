#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

#define PORT 9000
#define MAX_CONNECTIONS 8
#define BUFFER_SIZE 1024

void handle_client(int client_fd) {
    char buf[BUFFER_SIZE];
    int ret;

    while (1) {
        ret = recv(client_fd, buf, sizeof(buf), 0);
        if (ret <= 0) {
            close(client_fd);
            continue;
        }

        if (ret < sizeof(buf))
            buf[ret] = '\0';
        printf("%s\n", buf);

        char msg[] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body><h1>Hello World</h1></body></html>";
        send(client_fd, msg, strlen(msg), 0);

        close(client_fd);
    }
}

int main() {
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1) {
        perror("socket() failed");
        return 1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(PORT);

    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr))) {
        perror("bind() failed");
        return 1;
    }

    if (listen(listener, MAX_CONNECTIONS)) {
        perror("listen() failed");
        return 1;
    }

    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        if (fork() == 0) {
            // Child process
            signal(SIGCHLD, SIG_IGN); // Ignore child termination signals
            int client_fd;
            while (1) {
                client_fd = accept(listener, NULL, NULL);
                if (client_fd < 0) {
                    perror("accept() failed");
                    continue;
                }
                handle_client(client_fd);
            }
            exit(0);
        }
    }

    getchar();
    killpg(0, SIGKILL);

    return 0;
}
