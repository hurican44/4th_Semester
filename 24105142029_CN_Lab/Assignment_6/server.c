#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <signal.h>

#define PORT 8080
#define BUF_SIZE 4096

void handle_client(int client_fd) {
    char buffer[BUF_SIZE];
    memset(buffer, 0, BUF_SIZE);

    int n = read(client_fd, buffer, BUF_SIZE - 1);
    if (n <= 0) { close(client_fd); return; }
    buffer[n] = '\0';

    // Protocol: first line = Student ID, remaining lines = data.txt content
    char *newline = strchr(buffer, '\n');
    char response[BUF_SIZE];

    if (!newline) {
        snprintf(response, BUF_SIZE, "Record Not Found\n");
        write(client_fd, response, strlen(response));
        close(client_fd);
        return;
    }

    *newline = '\0';
    char student_id[64];
    strncpy(student_id, buffer, sizeof(student_id) - 1);
    student_id[sizeof(student_id) - 1] = '\0';
    char *file_data = newline + 1;

    int found = 0;
    char data_copy[BUF_SIZE];
    strncpy(data_copy, file_data, BUF_SIZE - 1);
    data_copy[BUF_SIZE - 1] = '\0';

    char *line = strtok(data_copy, "\n");
    while (line != NULL) {
        char id[20], name[50], dept[20];
        if (sscanf(line, "%19s %49s %19s", id, name, dept) == 3) {
            if (strcmp(id, student_id) == 0) {
                snprintf(response, BUF_SIZE, "Record Found: %s %s %s\n", id, name, dept);
                found = 1;
                break;
            }
        }
        line = strtok(NULL, "\n");
    }

    if (!found) {
        snprintf(response, BUF_SIZE, "Record Not Found\n");
    }

    write(client_fd, response, strlen(response));
    close(client_fd);
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    signal(SIGCHLD, SIG_IGN); // auto-reap finished children, no zombies

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) { perror("socket"); exit(1); }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind"); exit(1);
    }

    if (listen(server_fd, 10) < 0) { perror("listen"); exit(1); }

    printf("Server listening on port %d...\n", PORT);

    while (1) {
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
        if (client_fd < 0) { perror("accept"); continue; }

        pid_t pid = fork();
        if (pid == 0) {
            close(server_fd);       // child doesn't need the listening socket
            handle_client(client_fd);
            exit(0);
        } else if (pid > 0) {
            close(client_fd);       // parent doesn't need the connected socket
        } else {
            perror("fork");
            close(client_fd);
        }
    }

    close(server_fd);
    return 0;
}
