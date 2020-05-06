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
#include <linux/limits.h>
#include <sys/stat.h>
#include <pthread.h>
#include <errno.h>

#define true 1
#define false 0
//#define PATH_MAX 100 

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
    *buff = (char*)malloc(sizeof(char) * numOfBytes); //before sizeof(char*) ?
    bzero(*buff,numOfBytes);
    (*buff)[numOfBytes] = '\0';
    int fileD = open(path, O_RDONLY); 
    do
    {
        read_status = read(fileD, *(buff + bytesReadSoFar), numOfBytes - bytesReadSoFar);
        bytesReadSoFar += read_status;

    } while (read_status > 0 && bytesReadSoFar < numOfBytes); 
    printf("buff is %s\n", *buff); 
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
    chdir("..");
    close(manifestfd);
    closedir(directory);
} 
void upload_files(int sockfd,char* file_path, char* full_file_path){
    //printf("upload\n");
    char* content;
    r_file(full_file_path,&content);
    printf("lol\n");
    printf("contents is %s\n",content);
    char* message = (char*)(malloc(sizeof(char) * 12 + strlen(file_path) + strlen(content)));
    bzero(message,sizeof(char) * 12 + strlen(file_path) + strlen(content));
    sprintf(message,"f2:%d:%s:%d:%s;",strlen(file_path),file_path,strlen(content),content);
    printf("%s\n",message);
    write(sockfd,message,strlen(message));
}

void send_files(int connfd, char* folder_name){
    printf("folder name is %s\n", folder_name);
    DIR* directory = opendir(folder_name);
    struct dirent* currentElement = NULL;
    readdir(directory);
    readdir(directory);
    currentElement = readdir(directory);
    while(currentElement != NULL){
        printf("current is %s\n", currentElement->d_name);
        if(currentElement->d_type == 4){ //if its a directory
            //send directory here
            //write(connfd, 'd', 1);
            //printf("current2 is %s\n", currentElement->d_name);
            int len = strlen(currentElement->d_name);
            char project_len[10];
            sprintf(project_len,"%d",len);
            char message[5+strlen(currentElement->d_name)+strlen(project_len)];
            bzero(message,strlen(message));
            sprintf(message,"%c1:%d:%s;",'d',strlen(currentElement->d_name),currentElement->d_name);
            printf("message is %s\n", message);
            write(connfd,message,strlen(message));

            char file[strlen(folder_name)+1+strlen(currentElement->d_name)];
            bzero(file, strlen(file));
            strcat(file, folder_name);
            strcat(file, "/");
            strcat(file, currentElement->d_name);
            printf("file is %s\n", file);
            send_files(connfd, file);

            write(connfd, 'b',1); // tell the client to go back in pwd since we finished all th files in this directory
        }
        if(currentElement->d_type == 8){ //if its a file 
            //send files here
            char file[strlen(folder_name)+1+strlen(currentElement->d_name)];
            bzero(file, strlen(file));
            strcat(file, folder_name);
            strcat(file, "/");
            strcat(file, currentElement->d_name);
            upload_files(connfd, currentElement->d_name, file);
        }
        printf("Heloow finishi\n");
        currentElement = readdir(directory);
    }

}

void checkout(int connfd, char** arguments ){
    char* folder = arguments[0];
    //printf("Hellow\n");
    //printf("%s\n", arguments[0]);
    DIR* directory = opendir("./");
    struct dirent* currentElement = NULL;
    readdir(directory);
    readdir(directory);
    currentElement = readdir(directory);
    //printf("Hellow left \n");
    while(currentElement != NULL){
         if(currentElement->d_type == 4){ //if its a directory
         //printf("Hello folder name is %s\n", currentElement->d_name);
            //printf("folder name is %s\n", folder);
            if(strcmp(currentElement->d_name,folder) == 0){
                //found the directory
                write(connfd,"m",1); //m for made, letting client know that we found directory on server side 
                //printf("Hello folder %s\n", currentElement->d_name);
                //return 0;
                char file[2+strlen(currentElement->d_name)];
                bzero(file, strlen(file));
                file[0] = '.';
                file[1] = '/';
                strcat(file, currentElement->d_name);
                printf("Hello folder %s\n", file);
                send_files(connfd, file);
                printf("Got here\n");
                write(connfd,"&",1); // finished going through every file in the directory
                return ;
            }
         } 
         currentElement = readdir(directory);
    }
    printf("Directory doesn't exists.\n");
    write(connfd,"e",1);
    return -1;
}

void rollback(int connfd, char ** arguments){
    char* folder = arguments[0];

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

void destroy (int connfd, char** arguments){
    char* folder = arguments[0];

    DIR* directory = opendir("./");
    struct dirent* currentElement = NULL;
    currentElement = readdir(directory);
    while(currentElement != NULL){
         if(currentElement->d_type == 4){
             if(string_equal(currentElement->d_name,folder) == 0){
                   delete_files("./");
                   write(connfd,"s",1); // return success that we finished everything above
                   return;
             }
         } 
         currentElement = readdir(directory);
    }

    printf("Directory doesn't exists.\n");
    write(connfd,"e",1);
    return -1;
}
void history (int connfd, char** arguments){

    char* folder = arguments[0];
    int success = chdir(folder);
    if(success != 0){
        printf("folder doesn't exist.\n");
        write(connfd, 'e', 1);
        return;
    }
    int commitfd = open("./.Commit", O_RDWR, S_IRWXU);
    if(commitfd < 0){
        printf("Commit file doesn't exists.\n");
        write(connfd, 'e', 1);
        return -1;
    }
    write(connfd,"s",1); // return success we found it

    // send  here all containing  the history of all operations performed on all pushes
    //format: version number and new line seperating pushes log of changes 
    char c = ' ';
    int read_status = 0;
    int i = 0;
    char buffer[100];
    bzero(buffer, strlen(buffer));
    do {
        //printf("buffer is = %s\n", buffer);
        read_status = read(commitfd, &c, 1);
        if(read_status == 0){ // reached the end of the file
            break;
        }

        if(c == ' '){ // for type and file path
            buffer[i] = ' ';
            i = i+1;
            buffer[i] = '\0';
            write(connfd, buffer, strlen(buffer));
            memset(buffer, '\0', 100);
            i = 0;
        } else if (c == '/'){
            memset(buffer, '\0', 100);
            i = 0;
        }
        else if (c == '\n'){
            write(connfd,"\n",1);
            memset(buffer, '\0', 100);
            i = 0;
        }
        else {
            buffer[i] = c;
            i++;
        }

    }while(read_status > 0);

    //buffer[i] = '\s0';
    //printf("helow\n");
    write(connfd, "&",1); //reached end of files
    close(commitfd);
    chdir("..");
    return;


}
void current_version(int connfd, char** arguments){
    char* folder = arguments[0];
    chdir(folder);
    int manifestfd = open("./.Manifest", O_RDWR, S_IRWXU);
    if(manifestfd < 0){
        write(connfd, 'e', 1);
        return -1;
    }
    write(connfd, "s",1); // success at finding manifest
    char c = ' ';
    read(manifestfd, &c, 1); // read the project version, dont need it
    read(manifestfd, &c, 1); // read the newline, dont need it

    int read_status = 0;
    int i = 0;
    char buffer[100];
    bzero(buffer, strlen(buffer));
    do {
        //printf("buffer is = %s\n", buffer);
        read_status = read(manifestfd, &c, 1);
        if(read_status == 0){ // reached the end of the file
            break;
        }

        if(c == ' '){ // for version number and file path
            buffer[i] = ' ';
            i = i+1;
            buffer[i] = '\0';
            write(connfd, buffer, strlen(buffer));
            memset(buffer, '\0', 100);
            i = 0;
        } else if (c == '/'){
            memset(buffer, '\0', 100);
            i = 0;
        }
        else if (c == '\n'){
            write(connfd,"\n",1);
            memset(buffer, '\0', 100);
            i = 0;
        }
        else {
            buffer[i] = c;
            i++;
        }

    }while(read_status > 0);

    //buffer[i] = '\0';
    printf("helow\n");
    write(connfd, "&",1); //reached end of files
    close(manifestfd);
    chdir("..");
    return;
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
    } else if (command == 'u'){ // upload
        upload(connfd,arguments);
    } else if (command == 'o'){ //checkout
        printf("%s\n", arguments[0]);
        //return -1;
        checkout(connfd,arguments);
    } else if(command == 'r'){ // rollback
        rollback(connfd, arguments);
    } else if(command == 'd'){ // destroy 
        destroy(connfd, arguments);
    }else if (command == 'h'){ // history
        history(connfd, arguments);
    } else if (command == 'v'){ //current version
        current_version(connfd, arguments);
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
