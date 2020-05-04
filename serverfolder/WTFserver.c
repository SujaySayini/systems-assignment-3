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
#include <errno.h>

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

int r_file(char * path,char** buff){
    struct stat stats;
    int read_status = 0;
    stat(path,&stats);
    int bytesReadSoFar = 0, numOfBytes = stats.st_size;
    *buff = (char*)malloc(sizeof(char*) * stats.st_size);
    int fileD = open(path, O_RDONLY); 
    do
    {
        read_status = read(fileD, *(buff + bytesReadSoFar), numOfBytes - bytesReadSoFar);
        bytesReadSoFar += read_status;

    } while (read_status > 0 && bytesReadSoFar < numOfBytes);  
    if(read_status > 0)
        return stats.st_size;
    return read_status;

}

void upload(int connfd,char** arguments){
    //arg[0] = filepath
    //arg[1] = file content
    int depth = 0;
    int i = 0;
    int tracker = 0;
    char holder[PATH_MAX];
    printf("%s arg\n",arguments[0]);
    while(i < strlen(arguments[0])){
        printf("llooop\n");
        if(arguments[0][i] == '/'){
            holder[tracker] = '\0';
            printf("status %d\n",mkdir(holder,0777));
            depth++;
            chdir(holder);
            tracker = 0;
            printf("%s %d\n",holder,depth);
        }
        else{
            holder[tracker] = arguments[0][i];
            tracker++;
        }
        i++;
    }
    holder[tracker] = '\0';
    char final[100] = "./";
    strcat(final,holder);
    int fd = open(final, O_RDWR | O_TRUNC | O_CREAT, 0644);
    printf("%s fd\n",arguments[1]);
    perror("");
    write(fd,arguments[1],strlen(arguments[1])); 
    int a;
    for(a = 0;a<depth;a++)
        chdir("..");
    
}

void create(int connfd,char ** arguments) { 
    char* folder;
    char c = ' ';
    int i = 0;
    int read_status;
    folder = arguments[0];
    printf("%s\n",folder);
    DIR* directory = opendir("./");
    
    struct dirent* currentElement = NULL;
    currentElement = readdir(directory);
    while(currentElement != NULL){
         if(currentElement->d_type == 4){
             printf("%s dir \n",currentElement->d_name);
             if(string_equal(currentElement->d_name,folder) == 1){
                    printf("Directory already exists.\n");
                    write(connfd,"e",1);
                    return;
             }
         } 
         currentElement = readdir(directory);
    }
    write(connfd, "m",1); // letting the client know that we made the folder 
    mkdir(folder, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH); // create our own project on the client side
    chdir(folder);
    int manifestfd = open("./.Manifest", O_CREAT | O_TRUNC | O_RDWR, S_IRWXU);
    chdir("..");
    write(manifestfd,"0\n",2);
    close(manifestfd);
    closedir(directory);

} 

void send_files(int connfd, char* folder_name){
    DIR* directory = opendir(folder_name);
    struct dirent* currentElement = NULL;
    readdir(directory);
    readdir(directory);
    currentElement = readdir(directory);
    while(currentElement != NULL){
        if(currentElement->d_type == 4){ //if its a directory


            char file[strlen(folder_name)+1+strlen(currentElement->d_name)];
            bzero(file, strlen(file));
            strcat(file, file);
            strcat(file, "/");
            strcat(file, currentElement->d_name);
            send_files(connfd, currentElement->d_name);
        }
        if(currentElement->d_type == 8){ //if its a file 

        }
    }

}

void checkout(int connfd){
    char folder[100];



    DIR* directory = opendir("./");
    struct dirent* currentElement = NULL;
    currentElement = readdir(directory);
    while(currentElement != NULL){
         if(currentElement->d_type == 4){ //if its a directory
            if(string_equal(currentElement->d_name,folder) == 0){
                //found the directory
                write(connfd,"m",1); //m for made, letting client know that we found directory on server side 
                
                char file[2+strlen(currentElement->d_name)];
                bzero(file, strlen(file));
                file[0] = '.';
                file[1] = '/';
                strcat(file, currentElement->d_name);
                send_files(connfd, file);
                
                write(connfd,":",1); // finished going through every file in the directory
            }
         } 
         currentElement = readdir(directory);
    }

}

void rollback(int connfd){
    char folder[100];

    DIR* directory = opendir("./");
    struct dirent* currentElement = NULL;
    currentElement = readdir(directory);
    while(currentElement != NULL){
        if(currentElement->d_type == 4){
            if(string_equal(currentElement->d_name,folder) == 0){
                chdir(folder);
                int manifestfd = open("./.Manifest", O_RDWR, S_IRWXU);
                // get all files that are in this manifest how idk?
                // Only revert the files on the server
            }
        } 
        currentElement = readdir(directory);
    }

    printf("Directory doesn't exists.\n");
    write(connfd,"e",1);
    return -1;

}

void delete_files(char* folder_name){
    DIR* directory = opendir(folder_name);
    struct dirent* currentElement = NULL;
    readdir(directory);
    readdir(directory);
    currentElement = readdir(directory);
    while(currentElement != NULL){
        if(currentElement->d_type == 4){ //if its a directory
            
            char file[strlen(folder_name)+1+strlen(currentElement->d_name)];
            bzero(file, strlen(file));
            strcat(file, folder_name);
            if(!(string_equal("./",folder_name))){
                strcat(file, "/");
            }
            strcat(file, currentElement->d_name);
            delete_files(currentElement->d_name);
            rmdir(currentElement->d_name);
        }
        if(currentElement->d_type == 8){ //if its a file 
            remove(currentElement->d_name);
        }
    }

}

void destroy (int connfd){
    char folder[100];

    DIR* directory = opendir("./");
    struct dirent* currentElement = NULL;
    currentElement = readdir(directory);
    while(currentElement != NULL){
         if(currentElement->d_type == 4){
             if(string_equal(currentElement->d_name,folder) == 0){
                   //found the directory
                   // need to lock directory, expire any commits, delete files

                   delete_files("./");
                   write(connfd,"s",1); // return success that we finished everything above
             }
         } 
         currentElement = readdir(directory);
    }

    printf("Directory doesn't exists.\n");
    write(connfd,"e",1);
    return -1;

}

void switcher(void* connfd_in_voidptr){
    
    int connfd = *((int*)connfd_in_voidptr);
    char** arguments;
    char command = ' ';
    char args[50];
    char c = ' ';
    int i = 0;
    int a = 0;
    int tracker = 0;
    int read_status;
    read(connfd,&c,1);
    command = c;
    while(true){
        read(connfd,&c,1);
        if(c == ':') break; 
        args[i] = c;
        i++;
    }
    args[i] = '\0';
    i = 0;
    arguments = (char**)malloc(sizeof(char*) * atoi(args));
    char length[50];
    int found_length = 0;
    for(a;a < atoi(args);a++){
        while(true){
            read(connfd,&c,1);
            if(c == ':'){
                length[i] = '\0';
                i = 0;
                break;
            }; 
            length[i] = c;
            i++;
        }
        int b = 0;
        arguments[a] = (char*)malloc(sizeof(char) * atoi(length));
        while(true){
            read(connfd,&c,1);
            if(c == ':' || c == ';'){
                arguments[a][b] = '\0';
                b = 0;
                break;
            }
            arguments[a][b] = c;
            b++;
        }

    }
    if(command == 'c'){ //create
        create(connfd,arguments);
<<<<<<< HEAD
    } else if (command == 'u'){
        upload(connfd,arguments);
=======
    } else if (command == 'o'){ //checkout

    } else if(command == 'r'){ // rollback
        rollback(connfd);
    } else if(command == 'd'){ // destroy 
        destroy(connfd);
    }else if(command == ' '){
    
>>>>>>> d26cc5fe1811fd0592bfb61c72ed5154ef7af3b3
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
