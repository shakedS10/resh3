#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/tcp.h>
#include <time.h>
#include "MAXSIZES.h"

void print_time_and_bandwidth(clock_t start, clock_t end, int totalReceived) {
    double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC; // in seconds
    double bandwidth = (totalReceived / 1024.0) / time_taken; // in KB/s
    printf("Time taken: %.2f seconds\n", time_taken);
    printf("Average Bandwidth: %.2f KB/s\n", bandwidth);
}
int main(int argc, char *argv[]) {
    

    int port = atoi(argv[1]);
    char *algo = argv[2];

    printf("Port: %d\n", port);
    printf("Algorithm: %s\n", algo);

    struct sockaddr_in receiver, sender;
    int server_socket, client_socket;

    // Create socket
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation failed");
        return 1;
    }

    // Setup server address structure
    receiver.sin_family = AF_INET;
    receiver.sin_addr.s_addr = INADDR_ANY;
    receiver.sin_port = htons(port);

    // Bind the socket to the specified port
    if (bind(server_socket, (struct sockaddr *)&receiver, sizeof(receiver)) < 0) {
        perror("Bind failed");
        close(server_socket);
        return 1;
    }
    
    // Listen for incoming connections
    if (listen(server_socket, 3) < 0) {
        perror("Listen failed");
        close(server_socket);
        return 1;
    }

    printf("Server listening on port %d...\n", port);

    // Accept an incoming connection
    socklen_t addrlen = sizeof(sender);
    if ((client_socket = accept(server_socket, (struct sockaddr *)&sender, &addrlen)) < 0) {
        perror("Accept failed");
        close(server_socket);
        return 1;
    }

    printf("Connection accepted from %s:%d\n", inet_ntoa(sender.sin_addr), ntohs(sender.sin_port));

    // Set TCP congestion control algorithm
    if (strcmp(algo, "reno") == 0) {
        if (setsockopt(client_socket, IPPROTO_TCP, TCP_CONGESTION, "reno", strlen("reno")) < 0) {
            perror("Error setting TCP congestion control algorithm");
            close(client_socket);
            close(server_socket);
            return 1;
        }
    } else if (strcmp(algo, "cubic") == 0) {
        if (setsockopt(client_socket, IPPROTO_TCP, TCP_CONGESTION, "cubic", strlen("cubic")) < 0) {
            perror("Error setting TCP congestion control algorithm");
            close(client_socket);
            close(server_socket);
            return 1;
        }
    } else {
        printf("Invalid congestion control algorithm\n");
        close(client_socket);
        close(server_socket);
        return 1;
    }
    clock_t start = clock();
    int totalReceived = 0;
    int counter = 1;
    char filename[100];
    sprintf(filename, "received_data%d.txt", counter);
    FILE *file = fopen(filename, "w");

    
        
    // Receive data and write to a file
        
    if (file == NULL) {
        perror("Error opening file");
        close(client_socket);
        close(server_socket);
        return 1;
    }

    char buffer[MAX_SIZE];
    
    int bytesRead;
    int onerecv = 0;
    int currentsize = BUFFER_SIZE;
    if(BUFFER_SIZE % MAX_SIZE != 0)
    {
        currentsize = BUFFER_SIZE+MAX_SIZE;
    }
    
    while ((bytesRead = recv(client_socket, buffer, sizeof(buffer), 0)) > 0) {
        if (onerecv == 0 && counter > 1 && bytesRead >0)
        {
            fclose(file);
            file = fopen(filename, "w");
            if (file == NULL) {
                perror("Error opening file");
                close(client_socket);
                close(server_socket);
                return 1;
            }
        }
        fwrite(buffer, sizeof(char), MAX_SIZE, file);
        totalReceived += bytesRead;
        onerecv += MAX_SIZE;
        printf("Received %d bytes\n", onerecv);
        if (onerecv >= currentsize)
        {         
            counter++;
            sprintf(filename, "received_data%d.txt", counter);
            onerecv = 0;
        }
        
    }
    
        

        if (bytesRead < 0) {
            perror("Receive failed");
        }
        printf("Data received and written to received_data.txt\n");
    
        printf("Total received: %d bytes\n", totalReceived);
        fclose(file);

    clock_t end = clock();
    print_time_and_bandwidth(start, end, totalReceived);
    

    // Close sockets and file
    
    close(client_socket);
    close(server_socket);

    return 0;
}

