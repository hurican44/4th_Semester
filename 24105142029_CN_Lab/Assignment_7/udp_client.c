
// Roll: 24105142029
// Assignment 7: UDP Sockets - Client

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
typedef SOCKET socket_t;
typedef int socklen_t;
#define CLOSE_SOCKET closesocket
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
typedef int socket_t;
#define CLOSE_SOCKET close
#endif

#define PORT 30029
#define BUFFER_SIZE 4096

int main(int argc, char *argv[])
{
    socket_t client_socket;
    struct sockaddr_in server_addr;
    socklen_t server_len;
    char input[BUFFER_SIZE];
    char message[BUFFER_SIZE];
    char response[BUFFER_SIZE];

    // Check command-line arguments
    if (argc != 3)
    {
        printf("Usage: %s <server_ip> <roll>\n", argv[0]);
        return 1;
    }

#ifdef _WIN32
    WSADATA wsa;

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        printf("WSAStartup failed.\n");
        return 1;
    }
#endif

    // Create UDP socket
    client_socket = socket(AF_INET, SOCK_DGRAM, 0);

#ifdef _WIN32
    if (client_socket == INVALID_SOCKET)
#else
    if (client_socket < 0)
#endif
    {
        perror("Socket creation failed");
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }

    // Configure server address
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, argv[1],
                  &server_addr.sin_addr) != 1)
    {
        printf("Invalid server IP address.\n");
        CLOSE_SOCKET(client_socket);
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }

    server_len = sizeof(server_addr);

    printf("Connected to server at %s:%d\n",
           argv[1], PORT);

    while (1)
    {
        printf("\nEnter text (EXIT to quit): ");

        if (fgets(input, sizeof(input), stdin) == NULL)
        {
            strcpy(input, "EXIT");
        }

        // Remove newline
        input[strcspn(input, "\n")] = '\0';

        // Add roll number prefix
        snprintf(message, sizeof(message),
                 "%s#%s", argv[2], input);

        // Send request to server
        if (sendto(client_socket, message,
                   (int)strlen(message), 0,
                   (struct sockaddr *)&server_addr,
                   server_len) < 0)
        {
            perror("Send failed");
            break;
        }

        // Receive server response
        int received = recvfrom(client_socket,
                                response,
                                BUFFER_SIZE - 1,
                                0,
                                (struct sockaddr *)&server_addr,
                                &server_len);

        if (received < 0)
        {
            perror("Receive failed");
            break;
        }

        response[received] = '\0';

        printf("Server response: %s\n", response);

        // Exit after sending EXIT command
        if (strcmp(input, "EXIT") == 0)
        {
            break;
        }
    }

    // Close socket
    CLOSE_SOCKET(client_socket);

#ifdef _WIN32
    WSACleanup();
#endif

    printf("\nClient closed successfully.\n");

    return 0;
}
