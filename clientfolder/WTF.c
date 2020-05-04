#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <math.h>
#include <limits.h>
#include <libgen.h>
#include <dirent.h>
#include <netdb.h> 

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
void configure(char* IP_address, char* port){
    int fd = open(".configure", O_RDWR | O_CREAT | O_TRUNC, 00744);
    //strcat(IP_address, " ");
    write(fd,IP_address, strlen(IP_address));
    write(fd," ", 1);
    write(fd,port, strlen(port));
    write(fd,"\n",1);
    close(fd);
}

void checkout(int sockfd, char* project_name){
    char message[1+strlen(project_name)];
    message[0] = 'o'; // telling checkout here

    strcat(message,project_name);
    write(sockfd,message,strlen(message));

    char c;
    read(sockfd,&c,1);
    if(c == 'e'){ // e for error 
        printf("Error, project doesn't exists on the server");
    } else if (c == 'm'){ // m for made 
        
    
    }


}

void create(int sockfd, char* project_name){
    char message[1+strlen(project_name)];
    message[0] = 'c'; //telling it here that we are creating our project
    strcat(message,project_name);
    write(sockfd,message,strlen(message));

    char c;
    read(sockfd,&c,1);
    if(c == 'e'){ // e for error 
        printf("Error, project already exists on the server");
    } else if (c == 'm'){ // made 
        DIR* directory = opendir("./");
        mkdir(project_name, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH); // create our own project on the client side
        chdir(project_name);
        int manifestfd = open("./.Manifest", O_CREAT | O_TRUNC | O_RDWR, S_IRWXU);
        write(manifestfd,"0\n",2);
        close(manifestfd);
        closedir(directory);
    }
}
void add(int sockfd, char* project_name, char* file_name){


}
void removefd(int sockfd, char* project_name, char* file_name){


}
void update(int sockfd, char* project_name){

}
void upgrade(int sockfd, char* project_name){

}
void commit(int sockfd, char* project_name){

}
void push(int sockfd, char* project_name){

}
void destroy(int sockfd, char* project_name){

}
void current_version(int sockfd, char* project_name){

}
void history(int sockfd, char* project_name){

}

void rollback(int sockfd, char* project_name, int version){

}

int main(int argc, char **argv){
    // client side 

    if(string_equal(argv[1], "configure")){
        configure(argv[2],argv[3]);
        return 0;
    }
    //checking if configure exists
    int fd = open("./.configure", O_RDWR);
    if(fd < 0){
        printf("Configure doesn't exist, run configure command before any other command.\n");
        return -1;  
    }
    
    char ip[20];
    char port[6];

    int read_status = 0;
    char c = ' ';
    int i = 0;
    char buffer[100];    
    
    do {
        printf("buffer is = %s\n", buffer);
        read_status = read(fd, &c, 1);
        if(read_status == 0){ // reached the end of the file
            break;
        }

        if(c == ' '){
            buffer[i] = '\0';
            strcpy(ip,buffer);
            memset(buffer, '\0', 100);
            i = 0;
        }
        else {
            buffer[i] = c;
            i++;
        }

    }while(read_status > 0);

    buffer[i] = '\0';
    strcpy(port,buffer);

    int port_in_int = atoi(port);
    if(port_in_int < 0){
        printf("Invalid port number, please try again.\n");
        return -1;
    }

    //printf("ip is %s and port is %d\n", ip, port_in_int);
    
    // Now creating our connection to the server
    int sockfd; 
    struct sockaddr_in servaddr; 
  
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd == -1) { 
        printf("Creating Socket failed\n"); 
        return -1;
    } else{
        printf("Socket successfully created.\n"); 
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = inet_addr(ip); 
    servaddr.sin_port = htons(port_in_int); 

    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0) { 
        printf("Connection with the server failed.\n"); 
        return -1;
    } 
    else{
        printf("Connected to the server.\n"); 
    }

    if (string_equal(argv[1], "checkout")){ 
        DIR* directory = opendir("./");
        struct dirent* currentElement = NULL;
        currentElement = readdir(directory);
        while(currentElement != NULL){
            if(currentElement->d_type == 4){
                if(string_equal(currentElement->d_name,argv[2]) == 0){
                    printf("Directory already exists on the client side.\n");
                    return -1;
                }
            } 
        currentElement = readdir(directory);
    }
        checkout(sockfd, argv[2]);

    } else if (string_equal(argv[1], "update")){
        update(sockfd, argv[2]);
    } else if (string_equal(argv[1], "upgrade")){
        upgrade(sockfd, argv[2]);
    } else if (string_equal(argv[1], "commit")){
        commit(sockfd, argv[2]);
    } else if (string_equal(argv[1], "push")){
        push(sockfd, argv[2]);
    } else if (string_equal(argv[1], "create")){
        create(sockfd,argv[2]);

    } else if (string_equal(argv[1], "destroy")){
        destroy(sockfd, argv[2]);

    } else if (string_equal(argv[1], "add")){
        add(sockfd, argv[2], argv[3]);

    } else if (string_equal(argv[1], "remove")){
        removefd(sockfd, argv[2], argv[3]);

    } else if (string_equal(argv[1], "currentversion")){
        current_version(sockfd, argv[2]);

    } else if (string_equal(argv[1], "history")){
        history(sockfd, argv[2]);

    } else if (string_equal(argv[1], "rollback")){
        int version = atoi(argv[3]);
        if(version == 0){
            printf("Invalid number, please try again.\n");
            return -1;
        }
        rollback(sockfd, argv[2], version);

    } else {
        printf("Invalid command, please try again.\n");
        return -1;
    }

    close(sockfd);
    return 0;
}