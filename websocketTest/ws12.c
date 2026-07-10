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
    struct msg *msg = _msg;
    free(msg->payload);
    msg->payload = NULL;
    msg->len = 0;
}

static int callbackFunc(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len){
    struct per_session_data_prot1 *pss = (struct per_session_data_prot1 *)user;
    struct per_vhost_data_prot1 *vhd = (struct per_vhost_data_prot1 *)lws_protocol_vh_priv_get(lws_get_vhost(wsi), lws_get_protocol(wsi));
    int m;

    switch(reason){
        case LWS_CALLBACK_PROTOCOL_INIT:
            
            break;
        case LWS_CALLBACK_ESTABLISHED:
            break;
        case LWS_CALLBACK_CLOSED:
            break;
        case LWS_CALLBACK_SERVER_WRITEABLE:
            break;
        case LWS_CALLBACK_RECEIVE:
            break;
        default:
            break;
    }
    return 0;
}

#define CALLBACKPROTOCOL1 \ 
{ \
    "name?" \
    callbackFunc, \
    sizeof(struct per_session_data_prot1), \ 
    128, \
    0, NULL, 0 \
}
// max size? other stuff in here gotta add




