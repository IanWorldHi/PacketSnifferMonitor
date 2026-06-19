//#define _GNU_SOURCE
// I added the define to the configs
//feature-test macro or smth, makes net/if.h and strlcpy etc work bc thier headers are configured differently
//Alt:
//Change: C/C++ Edit Configurations to add -D_GNU_SOURCE to compiler flags
//Or, change the configs to -std=gnu23

//sniff sniff - add security stuff later before testing
#include <stdio.h>
#include <stdlib.h>//malloc

#include <unistd.h> 
#include <getopt.h> //gives optarg for getopt(linux, the ^ otherwise i think)

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
} packet_filter_t;

struct sockaddr_in source_addr, dest_addr; //ipv4 socket addresses, if want can add ipv6 later
    //used in process_paccket

void get_mac(char *ifname, packet_filter_t *filter, char *if_type){
    int fd;
    struct ifreq ifr; //interface request struct for ioctl given interface name return oen field
    fd = socket(AF_INET, SOCK_DGRAM, 0); //need a fd for ioctl
    if(fd < 0){
        exit_with_error("socket creation failed for ioctl");
    }
    strlcpy(ifr.ifr_name, ifname, IFNAMSIZ); //copy interface name to ifreq w/size constraint 
    
    if(ioctl(fd, SIOCGIFHWADDR, &ifr) < 0){
        close(fd);
        exit_with_error("ioctl failed to get mac address");
    }
    close(fd);
    
    if(strcmp(if_type, "source") == 0){
        //strcpy might not be right
        strcpy(filter->source_mac, (uint8_t*)ifr.ifr_hwaddr.sa_data); //copying mac addr
    }
    else{
        strcpy(filter->dest_mac, (uint8_t*)ifr.ifr_hwaddr.sa_data);
    }
}

bool cmpmac(uint8_t *mac1, uint8_t *mac2){
    for(int i = 0; i < 6; i++){
        if(mac1[i] != mac2[i]){
            return false;
        }
    }
    return true;
}

bool filtering_IP(packet_filter_t *filter, struct iphdr *ip){
    //have to convert here - wait depends on user input into filter no?
    //wait let me check their types if they strings

    if(filter->source_ip != NULL && strcmp(filter->source_ip, ip->saddr) != 0){
        return false;
    }
    if(filter->dest_ip != NULL && strcmp(filter->dest_ip, ip->daddr) != 0){
        return false;
    }
    return true;
}

void process_packet(uint8_t *buffer, int buf_len, packet_filter_t *filter, FILE *log_file){
    //raw packet data order, hdr = header
    //ethernet header -> ip header -> transport layer header (tcp/udp) -> user data
    //All countain information on what layer above its' protocol is, ie) eth knows network layer's protocol (ip)
    
    //extract eth header
    struct ethhdr *eth = (struct ethhdr *)buffer; //typecast buffer to parse eth header

    if(ntohs(eth->h_proto) != ETH_P_IP){ //checks if IP protocol (IPv4)
        return; 
    }
    if(filter->source_ifname != NULL && cmpmac(filter->source_mac, eth->h_source) == false){
        return;
    }
    if(filter->dest_ifname != NULL && cmpmac(filter->dest_mac, eth->h_dest) == false){
        return;
    }

    //extract ip header
    struct iphdr *ip = (struct iphdr *)(buffer + sizeof(struct ethhdr));
    //iphdr has variable options, makes getting size difficult
    int ip_header_len = ip->ihl*4; //ihl is internet header length in 32 bit words, convert to bytes

    memset(&source_addr, 0, sizeof(source_addr)); //zeroing out/resetting
    memset(&dest_addr, 0, sizeof(dest_addr));

    //sin_addr is network byte order, saddr and daddr in ip header are also network byte order
    //given that they are from buffer which is from the netework
    source_addr.sin_addr.s_addr = ip->saddr; //store as source id in source_addr ip from ip header (saddr)
    dest_addr.sin_addr.s_addr = ip->daddr; 

    //ip filter

}

int main(int argc, char *argv[]){
    char log[225]; //log message, 225 is max size, taken as input from user
    FILE *log_file = NULL; //file pointer for log file, std I/O setup
    
    packet_filter_t filter = {0, NULL, NULL, 0, 0, NULL, NULL}; //initialize default

    struct sockaddr saddr; //pass to recieve from
    int sockfd, saddr_len, buf_len;

    uint8_t *buffer = (uint8_t *)malloc(65536); //buffer hold packet data, 65536 max IP packet
    memset(buffer, 0, 65536); //zero out buffer

    sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL)); //family packet, type raw, have to use ETH_P_ALL bc it will not give u IP packets going out otherwise
    //sits below IP layer, not necessarily actually ethernet just in that format, wifi gets changed to it too - its just an old name
    if(sockfd < 0){
        exit_with_error("socket creation failed raw socket at packet layer aka packet socket");
    }

    int c;
    while(1){
        //getopt to parse comd line args
        // so --sip and -s both work
        static struct option long_options[] = {
            {"sip", required_argument, NULL, 's'}, //source ip
            {"dip", required_argument, NULL, 'd'}, //dest ip
            {"sport", required_argument, NULL, 'p'}, //source port
            {"dport", required_argument, NULL, 'q'}, //dest port
            {"sif", required_argument, NULL, 'i'}, //source interface
            {"dif", required_argument, NULL, 'j'}, //dest interface
            {"logfile", required_argument, NULL, 'f'}, //log file name
            {"tcp", no_argument, NULL, 't'},
            {"udp", no_argument, NULL, 'u'},
            {0, 0, 0, 0} //end of options
        };
        c = getopt_long(argc, argv, "s:d:p:q:i:j:l:tu", long_options, NULL); 
        //: means required argument, :: means optional, none means no args

        if(c == -1){
            break;
        }
        switch(c){
            case 's': //sip
                filter.source_ip = optarg;
                break;
            case 'd': //dip
                filter.dest_ip = optarg;
                break;
            case 'p': //sport
                filter.source_port = atoi(optarg); //from getopt, string to int atoi ofc
                break;
            case 'q': //dport
                filter.source_port = atoi(optarg);
                break;
            case 'i': //sif
                filter.source_ifname = optarg;
                break;
            case 'j': //dif
                filter.dest_ifname = optarg;
                break;
            case 'f': //logfile
                strlcpy(log, optarg, sizeof(log)); //copy log file name to var
                //can change to char* or i gotta do bound checks 
                break;
            case 't': //tcp
                filter.transfer_protocol = IPPROTO_TCP; 
                break;
            case 'u': //udp
                filter.transfer_protocol = IPPROTO_UDP;
                break;
            default:
                fprintf(stderr, "Usage: %s [--sip source_ip] [--dip dest_ip] [--sport source_port] [--dport dest_port] [--sif source_interface] [--dif dest_interface] [--logfile log_file_name] [--tcp|--udp]\n", argv[0]);
                //double chefk afterlol
                exit(EXIT_FAILURE);    
        }
    }

    //haven't done option for Any yet
    printf("Filter settings:\n");
    printf("Source IP: %s\n", filter.source_ip ? filter.source_ip : "Any");
    printf("Destination IP: %s\n", filter.dest_ip ? filter.dest_ip : "Any");
    printf("Source Port: %u\n", filter.source_port ? filter.source_port : 0);
    printf("Destination Port: %u\n", filter.dest_port ? filter.dest_port : 0);
    printf("Source Interface: %s\n", filter.source_ifname ? filter.source_ifname : "Any");
    printf("Destination Interface: %s\n", filter.dest_ifname ? filter.dest_ifname : "Any");
    printf("Transfer Protocol: %s\n", filter.transfer_protocol == IPPROTO_TCP ? "TCP" : filter.transfer_protocol == IPPROTO_UDP ? "UDP" : "Any");
    printf("Log File: %s\n", log[0] ? log : "None");
    if(strlen(log) == 0){
        strcpy(log, "sniff_log.txt");
    }
    log_file = fopen(log, "w"); //open log file, w write
    if(!log_file){
        exit_with_error("Failed to open log file");
    }

    //getting mac address for given if
    if(filter.source_ifname && !filter.dest_ifname){
        get_mac(filter.source_ifname, &filter, "source");
        get_mac(filter.dest_ifname, &filter, "dest");
    }

    /* struct sockaddr saddr; 
    int sockfd, saddr_len, buf_len; */
    //MAIN LOOOP
    while(1){ //do i need -1?
        saddr_len = sizeof(saddr);
        buf_len = recvfrom(sockfd, buffer, 65536-1, 0, &saddr, saddr_len);
        if(buf_len < 0){
            exit_with_error("Failed to receive packets");
        }
        process_packet(buffer, buf_len, &filter, log_file);
        fflush(log_file);
    }
    
}


















