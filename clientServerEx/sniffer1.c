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
    
}



















