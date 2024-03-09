#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h> // For the in_addr structure and the inet_pton function
#include <sys/socket.h> // For the socket function
#include <unistd.h> // For the close function
#include <netinet/tcp.h> // For the TCP_CONGESTION option
#include <time.h>
#include "MAXSIZES.h"

char *util_generate_random_data(unsigned int size) {
 char *buffer = NULL;
 // Argument check.
 if (size == 0)
 return NULL;
 buffer = (char *)calloc(size, sizeof(char));
 // Error checking.
 if (buffer == NULL)
 return NULL;
 // Randomize the seed of the random number generator.
 srand(time(NULL));
 for (unsigned int i = 0; i < size; i++)
 *(buffer + i) = ((unsigned int)rand() % 256);
 return buffer;
}

int main(int argc, char *argv[])
{
    int port = atoi(argv[2]);
    char* ip = argv[1];
    char* algo = argv[3];
    printf("Port: %d\n", port);
    printf("IP: %s\n", ip);
    printf("Algo: %s\n", algo);
    

    struct sockaddr_in sender;

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        printf("Could not create socket\n");
        return 1;
    }
    printf("Socket created\n");

    sender.sin_family = AF_INET;
    sender.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &sender.sin_addr) <= 0)
    {
        printf("Invalid address/ Address not supported\n");
        close(sock);
        return 1;
    
    }
    printf("Address set\n");
    if (connect(sock, (struct sockaddr *)&sender, sizeof(sender)) < 0)
    {
        printf("Connection Failed\n");
        close(sock);
        return 1;
    }
    printf("Connected\n");
    if(strcmp(algo, "reno"))
    {
        if (setsockopt(sock, IPPROTO_TCP, TCP_CONGESTION, "reno", strlen("reno")) < 0)
        {
            printf("Error setting TCP congestion control algorithm\n");
            close(sock);
            return 1;
        }
        
    }
    else
    {
        if (setsockopt(sock, IPPROTO_TCP, TCP_CONGESTION, "cubic", strlen("cubic")) < 0)
        {
            printf("Error setting TCP congestion control algorithm\n");
            close(sock);
            return 1;
        }
    }
    char *fdata = util_generate_random_data(BUFFER_SIZE);
    FILE *file = fopen("a.txt", "w");

    if (file == NULL) {
        perror("Error opening file");
        return 1; // Return an error code
    }

    // Write the buffer data to the file
    fwrite(fdata, sizeof(char), BUFFER_SIZE, file);

    // Close the file
    fclose(file);
    FILE *fr = fopen("a.txt", "r");
    if (fr == NULL) {
        perror("Error opening file");
        return 1; // Return an error code
    }
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);
    printf("File size: %ld\n", fileSize);
    char *data = (char *)malloc(fileSize + 1);
     if (data == NULL) {
        perror("Error allocating memory");
        fclose(file);
        return 1;
    }
    size_t bytesRead = fread(data, sizeof(char), fileSize, file);
    data[bytesRead] = '\0';
    fclose(file);
    int check = 1;
    int sent = 0;
    int counter = 0;
    int i = 0;
    while (check == 1)
    {
        sent = 0;
        counter = MAX_SIZE;
        i = 1;
        if(fileSize % counter != 0)
        {
            i = 0;
        }
        while ((i <= fileSize / counter))
        {
            
            int isent = send(sock, data, counter, 0);
            printf("Sending data %d\n", isent);
            if (isent < 0)
            {
                printf("Send failed\n");
                close(sock);
                return 1;
            }
            sent +=isent;
            
            i++;
        }    
        char act; 
        printf("Do you want to send more data? (y/n)\n");
        scanf(" %c", &act);
        if (act == 'n')
        {
            check = 0;
        }
        else if (act == 'y')
        {
            check = 1;
        }
       
    }
    printf("Data sent\n");
    printf("Sent: %d\n", sent);
    close(sock);
    return 0;
    
    
    


  
    
}

