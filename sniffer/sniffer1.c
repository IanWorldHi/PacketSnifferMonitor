//#define _GNU_SOURCE
// I added the define to the configs
//feature-test macro or smth, makes net/if.h and strlcpy etc work bc thier headers are configured differently
//Alt:
//Change: C/C++ Edit Configurations to add -D_GNU_SOURCE to compiler flags
//Or, change the configs to -std=gnu23
//use -lcap to link with libcap for capabilities 
//gcc sniffer1.c -o sniffer1 -D_GNU_SOURCE -Wall -Wextra -lcap  
//sudo setcap cap_net_raw+p ./sniffer1 
//./sniffer1 -f sniff.log
//tail -f sniff.log
//less sniff.log

//curl
//whois
//dig 

//sniff sniff - add security stuff later before testing
#include <stdio.h>
#include <stdlib.h>//malloc

#include <signal.h> //signal handling
#include <sys/capability.h> //for capabilities
//sudo apt install libcap-dev

#include <poll.h>

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

char* htmlAddr = "";
char* browser = "chrome"; //change to edge (msedge), firefox wtv u use 

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

static volatile sig_atomic_t stop = 0;
//voltaile means reread from memory every loop, sig_atomic_t gaurantees it's read without being interrupted by signal
void signalHandler(int sig){
    (void)sig; 
    stop = 1;
}

static int capnetraw_onoff(cap_flag_value_t val){
    cap_t caps = cap_get_proc(); //copies processes current capability returning in cap_t
    if(caps == NULL){
        exit_with_error("Failed to get process capabilities");
    }
    cap_value_t lookingfor = CAP_NET_RAW;
    if(cap_set_flag(caps, CAP_EFFECTIVE, 1, &lookingfor, val) == -1){ 
        //edits buffer in memory, CAP_EFFECTIVE means set effective flag for the 1 capability listed in the array
        //set flag to val, where val is CAP_SET or CAP_CLEAR (on or off)
        cap_free(caps);
        exit_with_error("Failed to set capability flag");
    }
    int rc = cap_set_proc(caps); //writes buffer into kernel - change becomes real
    if(rc == -1){
        cap_free(caps);
        exit_with_error("Failed to set process capabilities");
    }
    cap_free(caps); //releases memory allocateed for buffer
    return rc;
}


void get_mac(char *ifname, packet_filter_t *filter, char *if_type){
    //get mac address from interface name
    int fd;
    struct ifreq ifr; //interface request struct for ioctl given interface name return oen field
    memset(&ifr, 0, sizeof(ifr));
    fd = socket(AF_INET, SOCK_DGRAM, 0); //need a fd for ioctl
    if(fd < 0){
        exit_with_error("socket creation failed for ioctl");
    }
    if(strlcpy(ifr.ifr_name, ifname, IFNAMSIZ) >= sizeof(ifr.ifr_name)){ //copy interface name to ifreq w/size constraint 
        close(fd);
        exit_with_error("Failed to copy interface name");
    }
    if(ioctl(fd, SIOCGIFHWADDR, &ifr) < 0){ //io control, SIOCGIFHWADDR fills hwaddr
        close(fd);
        exit_with_error("ioctl failed to get mac address");
    }
    close(fd);
    
    if(strcmp(if_type, "source") == 0){
        memcpy(filter->source_mac, ifr.ifr_hwaddr.sa_data, ETH_ALEN); //6 bytes cpy, (hwaddr = hardware addr = mac)
    }
    else{
        memcpy(filter->dest_mac, ifr.ifr_hwaddr.sa_data, ETH_ALEN);
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

bool filtering_IP(packet_filter_t *filter){
    //inet_ntoa converts bytes to decimal dot ie) what we view when seeing ip addr
    //can't use ip.saddr as it's raw bytes not wrapped by struct sin_addr which is what inet_ntoa takes
    if(filter->source_ip != NULL && strcmp(filter->source_ip, inet_ntoa(source_addr.sin_addr)) != 0){
        //source_addr.sin_addr, converts internet host address
        //my machine stores ip as network order too as it's in struct sockaddr_in
        return false;
    }
    if(filter->dest_ip != NULL && strcmp(filter->dest_ip, inet_ntoa(dest_addr.sin_addr)) != 0){
        return false;
    }
    return true;
}

bool filtering_port(packet_filter_t *filter, uint16_t source_port, uint16_t dest_port){
    if(filter->source_port != 0 && filter->source_port != source_port){
        return false; 
    }
    if(filter->dest_port != 0 && filter->dest_port != dest_port){
        return false;
    }
    return true;
}

void logETH(struct ethhdr *eth, FILE *log_file){
    fprintf(log_file, "Ethernet Header:\n");
    fprintf(log_file, "\tSource MAC: %.2X:%.2X:%.2X:%.2X:%.2X:%.2X\n", eth->h_source[0], eth->h_source[1], eth->h_source[2], eth->h_source[3], eth->h_source[4], eth->h_source[5]);
    fprintf(log_file, "\tDestination MAC: %.2X:%.2X:%.2X:%.2X:%.2X:%.2X\n", eth->h_dest[0], eth->h_dest[1], eth->h_dest[2], eth->h_dest[3], eth->h_dest[4], eth->h_dest[5]);
    fprintf(log_file, "\tProtocol: %d\n", ntohs(eth->h_proto));
}
void logIP(struct iphdr *ip, FILE *log_file){ //just going throuhg the fields of iphdr mostly - can edit later
    fprintf(log_file, "IP Header:\n");
    fprintf(log_file, "\tVersion: %d\n", (uint32_t)ip->version); //4 or 6, IPv
    fprintf(log_file, "\tHeader Length: %d bytes\n", (uint32_t)ip->ihl*4); //dcsp ecn smth
    fprintf(log_file, "\tType of Service: %d\n", (uint32_t)ip->tos); //why convert
    fprintf(log_file, "\tTotal Length: %d bytes\n", ntohs(ip->tot_len)); //
    fprintf(log_file, "\tIdentification: %d\n", (uint32_t)(ntohs(ip->id))); //id for fragementaiton, if a packet is split into multiple bc too big
    fprintf(log_file, "\tTime to Live: %d\n", (uint32_t)ip->ttl); //packets only be forwarded a certain amt of times before expires
    fprintf(log_file, "\tSource IP: %s\n", inet_ntoa(source_addr.sin_addr));
    fprintf(log_file, "\tDestination IP: %s\n", inet_ntoa(dest_addr.sin_addr));
    fprintf(log_file, "\tProtocol: %d\n", (uint32_t)ip->protocol);
    fprintf(log_file, "\tChecksum: %d\n", ntohs(ip->check)); //This is the security thingy to detect corruption of header
    //frag offset too - todo with flags etc
}

void logTCP(struct tcphdr *tcp, FILE *log_file){ //buncha flags - should go over them & 3 way handshake etc
    fprintf(log_file, "\nTCP Header\n");
    fprintf(log_file, "\t-Source Port : %d\n", ntohs(tcp->source));
    fprintf(log_file, "\t-Destination Port : %u\n", ntohs(tcp->dest));
    fprintf(log_file, "\t-Sequence Number : %u\n", ntohl(tcp->seq));
    fprintf(log_file, "\t-Acknowledgement Number : %u\n", ntohl(tcp->ack_seq));
    fprintf(log_file, "\t-Header Length (Bytes) : %d\n", (uint32_t)tcp->doff*4); //doff: number of (32bit) in hdr, variable bc options
    fprintf(log_file, "\t ------- Flags -------\n");
    fprintf(log_file, "\t-Urgent Flag : %d\n", (uint32_t)tcp->urg);
    fprintf(log_file, "\t-Acknowledgement Flag : %d\n", (uint32_t)tcp->ack);
    fprintf(log_file, "\t-Push Flag : %d\n", (uint32_t)tcp->psh);
    fprintf(log_file, "\t-Reset Flag : %d\n", (uint32_t)tcp->rst);
    fprintf(log_file, "\t-Synchronise Flag : %d\n", (uint32_t)tcp->syn);
    fprintf(log_file, "\t-Finish Flag : %d\n", (uint32_t)tcp->fin);
    fprintf(log_file, "\t-Window Size : %d\n", ntohs(tcp->window)); //flow ctrl, how many bytes sender willing to recieve to not overwhelm
    fprintf(log_file, "\t-Checksum : %d\n", ntohs(tcp->check));
    fprintf(log_file, "\t-Urgent pointer : %d\n", ntohs(tcp->urg_ptr));
}

void logUDP(struct udphdr *udp, FILE *log_file){ //buncha flags, less cuz it's just send
    fprintf(log_file, "UDP Header:\n");
    fprintf(log_file, "\tSource Port: %d\n", ntohs(udp->source));
    fprintf(log_file, "\tDestination Port: %d\n", ntohs(udp->dest));
    fprintf(log_file, "\tLength: %d bytes\n", ntohs(udp->len));
    fprintf(log_file, "\tChecksum: %d\n", ntohs(udp->check)); //sometimes set to 0, skipped they dont use it lol
}

void logpayload(uint8_t *buffer, int buf_len, int iphdrlen, uint8_t t_protocol, FILE *log_file, struct tcphdr *tcp){
    uint32_t proto_hdr_len = sizeof(struct udphdr);
    if(t_protocol == IPPROTO_TCP){
        proto_hdr_len = (uint32_t)tcp->doff*4;
    }
    uint8_t *payload = buffer + sizeof(struct ethhdr) + iphdrlen + proto_hdr_len;
    int payload_len = buf_len - (sizeof(struct ethhdr) + iphdrlen + proto_hdr_len);
    //can have 0 bytes bc of min size of ethhdr slight inaccuracy to fix

    fprintf(log_file, "Payload (%d bytes):\n", payload_len);
    for(int i = 0; i < payload_len; i++){
        if(i!=0 && i%16 == 0){
            fprintf(log_file, "\n");
        }
        fprintf(log_file, "%02X ", payload[i]);
    }
    fprintf(log_file, "\n");
}


//change to return smth later if i want to log more info
void process_packet(uint8_t *buffer, int buf_len, packet_filter_t *filter, FILE *log_file){
    //raw packet data order, hdr = header
    //ethernet header -> ip header -> transport layer header (tcp/udp) -> user data
    //All countain information on what layer above its' protocol is, ie) eth knows network layer's protocol (ip)

    //extract eth header
    if(buf_len < (int)sizeof(struct ethhdr)){ return; }
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
    if(buf_len < (int)sizeof(struct ethhdr) + (int)sizeof(struct iphdr)){ return; }
    struct iphdr *ip = (struct iphdr *)(buffer + sizeof(struct ethhdr));
    //iphdr has variable options, makes getting size difficult
    int ip_header_len = ip->ihl*4; //ihl is internet header length in 32 bit words, convert to bytes
    if(ip_header_len < (int)sizeof(struct iphdr)){ return; }
    if(buf_len < (int)sizeof(struct ethhdr) + ip_header_len){ return; }

    memset(&source_addr, 0, sizeof(source_addr)); //zeroing out/resetting
    memset(&dest_addr, 0, sizeof(dest_addr));

    //sin_addr is network byte order, saddr and daddr in ip header are also network byte order
    //given that they are from buffer which is from the netework
    source_addr.sin_addr.s_addr = ip->saddr; //store as source id in source_addr ip from ip header (saddr)
    dest_addr.sin_addr.s_addr = ip->daddr; 

    //ip filter
    if(filtering_IP(filter) == false){
        return;
    }

    //protocol - tcp udp
    if(filter->transfer_protocol != 0 && ip->protocol != filter->transfer_protocol){
        return;
    }
    struct tcphdr *tcp = NULL;
    struct udphdr *udp = NULL;
    if(filter->transfer_protocol == IPPROTO_UDP && ip->protocol!=IPPROTO_UDP){ return; }
    if(filter->transfer_protocol == IPPROTO_TCP && ip->protocol!=IPPROTO_TCP){ return; }

    if(ip->protocol == IPPROTO_TCP){ //how to check if it is tcp
        if(buf_len < (int)sizeof(struct ethhdr) + ip_header_len + (int)sizeof(struct tcphdr)){ return; }
        tcp = (struct tcphdr*)(buffer + ip_header_len + sizeof(struct ethhdr));
        //ntohl or s, wait so when i get port from my machine it gives host?
        if(tcp->doff*4 < (int)sizeof(struct tcphdr)){ return; }
        if(buf_len < (int)sizeof(struct ethhdr) + ip_header_len + tcp->doff*4){ return; }
        if(filtering_port(filter, ntohs(tcp->source), ntohs(tcp->dest)) == false){
            return;
        }
    }
    else if(ip->protocol == IPPROTO_UDP){
        if(buf_len < (int)sizeof(struct ethhdr) + ip_header_len + (int)sizeof(struct udphdr)){ return; }
        udp = (struct udphdr*)(buffer + ip_header_len + sizeof(struct ethhdr));
        if(filtering_port(filter, ntohs(udp->source), ntohs(udp->dest)) == false){
            return;
        }
    }
    else{
        return;
    }

    logETH(eth, log_file);
    logIP(ip, log_file);
    if(ip->protocol == IPPROTO_TCP){
        logTCP(tcp, log_file);
    }
    else if(ip->protocol == IPPROTO_UDP){
        logUDP(udp, log_file);
    }
    logpayload(buffer, buf_len, ip_header_len, ip->protocol, log_file, tcp);

}

int main(int argc, char *argv[]){
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa)); //zero out
    sa.sa_handler = &signalHandler;
    sa.sa_flags = 0; //not SA_RESTART  
    sigemptyset(&sa.sa_mask); //default blocking behaviour
    sigaction(SIGINT, &sa, NULL); //third is for old sigaction so u can restore old handler if want
    sigaction(SIGTERM, &sa, NULL);
    //same functionality: signal(SIGINT, signalHandler); - not recommended for portability see man page
    
    char log[225] = {0}; //log message, 225 is max size, taken as input from user
    FILE *log_file = NULL; //file pointer for log file, std I/O setup
    
    packet_filter_t filter = {0, NULL, NULL, 0, 0, NULL, NULL, {0}, {0}}; //initialize default

    struct sockaddr saddr; //pass to recieve from
    int sockfd, saddr_len, buf_len;

    uint8_t *buffer = (uint8_t *)malloc(65536); //buffer hold packet data, 65536 max IP packet - by bytesn(uint8_t)
    if(buffer == NULL){
        exit_with_error("Failed to allocate memory for packet buffer");
    }
    memset(buffer, 0, 65536); //zero out buffer

    capnetraw_onoff(CAP_SET); //enable CAP_NET_RAW
    sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL)); //family packet, type raw, have to use ETH_P_ALL bc it will not give u IP packets going out otherwise
    //sits below IP layer, not necessarily actually ethernet just in that format, wifi gets changed to it too - its just an old name
    cap_t caps = cap_get_proc();
    if(!caps){
        exit_with_error("Failed to get process capabilities");
    }
    if(cap_clear(caps) == -1){
        cap_free(caps);
        exit_with_error("Failed to clear capabilities");
    }
    if(cap_set_proc(caps) == -1){
        cap_free(caps);
        exit_with_error("Failed to set process capabilities");
    }
    cap_free(caps);
    
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
        c = getopt_long(argc, argv, "s:d:p:q:i:j:f:tu", long_options, NULL); 
        //: means required argument, :: means optional, none means no args
        //required arg just means, if flag given, must have arg after it

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
                filter.dest_port = atoi(optarg);
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

    //haven't done option for Any yet - functionality is NOT there yet
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
    if(filter.source_ifname){
        get_mac(filter.source_ifname, &filter, "source");
    }
    if(filter.dest_ifname){
        get_mac(filter.dest_ifname, &filter, "dest");
    }


    //nevermind again - using websocket server 

    //This instead? -- edit path ofc
    //system("start %s %s", browser, htmlAddr);

    //system("xdg-open http://localhost:8080 >/dev/null 2>&1 &");
    //ignore browser output errors - just open it for gui
    //>/dev/null means redirect stdout to null
    //2>&1 means redirect stderr to stdout which is null so both go to null
    //& means run in background so it doesn't block the program


    /* struct sockaddr saddr; 
    int sockfd, saddr_len, buf_len; */
    //MAIN LOOOP

    //adding terminate by user input, poll to fix blocking
    struct pollfd fds[2]; //set to memory if need to reallocate for variable num of connection 
    fds[0].fd = sockfd;
    fds[0].events = POLLIN; //wait input on socket
    fds[1].fd = STDIN_FILENO; //stdin
    fds[1].events = POLLIN; 

    int n = 0;
    while(!stop){
        int poll_cnt = poll(fds, 2, -1); //wait indefinitely for input
        if(poll_cnt < 0){
            if(errno == EINTR){ //asleep in poll, sigint arrives, stop=1, poll returns -1, errno=EINTR, break
                break;
            }
            exit_with_error("poll failed");
        }

        if(fds[1].revents & POLLIN){
            char buffer2[100];
            ssize_t bytes_read = read(STDIN_FILENO, buffer2, sizeof(buffer2) - 1);
            if(bytes_read <= 0){
                break;
            }
            buffer2[bytes_read] = '\0'; //null ending for str
            if(buffer2[0] == 'q'){
                break;
            }
        }
        if(fds[0].revents & POLLIN){
            saddr_len = sizeof(saddr);
            buf_len = recvfrom(sockfd, buffer, 65536-1, 0, &saddr, (socklen_t*) &saddr_len);
            if(buf_len < 0){
                if(errno == EINTR){ //interrupted by signal, break
                    break;
                }
                exit_with_error("Failed to receive packets");
            }
            process_packet(buffer, buf_len, &filter, log_file);
            fflush(log_file);
            n++;
        }
    }
    fclose(log_file);
    free(buffer);
    close(sockfd);
    printf("Exiting nice, num of packets read %d\n", n);
    return 0;
}


















