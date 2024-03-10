#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/tcp.h>
#include <time.h>
#include "MAXSIZES.h"

void print_stats(clock_t start, clock_t end, int totalReceived) {
    double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC; // in seconds
    double bandwidth = (totalReceived / 1024.0) / time_taken; // in KB/s
    printf("Time taken: %.2f seconds\n", time_taken);
    printf("Average Bandwidth: %.2f KB/s\n", bandwidth);
}
int main(int argc, char *argv[]) {
    

    int port = atoi(argv[1]);
    char *algo = argv[2];
    // gettings all the parameters from the terminal

    printf("Port: %d\n", port);
    printf("Algorithm: %s\n", algo);

    struct sockaddr_in receiver, sender;
    int server_socket, client_socket;
    // creating the socket to be ipv4 and tcp
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation failed");
        return 1;
    }
    receiver.sin_family = AF_INET;
    receiver.sin_addr.s_addr = INADDR_ANY;
    receiver.sin_port = htons(port);
    // setting the address to be ipv4 and the port to be the one given in the terminal
    if (bind(server_socket, (struct sockaddr *)&receiver, sizeof(receiver)) < 0) {
        perror("Bind failed");
        close(server_socket);
        return 1;
    }
    // binding the socket to the address
    if (listen(server_socket, 3) < 0) {
        perror("Listen failed");
        close(server_socket);
        return 1;
    }
    // starting to listen for connections
    printf("Server started listening\n");
    socklen_t addrlen = sizeof(sender);
    if ((client_socket = accept(server_socket, (struct sockaddr *)&sender, &addrlen)) < 0) {
        perror("Accept failed");
        close(server_socket);
        return 1;
    }
    // accepting the connection print the address of the sender
    printf("Connection made with %s:%d\n", inet_ntoa(sender.sin_addr), ntohs(sender.sin_port));
    // setting the congestion control algorithm
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
    clock_t start = clock(); // start the clock
    int totalReceived = 0;
    int counter = 1;
    char filename[40]; // to store the name of the file
    sprintf(filename, "received%d.txt", counter); // setting the name of the file
    FILE *file = fopen(filename, "w"); // creating the file to store the data
    if (file == NULL) {
        perror("Error opening file");
        close(client_socket);
        close(server_socket);
        return 1;
    }
    char buffer[MAX_SIZE]; // buffer to store the data of each send
    int bytesRead;
    int onerecv = 0;
    int currentsize = BUFFER_SIZE;
    if(BUFFER_SIZE % MAX_SIZE != 0)
    {
        currentsize = BUFFER_SIZE+MAX_SIZE;
    } // setting the size of the file to be the same as the sender incase of extra data extend it by the size of the buffer to fill with blank data
    // receiving the data and writing it to the file until the data is finished
    while ((bytesRead = recv(client_socket, buffer, sizeof(buffer), 0)) > 0) {
        if (onerecv > currentsize - MAX_SIZE)
        {         
            counter++;
            sprintf(filename, "received%d.txt", counter); // setting the name of the file
            fclose(file); // closing the file
            onerecv = 0; // reseting the current size of the file to be 0 because its a new file
        }
        if (onerecv == 0 && bytesRead>0)
        {
            
            file = fopen(filename, "w"); // creating the new file to store the data
            if (file == NULL) {
                perror("Error opening file");
                close(client_socket);
                close(server_socket);
                return 1;
            }
        }
        fwrite(buffer, sizeof(char), bytesRead, file); //writing the data to the file
        totalReceived += bytesRead; // adding the size of the data to the total size of the data received
        onerecv += bytesRead; // adding the size of the data to the current size of the file
        printf("Received %d bytes\n", onerecv);
        
    }
    
        

    if (bytesRead < 0) {
        perror("Receive failed");
    } // if the data is not received print an error
    printf("Data received and written\n"); // print that the data is received and written
    printf("Total received: %d bytes\n", totalReceived); // print the total size of the data received
    fclose(file); // close the last file

    clock_t end = clock(); //end timer 
    print_stats(start, end, totalReceived); // print the statistics of the data received
    close(client_socket);
    close(server_socket);
    // close the sockets
    return 0;
}

