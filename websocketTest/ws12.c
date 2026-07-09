#include <libwebsockets.h>

//one per msg
struct msg{
    void *payload; //malloced
    size_t len;
};

//one per connected client
struct per_session_data_prot1{ 
    struct per_session_data_prot1 *pss_list; //linked list of all sessions
    struct lws *wsi; //websocket instance
    int last; //last msg sent num
};

//One for vhost
struct per_vhost_data_prot1{
    struct lws_context *context;
    struct lws_vhost *vhost;
    const struct lws_protocols *protocol;
    struct per_session_data_prot1 *pss_list; 
    struct msg *a_msg; //one pending msg
    int current; //cuurent msg we are caching
};

static void __prot1_destroy_msg(void *_msg){ //why the underscore?

}

callbackFunc(){

}

#define CALLBACKPROTOCOL1 \ 
{ \
    "name?" \
    callbackFunc, \
    sizeof(), \ 
}
// max size? other stuff in here gotta add




