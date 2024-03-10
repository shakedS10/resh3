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
    // gettings all the parameters from the terminal

    struct sockaddr_in sender;
    // creating the socket to be ipv4 and tcp
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
    
    } //checking if the address is valid

    printf("Address set\n");
    if (connect(sock, (struct sockaddr *)&sender, sizeof(sender)) < 0)
    {
        printf("Connection Failed\n");
        close(sock);
        return 1;
    }// connecting to the reciver
    printf("Connected\n");
    if(strcmp(algo, "reno")) //setting the congestion control algorithm
    {
        if (setsockopt(sock, IPPROTO_TCP, TCP_CONGESTION, "reno", strlen("reno")) < 0)
        {
            printf("Invalid algorithm\n");
            close(sock);
            return 1;
        }
        
    }
    else
    {
        if (setsockopt(sock, IPPROTO_TCP, TCP_CONGESTION, "cubic", strlen("cubic")) < 0) //setting the congestion control algorithm
        {
            printf("Invalid algorithm\n");
            close(sock);
            return 1;
        }
    }
    char *fdata = util_generate_random_data(BUFFER_SIZE);
    FILE *file = fopen("a.txt", "w");
    //creating a file to store the random data

    if (file == NULL) {
        perror("Error opening file");
        return 1; // Return an error code
    }
    fwrite(fdata, sizeof(char), BUFFER_SIZE, file); //writing the data to the file
    fclose(file);
    FILE *fr = fopen("a.txt", "r"); //opening the file to read the data
    if (fr == NULL) {
        perror("Error opening file");
        return 1; // Return an error code
    }
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET); //getting the size of the file altough we know it already it is a good practice to get it from the file
    printf("File size: %ld\n", fileSize);
    char *data = (char *)malloc(fileSize + 1); //allocating memory for the data
     if (data == NULL) {
        perror("Error allocating memory");
        fclose(file);
        return 1;
    }
    size_t bytesRead = fread(data, sizeof(char), fileSize, file);  //reading the data from the file
    data[bytesRead] = '\0';
    fclose(file);
    int check = 1;
    int sent = 0;
    int counter = 0;
    int i = 0;
    while (check == 1) //a loop to send the data until the input to stop is given
    {
        
        counter = MAX_SIZE;
        i = 1;
        //checking if the data is divisible by the buffer size if not then we need to send the remaining data and fill the rest of the buffer with blanks
        if(fileSize % counter != 0) 
        {
            counter = fileSize + MAX_SIZE;
        }
        {
            i = 0;
        }
        //sending the data in chunks of the buffer size until the whole data is sent
        while (i <= fileSize / counter)
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
        printf("%d\n", sent);
        printf("Do you want to send more data? (y/n)\n");
        scanf(" %c", &act); //asking the user if they want to send the data again
        if (act == 'n')
        {
            check = 0;
        }
        else if (act == 'y')
        {
            check = 1;
        }
       
    }
    free(data); //freeing the memory
    printf("Data sent\n");
    printf("Sent: %d\n", sent);
    close(sock); //closing the socket
    return 0;
    
    
    


  
    
}

