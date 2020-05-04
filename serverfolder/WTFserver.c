#include <stdio.h> 
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <dirent.h>
#include <limits.h>
#include <sys/stat.h>
#include <pthread.h>

#define true 1
#define false 0

int string_equal(char *arg1, char *arg2)
{
    int a = 0;
    //We loop until both equal the null terminator
    //The two strings will differ either in character or length before
    //we hit an out of bounds
    while (arg1[a] != '\0' || arg2[a] != '\0')
    {
        if (arg1[a] != arg2[a]){
            return false;
        }
        a++;
    }
    return true;
}
  
void create(int connfd) { 

    char folder[100];
    char c = ' ';
    int i = 0;
    int read_status;

    do {
        read_status = read(connfd, &c, 1);
        if(read_status == 0){ // reached the end of the file
            break;
        }
        folder[i] = c;
        i++;
    
    }while(read_status > 0);
    folder[i] = '\0';

    DIR* directory = opendir("./");
    struct dirent* currentElement = NULL;
    currentElement = readdir(directory);
    while(currentElement != NULL){
         if(currentElement->d_type == 4){
             if(string_equal(currentElement->d_name,folder) == 0){
                    printf("Directory already exists.\n");
                    write(connfd,"e",1);
                    return -1;
             }
         } 
    }
    write(connfd, "m",1); // letting the client know that we made the folder 
    mkdir(folder, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH); // create our own project on the client side
    chdir(folder);
    int manifestfd = open("./.Manifest", O_CREAT | O_TRUNC | O_RDWR, S_IRWXU);
    write(manifestfd,"0\n",2);
    close(manifestfd);
    closedir(directory);

} 
void switcher(void* connfd_in_voidptr){
    int connfd = *((int*)connfd_in_voidptr);
    char c;
    read(connfd,&c,1);
    if(c == 'c'){ //create
        create(connfd);
    } else if (c == 'o'){

    }
}

int main(int argc, char **argv){

    int port = atoi(argv[1]);

    int sockfd; 
    struct sockaddr_in servaddr;
  
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
  
    // Binding ip and verification
    if ((bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) != 0) { 
        printf("Socket bind failed.\n"); 
        exit(0); 
    } 
    else{
        printf("Socket successfully binded..\n");
    } 
  
    // Now listen and verification 
    if ((listen(sockfd, 5)) != 0) { 
        printf("Listen failed...\n"); 
        return -1;
    } 
    else{
        printf("Server listening..\n"); 
    }
    
  // ------------------------------ multithreading ---------
    while(1){
        struct sockaddr_in cli; 
        int len = sizeof(cli); 
        int connfd = accept(sockfd, (struct sockaddr *)&cli, (socklen_t*) &len); 
        if (connfd < 0) { 
            printf("server acccept failed...\n"); 
            return -1;
        } 
        else{
            printf("server acccept the client...\n");
        } 
        struct sockaddr_in* client = (struct sockaddr_in*) &cli;
        struct in_addr ip_address = client->sin_addr;

        char client_ip_address[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &ip_address, client_ip_address, INET_ADDRSTRLEN);

        pthread_t my_thread;

        void* argument = &connfd;
        pthread_create(&my_thread, NULL, &switcher, argument);
        pthread_join(my_thread, NULL);

        close(connfd);
    }
    // -------------------------------------------------
  
    close(sockfd); 
    return 0;
} 