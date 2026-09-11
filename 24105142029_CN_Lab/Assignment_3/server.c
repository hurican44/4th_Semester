// server code

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8081
#define BUFFER_SIZE 1024

int main()
{
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE];

    
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1)
    {
        perror("Socket failed");
        exit(1);
    }

    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("Bind failed");
        exit(1);
    }

    
    if (listen(server_fd, 3) < 0)
    {
        perror("Listen failed");
        exit(1);
    }

    printf("Waiting for client...\n");

    
    new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
    if (new_socket < 0)
    {
        perror("Accept failed");
        exit(1);
    }

    printf("Client Connected.\n");


    while (1)
    {
        memset(buffer, 0, BUFFER_SIZE);

        recv(new_socket, buffer, BUFFER_SIZE, 0);
        printf("Client: %s\n", buffer);

        if (strcmp(buffer, "bye") == 0)
        {
            printf("Client disconnected.\n");
            break;
        }

        printf("Server: ");
        fgets(buffer, BUFFER_SIZE, stdin);
        buffer[strcspn(buffer, "\n")] = '\0';

        send(new_socket, buffer, strlen(buffer) + 1, 0);

        if (strcmp(buffer, "bye") == 0)
            break;
    }

    close(new_socket);
    close(server_fd);

    return 0;
}
