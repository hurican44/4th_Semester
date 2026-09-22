#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define BUF_SIZE 4096
#define SERVER_IP "127.0.0.1"

int main() {
    int sock_fd;
    struct sockaddr_in server_addr;
    char buffer[BUF_SIZE];
    char file_content[BUF_SIZE] = {0};
    char student_id[64];

    // Step 1: Read local data.txt
    FILE *fp = fopen("data.txt", "r");
    if (!fp) { perror("fopen data.txt"); exit(1); }

    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        strncat(file_content, line, BUF_SIZE - strlen(file_content) - 1);
    }
    fclose(fp);

    // Step 2: Ask user for Student ID
    printf("Enter Student ID to search: ");
    scanf("%63s", student_id);

    // Step 3: Connect to server
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) { perror("socket"); exit(1); }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("inet_pton"); exit(1);
    }

    if (connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect"); exit(1);
    }

    // Step 4: Send "StudentID\n" followed by full file content
    snprintf(buffer, BUF_SIZE, "%s\n%s", student_id, file_content);
    write(sock_fd, buffer, strlen(buffer));

    // Step 5: Wait for and print server's response
    memset(buffer, 0, BUF_SIZE);
    int n = read(sock_fd, buffer, BUF_SIZE - 1);
    if (n > 0) {
        printf("Server Response: %s", buffer);
    }

    close(sock_fd);
    return 0;
}
