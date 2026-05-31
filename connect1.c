#include <stdio.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netdb.h> 
#include <arpa/inet.h>
#include <netinet/in.h>

int main(){
    struct addrinfo hints, *res;
    int sockfd;

    //loading addr struct
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    
    getaddrinfo("www.example.com", "3490", &hints, &res); //localhost, port 3490, etc
    //technically need error handle gai too
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol); 
    if(sockfd==-1){
        perror("socket");
        exit(1);
    }

    //kernel picks rand local port and passes that info to whatever site we connecting to
    int c = connect(sockfd, res->ai_addr, res->ai_addrlen); 
    if(c==-1){
        perror("connect");
        exit(1);
    }
}










