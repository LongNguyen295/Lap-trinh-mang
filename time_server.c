#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <time.h>

#define PORT 9000
#define MAX_CONNECTIONS 8
#define BUFFER_SIZE 256

char* get_current_time(char* format) {
    time_t now;
    struct tm *tm_info;
    char *time_str = (char*)malloc(256 * sizeof(char));

    time(&now);
    tm_info = localtime(&now);

    if(strncmp(format, "dd/mm/yyyy", 10) == 0) {
        strftime(time_str, 256, "%d/%m/%Y", tm_info);
    } else if(strncmp(format, "dd/mm/yy", 8) == 0) {
        strftime(time_str, 256, "%d/%m/%y", tm_info);
    } else if(strncmp(format, "mm/dd/yyyy", 10) == 0) {
        strftime(time_str, 256, "%m/%d/%Y", tm_info);
    } else if(strncmp(format, "mm/dd/yy", 8) == 0) {
        strftime(time_str, 256, "%m/%d/%y", tm_info);
    } else {
        strcpy(time_str, "Invalid format!");
    }
    return time_str;
}

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

        // Check client command and send corresponding time
        if (strncmp(buf, "GET_TIME", 8) == 0) {
            char* format = buf + 9;
            printf("Format: %s\n", format);
            char* time_str = get_current_time(format);
            send(client_fd, time_str, strlen(time_str), 0);
            free(time_str);
        } else {
            char* error_msg = "Invalid command!";
            send(client_fd, error_msg, strlen(error_msg), 0);
        }

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
    close(listener);
    return 0;
}
