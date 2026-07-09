#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "ws12.c"

#include <libwebsockets.h>
//can use compiler flag to shorten includes from libaries
//event driven library
//Notes:
//
// 

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

static int interrupted;

//i have to edit the sizes no? 0, NULL, for the last 4 mean defaults?
static struct lws_protocols protocols[] = {
    {"html", lws_callback_http_dummy, 0, 0, 0, NULL, 0},
    {"prot1", callbackFunc, 0, 0, 0, NULL, 0},
    //LWS_PLUGIN_PROTOCOL_MINIMAL, //giving undefined error
    LWS_PROTOCOL_LIST_TERM, //sentinel marking end of protocols list
};

static const struct lws_http_mount mount = { 
    //static file webserver (software to send pre-exisiting assets like html,css,js directly to client browser)
    //Maps a url to a file on disk that get sent (from disk?)
    .mountpoint = "/", //urls startingn with this
    .origin = "./mount-origin", //origin directory 
    .def = "index.html", //default file forr "/"
    .origin_protocol = LWSMPRO_FILE, //origin is a filesystem dr, thi sis the protocol
    .mountpoint_len = 1, //strlen("/") == 1
};

void sigint_handler(int sig){
    interrupted = 1;
}


int main(int argc, char **argv){
    struct lws_context_creation_info info;
    struct lws_context *context;
    signal(SIGINT, sigint_handler);


    lwsl_user("LWS minimal ws client | visit https://libwebsockets.org\n");
    lws_context_info_defaults(&info, NULL);
    info.port = 7681;
    info.mounts = &mount; //no idea where this comes from
    info.protocols = protocols;
    info.vhost_name = "localhost";
    info.options = LWS_SERVER_OPTION_HTTP_HEADERS_SECURITY_BEST_PRACTICES_ENFORCE;

    context = lws_create_context(&info);
    if(!context){
        lwsl_error("Lws initialization failed");
        return 1;
    }

    int n = 0;
    while(n>=0 && !interrupted){
        n = lws_service(context, 0);
    }

    lws_context_destroy(context);

    printf("test\n");
    return 0;
}













