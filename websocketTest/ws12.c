#include <libwebsockets.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

static int interrupted;

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

static void prot1_destroy_msg(void *_msg){ 
    struct msg *msg = _msg;
    free(msg->payload);
    msg->payload = NULL;
    msg->len = 0;
}

static void sender(struct per_vhost_data_prot1 *vhd, char *msg, size_t len){
    if(vhd->a_msg.payload){
        prot1_destroy_msg(&vhd->a_msg);
    }
    vhd->a_msg.payload = malloc(LWS_PRE + len);
    if(!vhd->a_msg.payload){
        lwsl_user("OOM: dropping\n");
        return;
    }
    memcpy((char*)vhd->a_msg.payload + LWS_PRE, msg, len);
    vhd->a_msg.len = len;
    vhd->current++;
    lws_start_foreach_llp(struct per_session_data_prot1 **, ppss, vhd->pss_list){
        lws_callback_on_writable((*ppss)->wsi);
    } lws_end_foreach_llp(ppss, pss_list);
}

static int callbackFunc(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len){
    struct per_session_data_prot1 *pss = (struct per_session_data_prot1 *)user;
    struct per_vhost_data_prot1 *vhd = (struct per_vhost_data_prot1 *)lws_protocol_vh_priv_get(lws_get_vhost(wsi), lws_get_protocol(wsi));
    int m;
    lws_sock_file_fd_type u;

    switch(reason){
        //for fun really the first case
        //check origin to make sure only the one i want is connecting
        //need curly brackets cuz delceration cant come after case label
        case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION:{
            char origin[128];
            int n = lws_hdr_copy(wsi, origin, sizeof(origin), WSI_TOKEN_ORIGIN);
            if(n<=0){
                lwsl_user("rejecting not origin\n");
                return -1;
            }
            if(strcmp(origin, "http://localhost:7681") != 0){
                lwsl_user("rejecting origin %s\n", origin);
                return -1;
            }
            break;
        }
        case LWS_CALLBACK_PROTOCOL_INIT: //intialization
            vhd = lws_protocol_vh_priv_zalloc(lws_get_vhost(wsi), lws_get_protocol(wsi), sizeof(struct per_vhost_data_prot1));
            if(!vhd){
                return 1;
            }
            vhd->context = lws_get_context(wsi);
            vhd->protocol = lws_get_protocol(wsi);
            vhd->vhost = lws_get_vhost(wsi);
            
            u.filefd = (lws_filefd_type)(long long)STDIN_FILENO;
            if(!lws_adopt_descriptor(lws_get_vhost(wsi), LWS_ADOPT_RAW_FILE_DESC, u, "prot1", NULL)){
                lwsl_err("Failed to adopt stdin\n");
                return 1;
            }
            break;
        case LWS_CALLBACK_ESTABLISHED: //new connection/client established
            lws_ll_fwd_insert(pss, pss_list, vhd->pss_list);
            pss->wsi = wsi;
            pss->last = vhd->current;
            break;
        case LWS_CALLBACK_CLOSED:
            lws_ll_fwd_remove(struct per_session_data_prot1, pss_list, pss, vhd->pss_list);
            break;
        case LWS_CALLBACK_RAW_CLOSE_FILE:
            lwsl_user("stdin closed\n");
            interrupted = 1;
            break;
        case LWS_CALLBACK_SERVER_WRITEABLE:
            if(!vhd->a_msg.payload) //funny syntax messes with my muscle memory
                break;
            if(pss->last == vhd->current) //check not old msg and not empty
                break;
            m = lws_write(wsi, (unsigned char *)vhd->a_msg.payload + LWS_PRE, vhd->a_msg.len, LWS_WRITE_TEXT);
            if(m < (int)vhd->a_msg.len){
                lwsl_err("ERROR %d writing to ws socket\n", m);
                return -1;
            }
            pss->last = vhd->current;
            break;
        case LWS_CALLBACK_RECEIVE: //technically dont need it for my use case for now
            /* if(vhd->a_msg.payload){ //make it better with ringn examlpe with queue
                //basically if client sends msg, we free old msg and store new one
                prot1_destroy_msg(&vhd->a_msg);
            }
            vhd->a_msg.len = len;
            vhd->a_msg.payload = malloc(LWS_PRE + len);
            if(!vhd->a_msg.payload){
                lwsl_user("OOM: dropping\n"); //literally a print statement - lvls and all but yeah
                break;
            }
            memcpy((char*)vhd->a_msg.payload + LWS_PRE, in, len);
            vhd->current++;
            lws_start_foreach_llp(struct per_session_data_prot1 **, ppss, vhd->pss_list){
                lws_callback_on_writable((*ppss)->wsi);
            } lws_end_foreach_llp(ppss, pss_list);
            //loops through each client and calls callback on each one */
            break;
        case LWS_CALLBACK_RAW_RX_FILE:
            static char buf[65536]; //overarching buffer
            static size_t acc_len = 0;
            char *p;
            int r;
            p = (char *)vhd->a_msg.payload + LWS_PRE;
            r = (int)read(STDIN_FILENO, p, 65536); //overkill size
            if(r==0){
                lwsl_user("EOF: exiting\n");
                interrupted = 1;
                break;
            }
            if(r<0){
                lwsl_user("Failed read\n");
                break;
            }
            if(acc_len + (size_t)r > sizeof(buf)){
                acc_len = 0; //reset buffer if overflow
                //memset(buf, 0, 65536); 
            }
            memcpy(buf+acc_len, p, r);
            acc_len+=r;
            
            size_t start = 0;
            for(size_t i = 0; i<acc_len; i++){
                if(buf[i] == '\n'){
                    if(i>start){
                        sender(vhd, buf+start, i-start);
                    }
                    start = i+1;
                }
            }
            acc_len -= start;
            memmove(buf, buf+start, acc_len);
            break;
        default:
            break;
    }
    return 0;
}

/* #define CALLBACKPROTOCOL1 \
{ \
    "prot1", \
    callbackFunc, \
    sizeof(struct per_session_data_prot1), \
    65536, \
    0, NULL, 0 \
} */
// max size? other stuff in here gotta add
//gonna hve to make it a lot bigger or reasmmble it
//name has to match exact




