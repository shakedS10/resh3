#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/tcp.h>
#include <time.h>
#include <sys/time.h>
#include "MAXSIZES.h"

struct timeval start, end;
void print_stats(struct timeval start, struct timeval end, int totalReceived) {
    double time_taken = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;
    double bandwidth = (totalReceived / 1024.0) / time_taken; 
    static double avgBandwidth = 0;
    avgBandwidth += bandwidth;
    static double avgTime = 0;
    avgTime += time_taken;
    static int counter = 0;
    printf("Time taken: %.2f seconds\n", time_taken);
    printf("Average Bandwidth: %.2f KB/s\n", bandwidth);
    printf("--------------------------------\n");
    printf("Average Time: %.2f seconds\n", avgTime / ++counter);
    printf("Average Bandwidth: %.2f KB/s\n", avgBandwidth / counter);
}

int main(int argc, char *argv[]) {
    
    int port = atoi(argv[1]);
    char *algo = argv[2];

    printf("Port: %d\n", port);
    printf("Algorithm: %s\n", algo);

    struct sockaddr_in receiver, sender;
    int server_socket, client_socket;
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation failed");
        return 1;
    }
    receiver.sin_family = AF_INET;
    receiver.sin_addr.s_addr = INADDR_ANY;
    receiver.sin_port = htons(port);
    if (bind(server_socket, (struct sockaddr *)&receiver, sizeof(receiver)) < 0) {
        perror("Bind failed");
        close(server_socket);
        return 1;
    }
    if (listen(server_socket, 3) < 0) {
        perror("Listen failed");
        close(server_socket);
        return 1;
    }
    printf("Server started listening\n");
    socklen_t addrlen = sizeof(sender);
    if ((client_socket = accept(server_socket, (struct sockaddr *)&sender, &addrlen)) < 0) {
        perror("Accept failed");
        close(server_socket);
        return 1;
    }

    printf("Connection made with %s:%d\n", inet_ntoa(sender.sin_addr), ntohs(sender.sin_port));

    if (strcmp(algo, "reno") == 0) {
        if (setsockopt(client_socket, IPPROTO_TCP, TCP_CONGESTION, "reno", strlen("reno")) < 0) {
            printf("Invalid algorithm\n");
            close(client_socket);
            close(server_socket);
            return 1;
        }
    } else if (strcmp(algo, "cubic") == 0) {
        if (setsockopt(client_socket, IPPROTO_TCP, TCP_CONGESTION, "cubic", strlen("cubic")) < 0) {
            printf("Invalid algorithm\n");
            close(client_socket);
            close(server_socket);
            return 1;
        }
    } else {
        printf("Invalid algorithm\n");
        close(client_socket);
        close(server_socket);
        return 1;
    }
    //clock_t start = clock();
    int totalReceived = 0;
    int counter = 1;
    char filename[40];
    sprintf(filename, "received%d.txt", counter);
    FILE *file = fopen(filename, "w");

    
        

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
    gettimeofday(&start, NULL);
    while ((bytesRead = recv(client_socket, buffer, sizeof(buffer), 0)) > 0) {
        if (onerecv > currentsize - MAX_SIZE)
        {         
            counter++;
            sprintf(filename, "received%d.txt", counter);
            fclose(file);
            onerecv = 0;
            
        }
        if (onerecv == 0 && bytesRead>0)
        {
            
            file = fopen(filename, "w");
            if (file == NULL) {
                perror("Error opening file");
                close(client_socket);
                close(server_socket);
                return 1;
            }
        }
        fwrite(buffer, sizeof(char), bytesRead, file);
        totalReceived += bytesRead;
        onerecv += bytesRead;
        printf("Received %d bytes\n", onerecv);
        
    }
    
        

    if (bytesRead < 0) {
        perror("Receive failed");
    }
    printf("Data received and written\n");

    printf("Total received: %d bytes\n", totalReceived);
    fclose(file);

    gettimeofday(&end, NULL);
    print_stats(start, end, totalReceived);
    close(client_socket);
    close(server_socket);

    return 0;
}

