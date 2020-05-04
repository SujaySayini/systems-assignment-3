#include <stdio.h> 
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#define MAX 80 
  
void readFile(char* f,char* buff){
    int fd = open(f,O_RDONLY);
    int size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    buff = (char*)malloc(sizeof(char) * size);
}
    

// Function designed for chat between client and server. 
void func(int sockfd) 
{ 
    char buff[MAX]; 
    int n; 
    // infinite loop for chat 
    for (;;) { 
        bzero(buff, MAX); 
  
        // read the message from client and copy it in buffer 
        read(sockfd, buff, sizeof(buff)); 
        // print buffer which contains the client contents 
        printf("From client: %s\t To client : ", buff); 
        bzero(buff, MAX); 
        n = 0; 
        // copy server message in the buffer 
        while ((buff[n++] = getchar()) != '\n') 
            ; 
  
        // and send that buffer to client 
        write(sockfd, buff, sizeof(buff)); 
  
        // if msg contains "Exit" then server exit and chat ended. 
        if (strncmp("exit", buff, 4) == 0) { 
            printf("Server Exit...\n"); 
            break; 
        } 
    } 
} 
  

int main(int argc, char **argv){

    int port = atoi(argv[1]);

    int sockfd, connfd, len; 
    struct sockaddr_in servaddr, cli; 
  
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd == -1) { 
        printf("Socket creation failed.\n"); 
        return 0;
    } else{
        printf("Socket successfully created.\n"); 
    }
    
    bzero(&servaddr, sizeof(servaddr)); 
  
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    servaddr.sin_port = htons(port); 
  
    // Binding newly created socket to given IP and verification 
    if ((bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) != 0) { 
        printf("Socket bind failed.\n"); 
        exit(0); 
    } 
    else{
        printf("Socket successfully binded..\n");
    } 
  
    // Now server is ready to listen and verification 
    if ((listen(sockfd, 5)) != 0) { 
        printf("Listen failed...\n"); 
        return -1;
    } 
    else{
        printf("Server listening..\n"); 
    }
    
    
    len = sizeof(cli); 
  // ------------------------------ multithreading ---------
    connfd = accept(sockfd, (struct sockaddr *)&cli, (socklen_t*) &len); 
    if (connfd < 0) { 
        printf("server acccept failed...\n"); 
        return -1;
    } 
    else{
        printf("server acccept the client...\n");
    } 
  
    // Function for chatting between client and server 
    func(connfd); 

    // -------------------------------------------------
  
    close(sockfd); 
    return 0;
} 
