#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#include <libwebsockets.h>
#include "ws12.c"
//can use compiler flag to shorten includes from libaries
//event driven library

//don't really need these for my application, can add later tho - basically if cases for each then code
/* enum { 
	LWS_SW_D, //0
	LWS_SW_H, //1
	LWS_SW_S, //2
	LWS_SW_V, //3
	LWS_SW_HELP, //4
};

static const struct lws_switches switches[] = {
	[LWS_SW_D]	= { "-d",              "Debug logs (e.g. -d 15)" },
	[LWS_SW_H]	= { "-h",              "Strict Host Check / Help" },
	[LWS_SW_S]	= { "-s",              "Use TLS / https" },
	[LWS_SW_V]	= { "-v",              "Set retry and idle policy" },
	[LWS_SW_HELP]	= { "--help",		"Show this help information" },
}; */

//i have to edit the sizes no? 0, NULL, for the last 4 mean defaults?
static struct lws_protocols protocols[] = {
    {"http", lws_callback_http_dummy, 0, 0, 0, NULL, 0},
    {"prot1", callbackFunc, sizeof(struct per_session_data_prot1), 65536, 0, NULL, 0},
    //LWS_PLUGIN_PROTOCOL_MINIMAL, //giving undefined error
    LWS_PROTOCOL_LIST_TERM, //sentinel marking end of protocols list
};

static const struct lws_http_mount mount = { 
    //static file webserver (software to send pre-exisiting assets like html,css,js directly to client browser)
    //Maps a url to a directory on server's filesystem (disk if just run on local machine)
    .mountpoint = "/", //urls startingn with this
    .origin = "./frontbeginning", //origin directory 
    .def = "index.html", //default file forr "/"
    .origin_protocol = LWSMPRO_FILE, //origin is a filesystem dr, this is the protocol for this case
    .mountpoint_len = 1, //strlen("/") == 1
};

void sigint_handler(int sig){
    interrupted = 1;
}

static const lws_retry_bo_t retry = { //basically checks if client dead
    .secs_since_valid_ping = 10,
    .secs_since_valid_hangup = 30
};

int main(int argc, char **argv){
    struct lws_context_creation_info info;
    struct lws_context *context;
    signal(SIGINT, sigint_handler);


    lwsl_user("Transmission of parsed raw packet data through websockets(https://libwebsockets.org)\n");
    lws_context_info_defaults(&info, NULL);
    info.port = 7681;
    info.iface = "127.0.0.1"; 
    info.mounts = &mount;
    info.protocols = protocols;
    info.vhost_name = "localhost";
    info.options = LWS_SERVER_OPTION_HTTP_HEADERS_SECURITY_BEST_PRACTICES_ENFORCE;

    //info.retry_and_idle_policy = &retry;
    //not needed for local dev ofc

    context = lws_create_context(&info);
    if(!context){
        lwsl_err("Lws initialization failed");
        return 1;
    }

    system("xdg-open http://localhost:7681");

    int n = 0;
    while(n>=0 && !interrupted){
        n = lws_service(context, 0);
    }

    lws_context_destroy(context);

    printf("test\n");
    return 0;
}













