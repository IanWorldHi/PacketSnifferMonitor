//Show IP addr for host
#include <stdio.h>
#include <string.h>
#include <sys/types.h> //some types ie) size_t cuz used in some of the funcs i think - just std syntax
#include <sys/socket.h> //core API
#include <netdb.h> //network db ops ie) getaddrinfo
#include <arpa/inet.h> //address conversion ie) htons
#include <netinet/in.h> //ip structs ie) sockaddr_in
//} std includes convention

//Example usage: ./gai1 www.google.com or ./gai1 localhost (www.google.com looks for google's ips)
//It asks DNS server which converts these public IPs
int main(int argc, char *argv[]){
    int status;
    struct addrinfo hints, *res, *p;
    char ipstr[INET6_ADDRSTRLEN];
    if(argc!=2){
        fprintf(stderr, "usage: '%s hostname'\n", argv[0]);
        return 1;
    }
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; 
    hints.ai_socktype = SOCK_STREAM;
    //passing null means i am host
    if((status = getaddrinfo(argv[1], NULL, &hints, &res) != 0)){
        fprintf(stderr, "gai (getaddrinfo) error: %s\n", gai_strerror(status)); //fprint can print to any stream
        return(2); 
    }
    printf("IP address: %s\n", argv[1]);
    for(p = res; p!=NULL; p=p->ai_next){
        void *addr;
        char *ipver;
        struct sockaddr_in *ipv4;
        struct sockaddr_in6 *ipv6;
        
        //get pointer to address - diff for ip4 ip6
        if(p->ai_family == AF_INET){
            ipv4 = (struct sockaddr_in*)p->ai_addr;
            addr = &(ipv4->sin_addr); //internet addr from the infos
            ipver = "IPv4";
        }
        else{
            ipv6 = (struct sockaddr_in6*)p->ai_addr;
            addr = &(ipv6->sin6_addr); 
            ipver = "IPv6";
        }
        //convert ip to str
        inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);

        int s;
        s = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if(s==-1){
            perror("socket");
        }
        /*
        tryagain:
        if(select(n, &readfds, NULL, NULL)==-1){
            if(errno==EINTR){
                goto tryagain;
            }
            perror("select");
            exit(1);
        }
        */
        printf("socket (testLearning): %d\n", s);
        printf("IP version: %s -- address: %s\n", ipver, ipstr);
        close(s);
    }
    freeaddrinfo(res);
    return 0;
}












