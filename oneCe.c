/* #include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

int getaddrinfo(const char *node,   // e.g. "www.example.com" or IP
                const char *service,  // e.g. "http" or port number
                const struct addrinfo *hints,
                struct addrinfo **res);
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












