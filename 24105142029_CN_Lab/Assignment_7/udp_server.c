
// Roll: 24105142029 Service Type: C d = 2
// Assignment 7: UDP Sockets
// Service: String Statistics
// Port: 30029

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

#define PORT 30029
#define ROLL "24105142029"
#define BUFFER_SIZE 4096

#ifdef _WIN32
#define CLOSE_SOCKET closesocket
#else
#define CLOSE_SOCKET close
#endif

int main()
{
    int server_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    char buffer[BUFFER_SIZE];
    char response[BUFFER_SIZE];
    char *text;
    int i, len, vowels, consonants, digits;

#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        printf("WSAStartup failed.\n");
        return 1;
    }
#endif

    // Create UDP socket
    server_socket = socket(AF_INET, SOCK_DGRAM, 0);

    if (server_socket < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    // Configure server address
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind socket to port
    if (bind(server_socket, (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0)
    {
        perror("Bind failed");
        CLOSE_SOCKET(server_socket);
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }

    printf("Server running on port %d\n", PORT);

    // Continuously receive client requests
    while (1)
    {
        client_len = sizeof(client_addr);

        int received = recvfrom(server_socket, buffer,
                                BUFFER_SIZE - 1, 0,
                                (struct sockaddr *)&client_addr,
                                &client_len);

        if (received < 0)
        {
            perror("Receive failed");
            continue;
        }

        buffer[received] = '\0';

        printf("\nClient IP: %s\n",
               inet_ntoa(client_addr.sin_addr));

        printf("Client Port: %d\n",
               ntohs(client_addr.sin_port));

        // Check roll number
        char prefix[50];
        sprintf(prefix, "%s#", ROLL);

        if (strncmp(buffer, prefix, strlen(prefix)) != 0)
        {
            strcpy(response, "ERROR: Unauthorized client");
        }
        else
        {
            // Remove roll number prefix
            text = buffer + strlen(prefix);

            printf("Received message: %s\n", text);

            // Exit command
            if (strcmp(text, "EXIT") == 0)
            {
                strcpy(response, "Server shutting down");

                printf("Sent response: %s\n", response);

                sendto(server_socket, response,
                       strlen(response), 0,
                       (struct sockaddr *)&client_addr,
                       client_len);

                break;
            }

            // Calculate statistics
            len = strlen(text);
            vowels = 0;
            consonants = 0;
            digits = 0;

            for (i = 0; text[i] != '\0'; i++)
            {
                char ch = (char)tolower((unsigned char)text[i]);

                if (isalpha((unsigned char)text[i]))
                {
                    if (ch == 'a' || ch == 'e' ||
                        ch == 'i' || ch == 'o' ||
                        ch == 'u')
                    {
                        vowels++;
                    }
                    else
                    {
                        consonants++;
                    }
                }
                else if (isdigit((unsigned char)text[i]))
                {
                    digits++;
                }
            }

            // Prepare response
            sprintf(response,
                    "LEN=%d;VOWELS=%d;CONSONANTS=%d;DIGITS=%d",
                    len, vowels, consonants, digits);
        }

        printf("Sent response: %s\n", response);

        // Send response to client
        sendto(server_socket, response,
               strlen(response), 0,
               (struct sockaddr *)&client_addr,
               client_len);
    }

    // Close socket
    CLOSE_SOCKET(server_socket);

#ifdef _WIN32
    WSACleanup();
#endif

    printf("\nServer closed successfully.\n");

    return 0;
}
