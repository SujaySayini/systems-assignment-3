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
#include <linux/limits.h>
#include <libgen.h>
#include <dirent.h>
#include <netdb.h> 
#include <arpa/inet.h>
#include <openssl/sha.h>

#define true 1
#define false 0
//#define PATH_MAX 100 


int r_file(char * path,char** buff){
    struct stat stats;
    int read_status = 0;
    stat(path,&stats);
    int bytesReadSoFar = 0, numOfBytes = stats.st_size;
    
    *buff = (char*)malloc(sizeof(char) * numOfBytes + 1);
    bzero(*buff,numOfBytes);
    (*buff)[numOfBytes] = '\0';
    printf("%s %d\n",path,stats.st_size);
    
    int fileD = open(path, O_RDONLY); 
    do
    {
        read_status = read(fileD, *(buff + bytesReadSoFar), numOfBytes - bytesReadSoFar);
        bytesReadSoFar += read_status;

    } while (read_status > 0 && bytesReadSoFar < numOfBytes);
    printf("%s",*buff);
    if(read_status > 0)
        return stats.st_size;
    return read_status;
    /*
    */

}

char* getHash(char* buff){
    unsigned char *d = SHA256(buff, strlen(buff), 0);
    int i;
    printf("%s contents\n",buff);
    char* hash = (char*)malloc(sizeof(char) * SHA256_DIGEST_LENGTH * 2 + 1);
    for (i = 0; i < SHA256_DIGEST_LENGTH; i++){ 
        sprintf(hash + 2*i,"%02x", d[i]);
    }
    hash[2 * SHA256_DIGEST_LENGTH + 1] = '\0';
    return hash;
}

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

void upload_files(int connfd,char** arguments){
    //arg[0] = filepath
    //arg[1] = file content
    printf("data %s\n", arguments[1]);
    int depth = 0;
    int i = 0;
    int tracker = 0;
    char holder[PATH_MAX];
    printf("%s arg\n",arguments[0]);
    // while(i < strlen(arguments[0])){
    //     printf("llooop\n");
    //     printf("holder name is %s\n", holder);
    //     if(arguments[0][i] == '/'){
    //         holder[tracker] = '\0';
    //         printf("status %d\n",mkdir(holder,0777));
    //         depth++;
    //         chdir(holder);
    //         tracker = 0;
    //         printf("%s %d\n",holder,depth);
    //     }
    //     else{
    //         holder[tracker] = arguments[0][i];
    //         tracker++;
    //     }
    //     i++;
    // }
    holder[tracker] = '\0';
    char final[100] = "./";
    //strcat(final,holder);
    strcat(final,arguments[0]);
    printf("final is %s\n", final);
    int fd = open(final, O_RDWR | O_TRUNC | O_CREAT, 0644);
    printf("%s fd\n",arguments[1]);
    perror("");
    write(fd,arguments[1],strlen(arguments[1])); 
    int a;
    for(a = 0;a<depth;a++)
        chdir("..");
    
}

void checkout(int sockfd, char* project_name){
    printf("%s\n", project_name);

    int len = strlen(project_name); //10 - 2
    char project_len[10];
    sprintf(project_len,"%d",len);
    char message[5+strlen(project_name)+strlen(project_len)];
    //printf("Hello3\n");
    bzero(message,strlen(message));
    printf("%d , %s\n", strlen(project_name), project_name);
    sprintf(message,"%c1:%d:%s;",'o',strlen(project_name),project_name);
    write(sockfd,message,strlen(message));
    printf("%s\n",message);
    char c = ' ';
    read(sockfd,&c,1);
    printf("Hello2\n");
    if(c == 'e'){ // e for error 
        printf("Error, project doesn't exist on the server");
    } else if (c == 'm'){ // made
        //char command = read(sockfd,&c,1);
        mkdir(project_name, 00700);
        chdir(project_name);
        //get all the files and directories here
        char command = ' ';
       
        while( command != '&'){ // ending
            read(sockfd,&command,1);
            if(command == 'b'){
                chdir("..");
                continue;
            }
            char** arguments;
                char args[50];
                char c = ' ';
                int i = 0;
                int a = 0;
                int tracker = 0;
                int read_status;
                while(true){
                    read(sockfd,&c,1);
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
                        read(sockfd,&c,1);
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
                    read(sockfd,&c,1);
                    if(c == ':' || c == ';'){
                        arguments[a][b] = '\0';
                        b = 0;
                        break;
                    }
                    arguments[a][b] = c;
                    b++;
                    }
                }  
                printf("arguemnts %s\n", arguments[0]);
                printf("enter command %c\n", command);
                //return 0;
            if(command == 'f'){ // if its a file 
                printf("entered here\n");
                upload_files(sockfd, arguments);
            } else if(command == 'd'){ //if its a directory
                char* path = arguments[0];
                mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
                chdir(path);
            } 
            // if (command == 'b'){ // go back a directory
            //     chdir("..");
            // }
            
        }
    }
}

void upload(int sockfd,char* file_path){
    char* content;
    r_file(file_path,&content);
    char* message = (char*)(malloc(sizeof(char) * 12 + strlen(file_path) + strlen(content)));
    bzero(message,sizeof(char) * 12 + strlen(file_path) + strlen(content));
    sprintf(message,"u2:%d:%s:%d:%s;",strlen(file_path),file_path,strlen(content),content);
    printf("%s\n",message);
    write(sockfd,message,strlen(message));
}

int ten_pow(int power){
    int tracker = 1;
    int a;
    for(a = 1;a <= power; a++){
        tracker = tracker * 10;
    }
    return tracker;
}

int make_dir(char* path){
    int depth = 0;
    printf("HELLO\n");
    int i = 0;
    int tracker = 0;
    char holder[PATH_MAX];
    while(i < strlen(path)){
        if(path[i] == '/'){
            holder[tracker] = '\0';
            printf("%s\n",holder);
            depth++;
            mkdir(holder,0777);
            chdir(holder);
            tracker = 0;
        }
        else{
            holder[tracker] = path[i];
            tracker++;
        }
        i++;
    }
    int a;
    for(a = 0; a < depth; a++){
        chdir("..");
    }
}

void download(int sockfd,char* target_path,char* file_path){
    char* message = (char*)(malloc(sizeof(char) * 12 + strlen(file_path)));
    bzero(message,sizeof(char) * 12 + strlen(file_path));
    sprintf(message,"x1:%d:%s;",strlen(file_path),file_path);
    printf("%s\n",message);
    write(sockfd,message,strlen(message));
    char c;
    read(sockfd,&c,1);
    printf("%c code\n",c);
    if(c == 'e'){
    
    }
    else{
        int num_bytes = 0;
        int tracker = 0;
        while(c != ':'){
            read(sockfd,&c,1);
            if(c != ':'){
                num_bytes += (c - '0') * ten_pow(tracker);
                tracker++;
            }
        }
        int i;
        char* content = (char*)malloc(sizeof(char) * num_bytes + 1);
        int bytes_read = 0;
        int read_status = 0;
        while(read_status > -1 && bytes_read < num_bytes){
            read_status = read(sockfd,content + bytes_read ,num_bytes - bytes_read);
            bytes_read = bytes_read + read_status;
        }
        content[num_bytes] = '\0';
        make_dir(target_path);
        int fd = open(target_path, O_RDWR | O_TRUNC | O_CREAT,0644);
        write(fd,content,strlen(content)); 
    }
}

int stoi(char* x){
    int a;
    int total = 0;
    for(a = 0;a < strlen(x);a++){
        total += x[a] - '0' * ten_pow(a);
    }
    return total;
}

void create(int sockfd, char* project_name){
    char* len = strlen(project_name);
    //char message[5+strlen(project_name)+strlen(len)];
    char* message = (char*)malloc(sizeof(char) * strlen(project_name));
    bzero(message,strlen(message));
    sprintf(message,"%c1:%d:%s;",'c',strlen(project_name),project_name);
    write(sockfd,message,strlen(message));
    printf("%s\n",message);
    char c;
    read(sockfd,&c,1);
    if(c == 'e'){ // e for error 
        printf("Error, project already exists on the server");
    } else if (c == 'm'){ // made 
        DIR* directory = opendir("./");
        mkdir(project_name, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH); // create our own project on the client side
        chdir(project_name);
        int manifestfd = open("./.Manifest", O_CREAT | O_TRUNC | O_RDWR, S_IRWXU);
        chdir("..");
        write(manifestfd,"0\n",2);
        close(manifestfd);
        closedir(directory);
    }
}
void add(int sockfd, char* project_name, char* file_name){
    char file_path[80] = {};
    char manifest_path[80] = {};
    sprintf(manifest_path,"./%s/.Manifest",project_name);
    printf("%s\n",manifest_path);
    int fd = open(manifest_path, O_CREAT | O_APPEND | O_RDWR, S_IRWXU);
    sprintf(file_path,"./%s/%s",project_name,file_name);
    char* file_content;
    char* hash; 
    r_file(file_path,&file_content);
    hash = getHash(file_content);
    char towrite[255] = {};
    sprintf(towrite,"0 %s %s\n",file_path,hash);
    printf("%s\n",towrite);
    write(fd,towrite,strlen(towrite));
    free(hash);
}
void removefd(int sockfd, char* project_name, char* file_name){


}
void update(int sockfd, char* project_name){

}
void commit(int sockfd, char* project_name){
    download(sockfd,"project1/folder/test.txt","project1/test.txt");
}

struct manifest_entry{
    int version;
    char hash[64];
    char path[PATH_MAX];
    struct manifest_entry* next;
};
struct manifest_entry* parseManifest(char* content){
    int tracker = 0;
    int i = 0;
    int holder_t = 0;
    struct manifest_entry* head = (struct manifest_entry*) malloc(sizeof(struct manifest_entry));
    struct manifest_entry* current = head;
    while(i < strlen(content)){
        if(content[i] == ' '){
            if(tracker == 1){
                current->path[holder_t] = '\0';
                printf("%s\n",current->path);
            }
            holder_t = 0;
            tracker++;
        }
        else if(content[i] == '\n'){
            current->hash[holder_t] = '\0';
            current->next = (struct manifest_entry*) malloc(sizeof(struct manifest_entry));
            current = current->next;
            tracker = 0;
            holder_t = 0;
        }
        else{
            if(tracker == 0){
               current->version = current->version + (content[i] - '0')*ten_pow(holder_t);
            }
            else if(tracker == 1){
                current->path[holder_t] = content[i];
            }
            else if(tracker == 2){
                current->hash[holder_t] = content[i];
            }
            holder_t++;
        }
        i++;
    }
    return head;
}

void upgrade(int sockfd, char* project_name){
    chdir(project_name);
    char* manifest;
    char* file_to_read = (char*) malloc(sizeof(char) * (strlen(project_name) + strlen(".Manifest")));
    sprintf(file_to_read,"%s/.Manifest",project_name);
    download(sockfd,".tmp",file_to_read);
    r_file(".tmp",&manifest);
    struct manifest_entry* manifest_str = parseManifest(manifest);
    printf("%s\n",manifest);
    char* update_c;
    int status = r_file(".Update",&update_c);
    if(status < 0){
        printf("No .update file present!");
    }
    
    chdir("..");
}
void push(int sockfd, char* project_name){
}
void destroy(int sockfd, char* project_name){
    char* len = strlen(project_name);
    char message[5+strlen(project_name)+strlen(len)];
    //char* message = (char*)malloc(sizeof(char) * strlen(project_name));
    bzero(message,strlen(message));
    sprintf(message,"%c1:%d:%s;",'d',strlen(project_name),project_name);
    write(sockfd,message,strlen(message));
    char c;
    read(sockfd,&c,1);
    if(c == 'e'){ // e for error 
        printf("Error, project doesn't exist on the server");
    } else if (c == 's'){ // success
        printf("Sucessfully deleting files.\n");
    }

}
void current_version(int sockfd, char* project_name){
    char* len = strlen(project_name);
    char message[5+strlen(project_name)+strlen(len)];
    //char* message = (char*)malloc(sizeof(char) * strlen(project_name));
    bzero(message,strlen(message));
    sprintf(message,"%c1:%d:%s;",'v',strlen(project_name),project_name);
    write(sockfd,message,strlen(message));
    char c;
    read(sockfd,&c,1);
    if(c == 'e'){ // e for error 
        printf("Error, project doesn't exist on the server");
    } else if (c == 's'){ // success
        printf("Sucessfully deleting files.\n");
    }


}
void history(int sockfd, char* project_name){

    char* len = strlen(project_name);
    char message[5+strlen(project_name)+strlen(len)];
    //char* message = (char*)malloc(sizeof(char) * strlen(project_name));
    bzero(message,strlen(message));
    sprintf(message,"%c1:%d:%s;",'h',strlen(project_name),project_name);
    write(sockfd,message,strlen(message));
    char c;
    read(sockfd,&c,1);
    if(c == 'e'){ // e for error 
        printf("Error, project doesn't exist on the server");
    } else if (c == 's'){ // success
        //printf("Sucessfully deleting files.\n");
        
    }

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
        // DIR* directory = opendir("./");
        // struct dirent* currentElement = NULL;
        // currentElement = readdir(directory);
        // while(currentElement != NULL){
        //     if(currentElement->d_type == 4){
        //         printf("%s and %s", argv[2], currentElement->d_name);
        //         if(string_equal(currentElement->d_name,argv[2]) == 0){
        //             printf("Directory already exists on the client side.11\n");
        //             return -1;
        //         }
        //     } 
        // currentElement = readdir(directory);
        // }
        printf("%s\n", argv[2]);
        checkout(sockfd, argv[2]);

    } else if (string_equal(argv[1], "update")){
        update(sockfd, argv[2]);
    } else if (string_equal(argv[1], "upgrade")){
        upgrade(sockfd, argv[2]);
    } else if (string_equal(argv[1], "commit")){
        commit(sockfd, argv[2]);
    } else if (string_equal(argv[1], "push")){
        //upload(sockfd,"testersmile/lol.txt");       
        push(sockfd, argv[2]);
    } else if (string_equal(argv[1], "create")){
        create(sockfd,argv[2]);

    } else if (string_equal(argv[1], "destroy")){
        destroy(sockfd, argv[2]);

    } else if (string_equal(argv[1], "add")){
        printf("adding\n");

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
