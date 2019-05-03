#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h>
#include <iostream>
#include <arpa/inet.h>
using namespace std;
#define PORT 8080 
   
int main(int argc, char const *argv[]) 
{ 
    int sock = 0, valread; 
    struct sockaddr_in serv_addr; 
    int buffer_size = 1024;
    char buffer[buffer_size] = {0};
    memset(&serv_addr, '0', sizeof(serv_addr)); 
       
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(PORT); 
       
    // Convert IPv4 and IPv6 addresses from text to binary form 
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)  { 
        printf("\nInvalid address/ Address not supported \n"); 
        return -1; 
    }


    while (true) {
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) { 
            printf("\n Socket creation error \n"); 
            return -1; 
        }
        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) { 
            printf("\nConnection Failed \n"); 
            return -1; 
        }

        system ("clear");
        while ((valread = read( sock , buffer, buffer_size)) > 0)
            printf("%s",buffer);

        sleep(1);
        close(sock);
    }
    return 0; 
} 