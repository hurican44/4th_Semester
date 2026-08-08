
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8081
#define BUFFER_SIZE 1024

int main()
{
    int sock;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE];
    char input[BUFFER_SIZE];

    // 1. Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 address from text to binary
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
    {
        printf("\n Invalid address/ Address not supported \n");
        return -1;
    }

    // 2. Connect to server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\n Connection Failed \n");
        return -1;
    }

    printf("Connected to the server. Format: ANALYZE|Your text here\n");

    while (1)
    {
        printf("\nEnter command (or type 'bye' to exit): ");

        // Read input from standard input
        if (fgets(input, BUFFER_SIZE, stdin) == NULL)
        {
            break;
        }

        // Remove trailing newline character
        input[strcspn(input, "\r\n")] = 0;

        // Send input to server
        send(sock, input, strlen(input), 0);

        // Clear buffer before receiving
        memset(buffer, 0, BUFFER_SIZE);

        // Receive response from server
        int bytes = recv(sock, buffer, BUFFER_SIZE - 1, 0);
        if (bytes > 0)
        {
            buffer[bytes] = '\0';
            printf("Server response: %s\n", buffer);
        }
        else
        {
            printf("Server disconnected.\n");
            break;
        }

        // Exit loop if user typed "bye"
        if (strcmp(input, "bye") == 0)
        {
            printf("Connection closed.\n");
            break;
        }
    }

    close(sock);
    return 0;
}
