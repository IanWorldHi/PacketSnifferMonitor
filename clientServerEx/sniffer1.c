//sniff sniff
#include <stdio.h>
#include <stdlib.h>//malloc
#include <unistd.h>
#include <errno.h>
#include <string.h>//memset
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <sys/ioctl.h>	//ioctl() and SIOCGIFADDR
#include <sys/time.h>   //time related funcs
#include <sys/types.h>	//comon types
#include <unistd.h>     //POSIX functions read write clsoe fork usleep etc
#include <net/if.h>	//struct ifreq, not sure if need

#include <netinet/ip_icmp.h>	//Provides declarations for icmp header
#include <netinet/udp.h>	//Provides declarations for udp header
#include <netinet/tcp.h>	//Provides declarations for tcp header
#include <netinet/ip.h>	//Provides declarations for ip header
#include <netinet/if_ether.h>	//For ETH_P_ALL
#include <net/ethernet.h>	//For ether_header

#include <getopt.h>
#include <net/if.h>


#define exit_with_error(msg) do {perror(msg); exit(EXIT_FAILURE);} while(0)
//do while is to safely contain block when it's called outside
//you would call exit_with_error for when you want to throw an error and exit
//alternative, #include <err.h> err(EXIT_FAILURE, "msg") or errx

typedef struct{
    uint8_t transfer_protocol;
    char *source_ip;
    char *dest_ip;
    uint16_t source_port;
    uint16_t dest_port;
    char *source_ifname; //interface name
    char *dest_ifname;
    uint8_t source_mac[6]; //6 bytes
    uint8_t dest_mac[6];
} package_filter_t;

struct sockaddr_in source_addr, dest_addr; //ipv4 socket addresses

int main(int argc, char *argv[]){
    int c;
    char log[225]; //log message, 225 is max size, taken as input from user
    FILE *log_file = NULL; //file pointer for log file, std I/O setup
    
    packet_filter_t filter = {0, NULL, NULL, 0, 0, NULL, NULL}; //initialize default
    
}


















