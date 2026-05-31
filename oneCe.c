/* 
struct addrinfo {
    int              ai_flags;     // AI_PASSIVE, AI_CANONNAME, etc.
    int              ai_family;    // AF_INET, AF_INET6, AF_UNSPEC
    int              ai_socktype;  // SOCK_STREAM, SOCK_DGRAM
    int              ai_protocol;  // use 0 for "any"
    size_t           ai_addrlen;   // size of ai_addr in bytes
    struct sockaddr *ai_addr;      // struct sockaddr_in or _in6
    char            *ai_canonname; // full canonical hostname
    struct addrinfo *ai_next;      // linked list, next node
};
struct sockaddr {
    unsigned short    sa_family;    // address family, AF_xxx
    char              sa_data[14];  // 14 bytes of protocol address
}; 
struct sockaddr_in {
    short int          sin_family;  // Address family, AF_INET
    unsigned short int sin_port;    // Port number
    struct in_addr     sin_addr;    // Internet address
    unsigned char      sin_zero[8]; // Same size as struct sockaddr
};
int getaddrinfo(const char *node,   // e.g. "www.example.com" or IP
                const char *service,  // e.g. "http" or port number
                const struct addrinfo *hints,
                struct addrinfo **res);
struct sockaddr_storage { //fore if dont konw ipv4 ipv6
    sa_family_t  ss_family;     // address family
    char        __ss_pad1[_SS_PAD1SIZE];
    uint64_t    __ss_align;   
    char        __ss_pad2[_SS_PAD2SIZE];
};

int socket(int domain, int type, int protocol); ie) socket(AF_INET, SOCK_STREAM, 0) for TCP
int bind(int sockfd, struct sockaddr *my_addr, int addrlen); //sockfd from socket, my_addr ur address info (as the server?)
int connect(int sockfd, struct sockaddr *serv_addr, int addrlen); //sockaddr info on desitination ipaddr and port
int listen(int sockfd, int backlog); //backlog num of connections allowed on incoming queue
int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen); 
int send(int sockfd, const void *msg, int len, int flags); 
int recv(int sockfd, void *buf, int len, int flags);
 */


#include <stdio.h>
#include <string.h>
#include <sys/types.h> //some types ie) size_t cuz used in some of the funcs i think - just std syntax
#include <sys/socket.h> //core API
#include <netdb.h> //network db ops ie) getaddrinfo
#include <arpa/inet.h> //address conversion ie) htons
#include <netinet/in.h> //ip structs ie) sockaddr_in
//} std includes convention


int main(){
    //Example server - sets up structures for info
    int status;
    struct addrinfo hints;
    struct addrinfo *res; //server info, we return a pointer to a struct?
    //linked list of possible addresses
    memset(&hints, 0, sizeof hints); //makes struct empty
    hints.ai_family = AF_UNSPEC; //means can be IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; //TCP stream sockets
    hints.ai_flags = AI_PASSIVE; //it fills in my IP for me, assigning my localhost address

    //3490 example localhost? port num, can replace NULL with ur ip address ur searching for
    //first param is which machine (ai_flags), second is what port on machine
    if((status = getaddrinfo(NULL, "3490", &hints, &res) != 0)){
        fprintf(stderr, "gai (getaddrinfo) error: %s\n", gai_strerror(status)); //fprint can print to any stream
        exit(1); 
    }

    //addrinfo use

    freeaddrinfo(res); //frees linked list - 

    printf("Man it's been a while %d\n", status);
    return 0;
}












