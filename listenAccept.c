#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define MYPORT "3490"  // the port users will be connecting to
#define BACKLOG 10     // how many pending connections queue holds

int main(void){
    struct sockaddr_storage their_addr;
    socklen_t addr_size; //specific type for storing socket addresses
    struct addrinfo hints, *res; 
    int sockfd, new_fd;

    //loading addr structs
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;     
    getaddrinfo(NULL, MYPORT, &hints, &res);
    //technically need error handle gai too

    // make a socket, bind it, and listen on it:
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if(sockfd==-1){
        perror("socket");
        exit(1);
    }
    int b = bind(sockfd, res->ai_addr, res->ai_addrlen);
    if(b==-1){
        perror("bind");
        exit(1);
    }
    int l = listen(sockfd, BACKLOG);
    if(l==-1){
        perror("listen");
        exit(1);
    }

    //accepting incoming connection
    addr_size = sizeof their_addr;
    //normally want sockaddr* but that reequires us to know the ipv4 or ipv6 (it can only store one)
    //typecast sockaddr_storage bc that's the convention and they handle it i guess we can do both then
    new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);
    if(new_fd==-1){
        perror("accept");
        exit(1);
    }

}















