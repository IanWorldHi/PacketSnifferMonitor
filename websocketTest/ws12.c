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
    struct msg a_msg; //one pending msg
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
        case LWS_CALLBACK_PROTOCOL_INIT: //intialization
            vhd = lws_protocol_vh_priv_zalloc(lws_get_vhost(wsi), lws_get_protocol(wsi), sizeof(struct per_vhost_data_prot1));
            if(!vhd){
                return 1;
            }
            vhd->context = lws_get_context(wsi);
            vhd->protocol = lws_get_protocol(wsi);
            vhd->vhost = lws_get_vhost(wsi);
            break;
        case LWS_CALLBACK_ESTABLISHED: //new connection/client established
            lws_ll_fwd_insert(pss, pss_list, vhd->pss_list);
            pss->wsi = wsi;
            pss->last = vhd->current;
            break;
        case LWS_CALLBACK_CLOSED:
            lws_ll_fwd_remove(struct per_session_data_prot1, pss_list, pss, vhd->pss_list);
            break;
        case LWS_CALLBACK_SERVER_WRITEABLE:
            if(!vhd->a_msg.payload) //funny syntax messes with my muscle memory
                break;
            if(pss->last == vhd->current) //check not old msg and not empty
                break;
            m = lws_write(wsi, (unsigned char *)vhd->a_msg.payload + LWS_PRE, vhd->a_msg.len, LWS_WRITE_TEXT);

            break;
        case LWS_CALLBACK_RECEIVE: //technically dont need it for my use case for now
            if(vhd->a_msg.payload){ //make it better with ringn examlpe with queue
                //basically if client sends msg, we free old msg and store new one
                __prot1_destroy_msg(&vhd->a_msg);
            }
            vhd->a_msg.len = len;
            vhd->a_msg.payload = malloc(LWS_PRE + len);
            if(!vhd->a_msg.payload){
                lwsl_user("OOM: dropping\n");
                break;
            }
            memcpy((char*)vhd->a_msg.payload + LWS_PRE, in, len);
            vhd->current++;
            lws_start_foreach_llp(struct per_session_data_prot1 **, ppss, vhd->pss_list){
                lws_callback_on_writable((*ppss)->wsi);
            } lws_end_foreach_llp(ppss, pss_list);
            //loops through each client and calls callback on each one
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




