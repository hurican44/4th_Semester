#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ctype.h>

#define PORT 8081
#define BUFFER_SIZE 1024

int count_words(char *str)
{
    int count = 0;
    char *token = strtok(str, " ");
    while (token != NULL)
    {
        count++;
        token = strtok(NULL, " ");
    }
    return count;
}

int count_vowels(char *str)
{
    int count = 0;
    for (int i = 0; str[i] != '\0'; i++)
    {
        char c = tolower(str[i]);
        if (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u')
        {
            count++;
        }
    }
    return count;
}

int main()
{
    int server_fd, new_socket;
    struct sockaddr_in serv_addr, address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE];

    // 1. Create socket file descriptor
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0)
    {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // 2. Bind socket to port
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // 3. Listen for connections
    if (listen(server_fd, 3) < 0)
    {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d...\n", PORT);

    while (1)
    {
        // 4. Accept incoming client connection
        new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        if (new_socket < 0)
        {
            perror("Accept failed");
            continue;
        }

        memset(buffer, 0, BUFFER_SIZE);
        int valread = read(new_socket, buffer, BUFFER_SIZE);
        if (valread <= 0)
        {
            close(new_socket);
            continue;
        }

        // Remove trailing newline or carriage return
        buffer[strcspn(buffer, "\r\n")] = 0;

        if (strcmp(buffer, "bye") == 0)
        {
            char *exit_msg = "Goodbye!";
            send(new_socket, exit_msg, strlen(exit_msg), 0);
            close(new_socket);
            break;
        }

        char command[20], text[BUFFER_SIZE];
        // Expected format from client: ANALYZE|Your text here
        if (sscanf(buffer, "%[^|]|%[^\n]", command, text) == 2)
        {
            if (strcmp(command, "ANALYZE") == 0)
            {
                int chars = strlen(text);
                int words = count_words(text);
                int vowels = count_vowels(text);

                char response[BUFFER_SIZE];
                snprintf(response, sizeof(response), "Chars=%d, Words=%d, Vowels=%d", chars, words, vowels);
                send(new_socket, response, strlen(response), 0);
            }
            else
            {
                char *msg = "Invalid Command";
                send(new_socket, msg, strlen(msg), 0);
            }
        }
        else
        {
            char *msg = "Invalid Format. Use: ANALYZE|Text";
            send(new_socket, msg, strlen(msg), 0);
        }

        close(new_socket);
    }

    close(server_fd);
    return 0;
}
