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
    hints.ai_flags = AI_PASSIVE; 
    
    getaddrinfo(NULL, "3490", &hints, &res); //localhost, port 3490, etc
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol); 
    if(sockfd==-1){
        perror("socket");
        exit(1);
    }

    int yes = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes); //setsockopt to reuse port if in TIME_WAIT (from killing server)
    //SOL_SOCKET is the level for socket options - imported constant
    //SO_REUSEADDR num meaning reuse, &yes is (1 or 0) for SO_REUSEADDR

    int b = bind(sockfd, res->ai_addr, res->ai_addrlen); 
    if(b==-1){
        perror("bind");
        exit(1);
    }
}










